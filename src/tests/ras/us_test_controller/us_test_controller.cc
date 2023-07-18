/*
 * Copyright 2018-2023, Intel Corporation
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

#include "us_test_controller.h"

void USTestController::SetUp() {
  ASSERT_EQ(WaitForDutsConnection(15), 0);
}

int USTestController::PhaseExecute(const std::string& phase_number,
                                   const std::string& arg) const {
  auto& primary_dut = (*ras_config)[0];

  std::string cmd = primary_dut.GetBinDir() + "/UNSAFE_SHUTDOWN " +
                    phase_number + " " + arg + " " +
                    primary_dut.GetInjectPolicy() + " --gtest_filter=" +
                    *gtest_filter;

  std::cout << "Executing command: " << cmd << std::endl;

  auto out = primary_dut.ExecuteCmd(cmd);
  std::cout << out.GetContent() << std::endl;
  return out.GetExitCode();
}

int USTestController::RunPowerCycle() const {
  int ret = 0;

  std::vector<std::future<Output<char>>> threads;
  for (auto& dut : *ras_config) {
    threads.emplace_back(std::async(&DUT::PowerCycle, &dut));
  }
  for (const auto& thread : threads) {
    thread.wait();
  }
  for (auto& thread : threads) {
    auto out = thread.get();
    if (out.GetExitCode() != 0) {
      std::cerr << out.GetContent() << std::endl
                << "Exit code: " << out.GetExitCode() << std::endl;
      ret = out.GetExitCode();
    }
  }

  return ret;
}

int USTestController::WaitForDutsConnection(unsigned int timeout) const {
  int ret = 0;

  std::vector<std::future<bool>> threads;
  for (auto& dut : *ras_config) {
    threads.emplace_back(std::async(&DUT::WaitForConnection, &dut, timeout));
  }
  for (const auto& thread : threads) {
    thread.wait();
  }
  for (auto& thread : threads) {
    if (!thread.get()) {
      ret = -1;
    }
  }

  return ret;
}

TEST_F(USTestController, USC_TEST) {
  int phases_count = ras_config->GetPhasesCount();
  for (int i = 1; i <= phases_count; ++i) {
    std::string action{i == phases_count ? "cleanup" : "inject"};
    int exit_code = PhaseExecute(std::to_string(i), action);

    EXPECT_EQ(0, exit_code) << "At least one test failed during phase " << i;

    if (IsFatalError(exit_code)) {
      PhaseExecute("-1", "cleanup");
      FAIL() << "Fatal error, execution will not be continued";
    }

    if (i < phases_count) {
      ASSERT_EQ(0, RunPowerCycle()) << "Power cycle on DUTs failed";
      ASSERT_EQ(0, WaitForDutsConnection(15 * 60))
          << "Could not connect to at least one of DUTs";
    }
  }
}
