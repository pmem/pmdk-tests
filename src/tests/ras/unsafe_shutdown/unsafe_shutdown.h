/*
 * Copyright 2018-2024, Intel Corporation
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in
 *       the documentation and/or other materials provided with the
 *       distribution.
 *
 *     * Neither the name of the copyright holder nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
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
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef UNSAFE_SHUTDOWN_H
#define UNSAFE_SHUTDOWN_H

#include "configXML/local_dimm_configuration.h"
#include "gtest/gtest.h"
#include "libpmempool.h"
#include "pool_data/pool_data.h"
#include "poolset/poolset_management.h"
#include "shell/i_shell.h"
#include "test_phase/local_test_phase.h"

class UnsafeShutdown : public ::testing::Test {
 public:
  UnsafeShutdown() {
    this->close_pools_at_end_ = !test_phase_.HasInjectAtEnd();
  }

  IShell shell_;
  LocalTestPhase& test_phase_ = LocalTestPhase::GetInstance();
  PMEMobjpool* pop_ = nullptr;

  /* Test values used for checking data consistency on obj pools. */
  std::vector<int> obj_data_ = {-2, 0, 12345, 1412, 1231, 23, 432, 34, 3};

  bool PassedOnPreviousPhase() const;
  std::string GetNormalizedTestName() const;
  int PmempoolRepair(std::string pool_file_path) const;

  ~UnsafeShutdown() {
    StampPassedResult();
    if (close_pools_at_end_) {
      if (pop_) {
        pmemobj_close(pop_);
      }
    }
  }

 protected:
  bool close_pools_at_end_ = true;

 private:
  const ::testing::TestInfo& GetTestInfo() const {
    return *::testing::UnitTest::GetInstance()->current_test_info();
  }
  std::string GetPassedStamp() const {
    return test_phase_.GetTestDir() + GetNormalizedTestName() + "_passed";
  }
  void StampPassedResult() const;
};

void set_sds_at_create_func(bool state);

#endif  // UNSAFE_SHUTDOWN_H
