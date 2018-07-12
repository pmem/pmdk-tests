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

#ifdef __linux__

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <fts.h>
#include <ftw.h>
#include <libgen.h>
#include <sys/statvfs.h>
#include <unistd.h>
#include <cstring>
#include "api_c.h"

int ApiC::AllocateFileSpace(const std::string &path, size_t length) {
  if (static_cast<off_t>(length) < 0) {
    std::cerr << "length should be >= 0" << std::endl;
    return -1;
  }

  int fd = open(path.c_str(), O_CREAT | O_RDWR);

  if (fd == -1) {
    std::cerr << "Unable to create file: " << strerror(errno) << std::endl;
    return -1;
  }

  int ret = posix_fallocate(fd, 0, static_cast<off_t>(length));

  if (ret != 0) {
    std::cerr << "Unable to allocate disk space: " << strerror(ret)
              << std::endl;
    close(fd);
    RemoveFile(path);
  }

  return ret;
}

int ApiC::GetExecutableDirectory(std::string &path) {
  char file_path[FILENAME_MAX + 1] = {0};
  ssize_t count = readlink("/proc/self/exe", file_path, FILENAME_MAX);

  if (count == -1) {
    return -1;
  }

  file_path[count] = '\0';
  path = std::string(dirname(file_path)) + "/";

  return 0;
}

long long ApiC::GetFreeSpaceT(const std::string &path) {
  struct statvfs fs;
  if (statvfs(path.c_str(), &fs) != 0) {
    std::cerr << "Unable to get file system statistics: " << strerror(errno)
              << std::endl;
    return -1;
  }

  return fs.f_bsize * fs.f_bavail;
}

int ApiC::CreateDirectoryT(const std::string &path) {
  if (mkdir(path.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) == -1) {
    std::cerr << "mkdir failed: " << strerror(errno) << std::endl;
    return -1;
  }

  return 0;
}

int ApiC::CleanDirectory(const std::string &dir) {
  auto RemFile = [](const char *fpath, const struct stat *, int,
                    struct FTW *ftwbuf) {
    if (ftwbuf->level)
      return remove(fpath);
    return 0;
  };

  // Iterate over directories structure and remove from the bottom
  int ret = nftw(dir.c_str(), RemFile, 64, FTW_DEPTH | FTW_PHYS);

  if (ret != 0) {
    std::cerr << "Unable to clean directory: " << strerror(errno) << std::endl;
    return ret;
  }

  return 0;
}

#include <ftw.h>
int ApiC::RemoveDirectoryT(const std::string &path) {
  auto RemFile = [](const char *fpath, const struct stat *, int, struct FTW *) {
    int rv = remove(fpath);
    if (rv)
      perror(fpath);
    return rv;
  };

  // Iterate over directories structure and remove from the bottom
  int ret = nftw(path.c_str(), RemFile, 64, FTW_DEPTH | FTW_PHYS);

  if (ret != 0) {
    std::cerr << "Unable to remove directory: " << strerror(errno) << std::endl;
    return ret;
  }

  return 0;
}

int ApiC::SetEnv(const std::string &name, const std::string &value) {
  return setenv(name.c_str(), value.c_str(), 1);
}

int ApiC::UnsetEnv(const std::string &name) {
  return unsetenv(name.c_str());
}

void ApiC::CountFilesWithAction(const std::string &path,
                                std::function<void(const std::string &)> func,
                                int *file_count) {
  if (auto dir = opendir(path.c_str())) {
    while (auto f = readdir(dir)) {
      if (f->d_name[0] == '.')
        continue;
      if (f->d_type == DT_DIR) {
        CountFilesWithAction(path + f->d_name + "/", func, file_count);
      }
      if (f->d_type == DT_REG) {
        func(path + f->d_name);
        ++(*file_count);
      }
    }
    closedir(dir);
  }
}

#endif  // __linux__
