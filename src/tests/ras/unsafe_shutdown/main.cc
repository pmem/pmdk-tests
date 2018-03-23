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
#include <string>
#include "api_c/api_c.h"
#include "configXML/local_dimm_configuration.h"
#include "dimm/dimm.h"
#include "gtest/gtest.h"
#include "shell/i_shell.h"

std::unique_ptr<LocalDimmConfiguration> local_dimm_config{
    new LocalDimmConfiguration()};

std::vector<DimmCollection> us_dimms;
std::vector<DimmCollection> non_us_dimms;

void Reboot() {
  std::cout << "Rebooting." << std::endl;
  IShell shell;
  shell.ExecuteCommand("reboot");
}

void InitializeDimms() {
  if (local_dimm_config->GetSize() > 0) {
    us_dimms.emplace_back(local_dimm_config.get()->operator[](0));
  }

  if (local_dimm_config->GetSize() > 1) {
    non_us_dimms.emplace_back(local_dimm_config.get()->operator[](1));
  }

  if (local_dimm_config->GetSize() > 2) {
    for (int i = 2; i < local_dimm_config->GetSize(); ++i) {
      us_dimms.emplace_back(local_dimm_config.get()->operator[](i));
    }
  }
}

void SetUp() {
  for (int i = 0; i < local_dimm_config->GetSize(); ++i) {
    ApiC::CreateDirectoryT(
        local_dimm_config.get()->operator[](i).GetMountpoint());
  }
}

void CleanUp() {
  for (int i = 0; i < local_dimm_config->GetSize(); ++i) {
    ApiC::CleanDirectory(
        local_dimm_config.get()->operator[](i).GetMountpoint());
    ApiC::RemoveDirectoryT(
        local_dimm_config.get()->operator[](i).GetMountpoint());
  }
  ApiC::CleanDirectory(local_dimm_config->GetTestDir());
  ApiC::RemoveDirectoryT(local_dimm_config->GetTestDir());
}

int main(int argc, char **argv) {
  int ret = 0;
  try {
    if (local_dimm_config->ReadConfigFile() != 0) {
      return 1;
    }

    ::testing::InitGoogleTest(&argc, argv);

    InitializeDimms();

    const std::vector<std::string> args(argv + 1, argv + argc);
    if (std::find(args.begin(), args.end(), "setup") != args.end()) {
      SetUp();
      return 0;
    }
    if (std::find(args.begin(), args.end(), "cleanup") != args.end()) {
      CleanUp();
      return 0;
    }

    std::cout << "Return code of tests execution: " << RUN_ALL_TESTS()
              << std::endl;

    if (std::find(args.begin(), args.end(), "shutdown") != args.end()) {
      Reboot();
    }
  } catch (const std::exception &e) {
    std::cerr << "Exception was caught: " << e.what() << std::endl;
    ret = 1;
  }

  return ret;
}
