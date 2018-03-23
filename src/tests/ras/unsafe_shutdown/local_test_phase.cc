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
#include "local_test_phase.h"

LocalTestPhase::LocalTestPhase() {
  if (local_dimm_config_.ReadConfigFile() != 0) {
    throw std::invalid_argument(
        "Reading config file for local DIMM configuration failed");
  }

  if (local_dimm_config_.GetSize() > 0) {
    unsafe_dimm_colls_.emplace_back(local_dimm_config_[0]);
  }

  if (local_dimm_config_.GetSize() > 1) {
    safe_dimm_colls_.emplace_back(local_dimm_config_[1]);
  }

  for (int i = 2; i < local_dimm_config_.GetSize(); ++i) {
    unsafe_dimm_colls_.emplace_back(local_dimm_config_[i]);
  }
}

int LocalTestPhase::Begin() const {
  for (const auto &dimm_collection : local_dimm_config_) {
    if (ApiC::CreateDirectoryT(dimm_collection.GetMountpoint()) != 0) {
      return -1;
    }
  }
  return 0;
}

int LocalTestPhase::Inject() const {
  InjectManager inject_mgmt{local_dimm_config_.GetTestDir(), policy_};
  if (inject_mgmt.RecordUSCAll(std::vector<DimmCollection>{
          local_dimm_config_.begin(), local_dimm_config_.end()}) != 0) {
    return -1;
  }

  if (inject_mgmt.Inject(unsafe_dimm_colls_)) {
    return -1;
  }

  return 0;
}

int LocalTestPhase::CheckUSC() const {
  InjectManager inject_mgmt{local_dimm_config_.GetTestDir(), policy_};
  if (inject_mgmt.CheckUSCDiff(safe_dimm_colls_, 0) &&
      inject_mgmt.CheckUSCDiff(unsafe_dimm_colls_, 1)) {
    return 0;
  }
  return -1;
}

int LocalTestPhase::End() const {
  ApiC::CleanDirectory(local_dimm_config_.GetTestDir());
  ApiC::RemoveDirectoryT(local_dimm_config_.GetTestDir());

  for (const auto &dimm_collection : local_dimm_config_) {
    ApiC::CleanDirectory(dimm_collection.GetMountpoint());
    ApiC::RemoveDirectoryT(dimm_collection.GetMountpoint());
  }
  return 0;
}
