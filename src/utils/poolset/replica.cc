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

#include "replica.h"
#include <sstream>

Replica::Replica(std::vector<std::string> content, const std::string &path,
                 int count)
    : count_(count) {
  this->header_ = content.front();
  content.erase(content.begin());

  const int ONLY_SIZE_GIVEN = 1;
  part_count_ = 0;

  for (const std::string &line : content) {
    std::vector<std::string> vec = Split(line);

    std::string part_size = vec[0];
    std::string part_path =
        vec.size() == ONLY_SIZE_GIVEN ? path + CreatePartName() : vec[1];

    this->parts_.emplace_back(part_size, part_path);
    ++part_count_;
  }
}

std::vector<std::string> Replica::Split(const std::string &str) const {
  const char DELIMITER = ' ';
  std::vector<std::string> vec;

  std::istringstream stream(str);
  std::string token;
  while (std::getline(stream, token, DELIMITER)) {
    vec.push_back(token);
  }
  return vec;
}

std::string Replica::CreatePartName() const {
  std::string replica_name =
      IsMasterReplica() ? "pool" : "replica" + std::to_string(this->count_);
  std::string part_name = "part" + std::to_string(this->part_count_);
  return std::string(replica_name + "." + part_name);
}
