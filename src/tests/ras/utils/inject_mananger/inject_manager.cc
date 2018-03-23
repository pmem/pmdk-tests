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

#include "inject_manager.h"

int InjectManager::ReadRecordedUSC(std::string usc_file_path) {
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

int InjectManager::RecordDimmUSC(Dimm dimm) {
  int usc = dimm.GetShutdownCount();
  if (usc == -1) {
    std::cerr << "Reading USC from dimm " << dimm.GetUid() << " failed"
              << std::endl;
    return -1;
  }

  if (ApiC::CreateFileT(test_dir_ + SEPARATOR + dimm.GetUid(),
                        std::to_string(usc)) == -1) {
    return -1;
  }
  return 0;
}

int InjectManager::RecordUSCAll(const std::vector<DimmCollection> &dimm_colls) {
  for (const auto &dc : dimm_colls) {
    for (const auto &d : dc) {
      if (RecordDimmUSC(d) != 0) {
        return -1;
      }
    }
  }
  return 0;
}

int InjectManager::Inject(const std::vector<DimmCollection> &us_dimm_colls) {
  for (const auto &dc : us_dimm_colls) {
    for (const auto &d : DimmsToInject(dc)) {
      if (d.InjectUnsafeShutdown() != 0) {
        return -1;
      }
    }
  }
  return 0;
}

std::vector<Dimm> InjectManager::DimmsToInject(
    const DimmCollection &us_dimm_coll) {
  switch (policy_) {
    case InjectPolicy::all:
      return std::vector<Dimm>(us_dimm_coll.begin(), us_dimm_coll.end());
    case InjectPolicy::first:
      return std::vector<Dimm>{us_dimm_coll.begin(), us_dimm_coll.begin() + 1};
    case InjectPolicy::last:
      return std::vector<Dimm>{us_dimm_coll.end() - 1, us_dimm_coll.end()};
    default:
      throw std::invalid_argument("Unknown injection strategy type");
  }
}

bool InjectManager::UnsafelyShutdown(
    const std::vector<DimmCollection> &dimm_colls) {
  for (const auto &dc : dimm_colls) {
    for (const auto &d : DimmsToInject(dc)) {
      int recorded_usc = ReadRecordedUSC(test_dir_ + SEPARATOR + d.GetUid());
      if (recorded_usc == -1) {
        std::cerr << "Could not read USC in dimm: " << d.GetUid()
                  << " on mountpoint: " << dc.GetMountpoint() << std::endl;
        return false;
      }
      if (recorded_usc >= d.GetShutdownCount()) {
        std::cerr << "Value of USC (" << d.GetShutdownCount() << ") on dimm "
                  << d.GetUid() << " on mounptoint: " << dc.GetMountpoint()
                  << "is not greater than recorded before shutdown ("
                  << recorded_usc << ")" << std::endl;
        return false;
      }
    }
  }
  return true;
}

bool InjectManager::SafelyShutdown(
    const std::vector<DimmCollection> &dimm_colls) {
  bool ret = true;
  for (const auto &dc : dimm_colls) {
    for (const auto &d : dc) {
      int recorded_usc = ReadRecordedUSC(test_dir_ + SEPARATOR + d.GetUid());
      if (recorded_usc == -1) {
        std::cerr << "Could not read USC in dimm: " << d.GetUid()
                  << " on mountpoint: " << dc.GetMountpoint() << std::endl;
        ret = false;
      }
      if (recorded_usc != d.GetShutdownCount()) {
        std::cerr << "Value of USC (" << d.GetShutdownCount() << ") on dimm "
                  << d.GetUid() << " on mounptoint: " << dc.GetMountpoint()
                  << "does not equal value recorded before shutdown ("
                  << recorded_usc << ")" << std::endl;
        ret = false;
      }
    }
  }
  return ret;
}
