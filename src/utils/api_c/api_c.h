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

#ifndef PMDK_TESTS_SRC_UTILS_API_C_API_C_H_
#define PMDK_TESTS_SRC_UTILS_API_C_API_C_H_

#include <sys/stat.h>
#include <iostream>
#include <string>
#include <vector>
#include "constants.h"
#include "non_copyable/non_copyable.h"

class ApiC final : NonCopyable {
 public:
  /*
   * GetExecutableDirectory -- assigns a path to a folder containing the
   * executable file to the input argument. Returns 0 on success, -1 otherwise.
   */
  static int GetExecutableDirectory(std::string &path);

  /*
   * CreateFileT -- creates file in given path and writes content. Returns 0 on
   * success, prints error message and returns -1 otherwise.
   */
  static int CreateFileT(const std::string &path, const std::string &content);

  /*
   * CreateFileT -- creates file in given path and writes content. Returns 0 on
   * success, prints error message and returns -1 otherwise.
   */
  static int CreateFileT(const std::string &path,
                         const std::vector<std::string> &content);

  /*
   * ReadFile -- opens given file and reads its content. Returns 0 on success,
   * prints error message and returns -1 otherwise.
   */
  static int ReadFile(const std::string &path, std::string &content);

  /*
   * RegularFileExists -- checks that file in given path is regular. Returns
   * true on success, prints error message (if errno is different than ENOENT)
   * and returns false otherwise.
   */
  static bool RegularFileExists(const std::string &path);

  /*
   * GetFileSize -- returns file size specified in bytes on success, prints
   * error message and
   * returns -1 otherwise.
   */
  static long long GetFileSize(const std::string &path);

  /*
   * GetFileSize -- returns vector of file sizes specified in bytes on success,
   * prints error message and returns empty vector otherwise.
   */
  static std::vector<long long> GetFilesSize(
      const std::vector<std::string> &paths);

  /*
   * GetFilePermission -- returns UNIX-style file mode bits in decimal on
   * success, prints error message and returns 0 otherwise.
   */
  static unsigned short GetFilePermission(const std::string &path);

  /*
   * SetFilePermission -- changes a file mode bits. Returns 0 on success, prints
   * error message and returns -1 otherwise.
   */
  static int SetFilePermission(const std::string &path, int permission);

  /*
   * RemoveFile -- removes specified file. Returns 0 on success, prints error
   * message and returns -1 otherwise.
   */
  static int RemoveFile(const std::string &path);

  /*
   * CreateDirectoryT -- creates a new directory with name 'path'. Returns 0 on
   * success, prints error message and returns -1 otherwise.
   */
  static int CreateDirectoryT(const std::string &path);

  /*
   * DirectoryExists -- checks that directory under given 'path' exists. Returns
   * true on success, prints error message (if errno is different than ENOENT)
   * and returns false otherwise.
   */
  static bool DirectoryExists(const std::string &path);

  /*
   * CleanDirectory -- recursively removes files and directories in given path.
   * Returns 0 on success, prints error message and returns -1 otherwise.
   */
  static int CleanDirectory(const std::string &path);

  /*
   * RemoveDirectoryT -- removes directory under given 'path'. Returns 0 on
   * success, prints error message and returns -1 otherwise.
   */
  static int RemoveDirectoryT(const std::string &path);

  /*
   * GetFreeSpaceT -- returns available free space in bytes for filesystem
   * containing given path, prints error message and returns -1 otherwise.
   */
  static long long GetFreeSpaceT(const std::string &path);

  /* SetEnv -- adds tne environment variable name to the environment
   * with the value value. Returns 0 on success, -1 otherwise. */
  static int SetEnv(const std::string &name, const std::string &value);

  /* UnsetEnv -- deletes the variable name from the environment.
   * Returns 0 on success, -1 otherwise. */
  static int UnsetEnv(const std::string &name);

#ifdef _WIN32
  /*
   * CreateFileT -- creates file in given path and writes content. Returns 0 on
   * success, prints error message and returns -1 otherwise.
   */
  static int CreateFileT(const std::wstring &path,
                         const std::vector<std::wstring> &content,
                         bool is_bom = false);

  /*
   * CreateFileT -- creates file in given path and writes content. Returns 0 on
   * success, prints error message and returns -1 otherwise.
   */
  static int CreateFileT(const std::wstring &path, const std::wstring &content,
                         bool is_bom = false);

  /*
   * ReadFile -- opens given file and reads its content. Returns 0 on success,
   * prints error message and returns -1 otherwise.
   */
  static int ReadFile(const std::wstring &path, std::wstring &content);

  /*
   * RegularFileExists -- checks that file in given path is regular. Returns
   * true on success, prints error message (if errno is different than ENOENT)
   * and returns false otherwise.
   */
  static bool RegularFileExists(const std::wstring &path);

  /*
   * GetFilePermission -- returns UNIX-style file mode bits in decimal on
   * success, prints error message and returns 0 otherwise.
   */
  static unsigned short GetFilePermission(const std::wstring &path);

  /*
   * SetFilePermission -- changes a file mode bits. Returns 0 on success, prints
   * error message and returns -1 otherwise.
  */
  static int SetFilePermission(const std::wstring &path, int permission);

  /*
   * RemoveFile -- removes specified file. Returns 0 on success, prints error
   * message and returns -1 otherwise.
   */
  static int RemoveFile(const std::wstring &path);

  /*
   * DirectoryExists -- checks that directory under given 'path' exists. Returns
   * true on success, prints error message (if errno is different than ENOENT)
   * and returns false otherwise.
   */
  static bool DirectoryExists(const std::wstring &dir);

  /*
   * GetFreeSpaceT -- returns available free space in bytes for filesystem
   * containing given path, prints error message and returns -1 otherwise.
   */
  static long long GetFreeSpaceT(const std::wstring &dir);
#endif
};

#endif  // !PMDK_TESTS_SRC_UTILS_API_C_API_C_H_
