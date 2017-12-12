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

#ifdef __linux__

#include "i_shell.h"

size_t IShell::GetFileSize(const std::string &path) {
  auto ret = ExecuteCommand("du -b " + path).GetExitCode();

  if (ret != 0) {
    std::cerr << output_.GetContent();
    return 0;
  }

  auto pos = output_.GetContent().find("\t");

  return std::stoul(output_.GetContent().substr(0, pos));
}

inline int IShell::CreateFileT(const std::string &path,
                               const std::string &content) {
  return ExecuteCommand("echo -e '" + content + "' > " + path).GetExitCode();
}

bool IShell::FileExists(const std::string &path) {
  if (path.empty()) {
    output_ = Output<>(1, "Path argument is empty");
    return false;
  }

  return ExecuteCommand("test -f " + path).GetExitCode() == 0;
}

bool IShell::DirectoryExists(const std::string &dir) {
  if (dir.empty()) {
    output_ = Output<>(1, "Dir argument is empty");
    return false;
  }

  return ExecuteCommand("test -d " + dir).GetExitCode() == 0;
}

int IShell::AllocateFileSpace(const std::string &path,
                              const std::string &size) {
  return ExecuteCommand("fallocate -l " + size + " " + path).GetExitCode();
}

#endif //__linux__
