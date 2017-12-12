/*
 * Copyright (c) 2017, Intel Corporation
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in
 * the documentation and/or other materials provided with the
 * distribution.
 *
 * * Neither the name of Intel Corporation nor the names of its
 * contributors may be used to endorse or promote products derived
 * from this software without specific prior written permission.
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
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY LOG OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifdef __linux__

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <libgen.h>
#include <string.h>
#include <sys/statvfs.h>
#include <unistd.h>
#include "api_c.h"

int ApiC::AllocateFileSpace(const std::string& path, size_t length) const {
  int f_d = open(path.c_str(), O_CREAT | O_RDWR);

  if (f_d == -1) {
    std::cout << "Cannot create file\nERRNO: " << errno << std::endl;
    return -1;
  }

  int ret = posix_fallocate(f_d, 0, static_cast<off_t>(length));

  if (ret != 0) {
    std::cout << "Cannot allocate disk space\nerrno returned: " << errno
              << std::endl;
    close(f_d);
    RemoveFile(path);
  }

  return ret;
}

std::string ApiC::GetExecutablePath() {
  char file_path[FILENAME_MAX + 1] = {0};
  ssize_t count = readlink("/proc/self/exe", file_path, FILENAME_MAX);

  if (count == -1) {
    return std::string();
  }

  file_path[count] = '\0';

  return std::string(dirname(file_path)) + "/" + config_;
}

size_t ApiC::GetFreespace(const std::string& dir) const {
  struct statvfs fs;
  if (0 != statvfs(dir.c_str(), &fs)) {
    std::cout << "Unable to get file system statistics";
    return 0;
  }

  return fs.f_bsize * fs.f_bfree;
}

int ApiC::CreateDirectoryT(const std::string& dir) const {
  if (-1 == mkdir(dir.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH)) {
    std::cout << "Received error while creating directory " << errno
              << std::endl;
    return -1;
  }

  return 0;
}

int ApiC::CleanDirectory(const std::string& dir) const {
  struct dirent* next_file;
  DIR* directory = nullptr;
  char path[256] = {0};
  int ret = 0;

  if ((directory = opendir(dir.c_str())) != nullptr) {
    while ((next_file = readdir(directory))) {
      if (strcmp(next_file->d_name, ".") && strcmp(next_file->d_name, "..")) {
        // build the full path for each file in the folder
        snprintf(path, sizeof(path), "%s%s", dir.c_str(), next_file->d_name);

        SetFilePermission(dir, 0777);

        if (DirectoryExists(path)) {
          if (rmdir(path) != 0) {
            std::cout << "Directory " << path
                      << " removing failed\nErrno received: " << errno
                      << std::endl;
            errno = 0;
            ret = -1;
          }
        } else {
          if (remove(path) != 0) {
            std::cout << "File " << path
                      << " removing failed\nErrno received: " << errno
                      << std::endl;
            errno = 0;
            ret = -1;
          }
        }
      }
    }
    if (closedir(directory) != 0) {
      std::cout << "Closing dir failed" << std::endl;
      errno = 0;
      return -1;
    }
  } else {
    std::cout << "Unable to open directory" << std::endl;
    return -1;
  }

  return ret;
}

int ApiC::RemoveDirectoryT(const std::string& dir) const {
  return rmdir(dir.c_str());
}

#endif  // __linux__