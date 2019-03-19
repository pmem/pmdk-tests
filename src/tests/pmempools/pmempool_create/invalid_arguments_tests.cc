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

#include "invalid_arguments.h"

/**
 * InvalidArgumentsTests.PMEMPOOL_CREATE
 * Creating pool of different type using wrong arguments:
 * - inheritance from a non-existent pool (long and short option)
 * - bsize option specified for log pool
 * - too long layout name
 * - bsize argument omitted for blk pool type
 * - unsupported option for blk pool type
 * - invalid bsize value (-1, x) specified for blk pool type
 * - layout option specified for log pool (long and short option)
 * - layout option with specified layout name for log and blk pool (long
 * option)
 * - write layout option for obj and blk pool
 * - wrong mode value specified for blk pool
 * - without specified mode for obj pool
 * - without specified size
 * - without specified size and pool type
 * - insufficient size specified
 * \test
 *          \li \c Step1. Check that it is impossible to create pool with given
 * argument \ FAIL: ret = 1
 *          \li \c Step2. Make sure that pool file does not exist
 *          \li \c Step3. Make sure that expected output is returned
 */
TEST_P(InvalidArgumentsTests, PMEMPOOL_CREATE) {
  /* Step 1 */
  EXPECT_EQ(1, CreatePool(pool_args, pool_path_)) << GetOutputContent();
  /* Step 2 */
  EXPECT_FALSE(api_c_.RegularFileExists(pool_path_));
  /* Step 3 */
  EXPECT_TRUE(
      string_utils::IsSubstrFound(pool_args.err_msg, GetOutputContent()))
      << "Expected: " << pool_args.err_msg
      << "\nActual: " << GetOutputContent();
}

INSTANTIATE_TEST_CASE_P(
    PmempoolCreateParam, InvalidArgumentsTests,
    ::testing::Values(
        PoolArgs{PoolType::None,
                 {{Option::Inherit, OptionType::Short, "/non/existing/file"}},
                 "No such file or directory"},
        PoolArgs{PoolType::None,
                 {{Option::Inherit, OptionType::Long, "/non/existing/file"}},
                 "No such file or directory"},
        PoolArgs{
            PoolType::Log,
            {{Option::BSize, OptionType::Short, "512"}},
            "error: invalid option specified for log pool type -- block size"},
        PoolArgs{PoolType::Obj,
                 {{Option::Layout, OptionType::Long, std::string(1025, 'a')}},
                 "error: Layout name is too long"},
        PoolArgs{PoolType::Blk, "error: blk pool requires <bsize> argument"},
        PoolArgs{PoolType::Blk,
                 {{Option::BSize, OptionType::Short, "512"},
                  {Option::Size, OptionType::Long, "16778174iB"}},
                 "error: invalid size value specified"},
        PoolArgs{PoolType::Blk,
                 {{Option::BSize, OptionType::Short, "-1"}},
                 "create: invalid option -- '1'"},
        PoolArgs{PoolType::Blk,
                 {{Option::BSize, OptionType::Short, "x"}},
                 "error: cannot parse 'x' as block size"},
        PoolArgs{PoolType::Log,
                 {{Option::Layout, OptionType::Short, "1"}},
                 "error: '--layout|-l' -- invalid option specified for pool "
                 "type 'log'"},
        PoolArgs{PoolType::Log,
                 {{Option::Layout, OptionType::Long, "1"}},
                 "error: '--layout|-l' -- invalid option specified for pool "
                 "type 'log'"},
        PoolArgs{PoolType::Blk,
                 {{Option::BSize, OptionType::Short, "512"},
                  {Option::Layout, OptionType::Long, "TEST_LAYOUT"}},
                 "error: '--layout|-l' -- invalid option specified for pool "
                 "type 'blk'"},
        PoolArgs{PoolType::Obj,
                 {{Option::WriteLayout, OptionType::Long}},
                 "error: '--write-layout|-w' -- invalid option specified for "
                 "pool type 'obj'"},
        PoolArgs{PoolType::Log,
                 {{Option::WriteLayout, OptionType::Short}},
                 "error: '--write-layout|-w' -- invalid option specified for "
                 "pool type 'log'"},
        PoolArgs{PoolType::Blk,
                 {{Option::BSize, OptionType::Short, "512"},
                  {Option::Mode, OptionType::Long, "999"}},
                 "error: invalid mode value specified '999'"},
        PoolArgs{PoolType::Log,
                 {{Option::Size, OptionType::ShortNoSpace, "1M"}},
                 "error: size must be >= " + std::to_string(PMEMLOG_MIN_POOL) +
                     " bytes"},
        PoolArgs{PoolType::Obj,
                 {{Option::Size, OptionType::ShortNoSpace, "7M"}},
                 "error: size must be >= " + std::to_string(PMEMOBJ_MIN_POOL) +
                     " bytes"},
        PoolArgs{PoolType::Blk,
                 {{Option::BSize, OptionType::Short, "512"},
                  {Option::Size, OptionType::Long, "15M"}},
                 "error: size must be >= " + std::to_string(PMEMBLK_MIN_POOL) +
                     " bytes"}));

/**
 * InvalidInheritTests.PMEMPOOL_INHERIT_PROPERTIES
 * Inheriting settings from other existing pools described by other pool file:
 * \test
 *          \li \c Step1. Inherit settings from existing pool / FAIL: ret = 1
 *          \li \c Step2. Make sure that pool file does not exist
 *          \li \c Step3. Make sure that expected output is returned
 */
TEST_P(InvalidInheritTests, PMEMPOOL_INHERIT_PROPERTIES) {
  /* Step 1 */
  EXPECT_EQ(1, CreatePool(pool_inherit.pool_inherited, inherit_file_path_))
      << GetOutputContent();
  /* Step 2 */
  EXPECT_FALSE(api_c_.RegularFileExists(inherit_file_path_));
  /* Step 3 */
  EXPECT_TRUE(string_utils::IsSubstrFound(pool_inherit.pool_inherited.err_msg,
                                          GetOutputContent()))
      << "Expected: " << pool_inherit.pool_inherited.err_msg
      << "\nActual: " << GetOutputContent();
}

INSTANTIATE_TEST_CASE_P(PmempoolCreateParam, InvalidInheritTests,
                        ::testing::Values(PoolInherit{
                            {PoolType::Log},
                            {PoolType::Obj,
                             {{Option::Inherit, OptionType::Long,
                               local_config->GetTestDir() + "pool.file"}},
                             "error: size must be >= " +
                                 std::to_string(PMEMOBJ_MIN_POOL) +
                                 " bytes"}}));

/**
 * InvalidArgumentsPoolsetTests.PMEMPOOL_POOLSET
 * Creating different pool types described by poolset file with arguments:
 * - valid poolset and using --size/--max-size
 * - invalid poolsets
 * arguments
 * \test
 *          \li \c Step1. Check that it is impossible to create pool decribed by
 * poolset with given argument \ FAIL: ret = 1
 *          \li \c Step2. Make sure that pool files does not exist
 *          \li \c Step3. Make sure that expected output is returned
 */
TEST_P(InvalidArgumentsPoolsetTests, PMEMPOOL_POOLSET) {
  /* Step 1 */
  EXPECT_EQ(1,
            CreatePool(poolset_args.args, poolset_args.poolset.GetFullPath()))
      << GetOutputContent();
  /* Step 2 */
  EXPECT_TRUE(p_mgmt_.NoFilesExist(poolset_args.poolset));
  /* Step 3 */
  EXPECT_TRUE(string_utils::IsSubstrFound(poolset_args.args.err_msg,
                                          GetOutputContent()))
      << "Expected: " << poolset_args.args.err_msg
      << "\nActual: " << GetOutputContent();
}

INSTANTIATE_TEST_CASE_P(
    PmempoolCreatePoolset, InvalidArgumentsPoolsetTests,
    ::testing::Values(
        PoolsetArgs{
            {PoolType::Obj,
             {{Option::Size, OptionType::Long, "30M"}},
             "error: -s|--size cannot be used with poolset file"},
            Poolset{local_config->GetTestDir(), {{"PMEMPOOLSET", "20M"}}}},
        PoolsetArgs{
            {PoolType::Obj,
             {{Option::MaxSize, OptionType::Long}},
             "error: -M|--max-size cannot be used with poolset file"},
            Poolset{local_config->GetTestDir(), {{"PMEMPOOLSET", "20M"}}}},
        PoolsetArgs{
            {PoolType::Obj,
             {{Option::Size, OptionType::Long, "30M"}},
             "error: -s|--size cannot be used with poolset file"},
            Poolset{local_config->GetTestDir(), {{"PMEMPOOLSET", "20M"}}}},
        PoolsetArgs{
            {PoolType::Obj, "reservation pool size 7340032 smaller than " +
                                std::to_string(PMEMOBJ_MIN_POOL) +
                                "\nerror: creating pool file failed"},
            Poolset{local_config->GetTestDir(), {{"PMEMPOOLSET", "7M"}}}},
        PoolsetArgs{
            {PoolType::Obj, "reservation pool size 7340032 smaller than " +
                                std::to_string(PMEMOBJ_MIN_POOL) +
                                "\nerror: creating pool file failed"},
            Poolset{local_config->GetTestDir(),
                    {{"PMEMPOOLSET", "20M"}, {"REPLICA", "7M"}}}},
        PoolsetArgs{
            {PoolType::Log,
             "replication not supported\nerror: creating pool file failed"},
            Poolset{local_config->GetTestDir(),
                    {{"PMEMPOOLSET", "20M"}, {"REPLICA", "20M"}}}},
        PoolsetArgs{
            {PoolType::Blk,
             {{Option::BSize, OptionType::Long, "512"}},
             " -- replication not supported\nerror: creating pool file failed"},
            Poolset{local_config->GetTestDir(),
                    {{"PMEMPOOLSET", "20M"}, {"REPLICA", "20M"}}}}));
