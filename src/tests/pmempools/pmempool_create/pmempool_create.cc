/*
 * Copyright (c) 2017, Intel Corporation
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in
 * the documentation and/or other materials provided with the
 * distribution.
 *
 * * Neither the name of Intel Corporation nor the names of its
 * contributors may be used to endorse or promote products derived
 * from this software without specific prior written permission.
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
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY LOG OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "pmempool_create.h"
#include <functional>

int PmempoolCreate::ValidateFile(const Poolset& poolset, int poolset_mode) {
  if (!p_mgmt_.AllFilesExist(poolset)) {
    err_msg_ = "Parts in the pool are missing";
    return -1;
  }

  size_t size = 0;
  for (const auto& part : poolset.GetParts()) {
    size = api_c_.GetFileSize(part.GetPath());
    if (SIZES_MiB[part.GetSize()] != size) {
      err_msg_ = "Size of parts are different than expected";
      return -1;
    }
  }

  int mode = 0;
  for (const auto& part : poolset.GetParts()) {
    mode = api_c_.GetFilePermission(part.GetPath());
    if (poolset_mode != mode) {
      err_msg_ = "Size of parts are different than expected";
      return -1;
    }
  }

  return 0;
}

int PmempoolCreate::ValidateFile(const Poolset& poolset) {
  if (!p_mgmt_.NoFilesExist(poolset)) {
    err_msg_ = "Parts in the pool exists but should not";
    return -1;
  }

  return 0;
}

int PmempoolCreate::ValidateFile(const std::string& path, size_t file_size,
                                 int file_mode) {
  if (!api_c_.FileExists(path)) {
    err_msg_ = "File is missing";
    return -1;
  }
  if (file_size != api_c_.GetFileSize(path)) {
    err_msg_ = "Size of file is different than expected";
    return -1;
  }
  if (file_mode != api_c_.GetFilePermission(path)) {
    err_msg_ = "Permission of file is different than expected";
    return -1;
  }

  return 0;
}

int PmempoolCreate::ValidateFile(const std::string& path) {
  if (api_c_.FileExists(path)) {
    err_msg_ = "File exists but should not";
    return -1;
  }

  return 0;
}

int PmempoolCreate::GetModeInt(const std::string& mode) const {
  int mode_ = static_cast<int>(std::strtoul(mode.c_str(), nullptr, 8));

#ifdef _WIN32
  /* On Windows there is no perrmition for execution. Also windows doesn't
  * support groups of users in UNIX way */
  mode_ &= 0600;
#else
  mode_ &= 0777;
#endif  // _WIN32

  return mode_;
}

void PmempoolCreate::TearDown() {
  api_c_.CleanDirectory(local_config->GetWorkingDir());
}
