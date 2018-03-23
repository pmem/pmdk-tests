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
  std::string test_name{std::string{test_info.test_case_name()} + "_" +
                        std::string{test_info.name()}};
  string_utils::ReplaceAll(test_name, SEPARATOR, std::string{"_"});
  string_utils::ReplaceAll(test_name, test_phase_.GetPhaseName(),
                           std::string{""});
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
  return test_phase_.GetTestDir() + GetNormalizedTestName() + "_passed";
}

void UnsafeShutdown::Repair(std::string pool_file_path, int expected_exit) {
  std::string cmd = "pmempool check -ry " + pool_file_path;
  auto out = shell_.ExecuteCommand(cmd);
  ASSERT_EQ(expected_exit, out.GetExitCode()) << cmd << std::endl
                                              << out.GetContent();
}

void UnsafeShutdown::Sync(std::string pool_file_path, int expected_exit) {
  std::string cmd = "pmempool sync " + pool_file_path;
  auto out = shell_.ExecuteCommand(cmd);
  ASSERT_EQ(expected_exit, out.GetExitCode()) << cmd << std::endl
                                              << out.GetContent();
}

void UnsafeShutdown::Transform(std::string src_path, std::string dest_path,
                               int expected_exit) {
  std::string cmd = "pmempool transform " + src_path + " " + dest_path;
  auto out = shell_.ExecuteCommand(cmd);
  ASSERT_EQ(expected_exit, out.GetExitCode()) << cmd << std::endl
                                              << out.GetContent();
}

const testing::TestInfo &UnsafeShutdown::GetTestInfo() {
  return *::testing::UnitTest::GetInstance()->current_test_info();
}
