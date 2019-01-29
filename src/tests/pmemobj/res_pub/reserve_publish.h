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

#ifndef PMDK_TESTS_RESERVE_PUBLISH_H
#define PMDK_TESTS_RESERVE_PUBLISH_H

#include <libpmemobj.h>
#include <future>
#include <memory>
#include "configXML/local_configuration.h"
#include "gtest/gtest.h"

extern std::unique_ptr<LocalConfiguration> local_config;

#define LAYOUT_NAME "res_pub_layout"

struct ReservePublishParams {
  size_t data_size;
  size_t nof_threads;
  size_t messages_per_thread;

  ReservePublishParams(size_t data_size, size_t nof_threads,
                       size_t messages_per_thread)
      : data_size(data_size),
        nof_threads(nof_threads),
        messages_per_thread(messages_per_thread) {
  }
};

struct ActionsObj {
  std::vector<struct pobj_action> publish_acts;
  std::vector<struct pobj_action> cancel_acts;

  ActionsObj(std::vector<struct pobj_action> publ_acts)
      : publish_acts(publ_acts) {
  }
  ActionsObj(std::vector<struct pobj_action> publ_acts,
             std::vector<struct pobj_action> canc_acts)
      : publish_acts(publ_acts), cancel_acts(canc_acts) {
  }
};

class PmemobjReservePublishTest : public ::testing::Test {
 private:
  const std::string test_dir_ = local_config->GetTestDir();

 protected:
  PMEMobjpool *pop;
  size_t data_size;
  size_t messages_per_thread;
  const int index_to_delete = 4;
  const std::string pool_path_ = test_dir_ + "pool";
  const size_t pool_size = 512 * MEBIBYTE;
  uint64_t flags;

 public:
  struct TestObj {
    std::vector<struct pobj_action> reservations;

    TestObj(std::vector<struct pobj_action> actions) {
      reservations.insert(reservations.end(), actions.begin(), actions.end());
    }
  };

  virtual void SetUp();
  virtual void TearDown();

  std::vector<pobj_action> MakeMaximumReservations();
  size_t GetNofStoredMessages();

  std::unique_ptr<ActionsObj> ReserveInThread();
  std::unique_ptr<ActionsObj> ReserveWithCancelInThread();
  std::unique_ptr<ActionsObj> DeferFreeInThread();
  std::unique_ptr<ActionsObj> XReserveInThread();
  int TxPublishInThread(TestObj &obj);
};

class PmemobjReservePublishParamTest
    : public PmemobjReservePublishTest,
      public ::testing::WithParamInterface<ReservePublishParams> {
 protected:
  size_t nof_threads;

 public:
  void SetUp() override;
};

class PmemobjReserveTxPublishParamTest
    : public PmemobjReservePublishTest,
      public ::testing::WithParamInterface<ReservePublishParams> {
 protected:
  size_t nof_threads;

 public:
  void SetUp() override;
};

struct message {
  char data[1024];
};

TOID_DECLARE(struct message, 1);

#endif  // PMDK_TESTS_RESERVE_PUBLISH_H
