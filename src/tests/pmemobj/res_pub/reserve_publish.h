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

#ifndef NVML_VAL_TOOLS_RESERVE_PUBLISH_H
#define NVML_VAL_TOOLS_RESERVE_PUBLISH_H

#include <libpmemobj.h>
#include <memory>
#include "configXML/local_configuration.h"
#include "gtest/gtest.h"

extern std::unique_ptr<LocalConfiguration> local_config;

#define LAYOUT_NAME "res_pub_layout"

struct reserve_publish_params {
  size_t data_size;
  size_t nof_threads;
  size_t messages_per_thread;

 public:
  reserve_publish_params(size_t data_size, size_t nof_threads,
                         size_t messages_per_thread)
      : data_size(data_size),
        nof_threads(nof_threads),
        messages_per_thread(messages_per_thread) {
  }
};

class PMemObjReservePublishTest : public ::testing::Test {
 private:
  std::string test_dir_ = local_config->GetTestDir();

 protected:
  PMEMobjpool *pop;
  size_t data_size;
  size_t messages_per_thread;
  const int index_to_delete = 4;

 public:
  struct test_obj {
    pobj_action *act;
    pobj_action *act_delete;
  };

  std::string pool_path_ = test_dir_ + "pool";
  size_t pool_size = 1024 * 1024 * 1024;
  void SetUp();
  void TearDown();

  int makeMaximumAllocations(PMEMobjpool *pop, size_t data_size,
                             struct pobj_action *actv);

  void ReservePublishInThread(test_obj &obj);
  void ReservePublishCancelInThread(test_obj &obj);
  void ReservePublishDeferFreeInThread(test_obj &obj);
};

class PMemObjReservePublishParamTest
    : public PMemObjReservePublishTest,
      public ::testing::WithParamInterface<reserve_publish_params> {
 protected:
  size_t nof_threads;

 public:
  void SetUp();
};

struct message {
  char data[];
};

TOID_DECLARE(struct message, 1);

#endif  // NVML_VAL_TOOLS_RESERVE_PUBLISH_H
