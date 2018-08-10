/*
 * Copyright 2017-2018, Intel Corporation
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

#ifndef PMDK_TESTS_SRC_UTILS_CONFIGXML_READ_CONFIG_H_
#define PMDK_TESTS_SRC_UTILS_CONFIGXML_READ_CONFIG_H_

#include "api_c/api_c.h"
#include "non_copyable/non_copyable.h"
#include "pugixml.hpp"

/*
 * ReadConfig -- class based on CRTP pattern. Checks that config.xml file exists
 * and it consists with appropriate nodes.
 */
template <class DerivedConfig>
class ReadConfig : public NonCopyable {
 private:
  const std::string config_{"config.xml"};

 protected:
  int SetTestDir(const pugi::xml_node &root, std::string &test_dir);
  int FillConfigFields(pugi::xml_node &&root) {
    return static_cast<DerivedConfig *>(this)->FillConfigFields(
        std::move(root));
  }

 public:
  /*
   * ReadConfigFile -- Checks that config.xml file exists and reads its content.
   * Returns 0 on success, print message and returns -1 otherwise.
   */
  int ReadConfigFile();
};

template <class DerivedConfig>
inline int ReadConfig<DerivedConfig>::SetTestDir(const pugi::xml_node &root,
                                                 std::string &test_dir) {
  test_dir = root.child("testDir").text().get();

  if (test_dir.empty()) {
    std::cerr << "TestDir field is empty. Please change testDir field value."
              << std::endl;
    return -1;
  }

  if (!ApiC::DirectoryExists(test_dir)) {
    std::cerr << "Directory " + test_dir +
                     " does not exist. Please change testDir field value."
              << std::endl;
    return -1;
  }

  if (!ApiC::DirectoryExists((test_dir + SEPARATOR + "pmdk_tests")) &&
      ApiC::CreateDirectoryT((test_dir + SEPARATOR + "pmdk_tests")) != 0) {
    return -1;
  }

  test_dir += SEPARATOR + "pmdk_tests" + SEPARATOR;

  return 0;
}

template <class DerivedConfig>
int ReadConfig<DerivedConfig>::ReadConfigFile() {
  pugi::xml_document config;
  std::string path;
  int ret = ApiC::GetExecutableDirectory(path);

  if (ret != 0) {
    std::cerr << "Cannot get path to executable" << std::endl;
    return -1;
  }

  path += config_;

  pugi::xml_parse_result result = config.load_file(path.c_str());

  if (!result) {
    std::cerr << "Config file is missing\nError description: "
              << result.description()
              << "\nPlease place it in the following directory: " << path
              << std::endl;
    return -1;
  }

  pugi::xml_node root = config.child("configuration");

  if (root.empty()) {
    std::cerr << "Cannot find 'configuration' node" << std::endl;
    return -1;
  }

  return FillConfigFields(std::move(root));
}

#endif  // !PMDK_TESTS_SRC_UTILS_CONFIGXML_READ_CONFIG_H_
