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

#include "ext_cfg.h"
#include "api_c/api_c.h"

void ObjCtlExtCfgTest::SetUp() {
  errno = 0;
  std::tie(write_arg_, scenario_) = GetParam();
  std::string env_val;
  std::string query = ToCtlString(write_arg_);
  if (scenario_ == ExternalCfg::FROM_ENV_VAR) {
    env_var_ = "PMEMOBJ_CONF";
    env_val = query;
  } else {
    env_var_ = "PMEMOBJ_CONF_FILE";
    env_val = cfg_file_path_;
    ApiC::CreateFileT(cfg_file_path_, query);
  }
  ApiC::SetEnv(env_var_, env_val);
}

void ObjCtlExtCfgTest::TearDown() {
  ApiC::UnsetEnv(env_var_);
  if (scenario_ == ExternalCfg::FROM_CFG_FILE) {
    ApiC::RemoveFile(cfg_file_path_);
  }
}

void ObjCtlExtCfgPosTest::TearDown() {
  ApiC::RemoveFile(pool_path_);
  ObjCtlExtCfgTest::TearDown();
}

std::string ObjCtlExtCfgTest::ToCtlString(
    const pobj_alloc_class_desc &desc) const {
  std::string query = "heap.alloc_class.";
  query +=
      desc.class_id == auto_class_id ? "new" : std::to_string(desc.class_id);
  query += ".desc=" + std::to_string(desc.unit_size) + "," +
           std::to_string(desc.units_per_block) + "," +
           AllocClassUtils::hdrs[desc.header_type].config_name + ";";
  return query;
}

int ObjCtlExtCfgTest::GetAllocClassId(PMEMobjpool *pop, size_t unit_size,
                                      unsigned units_per_block,
                                      pobj_header_type header_type) const {
  int id = -1;
  pobj_alloc_class_desc arg;
  for (int i = 0; i < 255; ++i) {
    std::string entry_point = "heap.alloc_class." + std::to_string(i) + ".desc";
    if (pmemobj_ctl_get(pop, entry_point.c_str(), &arg) == 0) {
      if (arg.unit_size == unit_size &&
          arg.units_per_block == units_per_block &&
          arg.header_type == header_type) {
        id = arg.class_id;
        break;
      }
    }
  }
  return id;
}
