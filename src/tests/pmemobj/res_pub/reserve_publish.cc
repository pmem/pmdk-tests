/*
 * Copyright (c) 2018, Intel Corporation
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in
 * the documentation and/or other materials provided with the
 * distribution.
 *
 * * Neither the name of Intel Corporation nor the names of its
 * contributors may be used to endorse or promote products derived
 * from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY LOG OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include "reserve_publish.h"

void PMemObjReservePublishTest::SetUp() {
  ApiC::RemoveFile(pool_path_);
  errno = 0;
}

void PMemObjReservePublishTest::TearDown() {
  ApiC::RemoveFile(pool_path_);
}

int PMemObjReservePublishTest::makeMaximumAllocations(
    PMEMobjpool *pop, size_t data_size, struct pobj_action *actv) {
  TOID(struct message) reserved_message;

  int count = 0;
  do {
    reserved_message =
        POBJ_RESERVE_ALLOC(pop, struct message, data_size, &actv[count]);
    if (!OID_IS_NULL(reserved_message.oid)) {
      count++;
    }
  } while (!OID_IS_NULL(reserved_message.oid));

  return count;
}

void PMemObjReservePublishTest::ReservePublishInThread(test_obj &obj) {
  struct pobj_action *act = obj.act;
  for (int i = 0; i < messages_per_thread; i++) {
    /* Step 2 */
    TOID(struct message)
    reserved_message =
        POBJ_RESERVE_ALLOC(pop, struct message, data_size, &act[i]);
    ASSERT_TRUE(!OID_IS_NULL(reserved_message.oid));
    /* Step 3 */
    memset(D_RW(reserved_message)->data, 0x0A, data_size);
  }
  /* Step 4 */
  int ret = pmemobj_publish(pop, act, messages_per_thread);
  ASSERT_TRUE(ret == 0);
}

void PMemObjReservePublishTest::ReservePublishCancelInThread(test_obj &obj) {
  struct pobj_action *act_publish = obj.act;
  struct pobj_action *act_cancel = obj.act_delete;
  int publish_index = 0;
  int cancel_index = 0;
  for (int i = 0; i < messages_per_thread; i++) {
    TOID(struct message) reserved_message;
    /* Step 2 */
    if (i % index_to_delete == 0) {
      reserved_message = POBJ_RESERVE_ALLOC(pop, struct message, data_size,
                                            &act_cancel[cancel_index]);
      ASSERT_TRUE(!OID_IS_NULL(reserved_message.oid)) << i;
      cancel_index++;
    } else {
      reserved_message = POBJ_RESERVE_ALLOC(pop, struct message, data_size,
                                            &act_publish[publish_index]);
      ASSERT_TRUE(!OID_IS_NULL(reserved_message.oid)) << i;
      publish_index++;
    }
    memset(D_RW(reserved_message)->data, i, data_size);
    /* Step 4 */
  }
}

void PMemObjReservePublishTest::ReservePublishDeferFreeInThread(test_obj &obj) {
  struct pobj_action *act_publish = obj.act;
  struct pobj_action *act_free = obj.act_delete;
  int free_count = 0;
  for (int i = 0; i < messages_per_thread; i++) {
    /* Step 2 */
    TOID(struct message)
    reserved_message =
        POBJ_RESERVE_ALLOC(pop, struct message, data_size, &act_publish[i]);
    ASSERT_FALSE(OID_IS_NULL(reserved_message.oid));
    /* Step 3 */
    memset(D_RW(reserved_message)->data, 0x0A, data_size);
    /* Step 4 */
    if (i % 2 == 0) {
      pmemobj_defer_free(pop, reserved_message.oid, &act_free[free_count++]);
    }
  }
}

void PMemObjReservePublishParamTest::SetUp() {
  PMemObjReservePublishTest::SetUp();
  messages_per_thread = GetParam().messages_per_thread;
  nof_threads = GetParam().nof_threads;
  data_size = GetParam().data_size;
}
