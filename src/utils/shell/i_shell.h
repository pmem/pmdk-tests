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

#ifndef PMDK_TESTS_SRC_UTILS_SHELL_I_SHELL_H_
#define PMDK_TESTS_SRC_UTILS_SHELL_I_SHELL_H_

#include <stdio.h>
#include <cstdio>
#include <exception>
#include <memory>
#include <string>
#include "non_copyable/non_copyable.h"
#include "output/output.h"
#include "string_utils.h"

const int BUFFER_SIZE = 128;

#ifdef _WIN32
#define popen _popen
#define pclose _pclose
#endif  // _WIN32

struct PipeDeleter {
  void operator()(FILE *pipe) {
    if (pipe) {
      pclose(pipe);
    }
  }
};

/*
 * IShell -- class serving as an OS-independent abstraction for command line
 * operations.
 */
class IShell : NonCopyable {
 private:
  Output<char> output_;
  bool print_log_ = false;

#ifndef _WIN32
  std::string address_;
#endif

#ifdef _WIN32
  Output<wchar_t> w_output_;
#endif

 public:
  IShell(){};
  IShell(bool print_log) : print_log_(print_log){};
#ifndef _WIN32
  IShell(const std::string &address) : address_(address){};
#endif
  /*
   * GetLastOutput -- returns last received output.
   */
  Output<char> GetLastOutput() const {
    return output_;
  }
  /*
   * ExecuteCommand -- performs command by creating pipe with read mode. Returns
   * Output object on success, throws std::exception otherwise.
   */
  Output<char> ExecuteCommand(const std::string &cmd);

#ifdef _WIN32
  /*
   * GetLastOutput -- returns last received output.
   */
  Output<wchar_t> GetWLastOutput() const {
    return w_output_;
  }

  /*
   * ExecuteCommand -- performs command by creating pipe with read mode. Returns
   * Output object on success, throws std::exception otherwise.
   */
  Output<wchar_t> ExecuteCommand(const std::wstring &cmd);
#endif  // _WIN32
};

#endif  // !PMDK_TESTS_SRC_UTILS_SHELL_I_SHELL_H_
