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

#include "dimm/dimm.h"
#include "inject_manager/inject_manager.h"

int CleanUp(const std::string &testdir,
            const std::vector<std::string> &mountpoints) {
  for (const auto &m : mountpoints) {
    ApiC::CleanDirectory(m);
  }
  ApiC::CleanDirectory(testdir);
  return 0;
}

int main(int argc, char **argv) {
  std::string usage =
      "./US_REMOTE_AGENT record|inject|check-safe|check-unsafe|cleanup "
      "inject_policy{all|first|last} <TESTDIR> "
      "<MOUNTPOINT1> <MOUNTPOINT2>...";

  try {
    if (argc < 5) {
      std::cerr << usage << std::endl;
      return 1;
    }

    std::string action = argv[1];
    std::string policy = argv[2];
    std::string test_dir = argv[3];
    std::vector<std::string> mountpoint_args(argv + 4, argv + argc);

    std::vector<DimmNamespace> dimm_colls;
    for (const auto &m : mountpoint_args) {
      dimm_colls.emplace_back(DimmNamespace{m});
    }

    InjectManager inject_mgmt{test_dir, policy};

    if (std::string{"inject"}.compare(action) == 0) {
      return inject_mgmt.Inject(dimm_colls);

    } else if (std::string{"record"}.compare(action) == 0) {
      return inject_mgmt.RecordUSC(dimm_colls);

    } else if (std::string{"check-safe"}.compare(action) == 0) {
      return inject_mgmt.IsLastShutdownSafe(dimm_colls) ? 0 : 1;

    } else if (std::string{"check-unsafe"}.compare(action) == 0) {
      return inject_mgmt.IsLastShutdownUnsafe(dimm_colls) ? 0 : 1;

    } else if (std::string{"cleanup"}.compare(action) == 0) {
      return CleanUp(test_dir, mountpoint_args);

    } else {
      std::cerr << usage << std::endl;
      return 1;
    }
  } catch (const std::exception &e) {
    std::cerr << "Exception was caught: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}
