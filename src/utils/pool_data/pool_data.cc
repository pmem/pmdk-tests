/*
 * Copyright 2018, Intel Corporation
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

#include "pool_data.h"

int LogData::Write(std::string log_text) {
  size_t chunks = log_text.size() / chunk_size_;

  int pos = 0;
  for (size_t i = 0; i < chunks; ++i) {
    if (pmemlog_append(plp_, log_text.substr(pos, chunk_size_).c_str(),
                       chunk_size_) != 0) {
      std::cerr << "Appending line to log pool failed. Errno: " << errno
                << std::endl;
      return -1;
    }
    pos += chunk_size_;
  }

  int last_chunk_size =
      (chunks == 0) ? log_text.size() : log_text.size() % chunks;

  if (pmemlog_append(plp_, log_text.substr(pos).c_str(), last_chunk_size) !=
      0) {
    std::cerr << "Appending line to log pool failed. Errno :" << errno
              << std::endl;
    return -1;
  }
  return 0;
}

std::string LogData::Read() {
  std::string ret;
  pmemlog_walk(plp_, 0, ReadLog, &ret);
  return ret;
}

int LogData::ReadLog(const void *buf, size_t len, void *arg) {
  static_cast<std::string *>(arg)->assign(static_cast<const char *>(buf), len);
  return 0;
}
