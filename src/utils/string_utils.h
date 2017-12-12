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

#ifndef PMDK_TESTS_SRC_UTILS_STRING_UTILS_H_
#define PMDK_TESTS_SRC_UTILS_STRING_UTILS_H_

#include <iostream>
#include <string>
#include <vector>

namespace string_utils {
template <typename To, typename From>
std::basic_string<To> Convert(std::basic_string<From> f) {
  return std::basic_string<To>{f.begin(), f.end()};
}

template <typename T, template <typename...> class container = std::vector>
container<std::basic_string<T>> Tokenize(std::basic_string<T> str) {
  size_t pos;
  container<std::basic_string<T>> cont;

  std::basic_string<T> separator = string_utils::Convert<T, char>("\n");

  while ((pos = str.find_first_of(separator, 0)) !=
         std::basic_string<T>::npos) {
    cont.emplace_back(str.substr(0, pos));
    str.erase(0, pos + 1);
  }

  if (!str.empty()) {
    cont.emplace_back(str);
  }

  return cont;
}

template <typename T>
bool IsSubstrFound(const std::basic_string<T> &substring,
                   const std::basic_string<T> &string) {
  return std::basic_string<T>::npos != string.find(substring);
}
}  // namespace string_utils

#endif  // !PMDK_TESTS_SRC_UTILS_STRING_UTILS_H_
