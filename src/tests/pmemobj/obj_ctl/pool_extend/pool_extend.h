/*
 * Copyright (c) 2018, Intel Corporation
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

#ifndef PMDK_POOL_EXTEND_H
#define PMDK_POOL_EXTEND_H

#include <libpmemobj.h>
#include <math.h>
#include <memory>
#include <string>
#include "api_c/api_c.h"
#include "configXML/local_configuration.h"
#include "constants.h"
#include "gtest/gtest.h"
#include "poolset/poolset.h"
#include "poolset/poolset_management.h"
#include "shell/i_shell.h"
#include "test_utils/file_utils.h"
#include "shell/i_shell.h"
#include "constants.h"
#include <libpmemobj.h>
#include <memory>
#include <string>
#include <math.h>

extern std::unique_ptr<LocalConfiguration> local_config;

struct ExtendTestDirs {
  const std::string test_dir = local_config->GetTestDir(),
                    pools_dir = test_dir + "pools/",
                    pool_dir1 = pools_dir + "1/", 
                    pool_dir2 = pools_dir + "2/";
};

class ObjCtlPoolExtendTest : public ExtendTestDirs, public ::testing::Test {
public:
  PMEMobjpool *pop = nullptr;

  std::string pool_path;
  const char *layout = "pool_extend";
  Poolset *poolset;
  std::unique_ptr<PoolsetManagement> p_mgmt;
  ApiC cmd;
  IShell shell;

  void SetUp();
  void TearDown();
  void ReopenAndCheckPool(PMEMobjpool *pop, std::string path);
};

#endif // PMDK_POOL_EXTEND_H
