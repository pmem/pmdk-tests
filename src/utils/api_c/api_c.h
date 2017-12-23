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

#include <sys/stat.h>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include "constants.h"
#include "non_copyable/non_copyable.h"

class ApiC final : NonCopyable {
 public:
  /*
   * GetExecutablePath -- assigns to the input argument a path to a folder
   * containing the executable file. Returns 0 on success, -1 otherwise.
   */
  static int GetExecutablePath(std::string &path);
  /*
   * CreateFileT -- creates file in given path and fills it with content. Returns
   * 0 on success, prints error message and returns -1 otherwise.
   */
  static int CreateFileT(const std::string &path, const std::string &content);
  /*
   * CreateFileT -- creates file in given path and fills it with content. Returns
   * 0 on success, prints error message and returns -1 otherwise.
   */
  static int CreateFileT(const std::string &path,
                         const std::vector<std::string> &content);
  /*
   * AllocateFileSpace -- allocates disk space in specifed path of given length.
   * Returns 0 on success, prints error message and returns -1 otherwise.
   */
  static int AllocateFileSpace(const std::string &path, size_t length);
  /*
   * ReadFile -- opens file in specified path and assigns content of the file.
   * Returns 0 on success, prints error message and returns -1 otherwise.
   */
  static int ReadFile(const std::string &path, std::string &content);
  /*
   * RegularFileExists -- checks that file in given path is regular. Returns true
   * on success, prints error message (if errno is different than ENOENT) and
   * returns false otherwise.
   */
  static bool RegularFileExists(const std::string &path);
  /*
   * GetFileSize -- returns file size on success, prints error message and
   * returns -1 otherwise.
   */
  static long long GetFileSize(const std::string &path);
  /*
   * GetFileSize -- returns vector of file sizes on success, prints error message
   * and returns empty vector otherwise.
   */
  static std::vector<long long> GetFilesSize(
      const std::vector<std::string> &paths);
  /*
   * GetFilePermission -- returns file permission on success, prints error
   * message and returns empty vector otherwise.
   */
  static unsigned short GetFilePermission(const std::string &path);
  /*
   * SetFilePermission -- sets permission to the file in given path. Returns 0 on
   * success, prints error message and returns -1 otherwise.
   */
  static int SetFilePermission(const std::string &path, int permissions);
  /*
   * RemoveFile -- removes file in given path. Returns 0 on success, prints error
   * message and returns -1 otherwise.
   */
  static int RemoveFile(const std::string &path);
  /*
   * CreateDirectoryT -- creates directory in given dir. Returns 0 on success,
   * prints error message and returns -1 otherwise.
   */
  static int CreateDirectoryT(const std::string &dir);
  /*
   * DirectoryExists -- check that directory in given dir exists. Returns true on
   * success, prints error message (if errno is different than ENOENT) and
   * returns false otherwise.
   */
  static bool DirectoryExists(const std::string &dir);
  /*
   * CleanDirectory -- search all files and directories in given dir and remove
   * each of them. Returns 0 on success, prints error message and returns -1
   * otherwise.
   */
  static int CleanDirectory(const std::string &dir);
  /*
   * RemoveDirectoryT -- removes directory in given path. Returns 0 on success,
   * prints error message and returns -1 otherwise.
   */
  static int RemoveDirectoryT(const std::string &dir);
  /*
   * GetFreeSpaceT -- returns available free space in bytes, prints error message
   * and returns -1 otherwise.
   */
  static long long GetFreeSpaceT(const std::string &dir);
};

#endif  // !PMDK_TESTS_SRC_UTILS_API_C_API_C_H_
