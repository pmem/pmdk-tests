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

#ifdef _WIN32

#include "i_shell.h"

bool IShell::TestPath(const std::string& path, const std::string& args) {
  if (path.empty()) {
    output_ = Output<>(1, "Path argument is empty");
    return false;
  }

  ExecuteCommand("Test-Path " + path + args);

  std::size_t found = output_.GetContent().find("False");
  if (found != std::string::npos) {
    output_ = Output<>(1, "false");
  }

  return output_.GetExitCode() == 0;
}

Output<wchar_t> IShell::ExecuteCommand(const std::wstring& cmd) {
  std::wstring command = L"PowerShell -Command " + cmd + L" 2>&1";

  std::unique_ptr<FILE, PipeDeleter> pipe(_wpopen(command.c_str(), L"r"));

  if (pipe) {
    wchar_t buffer[BUFFER_SIZE];
    std::wstring out_buffer;

    std::fflush(pipe.get());
    while (fgetws(buffer, BUFFER_SIZE, pipe.get())) {
      out_buffer.append(buffer);
    }

    auto s_pipe = pipe.release();
    int exit_code = pclose(s_pipe);
    w_output_ = Output<wchar_t>(exit_code, out_buffer);

    if (print_log_) {
      std::wcout << out_buffer << std::endl;
    }

    return w_output_;
  }

  throw std::exception("popen failed!!!");
}

std::string IShell::GetFileSize(const std::string& path) {
  auto ret =
      ExecuteCommand("\"Get-Childitem -file " + path + " | select length\"")
          .GetExitCode();

  if (ret != 0) {
    std::cout << output_.GetContent();
    return std::string();
  }

  auto lines = string_utils::Tokenize(output_.GetContent());

  return lines[3];
}

std::string ConvertNewLines(std::string content) {
  auto new_line = "\n";
  auto new_line_windows = "`r`n";
  auto space = " ";
  auto space_windows = "` ";

  size_t found = content.find(new_line);

  while (found != std::string::npos) {
    content.replace(found, 1, new_line_windows);
    found = content.find(new_line, found + 1);
  }

  found = content.find(space);

  while (found != std::string::npos) {
    content.replace(found, 1, space_windows);
    found = content.find(space, found + 2);
  }

  return content;
}

inline int IShell::CreateFileT(const std::string& path,
                               const std::string& content) {
  return ExecuteCommand("New-Item " + path + " -type file -value \"" +
                        ConvertNewLines(content) + "\"")
      .GetExitCode();
}

bool IShell::FileExists(const std::string& path) {
  return TestPath(path, " -PathType leaf");
}

bool IShell::DirectoryExists(const std::string& dir) {
  return TestPath(dir, " -PathType container");
}

int IShell::AllocateFileSpace(const std::string& path,
                              const std::string& size) {
  return ExecuteCommand("fsutil file createnew " + path + " " + size)
      .GetExitCode();
}

#endif
