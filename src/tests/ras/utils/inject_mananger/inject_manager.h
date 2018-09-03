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

#ifndef INJECT_MANAGER_H
#define INJECT_MANAGER_H

#include <functional>
#include "dimm/dimm.h"

/* Select whether the unsafe shutdown error is injected into the first, last or
 * each dimm from specific dimm namespace */
enum class InjectPolicy { all, first, last };

class InjectManager {
 public:
  InjectManager(std::string test_dir, InjectPolicy policy)
      : test_dir_(test_dir), policy_(policy) {
  }

  InjectManager(std::string test_dir, std::string policy);

  bool IsLastShutdownUnsafe(
      const std::vector<DimmNamespace> &dimm_namespaces) const;
  bool IsLastShutdownSafe(
      const std::vector<DimmNamespace> &dimm_namespaces) const;
  int RecordUSC(const std::vector<DimmNamespace> &dimm_namespaces) const;
  int Inject(const std::vector<DimmNamespace> &us_namespaces) const;

 private:
  std::string test_dir_;
  InjectPolicy policy_;
  bool CheckUSCDiff(const std::vector<DimmNamespace> &dimm_namespaces,
                    std::function<bool(int, int)> compare) const;

  int ReadRecordedUSC(const std::string &usc_file_path) const;

#ifdef __linux__
  int RecordDimmUSC(const Dimm &dimm) const;
  const std::vector<Dimm> GetDimmsToInject(
      const DimmNamespace &us_dimm_coll) const;
#endif
};

#endif  // INJECT_MANAGER_H
