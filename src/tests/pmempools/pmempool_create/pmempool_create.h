/*
 * Copyright 2017-2018, Intel Corporation
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

#ifndef PMDK_TESTS_SRC_TESTS_PMEMPOOLS_PMEMPOOL_CREATE_PMEMPOOL_CREATE_H_
#define PMDK_TESTS_SRC_TESTS_PMEMPOOLS_PMEMPOOL_CREATE_PMEMPOOL_CREATE_H_

#include "bins.h"
#include "configXML/local_configuration.h"
#include "gtest/gtest.h"
#include "output/output.h"
#include "shell/i_shell.h"
#include "structures.h"
#include "test_utils/file_utils.h"

extern std::unique_ptr<LocalConfiguration> local_config;

class PmempoolCreate : public ::testing::Test {
 private:
  std::string err_msg_;
  Output<> output_;

 public:
  ApiC api_c_;
  IShell shell_;
  PoolsetManagement p_mgmt_;
  const std::string pool_path_ = local_config->GetTestDir() + "pool.file";
  const std::string inherit_file_path_ =
      local_config->GetTestDir() + "inherited.pool";

  const std::string &GetOutputContent() const {
    return output_.GetContent();
  }

  int CreatePool(const PoolArgs &pool_args, const std::string &path);

  virtual void TearDown();
};

#endif  // !PMDK_TESTS_SRC_TESTS_PMEMPOOLS_PMEMPOOL_CREATE_PMEMPOOL_CREATE_H_
