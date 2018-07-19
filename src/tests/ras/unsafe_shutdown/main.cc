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

#include "configXML/local_dimm_configuration.h"
#include "exit_codes.h"
#include "gtest/gtest.h"
#include "inject_mananger/inject_manager.h"
#include "local_test_phase.h"
#include "shell/i_shell.h"

bool PartiallyPassed() {
  ::testing::UnitTest *ut = ::testing::UnitTest::GetInstance();
  return ut->successful_test_count() > 0 && ut->failed_test_count() > 0;
}

int main(int argc, char **argv) {
  int ret = 0;
  try {
    ::testing::InitGoogleTest(&argc, argv);
    LocalTestPhase &test_phase = LocalTestPhase::GetInstance();
    test_phase.ParseCmdArgs(argc, argv);

    if ((ret = test_phase.RunPreTestAction()) == 0) {
      ret = RUN_ALL_TESTS();
    }
    if (test_phase.RunPostTestAction() != 0) {
      return 1;
    }

    if (PartiallyPassed()) {
      ret = exit_codes::partially_passed;
    }

  } catch (const std::exception &e) {
    std::cerr << "Exception was caught: " << e.what() << std::endl;
    ret = 1;
  }
  return ret;
}
