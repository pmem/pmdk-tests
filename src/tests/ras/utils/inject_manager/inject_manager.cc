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

#include "inject_manager.h"

int InjectManager::ReadRecordedUSC(const std::string &usc_file_path) const {
  std::string content;
  if (ApiC::ReadFile(usc_file_path, content) != 0) {
    return -1;
  }

  try {
    return std::stoi(content);
  } catch (const std::invalid_argument &e) {
    std::cerr << e.what() << std::endl;
    return -1;
  } catch (const std::out_of_range &e) {
    std::cerr << e.what() << std::endl;
    return -1;
  }
}

int InjectManager::RecordDimmUSC(const Dimm &dimm) const {
  int usc = dimm.GetShutdownCount();
  if (usc == -1) {
    std::cerr << "Reading USC from DIMM " << dimm.GetUid() << " failed"
              << std::endl;
    return -1;
  }

  if (ApiC::CreateFileT(test_dir_ + SEPARATOR + dimm.GetUid(),
                        std::to_string(usc)) == -1) {
    return -1;
  }
  return 0;
}

int InjectManager::RecordUSC(
    const std::vector<DimmNamespace> &dimm_namespaces) const {
  for (const auto &dc : dimm_namespaces) {
    for (const auto &d : dc) {
      if (RecordDimmUSC(d) != 0) {
        return -1;
      }
    }
  }
  return 0;
}

int InjectManager::Inject(
    const std::vector<DimmNamespace> &us_namespaces) const {
  for (const auto &dn : us_namespaces) {
    for (const auto &d : GetDimmsToInject(dn)) {
      if (d.InjectUnsafeShutdown() != 0) {
        return -1;
      }
    }
  }
  return 0;
}

const std::vector<Dimm> InjectManager::GetDimmsToInject(
    const DimmNamespace &us_namespace) const {
  std::vector<Dimm> dimms;
  switch (policy_) {
    case InjectPolicy::all:
      dimms = std::vector<Dimm>(us_namespace.begin(), us_namespace.end());
      break;
    case InjectPolicy::first:
      dimms = std::vector<Dimm>{us_namespace.begin(), us_namespace.begin() + 1};
      break;
    case InjectPolicy::last:
      dimms = std::vector<Dimm>{us_namespace.end() - 1, us_namespace.end()};
      break;
  }
  return dimms;
}

bool InjectManager::CheckUSCDiff(
    const std::vector<DimmNamespace> &dimm_namespaces,
    std::function<bool(int, int)> compare) const {
  for (const auto &dn : dimm_namespaces) {
    for (const auto &d : GetDimmsToInject(dn)) {
      int recorded_usc = ReadRecordedUSC(test_dir_ + d.GetUid());

      if (recorded_usc == -1) {
        std::cerr << "Could not read USC in DIMM: " << d.GetUid()
                  << " with test dir: " << dn.GetTestDir() << std::endl;
        return false;
      }

      if (!compare(recorded_usc, d.GetShutdownCount())) {
        std::cerr << "NVDIMM: " << d.GetUid()
                  << " (test dir: " << dn.GetTestDir()
                  << "). Current USC: " << d.GetShutdownCount()
                  << " Last recorded USC: " << recorded_usc << std::endl;
        return false;
      }
    }
  }
  return true;
}

InjectManager::InjectManager(std::string test_dir, std::string policy) {
  std::map<std::string, InjectPolicy> string_reprs = {
      {"all", InjectPolicy::all},
      {"first", InjectPolicy::first},
      {"last", InjectPolicy::last}};

  auto search = string_reprs.find(policy);
  if (search == string_reprs.end()) {
    throw std::invalid_argument(policy +
                                " is not a valid policy string representation");
  }
  policy_ = search->second;
  test_dir_ = test_dir;
}

bool InjectManager::IsLastShutdownUnsafe(
    const std::vector<DimmNamespace> &dimm_namespaces) const {
  std::function<bool(int, int)> compare = [](int rec, int cur) -> bool {
    return cur > rec;
  };

  if (!CheckUSCDiff(dimm_namespaces, compare)) {
    std::cerr << "Unexpected safe shutdown of NVDIMM" << std::endl;
    return false;
  }
  return true;
}

bool InjectManager::IsLastShutdownSafe(
    const std::vector<DimmNamespace> &dimm_namespaces) const {
  std::function<bool(int, int)> compare = [](int rec, int cur) -> bool {
    return cur == rec;
  };

  if (!CheckUSCDiff(dimm_namespaces, compare)) {
    std::cerr << "Unexpected unsafe shutdown of NVDIMM" << std::endl;
    return false;
  }
  return true;
}
