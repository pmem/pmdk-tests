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
#ifndef LOCAL_TEST_PHASE_H
#define LOCAL_TEST_PHASE_H

#include "exit_codes.h"
#include "test_phase/test_phase.h"

class LocalTestPhase : public TestPhase<LocalTestPhase> {
  friend class TestPhase<LocalTestPhase>;

 public:
  const std::vector<DimmCollection> &GetSafeDimmNamespaces() {
    return this->safe_dimm_colls_;
  }
  const std::vector<DimmCollection> &GetUnsafeDimmNamespaces() {
    return this->unsafe_dimm_colls_;
  }

  const std::string &GetTestDir() {
    return this->local_dimm_config_.GetTestDir();
  }

 protected:
  int Begin();
  int Inject();
  int CheckUSC();
  int End();

 private:
  LocalDimmConfiguration local_dimm_config_;
  std::vector<DimmCollection> safe_dimm_colls_;
  std::vector<DimmCollection> unsafe_dimm_colls_;
  LocalTestPhase();
};
#endif  // LOCAL_TEST_PHASE_H
