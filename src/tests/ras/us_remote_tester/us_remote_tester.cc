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
}

void USRemoteTester::TearDown() {
  std::string cmd = *test_bin_path + " cleanup";
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
  std::string cmd = *test_bin_path + " " + arg + " " + filter;
  std::cout << "Executing command: " << cmd << std::endl;

  auto out = ssh_runner_.ExecuteRemote(cmd);
  std::cout << out.GetContent() << std::endl;

  if (arg.compare("inject") == 0) {
    ASSERT_EQ(out.GetExitCode(), ssh_runner_.connection_error_)
        << "Test phase did not end with shutdown";
    ASSERT_EQ(RunPowerCycle(), 0) << "power cycle failed" << std::endl;
  }
  EXPECT_TRUE(AllPassed(out)) << out.GetContent();
}

int USRemoteTester::RunPowerCycle() {
  int ret = 0;
  IShell shell;

  std::this_thread::sleep_for(std::chrono::seconds(15));
  std::cout << *address << ": turning power off." << std::endl;

  auto out = shell.ExecuteCommand(*power_off_cmd);
  if (out.GetExitCode() != 0) {
    std::cerr << out.GetContent() << std::endl;
    ret = 1;
  }

  std::this_thread::sleep_for(std::chrono::seconds(20));
  std::cout << *address << ": turning power on." << std::endl;

  out = shell.ExecuteCommand(*power_on_cmd);
  if (out.GetExitCode() != 0) {
    std::cerr << out.GetContent() << std::endl;
    ret = 1;
  }

  return ret;
}

TEST_F(USRemoteTester, USC_TEST) {
  std::string before_filter = "--gtest_filter=\"" + *filter + "*_before_us*\"";
  ASSERT_NO_FATAL_FAILURE(PhaseExecute(before_filter, "inject"));
  ASSERT_TRUE(ssh_runner_.WaitForConnection(15));

  std::string after_filter =
      "--gtest_filter=\"" + *filter + "*_after_first_us*\"";
  ASSERT_NO_FATAL_FAILURE(PhaseExecute(after_filter, "inject"));
  ASSERT_TRUE(ssh_runner_.WaitForConnection(15));

  std::string after_second_filter =
      "--gtest_filter=\"" + *filter + "*_after_second_us*\"";
  ASSERT_NO_FATAL_FAILURE(PhaseExecute(after_second_filter, ""));
}
