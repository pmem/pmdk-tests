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

#include "dimm.h"
#include <linux/limits.h>
#include <cstring>

const int USC_VALID_FLAG = 1 << 5;

int Dimm::GetShutdownCount() const {
  struct ndctl_cmd *cmd = ndctl_dimm_cmd_new_smart(dimm_);

  if (ndctl_cmd_submit(cmd)) {
    return -1;
  }

  if (!(ndctl_cmd_smart_get_flags(cmd) & USC_VALID_FLAG)) {
    return -1;
  }

  return ndctl_cmd_smart_get_shutdown_count(cmd);
}

int Dimm::InjectUnsafeShutdown() const {
  struct ndctl_cmd *cmd = ndctl_dimm_cmd_new_ack_shutdown_count(dimm_);

  if (ndctl_cmd_submit(cmd)) {
    return -1;
  }

  if (ndctl_cmd_get_firmware_status(cmd)) {
    std::cerr << "DIMM: " << ndctl_dimm_get_devname(dimm_)
              << " Latch System Shutdown setting failed" << std::endl;
    return -1;
  }

  cmd = ndctl_dimm_cmd_new_smart_inject(dimm_);

  if (ndctl_cmd_smart_inject_unsafe_shutdown(cmd, true)) {
    return -1;
  }

  if (ndctl_cmd_submit(cmd)) {
    return -1;
  }

  return 0;
}

ndctl_interleave_set *DimmNamespace::GetInterleaveSet(ndctl_ctx *ctx,
                                                      struct stat64 st) {
  struct ndctl_bus *bus;
  struct ndctl_region *region;
  struct ndctl_namespace *ndns;
  dev_t dev = S_ISCHR(st.st_mode) ? st.st_rdev : st.st_dev;

  FOREACH_BUS_REGION_NAMESPACE(ctx, bus, region, ndns) {
    struct ndctl_btt *btt = ndctl_namespace_get_btt(ndns);
    struct ndctl_dax *dax = ndctl_namespace_get_dax(ndns);
    struct ndctl_pfn *pfn = ndctl_namespace_get_pfn(ndns);
    const char *devname;

    if (btt) {
      devname = ndctl_btt_get_block_device(btt);
    } else if (pfn) {
      devname = ndctl_pfn_get_block_device(pfn);
    } else if (dax) {
      struct daxctl_region *dax_region;
      dax_region = ndctl_dax_get_daxctl_region(dax);
      /* there is always one dax device in dax_region */
      if (dax_region) {
        is_dax_ = true;
        struct daxctl_dev *dev;
        dev = daxctl_dev_get_first(dax_region);
        devname = daxctl_dev_get_devname(dev);
      } else {
        return nullptr;
      }
    } else {
      devname = ndctl_namespace_get_block_device(ndns);
    }

    if (*devname == '\0')
      continue;

    char path[PATH_MAX];
    struct stat64 stat;
    if (sprintf(path, "/dev/%s", devname) == -1) {
      return nullptr;
    }

    if (stat64(path, &stat)) {
      return nullptr;
    }

    if (dev == stat.st_rdev) {
      return ndctl_region_get_interleave_set(region);
    }
  }
  return nullptr;
}

DimmNamespace::DimmNamespace(const std::string &mountpoint) {
  ndctl_interleave_set *set;
  struct stat64 st;

  if (mountpoint.empty()) {
    throw std::invalid_argument(
        "mountPoint field is empty. Please set mountPoint value.");
  }

  if (!ApiC::DirectoryExists(mountpoint)) {
    throw std::invalid_argument("Directory " + mountpoint + " does not exist.");
  }

  if (stat64(mountpoint.c_str(), &st)) {
    throw std::invalid_argument(std::strerror(errno));
  }
  int ret = ndctl_new(&ctx_);
  if (ret) {
    throw std::invalid_argument(std::strerror(ret));
  }
  set = GetInterleaveSet(ctx_, st);

  if (set == nullptr) {
    throw std::invalid_argument("Cannot get interleave set from: " +
                                mountpoint);
  }

  struct ndctl_dimm *dimm;

  ndctl_dimm_foreach_in_interleave_set(set, dimm) {
    const char *dimm_uid = ndctl_dimm_get_unique_id(dimm);
    dimms_.emplace_back(Dimm{dimm, dimm_uid});
  }

  if (!is_dax_) {
    test_dir_ = mountpoint + SEPARATOR + "pmdk_tests" + SEPARATOR;
    if (!ApiC::DirectoryExists(test_dir_) &&
        ApiC::CreateDirectoryT(test_dir_) != 0) {
      throw std::invalid_argument("");
    }

  } else {
    test_dir_ = mountpoint + SEPARATOR;
  }
}

DimmNamespace::~DimmNamespace() {
  if (!ctx_) {
    ndctl_unref(ctx_);
  }

  ctx_ = nullptr;
}
