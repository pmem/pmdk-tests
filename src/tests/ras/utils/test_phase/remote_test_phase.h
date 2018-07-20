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

#ifdef __linux__

#ifndef REMOTE_TEST_PHASE_H
#define REMOTE_TEST_PHASE_H

#include "configXML/remote_dimm_configuration.h"
#include "exit_codes.h"
#include "test_phase/test_phase.h"

class RemoteTestPhase : public TestPhase<RemoteTestPhase> {
  friend class TestPhase<RemoteTestPhase>;

 public:
  const std::vector<std::string> GetSafeMountpoints(
      const RemoteDimmNode& node) const;
  const std::vector<std::string> GetUnsafeMountpoints(
      const RemoteDimmNode& node) const;

  const RemoteDimmNode& GetNode(int idx) const {
    return configs_[idx];
  }

  const std::string& GetRemotePoolsetsDir() const {
    return remote_poolsets_dir_;
  }

 protected:
  int Begin() const;
  int Inject() const;
  int CheckUSC() const;
  int End() const;

 private:
  const std::string GetRemoteAgentPath(const RemoteDimmNode& node) const {
    return node.GetBinsDir() + SEPARATOR + "US_REMOTE_AGENT";
  }

  const std::string remote_poolsets_dir_ = "usc_remote_testdir/";

  RemoteDimmConfigurationsCollection configs_;
  RemoteTestPhase();
};
#endif  // REMOTE_TEST_PHASE_H

#endif  // __linux__
