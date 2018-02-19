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

#include "dimm_configuration.h"

DimmConfiguration::DimmConfiguration(pugi::xml_node &&dimm_configuration) {
  device_name_ = dimm_configuration.child("deviceName").text().get();

  test_dir_ = dimm_configuration.child("testDir").text().get();

  if (test_dir_.empty()) {
    throw std::invalid_argument(
        "TestDir field is empty. Please set testDir value.");
  }

  if (!api_c_.DirectoryExists(this->test_dir_)) {
    throw std::invalid_argument(
        "Directory " + this->test_dir_ +
        " does not exist. Please change testDir field value.");
  }

  if (!api_c_.DirectoryExists((test_dir_ + SEPARATOR + "pmdk_tests")) &&
      api_c_.CreateDirectoryT((test_dir_ + SEPARATOR + "pmdk_tests").c_str()) !=
          0) {
    throw std::invalid_argument("");
  }

  test_dir_ += SEPARATOR + "pmdk_tests" + SEPARATOR;
}
