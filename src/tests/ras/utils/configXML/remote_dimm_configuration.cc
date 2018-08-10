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

#include "remote_dimm_configuration.h"
#include "shell/i_shell.h"

RemoteDimmNode::RemoteDimmNode(const std::string &address,
                               const std::string &test_dir,
                               const std::string &bins_dir,
                               pugi::xml_node &&node)
    : address_(address), test_dir_(test_dir), bins_dir_(bins_dir) {
  for (auto &&it : node.children("mountPoint")) {
    std::string mount_point = it.text().get();
    if (mount_point.empty() ||
        shell_.ExecuteCommand("test -d " + mount_point).GetExitCode() != 0) {
      throw std::invalid_argument("Cannot find " + mount_point +
                                  " on host described by address: " + address);
    }
    mountpoints_.emplace_back(mount_point);
  }
}

int RemoteDimmConfigurationsCollection::FillConfigFields(
    pugi::xml_node &&root) {
  IShell shell;
  std::string address;
  std::string port = "22";
  size_t pos;
  int ret = -1;

  for (auto &&it : root.children("remoteConfiguration")) {
    ret = 0;

    address = it.child("address").text().as_string();
    pos = address.find_last_of(":");

    if (pos != std::string::npos) {
      port = address.substr(pos + 1);
      address = address.substr(0, pos - 1);
    }

    if (shell.ExecuteCommand("ssh " + address + " -p " + port + " exit")
            .GetExitCode() != 0) {
      std::cerr << shell.GetLastOutput().GetContent() << std::endl;
      return -1;
    }

    try {
      remote_configurations_.emplace_back(RemoteDimmNode(
          address + " -p " + port, it.child("testDir").text().as_string(),
          it.child("binsDir").text().as_string(),
          root.child("dimmConfiguration")));
    } catch (const std::invalid_argument &e) {
      std::cerr << "Exception was caught: " << e.what() << std::endl;
      return -1;
    }
  }

  if (ret == -1) {
    std::cerr << "remoteConfiguration node does not exist" << std::endl;
  }

  return ret;
}
