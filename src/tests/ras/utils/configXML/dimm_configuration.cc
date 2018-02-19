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
  mount_point_ = dimm_configuration.text().get();

  if (mount_point_.empty()) {
    throw std::invalid_argument(
        "mountPoint field is empty. Please set mountPoint value.");
  }

  if (!ApiC::DirectoryExists(this->mount_point_)) {
    throw std::invalid_argument(
        "Directory " + this->mount_point_ +
        " does not exist. Please change mountPoint field value.");
  }

  if (!ApiC::DirectoryExists((mount_point_ + SEPARATOR + "pmdk_tests")) &&
      ApiC::CreateDirectoryT((mount_point_ + SEPARATOR + "pmdk_tests").c_str()) !=
          0) {
    throw std::invalid_argument("");
  }

  mount_point_ += SEPARATOR + "pmdk_tests" + SEPARATOR;
}

int DimmConfiguration::SetDimmDevices(
    const pugi::xml_node &node, std::vector<DimmConfiguration> &dimm_devices) {
  int ret = -1;

  for (auto &&it : node.child("dimmConfiguration").children("mountPoint")) {
    ret = 0;
    try {
      DimmConfiguration temp_device = DimmConfiguration(std::move(it));
      dimm_devices.emplace_back(std::move(temp_device));
    } catch (std::exception e) {
      std::cerr << e.what() << std::endl;
      return -1;
    }
  }

  if (ret == -1) {
    std::cerr << "dimmConfiguration node does not exist" << std::endl;
  }

  return ret;
}
