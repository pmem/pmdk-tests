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

#ifndef PMDK_TESTS_SRC_UTILS_CONFIGXML_RAS_CONFIGURATION_H_
#define PMDK_TESTS_SRC_UTILS_CONFIGXML_RAS_CONFIGURATION_H_

#include "configXML/read_config.h"
#include "shell/i_shell.h"

class DUT final {
 private:
  std::string address_;
  std::string power_cycle_command_;
  std::string bin_dir_;
  IShell ishell_{address_};

 public:
  DUT() = delete;
  DUT(const std::string& address, const std::string& power_cycle_command,
      const std::string& bin_dir);
  DUT(DUT&& temp)
      : address_(temp.address_),
        power_cycle_command_(temp.power_cycle_command_),
        bin_dir_(temp.bin_dir_){};
  const std::string& GetBinDir() const {
    return this->bin_dir_;
  }
  Output<char> ExecuteCmd(std::string cmd) {
    return ishell_.ExecuteCommand(cmd);
  }
  Output<char> PowerCycle() {
    IShell i_shell;
    return i_shell.ExecuteCommand(power_cycle_command_);
  }
};

class RASConfigurationCollection final
    : public ReadConfig<RASConfigurationCollection> {
 private:
  friend class ReadConfig<RASConfigurationCollection>;
  int FillConfigFields(pugi::xml_node&& root);
  std::vector<DUT> duts_collection_;

 public:
  DUT& GetDUT(int idx) {
    return duts_collection_.at(idx);
  }
  DUT& operator[](std::size_t idx) {
    return this->duts_collection_.at(idx);
  }
};

#endif  // !PMDK_TESTS_SRC_UTILS_CONFIGXML_RAS_CONFIGURATION_H_
