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
    ApiC::CreateFileT(GetPassedStamp(), "");
  }
}

bool UnsafeShutdown::PassedOnPreviousPhase() {
  bool ret = ApiC::RegularFileExists(GetPassedStamp());
  if (ret) {
    ApiC::RemoveFile(GetPassedStamp());
  }
  return ret;
}

std::string UnsafeShutdown::GetPassedStamp() {
  return local_dimm_config->GetTestDir() + GetNormalizedTestName() + "_passed";
}

void UnsafeShutdown::Repair(std::string pool_file_path) {
  std::string cmd = "pmempool check -ry " + pool_file_path;
  auto out = shell_.ExecuteCommand(cmd);
  ASSERT_EQ(0, out.GetExitCode()) << cmd << std::endl << out.GetContent();
}

int UnsafeShutdown::ReadRecordedUSC(std::string usc_file_path) {
  std::string content;
  if (ApiC::ReadFile(usc_file_path, content) != 0) {
    return -1;
  }

  try {
    return std::stoi(content);
  } catch (const std::invalid_argument &e) {
    std::cerr << e.what() << std::endl;
    return -1;
  } catch (const std::out_of_range &e) {
    std::cerr << e.what() << std::endl;
    return -1;
  }
}

void UnsafeShutdown::ConfirmRebootedWithUS() {
  for (auto &dc : us_dimm_colls_in_test_) {
    for (size_t i = 0; i < dc.GetSize(); ++i) {
      int recorded_usc = ReadRecordedUSC(local_dimm_config->GetTestDir() +
                                         SEPARATOR + dc[i].GetUid());
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
