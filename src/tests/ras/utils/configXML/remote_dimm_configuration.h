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

#ifndef PMDK_TESTS_SRC_UTILS_CONFIGXML_REMOTE_DIMM_CONFIGURATION_H_
#define PMDK_TESTS_SRC_UTILS_CONFIGXML_REMOTE_DIMM_CONFIGURATION_H_

#include "api_c/api_c.h"
#include "configXML/read_config.h"
#include "dimm.h"
#include "pugixml.hpp"
#include "shell/i_shell.h"

class RemoteDimmNode final {
 private:
  std::string address_;
  std::string test_dir_;
  std::string bins_dir_;
  IShell shell_{address_};
  std::vector<std::string> mountpoints_;

 public:
  RemoteDimmNode(const std::string &address, const std::string &test_dir,
                 const std::string &bins_dir, pugi::xml_node &&node);
  RemoteDimmNode(RemoteDimmNode &&temp)
      : address_(temp.address_),
        test_dir_(temp.test_dir_),
        bins_dir_(temp.bins_dir_),
        mountpoints_(temp.mountpoints_) {
  }

  const std::string &GetTestDir() const {
    return this->test_dir_;
  }

  const std::string &GetAddress() const {
    return this->address_;
  }

  const std::string &GetBinsDir() const {
    return this->bins_dir_;
  }

  std::string &operator[](int idx) {
    return mountpoints_.at(idx);
  }

  std::vector<std::string>::const_iterator begin() const {
    return mountpoints_.begin();
  }

  std::vector<std::string>::const_iterator end() const {
    return mountpoints_.end();
  }
};

class RemoteDimmConfigurationsCollection final
    : public ReadConfig<RemoteDimmConfigurationsCollection> {
 private:
  friend class ReadConfig<RemoteDimmConfigurationsCollection>;
  int FillConfigFields(pugi::xml_node &&root);
  std::vector<RemoteDimmNode> remote_configurations_;

 public:
  const RemoteDimmNode &GetNode(int idx) const {
    return remote_configurations_.at(idx);
  }
};

#endif  // !PMDK_TESTS_SRC_UTILS_CONFIGXML_REMOTE_DIMM_CONFIGURATION_H_
