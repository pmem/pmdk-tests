/*
 * Copyright 2017-2023, Intel Corporation
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

#include <cstring>

#include "api_c.h"

#include <errno.h>
#include <fcntl.h>
#include <fstream>
#include <fts.h>
#include <libgen.h>
#include <sstream>
#include <sys/statvfs.h>
#include <unistd.h>
#include <cstring>

int ApiC::CreateFileT(const std::string &path, const std::string &content) {
  std::ofstream file{path, std::ios::binary};

  if (!file) {
    std::cerr << "File creating failed" << std::endl;
    return -1;
  }

  file << content;

  return 0;
}

int ApiC::CreateFileT(const std::string &path,
                      const std::vector<std::string> &content) {
  std::string c;

  for (const auto &line : content) {
    c += line + "\n";
  }

  return CreateFileT(path, std::move(c));
}

int ApiC::ReadFile(const std::string &path, std::string &content) {
  std::ifstream file{path};

  if (!file.good()) {
    std::cerr << "File opening failed" << std::endl;
    return -1;
  }

  std::stringstream buffer;
  buffer << file.rdbuf();
  content = buffer.str();

  return 0;
}

bool ApiC::RegularFileExists(const std::string &path) {
  struct stat64 file_stat;

  if (stat64(path.c_str(), &file_stat) != 0) {
    switch (errno) {
      case ENOENT:
        break;
      default:
        std::cerr << "Unexpected error in stat64. Errno: " << strerror(errno)
                  << std::endl;
    }

    return false;
  }

  return (file_stat.st_mode & S_IFREG) != 0;
}

long long ApiC::GetFileSize(const std::string &path) {
  struct stat64 file_stat;

  if (stat64(path.c_str(), &file_stat) != 0) {
    std::cerr << "Unable to get file size: " << strerror(errno) << std::endl;

    return -1;
  }

  return file_stat.st_size;
}

std::vector<long long> ApiC::GetFilesSize(
    const std::vector<std::string> &paths) {
  std::vector<long long> sizes;
  long long file_size;

  for (const auto &path : paths) {
    file_size = GetFileSize(path);
    if (file_size == -1) {
      sizes.clear();
      break;
    }

    sizes.push_back(file_size);
  }

  return sizes;
}

int ApiC::SetFilePermission(const std::string &path, int permission) {
  int ret = chmod(path.c_str(), permission);

  if (ret != 0) {
    std::cerr << "Unable to change file permission: " << strerror(errno)
              << std::endl;
  }

  return ret;
}

unsigned short ApiC::GetFilePermission(const std::string &path) {
  struct stat64 file_stats;

  if (stat64(path.c_str(), &file_stats) != 0) {
    std::cerr << "Unable to get file permission: " << strerror(errno)
              << std::endl;

    return 0;
  }

  return file_stats.st_mode & PERMISSION_MASK;
}

int ApiC::RemoveFile(const std::string &path) {
  return std::remove(path.c_str());
}

bool ApiC::DirectoryExists(const std::string &path) {
  struct stat64 file_stats;

  if (stat64(path.c_str(), &file_stats) != 0) {
    switch (errno) {
      case ENOENT:
        break;
      default:
        std::cerr << "Unexpected error in stat64. Errno: " << strerror(errno)
                  << std::endl;
    }

    return false;
  }

  return (file_stats.st_mode & S_IFDIR) != 0;
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
  FTS *fts;
  FTSENT *f_sent;
  int options = FTS_COMFOLLOW | FTS_LOGICAL | FTS_NOCHDIR;
  int ret = 0;
  char *d[] = {(char *)dir.c_str(), nullptr};

  fts = fts_open(d, options, nullptr);

  if (fts == nullptr) {
    std::cerr << "fts_open failed: " << strerror(errno) << std::endl;
    return -1;
  }

  f_sent = fts_children(fts, 0);

  if (f_sent == nullptr) {
    if (fts_close(fts) != 0) {
      std::cerr << "fts_close failed: " << strerror(errno) << std::endl;
      return -1;
    }
    return 0;
  }

  while ((f_sent = fts_read(fts)) != nullptr) {
    switch (f_sent->fts_info) {
      case FTS_D:
        if (f_sent->fts_path != dir &&
            RemoveDirectoryT(f_sent->fts_path) != 0) {
          std::cerr << "Unable to remove directory " << f_sent->fts_path << ": "
                    << strerror(errno) << std::endl;
          ret = -1;
        }
        break;
      case FTS_F:
        if (RemoveFile(f_sent->fts_path) != 0) {
          std::cerr << "Unable to remove file: " << f_sent->fts_path
                    << std::endl;
          ret = -1;
        }
        break;
      default:
        break;
    }
  }

  if (fts_close(fts) != 0) {
    std::cerr << "fts_close failed: " << strerror(errno) << std::endl;
    ret = -1;
  }

  return ret;
}

int ApiC::RemoveDirectoryT(const std::string &path) {
  int ret = rmdir(path.c_str());

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
