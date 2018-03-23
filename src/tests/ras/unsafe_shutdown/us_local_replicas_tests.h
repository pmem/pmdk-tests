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

#ifndef US_LOCAL_REPLICAS_TESTS_H
#define US_LOCAL_REPLICAS_TESTS_H

#include "unsafe_shutdown.h"

struct poolset_tc {
  std::string description;
  Poolset poolset;
  std::vector<DimmCollection> us_dimms;
  bool enough_dimms;
  bool is_syncable;
};

class SyncLocalReplica : public UnsafeShutdown,
                         public ::testing::WithParamInterface<poolset_tc> {
 protected:
  void SetUp() override;
};

std::ostream& operator<<(std::ostream& stream, poolset_tc const& p);

std::vector<poolset_tc> GetPoolsetLocalReplicaParams();

class UnsafeShutdownTransform : public UnsafeShutdown {
 protected:
  Poolset origin_;
  Poolset added_;
  Poolset final_;

  void SetUp() override;
};

#endif  // US_LOCAL_REPLICAS_TESTS_H
