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

#ifndef PMDK_EXT_CFG_H
#define PMDK_EXT_CFG_H

#include <libpmemobj.h>
#include <limits>
#include <memory>
#include <string>
#include <tuple>
#include "alloc_class_utils.h"
#include "configXML/local_configuration.h"
#include "gtest/gtest.h"

extern std::unique_ptr<LocalConfiguration> local_config;
/* constant that indicates automatic class creation is requested */
const unsigned auto_class_id = (std::numeric_limits<unsigned>::max)() - 1;

enum class ExternalCfg { FROM_ENV_VAR, FROM_CFG_FILE };

class ObjCtlExtCfgTest : public ::testing::TestWithParam<
                             std::tuple<pobj_alloc_class_desc, ExternalCfg>> {
 private:
  std::string test_dir_ = local_config->GetTestDir();

 public:
  std::string env_var_;
  ExternalCfg scenario_;
  std::string pool_path_ = test_dir_ + "pool";
  std::string cfg_file_path_ = test_dir_ + "cfg_file";
  pobj_alloc_class_desc write_arg_;
  /* ToCtlString -- returns valid alloc class query string based on desc struct
   */
  std::string ToCtlString(const pobj_alloc_class_desc &desc) const;
  /* GetAllocClassId -- returns class id of the allocation class within pop pool
   * described by unit_size, units_per_block, header_type arguments.
   * Returns -1 if allocation class does not exist. */
  int GetAllocClassId(PMEMobjpool *pop, size_t unit_size,
                      unsigned units_per_block,
                      pobj_header_type header_type) const;
  virtual void SetUp();
  virtual void TearDown();
};

class ObjCtlExtCfgPosTest : public ObjCtlExtCfgTest {
 public:
  void TearDown() override;
};

class ObjCtlExtCfgNegTest : public ObjCtlExtCfgTest {};

#endif  // PMDK_EXT_CFG_H
