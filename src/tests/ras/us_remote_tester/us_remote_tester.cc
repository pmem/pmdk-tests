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

#include "us_remote_tester.h"

void USRemoteTester::SetUp() {
  ASSERT_TRUE(ssh_runner_.WaitForConnection(1));
  std::string cmd = test_bin_path + " setup";
  std::cout << "Executing command: " << cmd << std::endl;
  auto out = ssh_runner_.ExecuteRemote(cmd);
  ASSERT_EQ(out.GetExitCode(), 0) << "SetUp phase failed" << std::endl
                                  << out.GetContent();
}

void USRemoteTester::TearDown() {
  std::string cmd = test_bin_path + " cleanup";
  std::cout << "Executing command: " << cmd << std::endl;
  auto out = ssh_runner_.ExecuteRemote(cmd);
  ASSERT_EQ(out.GetExitCode(), 0) << "CleanUp phase failed" << std::endl
                                  << out.GetContent();
}

bool USRemoteTester::AllPassed(Output<char> out) {
  return out.GetContent().find("Return code of tests execution: 0") !=
         std::string::npos;
}

void USRemoteTester::PhaseExecute(std::string filter, std::string arg) {
  std::string cmd = test_bin_path + " " + arg + " " + filter;
  std::cout << "Executing command: " << cmd << std::endl;

  auto out = ssh_runner_.ExecuteRemote(cmd);
  std::cout << out.GetContent() << std::endl;

  if (arg.compare("shutdown") == 0) {
    ASSERT_EQ(out.GetExitCode(), ssh_runner_.connection_error_);
  }
  EXPECT_TRUE(AllPassed(out)) << out.GetContent();
}

TEST_F(USRemoteTester, USC_TEST) {
  std::string before_filter = "--gtest_filter=\"" + filter + "*_before_us*\"";
  ASSERT_NO_FATAL_FAILURE(PhaseExecute(before_filter, "shutdown"));
  ASSERT_TRUE(ssh_runner_.WaitForConnection(5));

  std::string after_filter =
      "--gtest_filter=\"" + filter + "*_after_first_us*\"";
  ASSERT_NO_FATAL_FAILURE(PhaseExecute(after_filter, "shutdown"));
  ASSERT_TRUE(ssh_runner_.WaitForConnection(5));

  std::string after_second_filter =
      "--gtest_filter=\"" + filter + "*_after_second_us*\"";
  ASSERT_NO_FATAL_FAILURE(PhaseExecute(after_second_filter, ""));
}
