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
#include "remote_test_phase.h"

RemoteTestPhase::RemoteTestPhase() {
  if (configs_.ReadConfigFile() != 0) {
    throw std::invalid_argument(
        "Reading config file for local DIMM configuration failed");
  }
}

int RemoteTestPhase::Begin() const {
  for (auto &node : configs_) {
    std::string mountpoints_arg;
    for (auto &mnt : GetUnsafeMountpoints(node)) {
      mountpoints_arg += mnt + " ";
    }
    for (auto &mnt : GetSafeMountpoints(node)) {
      mountpoints_arg += mnt + " ";
    }

    IShell shell{node.GetAddress()};
    std::string cmd = GetRemoteAgentPath(node) + " record " + policy_arg_ +
                      " " + node.GetTestDir() + " " + mountpoints_arg;

    auto out = shell.ExecuteCommand(cmd);
    if (out.GetExitCode() != 0) {
      std::cerr << cmd << ": " << out.GetContent() << std::endl;
      return 1;
    }
  }

  return 0;
}

int RemoteTestPhase::Inject() const {
  for (auto &node : configs_) {
    std::string mountpoints_arg;
    for (auto &mnt : GetUnsafeMountpoints(node)) {
      mountpoints_arg += mnt + " ";
    }

    IShell shell{node.GetAddress()};
    std::string cmd = GetRemoteAgentPath(node) + " inject " + policy_arg_ +
                      " " + node.GetTestDir() + " " + mountpoints_arg;

    auto out = shell.ExecuteCommand(cmd);
    if (out.GetExitCode() != 0) {
      std::cerr << cmd << ": " << out.GetContent() << std::endl;
      return 1;
    }

    /* rpmemd closes the remote pool after application end so to prevent it
     * remote machine needs to be powered off immediately after inject */
    out = shell.ExecuteCommand("poweroff");
    if (shell.ExecuteCommand(":").GetExitCode() == 0) {
      std::cerr << "poweroff on host " << node.GetAddress()
                << " was unsuccesful: " << out.GetContent() << std::endl;
      return 1;
    }
  }
  return 0;
}

int RemoteTestPhase::CheckUSC() const {
  for (auto &node : configs_) {
    std::string unsafe_mountpoints_arg;
    for (auto &mnt : GetUnsafeMountpoints(node)) {
      unsafe_mountpoints_arg += mnt + " ";
    }
    std::string safe_mountpoints_arg;
    for (auto &mnt : GetSafeMountpoints(node)) {
      safe_mountpoints_arg += mnt + " ";
    }

    IShell shell{node.GetAddress()};
    std::string cmd = GetRemoteAgentPath(node) + " check-unsafe " +
                      policy_arg_ + " " + node.GetTestDir() + " " +
                      unsafe_mountpoints_arg;

    auto out = shell.ExecuteCommand(cmd);
    if (out.GetExitCode() != 0) {
      std::cerr << cmd << ": " << out.GetContent() << std::endl;
      return 1;
    }

    cmd = GetRemoteAgentPath(node) + " check-safe " + policy_arg_ + " " +
          node.GetTestDir() + " " + safe_mountpoints_arg;
    out = shell.ExecuteCommand(cmd);
    if (out.GetExitCode() != 0) {
      std::cerr << cmd << ": " << out.GetContent() << std::endl;
      return 1;
    }
  }

  return 0;
}

int RemoteTestPhase::End() const {
  for (auto &node : configs_) {
    std::string mountpoints_arg;
    for (auto &mnt : node) {
      mountpoints_arg += mnt + " ";
    }

    IShell shell{node.GetAddress()};
    std::string cmd = GetRemoteAgentPath(node) + " cleanup " + policy_arg_ +
                      " " + node.GetTestDir() + " " + mountpoints_arg;

    auto out = shell.ExecuteCommand(cmd);
    if (out.GetExitCode() != 0) {
      std::cerr << cmd << ": " << out.GetContent() << std::endl;
      return 1;
    }
  }

  return 0;
}

/* Arbitrarily divide available NVDIMM namespaces between unsafe (to be
 * injected) and safe namespaces. This distribution is meant to enable running
 * the biggest number of tests with limited number of available NVDIMMS. */

const std::vector<std::string> RemoteTestPhase::GetSafeMountpoints(
    const RemoteDimmNode &node) const {
  std::vector<std::string> ret;
  int i = 0;
  std::copy_if(node.begin(), node.end(), std::back_inserter(ret),
               [&i, this](std::string) -> bool { return i++ == 1; });
  return ret;
}

const std::vector<std::string> RemoteTestPhase::GetUnsafeMountpoints(
    const RemoteDimmNode &node) const {
  std::vector<std::string> ret;
  int i = 0;
  std::copy_if(node.begin(), node.end(), std::back_inserter(ret),
               [&i, this](std::string) -> bool { return i++ != 1; });
  return ret;
}
