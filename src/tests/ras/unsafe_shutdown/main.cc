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

#include <algorithm>
#include <exception>
#include <iostream>
#include <memory>
#include <numeric>
#include <string>
#include "api_c/api_c.h"
#include "configXML/local_dimm_configuration.h"
#include "dimm/dimm.h"
#include "gtest/gtest.h"
#include "shell/i_shell.h"

std::unique_ptr<LocalDimmConfiguration> local_dimm_config{
    new LocalDimmConfiguration()};

std::vector<DimmCollection> us_dimm_colls;
std::vector<DimmCollection> non_us_dimm_colls;
std::vector<size_t> dimms_to_inject = {0};

bool SomePassed() {
  ::testing::UnitTest *ut = ::testing::UnitTest::GetInstance();
  return ut->successful_test_count() > 0;
}

int RecordUSC(Dimm dimm) {
  int usc = dimm.GetShutdownCount();
  if (usc == -1) {
    std::cerr << "Reading USC from dimm " << dimm.GetUid() << " failed"
              << std::endl;
    return -1;
  }

  if (ApiC::CreateFileT(
          local_dimm_config->GetTestDir() + SEPARATOR + dimm.GetUid(),
          std::to_string(usc)) == -1) {
    return -1;
  }
  return 0;
}

void Inject() {
  /* Indexes of dimms in DimmCollection namespace into which US is injected.
   */
  for (auto &dc : us_dimm_colls) {
    for (size_t i = 0; i < dc.GetSize(); ++i) {
      ASSERT_EQ(RecordUSC(dc[i]), 0);
      ASSERT_EQ(dc[i].InjectUnsafeShutdown(), 0);
    }
  }
}

void Shutdown() {
  std::cout << "Machine shutdown." << std::endl;
  IShell shell;
  shell.ExecuteCommand("poweroff");
}

void InitializeDimms() {
  if (local_dimm_config->GetSize() > 0) {
    us_dimm_colls.emplace_back(local_dimm_config.get()->operator[](0));
  }

  if (local_dimm_config->GetSize() > 1) {
    non_us_dimm_colls.emplace_back(local_dimm_config.get()->operator[](1));
  }

  for (int i = 2; i < local_dimm_config->GetSize(); ++i) {
    us_dimm_colls.emplace_back(local_dimm_config.get()->operator[](i));
  }
}

int main(int argc, char **argv) {
  int ret = 0;
  try {
    if (local_dimm_config->ReadConfigFile() != 0) {
      return 1;
    }

    for (int i = 0; i < local_dimm_config->GetSize(); ++i) {
      ApiC::CreateDirectoryT(
          local_dimm_config.get()->operator[](i).GetMountpoint());
    }

    ::testing::InitGoogleTest(&argc, argv);

    InitializeDimms();

    const std::vector<std::string> args(argv + 1, argv + argc);

    if (std::find(args.begin(), args.end(), "cleanup") != args.end()) {
      ApiC::CleanDirectory(local_dimm_config->GetTestDir());
      ApiC::RemoveDirectoryT(local_dimm_config->GetTestDir());

      for (int i = 0; i < local_dimm_config->GetSize(); ++i) {
        ApiC::CleanDirectory(
            local_dimm_config.get()->operator[](i).GetMountpoint());
        ApiC::RemoveDirectoryT(
            local_dimm_config.get()->operator[](i).GetMountpoint());
      }
      return 0;
    }

    ret = RUN_ALL_TESTS();
    std::cout << "Return code of tests execution: " << ret << std::endl;

    if (std::find(args.begin(), args.end(), "inject") != args.end()) {
      if (SomePassed()) {
        Inject();
        Shutdown();
      } else {
        std::cerr << "All tests failed in this phase, reboot not executed."
                  << std::endl;
      }
    }
  } catch (const std::exception &e) {
    std::cerr << "Exception was caught: " << e.what() << std::endl;
    ret = 1;
  }

  return ret;
}
