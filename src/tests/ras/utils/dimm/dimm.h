/*
 * Copyright 2018-2023, Intel Corporation
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

#ifndef PMDK_TESTS_SRC_RAS_UTILS_DIMM_H_
#define PMDK_TESTS_SRC_RAS_UTILS_DIMM_H_

#include <ndctl/libdaxctl.h>
#include <ndctl/libndctl.h>
#include <sys/stat.h>
#include "api_c/api_c.h"

class Dimm final {
 private:
  struct ndctl_dimm *dimm_ = nullptr;
  std::string uid_;

 public:
  Dimm(struct ndctl_dimm *dimm, const char *uid) : dimm_(dimm), uid_(uid) {
  }

  int GetShutdownCount() const;
  int InjectUnsafeShutdown() const;

  const std::string &GetUid() const {
    return this->uid_;
  }
};

class DimmNamespace final {
 private:
  bool is_dax_ = false;
  std::string test_dir_;
  std::vector<Dimm> dimms_;
  ndctl_ctx *ctx_ = nullptr;

  ndctl_interleave_set *GetInterleaveSet(ndctl_ctx *ctx,
                                         const struct stat64 &st);

 public:
  DimmNamespace(const std::string &mountpoint);

  std::string GetTestDir() const {
    return this->test_dir_;
  }

  Dimm &operator[](std::size_t idx) {
    return this->dimms_.at(idx);
  }

  const std::vector<Dimm>::const_iterator begin() const noexcept {
    return dimms_.cbegin();
  }

  const std::vector<Dimm>::const_iterator end() const noexcept {
    return dimms_.cend();
  }

  size_t GetSize() const {
    return dimms_.size();
  }

  ~DimmNamespace();
};

#endif  // !PMDK_TESTS_SRC_RAS_UTILS_DIMM_H_
