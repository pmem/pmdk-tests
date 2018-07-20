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
  if (config_.ReadConfigFile() != 0) {
    throw std::invalid_argument(
        "Reading config file for local DIMM configuration failed");
  }

  /* Arbitrarily divide available NVDIMM namespaces between unsafe (to be
   * injected) and safe namespaces. This distribution is meant to enable running
   * the biggest number of tests with limited number of available NVDIMMS. */

  int i = 0;
  std::copy_if(config_.begin(), config_.end(),
               std::back_inserter(unsafe_namespaces),
               [&i, this](DimmNamespace) -> bool { return i++ != 1; });

  i = 0;
  std::copy_if(config_.begin(), config_.end(),
               std::back_inserter(safe_namespaces),
               [&i, this](DimmNamespace) -> bool { return i++ == 1; });
}

int LocalTestPhase::Begin() const {
  return 0;
}

int LocalTestPhase::Inject() const {
  InjectManager inject_mgmt{config_.GetTestDir(), policy_};
  if (inject_mgmt.RecordUSC(
          std::vector<DimmNamespace>{config_.begin(), config_.end()}) != 0) {
    return -1;
  }

  if (inject_mgmt.Inject(unsafe_namespaces)) {
    return -1;
  }

  return 0;
}

int LocalTestPhase::CheckUSC() const {
  InjectManager inject_mgmt{config_.GetTestDir(), policy_};
  if (inject_mgmt.IsLastShutdownSafe(safe_namespaces) &&
      inject_mgmt.IsLastShutdownUnsafe(unsafe_namespaces)) {
    return 0;
  }
  return -1;
}

int LocalTestPhase::End() const {
  ApiC::CleanDirectory(config_.GetTestDir());
  ApiC::RemoveDirectoryT(config_.GetTestDir());

  for (const auto &dimm_namespace : config_) {
    ApiC::CleanDirectory(dimm_namespace.GetTestDir());
    ApiC::RemoveDirectoryT(dimm_namespace.GetTestDir());
  }
  return 0;
}
