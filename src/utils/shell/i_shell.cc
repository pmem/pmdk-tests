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

#include "i_shell.h"

Output<char> IShell::ExecuteCommand(const std::string& cmd) {
#ifdef _WIN32
  std::string command = "PowerShell -Command " + cmd + " 2>&1";
#else
  std::string command = "{ " + cmd + "; } 2>&1";
#endif  // _WIN32
  std::unique_ptr<FILE, PipeDeleter> pipe(popen(command.c_str(), "r"));

  if (pipe) {
    char buffer[BUFFER_SZIE];
    std::string out_buffer;

    std::fflush(pipe.get());

    while (fgets(buffer, BUFFER_SZIE, pipe.get())) {
      out_buffer.append(buffer);
    }

    auto s_pipe = pipe.release();
    int exit_code = pclose(s_pipe);
#ifdef _WIN32
    output_ = Output<char>(exit_code, out_buffer);
#else
    output_ = Output<char>(WEXITSTATUS(exit_code), out_buffer);
#endif  // _WIN32

    if (print_log_) {
      std::cout << out_buffer << std::endl;
    }

    return output_;
  }

  throw std::runtime_error("popen failed!!!");
}

template <template <typename...> class container>
container<std::string> IShell::GetFilesSizes(container<std::string> paths) {
  container<std::string> cont;

  for (const auto& path : paths) {
    auto size = GetFileSize(path);

    if (size.empty()) {
      cont.clear();
      return cont;
    }

    cont.emplace_back(size);
  }

  return cont;
}

int IShell::RemoveFile(const std::string& path) {
  return ExecuteCommand("rm " + path).GetExitCode();
}

template <template <typename...> class container>
int IShell::CreateFileT(const std::string& path,
                        container<std::string> content) {
  std::string temp_content;

  for (const auto& line : content) {
    temp_content += line + "\n";
  }

  return CreateFileT(path, temp_content);
}