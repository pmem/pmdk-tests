/*
 * Copyright (c) 2018, Intel Corporation
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in
 * the documentation and/or other materials provided with the
 * distribution.
 *
 * * Neither the name of Intel Corporation nor the names of its
 * contributors may be used to endorse or promote products derived
 * from this software without specific prior written permission.
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
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY LOG OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "ssh_runner.h"

bool SshRunner::WaitForConnection(unsigned int timeout_mins) {
  std::cout << "Waiting for connection, timeout: " << timeout_mins
            << " minutes." << std::endl;
  std::chrono::duration<float> timeout(timeout_mins * 60);
  auto elapsed = std::chrono::duration<float>::zero();
  auto start = std::chrono::system_clock::now();

  bool available = HostAvailable();
  while (!available && elapsed < timeout) {
    std::this_thread::sleep_for(std::chrono::seconds(1));
    available = HostAvailable();
    elapsed = std::chrono::system_clock::now() - start;
  }

  if (available) {
    std::cout << "Connected to " << address_ << std::endl;
  } else {
    std::cerr << "Could not connect to " << address_ << std::endl;
  }
  return available;
}

bool SshRunner::HostAvailable() {
  return ExecuteRemote(":").GetExitCode() != connection_error_;
}

Output<char> SshRunner::ExecuteRemote(std::string cmd) {
  return shell_.ExecuteCommand(
      "ssh -o ServerAliveInterval=1 -o PasswordAuthentication=no " + address_ +
      " \"" + cmd + "\"");
}
