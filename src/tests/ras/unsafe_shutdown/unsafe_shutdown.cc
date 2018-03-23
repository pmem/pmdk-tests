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

#include "unsafe_shutdown.h"

std::string UnsafeShutdown::GetNormalizedTestName() {
  auto &test_info = GetTestInfo();
  std::string test_name = std::string{test_info.test_case_name()} + "_" +
                          std::string{test_info.name()};
  test_name = std::regex_replace(test_name, std::regex(SEPARATOR), "_");

  std::string test_suffixes = "_before_us|_after_first_us|_after_second_us";
  test_name = std::regex_replace(test_name, std::regex(test_suffixes), "");
  return test_name;
}

void UnsafeShutdown::StampPassedResult() {
  if (GetTestInfo().result()->Passed()) {
    std::string file_name =
        local_dimm_config->GetTestDir() + GetNormalizedTestName() + "_passed";
    ApiC::CreateFileT(file_name, "");
  }
}

bool UnsafeShutdown::PassedOnPreviousPhase() {
  std::string passed_file_path =
      local_dimm_config->GetTestDir() + GetNormalizedTestName() + "_passed";
  bool ret = ApiC::RegularFileExists(passed_file_path);
  ApiC::RemoveFile(passed_file_path);
  return ret;
}

void UnsafeShutdown::Repair(std::string pool_file_path) {
  std::string cmd = "pmempool check -ry " + pool_file_path;
  auto out = shell_.ExecuteCommand(cmd);
  ASSERT_EQ(0, out.GetExitCode()) << cmd << std::endl << out.GetContent();
}

int UnsafeShutdown::RecordUSC(DimmCollection dimm_collection) {
  for (int i : dimms_to_inject_) {
    int usc = dimm_collection[i].GetShutdownCount();
    if (usc == -1) {
      std::cerr << "Reading USC from dimm " << dimm_collection[i].GetUid()
                << " on mountpoint: " << dimm_collection.GetMountpoint()
                << " failed" << std::endl;
      return -1;
    }

    if (ApiC::CreateFileT(dimm_collection.GetMountpoint() + SEPARATOR +
                              dimm_collection[i].GetUid(),
                          std::to_string(usc)) == -1) {
      return -1;
    }
  }

  return 0;
}

int UnsafeShutdown::ReadRecordedUSC(std::string usc_file_path) {
  std::string content;
  if (ApiC::ReadFile(usc_file_path, content) != 0) {
    return -1;
  }

  try {
    return std::stoi(content);
  } catch (std::invalid_argument &e) {
    std::cerr << e.what() << std::endl;
    return -1;
  } catch (std::out_of_range &e) {
    std::cerr << e.what() << std::endl;
    return -1;
  }
}

void UnsafeShutdown::Inject() {
  for (auto &dc : us_dimm_collections_) {
    ASSERT_EQ(RecordUSC(dc), 0);

    for (int i : dimms_to_inject_) {
      ASSERT_EQ(dc[i].InjectUnsafeShutdown(), 0);
    }
  }
}

void UnsafeShutdown::ConfirmRebootedWithUS() {
  for (auto &dc : us_dimm_collections_) {
    for (int i : dimms_to_inject_) {
      int recorded_usc =
          ReadRecordedUSC(dc.GetMountpoint() + SEPARATOR + dc[i].GetUid());
      ASSERT_NE(recorded_usc, -1)
          << "Could not read USC in dimm: " << dc[i].GetUid()
          << " on mountpoint: " << dc.GetMountpoint();

      ASSERT_EQ(dc[i].GetShutdownCount(), recorded_usc + 1)
          << "Dimm " << dc[i].GetUid()
          << " on mounptoint: " << dc.GetMountpoint()
          << " was not unsafely shutdown.";
    }
  }
}

UnsafeShutdown::~UnsafeShutdown() {
  StampPassedResult();
}

const testing::TestInfo &UnsafeShutdown::GetTestInfo() {
  return *::testing::UnitTest::GetInstance()->current_test_info();
}
