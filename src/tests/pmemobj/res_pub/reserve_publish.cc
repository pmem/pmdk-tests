/*
 * Copyright (c) 2018-2019, Intel Corporation
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

void PmemobjReservePublishTest::SetUp() {
  ApiC::RemoveFile(pool_path_);
}

void PmemobjReservePublishTest::TearDown() {
  pmemobj_close(pop);
  ApiC::RemoveFile(pool_path_);
}

std::vector<pobj_action> PmemobjReservePublishTest::MakeMaximumReservations() {
  std::vector<pobj_action> reservations;

  pobj_action reservation;
  TOID(struct message) reserved_message =
    POBJ_RESERVE_ALLOC(pop, struct message, data_size, &reservation);
  while (!OID_IS_NULL(reserved_message.oid)) {
    reservations.push_back(std::move(reservation));
    reserved_message =
      POBJ_RESERVE_ALLOC(pop, struct message, data_size, &reservation);
  }

  return reservations;
}

size_t PmemobjReservePublishTest::GetNofStoredMessages() {
  size_t count = 0;
  PMEMoid oid = pmemobj_first(pop);
  while (!OID_IS_NULL(oid)) {
    count++;
    oid = pmemobj_next(oid);
  }
  return count;
}

std::unique_ptr<ActionsObj> PmemobjReservePublishTest::ReserveInThread() {
  std::vector<struct pobj_action> publish_acts(messages_per_thread);

  for (size_t i = 0; i < messages_per_thread; i++) {
    TOID(struct message)
    reserved_message =
        POBJ_RESERVE_ALLOC(pop, struct message, data_size, &publish_acts[i]);

    EXPECT_FALSE(OID_IS_NULL(reserved_message.oid)) << pmemobj_errormsg();
  }

  std::unique_ptr<ActionsObj> actions =
      std::make_unique<ActionsObj>(publish_acts);
  return actions;
}

std::unique_ptr<ActionsObj>
PmemobjReservePublishTest::ReserveWithCancelInThread() {
  std::vector<struct pobj_action> publish_acts;
  std::vector<struct pobj_action> cancel_acts;

  for (size_t i = 0; i < messages_per_thread; i++) {
    pobj_action action;
    TOID(struct message)
    message = POBJ_RESERVE_ALLOC(pop, struct message, data_size, &action);

    EXPECT_FALSE(OID_IS_NULL(message.oid)) << pmemobj_errormsg();

    if (i % index_to_delete == 0) {
      cancel_acts.push_back(std::move(action));
    } else {
      publish_acts.push_back(std::move(action));
    }
  }

  std::unique_ptr<ActionsObj> actions =
      std::make_unique<ActionsObj>(publish_acts, cancel_acts);
  return actions;
}

std::unique_ptr<ActionsObj> PmemobjReservePublishTest::DeferFreeInThread() {
  std::vector<struct pobj_action> publish_acts;

  TOID(struct message) element;

  for (size_t i = 0; i < messages_per_thread; i++) {
    TOID_ASSIGN(element, OID_NULL);
    EXPECT_EQ(0,
              pmemobj_alloc(pop, &element.oid, data_size, 0, nullptr, nullptr))
        << pmemobj_errormsg();

    if (i % 2 == 0) {
      pobj_action action;
      pmemobj_defer_free(pop, element.oid, &action);
      publish_acts.push_back(std::move(action));
    }
  }

  std::unique_ptr<ActionsObj> actions =
      std::make_unique<ActionsObj>(publish_acts);
  return actions;
}

std::unique_ptr<ActionsObj> PmemobjReservePublishTest::XReserveInThread() {
  std::vector<struct pobj_action> publish_acts(messages_per_thread);

  for (size_t i = 0; i < messages_per_thread; i++) {
    PMEMoid oid = pmemobj_xreserve(pop, &publish_acts[i], data_size, 0,
                                   POBJ_CLASS_ID(flags));
    EXPECT_FALSE(OID_IS_NULL(oid));
  }

  std::unique_ptr<ActionsObj> actions =
      std::make_unique<ActionsObj>(publish_acts);
  return actions;
}

int PmemobjReservePublishTest::TxPublishInThread(TestObj &obj) {
  int ret;

  TX_BEGIN(pop) {
    ret = pmemobj_tx_publish(obj.reservations.data(), obj.reservations.size());
  }
  TX_END

  return ret;
}

void PmemobjReservePublishParamTest::SetUp() {
  PmemobjReservePublishTest::SetUp();
  messages_per_thread = GetParam().messages_per_thread;
  nof_threads = GetParam().nof_threads;
  data_size = GetParam().data_size;
}

void PmemobjReserveTxPublishParamTest::SetUp() {
  PmemobjReservePublishTest::SetUp();
  messages_per_thread = GetParam().messages_per_thread;
  nof_threads = GetParam().nof_threads;
  data_size = GetParam().data_size;
}
