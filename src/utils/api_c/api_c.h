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

#ifndef PMDK_TESTS_SRC_UTILS_API_C_API_C_H_
#define PMDK_TESTS_SRC_UTILS_API_C_API_C_H_


#include "constants.h"
#include "non_copyable/non_copyable.h"
#include <iostream>
#include <string>
#include <sys/stat.h>
#include <vector>

class ApiC final : NonCopyable {
public:
  static int GetExecutablePath(std::string &path);
  static int CreateFileT(const std::string &path, const std::string &content);
  static int CreateFileT(const std::string &path,
                         const std::vector<std::string> &content);
  static int AllocateFileSpace(const std::string &path, size_t length);
  static int ReadFile(const std::string &path, std::string &content);
  static bool RegularFileExists(const std::string &path);
  static long long GetFileSize(const std::string &path);
  static std::vector<long long>
  GetFilesSize(const std::vector<std::string> &paths);
  static unsigned short GetFilePermission(const std::string &path);
  static int SetFilePermission(const std::string &path, int permissions);
  static int RemoveFile(const std::string &path);
  static int CreateDirectoryT(const std::string &dir);
  static bool DirectoryExists(const std::string &dir);
  static int CleanDirectory(const std::string &dir);
  static int RemoveDirectoryT(const std::string &dir);
  static long long GetFreeSpaceT(const std::string &dir);
  /* SetEnv -- adds tne environment variable name to the environment
   * with the value value. Returns 0 on success, -1 otherwise. */
  static int SetEnv(const std::string &name, const std::string &value);
  /* UnsetEnv -- deletes the variable name from the environment.
   * Returns 0 on success, -1 otherwise. */
  static int UnsetEnv(const std::string &name);
};

#endif // !PMDK_TESTS_SRC_UTILS_API_C_API_C_H_
