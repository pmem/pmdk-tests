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

#ifndef PMDK_TESTS_SRC_UTILS_POOLSET_REPLICA_H_
#define PMDK_TESTS_SRC_UTILS_POOLSET_REPLICA_H_

#include <vector>
#include "part.h"

/*
 * Replica -- class that represents replica section of pool set file.
 */
class Replica final {
 private:
  std::string header_;
  std::vector<Part> parts_;
  int count_;
  int part_count_;

  std::string CreatePartName() const;
  bool IsMasterReplica() const {
    return this->header_.compare("PMEMPOOLSET") == 0;
  };
  std::vector<std::string> Split(const std::string &str) const;

 public:
  Replica(std::vector<std::string> content, const std::string &path, int count);
  const std::string &GetHeader() const {
    return this->header_;
  };
  const std::vector<Part> &GetParts() const {
    return this->parts_;
  };
  const Part &GetPart(unsigned index) const {
    return parts_.at(index);
  }
};

#endif  // !PMDK_TESTS_SRC_UTILS_POOLSET_REPLICA_H_
