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

#include "api_c.h"

int ApiC::CreateFileT(const std::string& path,
                      const std::string& content) const {
  std::ofstream file(path, std::ios::binary);

  if (!file) {
    std::cout << "File opening failed" << std::endl;
    return -1;
  }

  file << content;

  return 0;
}

int ApiC::CreateFileT(const std::string& path,
                      std::vector<std::string> content) const {
  std::string c;

  for (const auto& line : content) {
    c += line + "\n";
  }

  return CreateFileT(path, std::move(c));
}

std::string ApiC::ReadFile(const std::string& path) const {
  std::string lines;
  std::ifstream file{path, std::ios::binary | std::ios::ate};

  if (!file.good()) {
    return lines;
  }

  for (std::string line; std::getline(file, line);) {
    lines += line;
  }

  return lines;
}

bool ApiC::FileExists(const std::string& path) const {
  struct stat64 file_stat;

  return stat64(path.c_str(), &file_stat) == 0
             ? (file_stat.st_mode & S_IFREG) != 0
             : false;
}

size_t ApiC::GetFileSize(const std::string& path) const {
  struct stat64 file_stat;

  int result = stat64(path.c_str(), &file_stat);

  if (result != 0) {
    switch (errno) {
      case ENOENT:
        std::cout << "File " << path << " not found.\n";
        break;
      case EINVAL:
        std::cout << "Invalid parameter to _stat.\n";
        break;
      default:
        std::cout << "Unexpected error in _stat. Errno: \n" << errno;
    }

    return 0;
  }

  return file_stat.st_size;
}

std::vector<size_t> ApiC::GetFilesSize(
    const std::vector<std::string>& paths) const {
  std::vector<size_t> sizes;
  size_t file_size;

  for (const auto& path : paths) {
    file_size = GetFileSize(path);
    if (file_size == 0) {
      sizes.clear();
      break;
    }

    sizes.emplace_back(file_size);
  }

  return sizes;
}

int ApiC::SetFilePermission(const std::string& path, int permissions) const {
  return chmod(path.c_str(), permissions);
}

unsigned short ApiC::GetFilePermission(const std::string& path) const {
  struct stat64 file_stats;

  int retval = stat64(path.c_str(), &file_stats);

  if (retval != 0) {
    return false;
  }

  return file_stats.st_mode & PERMISSION_MASK;
}

int ApiC::RemoveFile(const std::string& path) const {
  return std::remove(path.c_str());
}

bool ApiC::DirectoryExists(const std::string& path) const {
  struct stat64 file_stat;

  return stat64(path.c_str(), &file_stat) == 0
             ? (file_stat.st_mode & S_IFDIR) != 0
             : false;
}
