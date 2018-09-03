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

#include "ras_configuration.h"

DUT::DUT(const std::string& address, const std::string& power_cycle_command,
         const std::string& bin_dir, const std::string& inject_policy)
    : address_(address),
      power_cycle_command_(power_cycle_command),
      bin_dir_(bin_dir),
      inject_policy_(inject_policy) {
  if (0 != ishell_.ExecuteCommand("test -d " + bin_dir_).GetExitCode()) {
    throw std::invalid_argument(ishell_.GetLastOutput().GetContent());
  }
}

bool DUT::WaitForConnection(unsigned int timeout_secs) {
  std::cout << "Waiting for connection, timeout: " << timeout_secs
            << " minutes." << std::endl;
  std::chrono::duration<float> timeout(timeout_secs);
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

int RASConfigurationCollection::FillConfigFields(pugi::xml_node&& root) {
  IShell shell;
  std::string address;
  std::string port = "22";
  size_t pos;
  int ret = -1;

  phases_count_ =
      root.child("RASConfiguration").child("phasesCount").text().as_int();
  if (phases_count_ <= 0) {
    throw std::invalid_argument("Wrong phases count value: " +
                                std::to_string(phases_count_));
  }

  for (auto&& it : root.child("RASConfiguration").children("DUT")) {
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
      duts_collection_.emplace_back(
          DUT(address + " -p " + port,
              it.child("powerCycleCommand").text().as_string(),
              it.child("binDir").text().as_string(),
              it.child("injectPolicy").text().as_string()));
    } catch (const std::invalid_argument& e) {
      std::cerr << e.what() << std::endl;
      return -1;
    }
  }

  if (ret == -1) {
    std::cerr << "RASConfiguration node does not exist" << std::endl;
  }

  return ret;
}
