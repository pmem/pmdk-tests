/*
 * Copyright 2017, Intel Corporation
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

#include "pmempool_create_valid_arguments.h"

/**
 * PmempoolCreateValidTests.PMEMPOOL_CREATE
 * Creating different type pools using arguments:
 * - blk pool with size (long and short option) and write layout (long and short
 * option)
 * - blk pool with specified block size
 * - obj pool
 * - log pool
 * \test
 *          \li \c Step1. Create pool with specified arguments
 *          \li \c Step2. Make sure that pool file exists
 */
TEST_P(PmempoolCreateValidTests, PMEMPOOL_CREATE) {
  /* Step 1 */
  EXPECT_EQ(0, CreatePool(GetParam(), pool_path_)) << GetOutputContent();
  /* Step 2 */
  EXPECT_EQ(0, file_utils::ValidateFile(pool_path_,
                                        struct_utils::GetPoolSize(GetParam()),
                                        struct_utils::GetPoolMode(GetParam())));
}

INSTANTIATE_TEST_CASE_P(
    PmempoolCreateParam, PmempoolCreateValidTests,
    ::testing::Values(PoolArgs{PoolType::Blk,
                               {{Option::BlockNumber, OptType::Short, "512"},
                                {Option::Size, OptType::Long, "20M"},
                                {Option::Mode, OptType::Short, "777"}}},
                      PoolArgs{PoolType::Blk,
                               {{Option::BlockNumber, OptType::Short, "512"},
                                {Option::Mode, OptType::Short, "444"}}},
                      PoolArgs{PoolType::Log}, PoolArgs{PoolType::Obj}));

/**
 * PmempoolCreateValidInheritTests.PMEMPOOL_INHERIT_PROPERTIES
 * Inheriting settings from existing pool file:
 * - obj pool described by blk pool
 * - log pool described by obj pool
 * \test
 *          \li \c Step1. Inherit settings from existing pool / SUCCESS
 *          \li \c Step2. Check existence and size of pool
 */
TEST_P(PmempoolCreateValidInheritTests, PMEMPOOL_INHERIT_PROPERTIES) {
  /* Step 1 */
  EXPECT_EQ(0, CreatePool(GetParam().pool_inherited, inherit_file_path_))
      << GetOutputContent();
  /* Step 2 */
  EXPECT_EQ(0, file_utils::ValidateFile(
                   inherit_file_path_,
                   struct_utils::GetPoolSize(GetParam().pool_base),
                   struct_utils::GetPoolMode(GetParam().pool_inherited)));
}

INSTANTIATE_TEST_CASE_P(
    PmempoolCreateParam, PmempoolCreateValidInheritTests,
    ::testing::Values(
        PoolInherit{
            {PoolType::Blk, {{Option::BlockNumber, OptType::Short, "512"}}},
            {PoolType::Obj,
             {{Option::Inherit, OptType::Long,
               local_config->GetTestDir() + "pool.file"}}}},
        PoolInherit{{PoolType::Obj},
                    {PoolType::Log,
                     {{Option::Inherit, OptType::Long,
                       local_config->GetTestDir() + "pool.file"}}}}));

/**
 * PmempoolCreateValidPoolsetTests.PMEMPOOL_POOLSET
 * Creating pool described by poolset file
 * \test
 *          \li \c Step1: Create pool described by poolset file / SUCCESS
 *          \li \c Step2: Check existence and size of pool files described by
 * poolset / SUCCESS
 */
TEST_P(PmempoolCreateValidPoolsetTests, PMEMPOOL_POOLSET) {
  /* Step 1 */
  EXPECT_EQ(0,
            CreatePool(GetParam().pool_args, GetParam().poolset.GetFullPath()))
      << GetOutputContent();
  /* Step 2 */
  EXPECT_EQ(0, file_utils::ValidatePoolset(
                   GetParam().poolset,
                   struct_utils::GetPoolMode(GetParam().pool_args)));
}

INSTANTIATE_TEST_CASE_P(
    PmempoolCreatePoolset, PmempoolCreateValidPoolsetTests,
    ::testing::Values(
        PoolsetArgs{
            {PoolType::Blk, {{Option::BlockNumber, OptType::Short, "8"}}},
            Poolset{{"PMEMPOOLSET", "20M"}}},
        PoolsetArgs{{PoolType::Obj}, Poolset{{"PMEMPOOLSET", "20M"}}},
        PoolsetArgs{{PoolType::Log}, Poolset{{"PMEMPOOLSET", "20M"}}},
        PoolsetArgs{{PoolType::Obj},
                    Poolset{{"PMEMPOOLSET", "20M"}, {"REPLICA", "20M"}}}));
