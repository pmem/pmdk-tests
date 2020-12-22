/*
 * Copyright 2017-2019, Intel Corporation
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

#include "valid_arguments.h"

namespace create {
/**
 * PmempoolCreateValidTests.PMEMPOOL_CREATE
 * Creating pools of different type with following arguments:
 * - blk pool with size (long and short option) and write layout (long and short
 * option)
 * - bsize and mode options specified for blk pool
 * - log pool
 * - obj pool
 * \test
 *          \li \c Step1. Create pool with specified arguments / SUCCESS
 *          \li \c Step2. Make sure that pool exists and validate it's size and
 * mode
 */
TEST_P(ValidTests, PMEMPOOL_CREATE) {
  /* Step 1 */
  EXPECT_EQ(0, CreatePool(pool_args, pool_path_)) << GetOutputContent();
  /* Step 2 */
  EXPECT_EQ(0, file_utils::ValidateFile(pool_path_,
                                        struct_utils::GetPoolSize(pool_args),
                                        struct_utils::GetPoolMode(pool_args)));
}

INSTANTIATE_TEST_CASE_P(
    PmempoolCreateParam, ValidTests,
    ::testing::Values(PoolArgs{PoolType::Blk,
                               {{Option::BSize, OptionType::Short, "512"},
                                {Option::Size, OptionType::Long, "20M"},
                                {Option::Mode, OptionType::Short, "777"}}},
                      PoolArgs{PoolType::Blk,
                               {{Option::BSize, OptionType::Short, "8"},
                                {Option::Mode, OptionType::Short, "444"}}},
                      PoolArgs{PoolType::Log}, PoolArgs{PoolType::Obj}));

/**
 * PmempoolCreateValidInheritTests.PMEMPOOL_INHERIT_PROPERTIES
 * Inheriting settings from existing pool file:
 * - obj pool described by blk pool
 * - log pool described by obj pool
 * \test
 *          \li \c Step1. Creating pool by inheriting settings from existing
 * pool / SUCCESS
 *          \li \c Step2. Make sure that pool exists and validate it's size and
 * mode
 */
TEST_P(ValidInheritTests, PMEMPOOL_INHERIT_PROPERTIES) {
  /* Step 1 */
  EXPECT_EQ(0, CreatePool(pool_inherit.pool_inherited, inherit_file_path_))
      << GetOutputContent();
  /* Step 2 */
  EXPECT_EQ(0, file_utils::ValidateFile(
                   inherit_file_path_,
                   struct_utils::GetPoolSize(pool_inherit.pool_base),
                   struct_utils::GetPoolMode(pool_inherit.pool_inherited)));
}

INSTANTIATE_TEST_CASE_P(
    PmempoolCreateParam, ValidInheritTests,
    ::testing::Values(
        PoolInherit{
            {PoolType::Blk, {{Option::BSize, OptionType::Short, "512"}}},
            {PoolType::Obj,
             {{Option::Inherit, OptionType::Long,
               local_config->GetTestDir() + "pool.file"}}}},
        PoolInherit{{PoolType::Obj},
                    {PoolType::Log,
                     {{Option::Inherit, OptionType::Long,
                       local_config->GetTestDir() + "pool.file"}}}}));

/**
 * PmempoolCreateValidPoolsetTests.PMEMPOOL_POOLSET
 * Creating pool described by poolset file
 * \test
 *          \li \c Step1: Create pool described by poolset file / SUCCESS
 *          \li \c Step2: Make sure that pool described by poolset exists and
 * validate it's size and mode
 */
TEST_P(ValidPoolsetTests, PMEMPOOL_POOLSET) {
  /* Step 1 */
  EXPECT_EQ(0,
            CreatePool(poolset_args.args, poolset_args.poolset.GetFullPath()))
      << GetOutputContent();
  /* Step 2 */
  EXPECT_EQ(0, file_utils::ValidatePoolset(
                   poolset_args.poolset,
                   struct_utils::GetPoolMode(poolset_args.args)));
}

INSTANTIATE_TEST_CASE_P(
    PmempoolCreatePoolset, ValidPoolsetTests,
    ::testing::Values(
        PoolsetArgs{
            {PoolType::Blk, {{Option::BSize, OptionType::Short, "8"}}},
            Poolset{local_config->GetTestDir(), {{"PMEMPOOLSET", "20M"}}}},
        PoolsetArgs{
            {PoolType::Obj},
            Poolset{local_config->GetTestDir(), {{"PMEMPOOLSET", "20M"}}}},
        PoolsetArgs{
            {PoolType::Log},
            Poolset{local_config->GetTestDir(), {{"PMEMPOOLSET", "20M"}}}},
        PoolsetArgs{{PoolType::Obj},
                    Poolset{local_config->GetTestDir(),
                            {{"PMEMPOOLSET", "20M"}, {"REPLICA", "20M"}}}}));
}
