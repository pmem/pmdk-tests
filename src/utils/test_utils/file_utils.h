/*
 * Copyright 2017, Intel Corporation
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

#ifndef PMDK_TESTS_SRC_UTILS_TEST_UTILS_FILE_UTILS_H_
#define PMDK_TESTS_SRC_UTILS_TEST_UTILS_FILE_UTILS_H_

#include <string>
#include "api_c/api_c.h"
#include "constants.h"
#include "poolset/poolset.h"
#include "poolset/poolset_management.h"

namespace file_utils {
static inline int ValidatePoolset(const Poolset& poolset, int poolset_mode) {
  PoolsetManagement p_mgmt;
  ApiC api_c;

  if (!p_mgmt.AllFilesExist(poolset)) {
    std::cout <<  "Parts in the pool are missing" << std::endl;
    return -1;
  }

  size_t size = 0;
  for (const auto& part : poolset.GetParts()) {
    size = api_c.GetFileSize(part.GetPath());
    if (SIZES_MiB.at(part.GetSize()) != size) {
			std::cout << "Size of parts are different than expected" << std::endl;;
      return -1;
    }
  }

  int mode = 0;
  for (const auto& part : poolset.GetParts()) {
    mode = api_c.GetFilePermission(part.GetPath());
    if (poolset_mode != mode) {
			std::cout << "Size of parts are different than expected" << std::endl;;
      return -1;
    }
  }

  return 0;
}

static inline int ValidateFile(const std::string& path, size_t file_size,
                               int file_mode) {
  ApiC api_c;
  if (!api_c.FileExists(path)) {
		std::cout << "File is missing" << std::endl;
    return -1;
  }
  if (file_size != api_c.GetFileSize(path)) {
    std::cout <<  "Size of file is different than expected" << std::endl;
    return -1;
  }
  if (file_mode != api_c.GetFilePermission(path)) {
    std::cout <<  "Permission of file is different than expected" << std::endl;
    return -1;
  }

  return 0;
}
}

#endif  // !PMDK_TESTS_SRC_UTILS_TEST_UTILS_FILE_UTILS_H_