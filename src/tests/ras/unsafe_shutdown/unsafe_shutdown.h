/*
 * Copyright 2018, Intel Corporation
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

#include <libpmem.h>
#include <libpmemobj.h>
#include <memory>
#include <regex>
#include <vector>
#include "api_c/api_c.h"
#include "configXML/local_dimm_configuration.h"
#include "dimm/dimm.h"
#include "gtest/gtest.h"
#include "pool_data/pool_data.h"
#include "poolset/poolset.h"
#include "poolset/poolset_management.h"
#include "shell/i_shell.h"
#include "ssh_runner/ssh_runner.h"

extern std::unique_ptr<LocalDimmConfiguration> local_dimm_config;
extern std::vector<DimmCollection> us_dimms;
extern std::vector<DimmCollection> non_us_dimms;

class UnsafeShutdown : public ::testing::Test {
 public:
  bool PassedOnPreviousPhase();

  std::string GetNormalizedTestName();
  void Repair(std::string pool_file_path);

  void Inject();
  void ConfirmRebootedWithUS();

  std::vector<DimmCollection> us_dimm_collections_;

  std::vector<int> test_data_ = {-2, 0, 12345, 1412, 1231, 23, 432, 34, 3};
  IShell shell_;

  ~UnsafeShutdown();

 private:
  const ::testing::TestInfo& GetTestInfo();
  int RecordUSC(DimmCollection dimm_collection);
  int ReadRecordedUSC(std::string usc_file_path);
  void StampPassedResult();
  /* Indexes of dimms in DimmCollection namespace into which US is injected. */
  const std::vector<int> dimms_to_inject_ = {0};
};

#endif  // UNSAFE_SHUTDOWN_H
