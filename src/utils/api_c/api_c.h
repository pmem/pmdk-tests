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
#include <cstdio>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include "constants.h"
#include "non_copyable/non_copyable.h"

#ifdef _WIN32
#include <io.h>
#include <windows.h>
#include <codecvt>
#include <locale>

#define stat64 _stat64
#define chmod _chmod
#endif  // _WIN32

class ApiC final : NonCopyable {
 public:
  std::string GetExecutablePath();
  int CreateFileT(const std::string& path, const std::string& content) const;
  int CreateFileT(const std::string& path,
                  std::vector<std::string> content) const;
  int AllocateFileSpace(const std::string& path, size_t length) const;
  std::string ReadFile(const std::string& path) const;
  bool FileExists(const std::string& path) const;
  size_t GetFileSize(const std::string& path) const;
  std::vector<size_t> GetFilesSize(const std::vector<std::string>& paths) const;
  unsigned short GetFilePermission(const std::string& path) const;
  int SetFilePermission(const std::string& path, int permissions) const;
  int RemoveFile(const std::string& path) const;
  int CreateDirectoryT(const std::string& dir) const;
  bool DirectoryExists(const std::string& dir) const;
  int CleanDirectory(const std::string& dir) const;
  int RemoveDirectoryT(const std::string& dir) const;
  size_t GetFreespace(const std::string& dir) const;

#ifdef _WIN32
  int CreateFileT(const std::wstring& path, std::vector<std::wstring> content,
                  bool is_bom = false) const;
  int CreateFileT(const std::wstring& path, const std::wstring& content,
                  bool is_bom = false) const;
  std::wstring ReadFile(const std::wstring& path) const;
  bool FileExists(const std::wstring& path) const;
  unsigned short GetFilePermission(const std::wstring& path) const;
  int SetFilePermission(const std::wstring& path, int permissions) const;
  int RemoveFile(const std::wstring& path) const;
  bool DirectoryExists(const std::wstring& dir) const;
  size_t GetFreespace(const std::wstring& dir) const;
#endif
};

#endif  // !PMDK_TESTS_SRC_UTILS_API_C_API_C_H_
