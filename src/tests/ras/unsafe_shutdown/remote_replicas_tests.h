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

#ifndef US_REMOTE_REPLICAS_TESTS_H
#define US_REMOTE_REPLICAS_TESTS_H

#include "configXML/remote_dimm_configuration.h"
#include "test_phase/remote_test_phase.h"
#include "unsafe_shutdown.h"

class RemotePoolset {
  friend class RemotePoolsetTC;

 public:
  std::string GetReplicaLine() const;
  int CreateRemotePoolsetFile() const;

 private:
  RemotePoolset(const std::string& h, const Poolset& p)
      : host_{h}, poolset_{p} {
  }
  std::string host_;
  Poolset poolset_;
};

class RemotePoolsetTC {
 public:
  RemotePoolsetTC(const std::string& d) : description_{d} {
  }
  RemotePoolset& AddRemotePoolset(const std::string& host, const Poolset& p) {
    remote_poolsets_.emplace_back(RemotePoolset{host, p});
    return remote_poolsets_.back();
  }
  Poolset poolset_;
  bool enough_dimms_ = false;
  bool is_syncable_;
  std::string description_;
  std::vector<RemotePoolset> remote_poolsets_;
};

class SyncRemoteReplica
    : public UnsafeShutdown,
      public ::testing::WithParamInterface<RemotePoolsetTC> {
 public:
  void SetUp() override;

 private:
  std::unique_ptr<RemoteDimmNode> remote_node_;
};

std::ostream& operator<<(std::ostream& stream, RemotePoolsetTC const& p);

std::vector<RemotePoolsetTC> GetPoolsetsWithRemoteReplicaParams();

#endif  // US_REMOTE_REPLICAS_TESTS_H
