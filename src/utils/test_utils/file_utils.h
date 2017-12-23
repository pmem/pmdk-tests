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

#ifndef PMDK_TESTS_SRC_UTILS_TEST_UTILS_FILE_UTILS_H_
#define PMDK_TESTS_SRC_UTILS_TEST_UTILS_FILE_UTILS_H_

#include <string>
#include "api_c/api_c.h"
#include "constants.h"
#include "poolset/poolset.h"
#include "poolset/poolset_management.h"

namespace file_utils {
static inline size_t GetSize(std::string size) {
  std::array<std::string, 9> suffix = {"KiB", "MiB", "GiB", "KB", "MB",
                                       "GB",  "K",   "M",   "G"};
  size_t pos;
  for (const auto &suf : suffix) {
    if ((pos = size.find(suf)) != std::string::npos) {
      size.erase(pos);

      return static_cast<size_t>(std::stoul(size)) * SIZES.at(suf);
    }
  }

  return static_cast<size_t>(std::stoul(size));
}

/*
 * ValidatePoolset -- checks that all parts in poolset exist, sizes of parts are
 * correct and specified mode is set. Returns 0 on success, print error message
 * and returns -1 otherwise.
 */
static inline int ValidatePoolset(const Poolset &poolset, int poolset_mode) {
  PoolsetManagement p_mgmt;

  if (!p_mgmt.AllFilesExist(poolset)) {
    std::cerr << "Part's from the pool set file are missing" << std::endl;
    return -1;
  }

  int ret = 0;
  size_t size = 0;
  for (const auto &part : poolset.GetParts()) {
    size = ApiC::GetFileSize(part.GetPath());
    if (GetSize(part.GetSize()) != size) {
      std::cerr << "Part's size mismatch\n" << part.GetPath()
                << "\nExpected: " << part.GetSize() << "\nActual: " << size
                << std::endl;
      ret = -1;
    }
  }

  if (ret == -1) {
    return -1;
  }

  int mode = 0;
  for (const auto &part : poolset.GetParts()) {
    mode = ApiC::GetFilePermission(part.GetPath());
    if (poolset_mode != mode) {
      std::cerr << "Part's permission mismatch\n" << part.GetPath()
                << "\nExpected: " << poolset_mode << "\nActual: " << mode
                << std::endl;
      ret = -1;
    }
  }

  return ret;
}

/*
 * ValidateFile -- checks that file in given path exists, size of file is the
 * same as file_size argument and specified mode is set. Returns 0 on success,
 * print error message and returns -1 otherwise.
 */
static inline int ValidateFile(const std::string &path, size_t file_size,
                               int file_mode) {
  if (!ApiC::RegularFileExists(path)) {
    std::cerr << "File is missing" << std::endl;
    return -1;
  }

  size_t size = 0;
  size = ApiC::GetFileSize(path);
  if (file_size != size) {
    std::cerr << "File's size mismatch\n" << path
              << "\nExpected: " << file_size << "\nActual: " << size
              << std::endl;
    return -1;
  }

  int ret = 0;
  int mode = 0;
  mode = ApiC::GetFilePermission(path);
  if (file_mode != mode) {
    std::cerr << "File's permission mismatch\n" << path
              << "\nExpected: " << file_mode << "\nActual: " << mode
              << std::endl;
    ret = -1;
  }

  return ret;
}
}

#endif  // !PMDK_TESTS_SRC_UTILS_TEST_UTILS_FILE_UTILS_H_
