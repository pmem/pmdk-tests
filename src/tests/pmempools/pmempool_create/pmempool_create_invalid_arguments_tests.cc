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

#include "pmempool_create_invalid_arguments.h"

/**
 * PmempoolCreateInvalidArgumentsTests.PMEMPOOL_CREATE
 * Creating pool of different type using wrong arguments:
 * - wrong pool inheritance (long and short option)
 * - use block size for log pool
 * - too long layout name
 * - omit bsize argument for blk pool type
 * - invalid option for blk pool type
 * - invalid bsize option for blk pool type
 * - use layout option for log pool (long and short option)
 * - use layout option with specified layout name for log and blk pool (long
 * option)
 * - use write layout option for obj and blk pool
 * - wrong mode value specified for blk pool
 * - without specified mode for obj pool
 * - without specified size
 * - without specified size and pool type
 * - insufficient size specified
 * \test
 *          \li \c Step1. Check that it is impossible to create pool with given
 * argument
 *          \li \c Step2. Make sure that pool does not exist
 *          \li \c Step3. Make sure that expected output is returned
 */
TEST_P(PmempoolCreateInvalidArgumentsTests, PMEMPOOL_CREATE) {
  /* Step 1 */
  EXPECT_EQ(1, CreatePool(GetParam(), pool_path_)) << GetStdOut();
  /* Step 2 */
  EXPECT_EQ(0, ValidateFile(pool_path_)) << GetErrMsg();
  /* Step 3 */
  EXPECT_TRUE(string_utils::IsSubstrFound(GetParam().err_msg, GetStdOut()))
      << "Output is different than expected\nExpected: " << GetParam().err_msg
      << "\nReceived: " << GetStdOut();
}

INSTANTIATE_TEST_CASE_P(
    PmempoolCreateParam, PmempoolCreateInvalidArgumentsTests,
    ::testing::Values(
        PoolArgs{PoolType::None,
                 {{Option::Inherit, OptType::Short, "/non/existing/file"}},
                 "No such file or directory"},
        PoolArgs{PoolType::None,
                 {{Option::Inherit, OptType::Long, "/non/existing/file"}},
                 "No such file or directory"},
        PoolArgs{
            PoolType::Log,
            {{Option::BlockNumber, OptType::Short, "512"}},
            "error: invalid option specified for log pool type -- block size"},
        PoolArgs{PoolType::Obj,
                 {{Option::Layout, OptType::Long, std::string(1025, 'a')}},
                 "error: Layout name is to long"},
        PoolArgs{PoolType::Blk, "error: blk pool requires <bsize> argument"},
        PoolArgs{PoolType::Blk,
                 {{Option::BlockNumber, OptType::Short, "512"},
                  {Option::Size, OptType::Long, "16778174iB"}},
                 "error: invalid size value specified"},
        PoolArgs{PoolType::Blk,
                 {{Option::BlockNumber, OptType::Short, "-1"}},
                 "create: invalid option -- '1'"},
        PoolArgs{PoolType::Blk,
                 {{Option::BlockNumber, OptType::Short, "x"}},
                 "error: cannot parse 'x' as block size"},
        PoolArgs{PoolType::Log,
                 {{Option::Layout, OptType::Short, "1"}},
                 "error: '--layout|-l' -- invalid option specified for pool "
                 "type 'log'"},
        PoolArgs{PoolType::Log,
                 {{Option::Layout, OptType::Long, "1"}},
                 "error: '--layout|-l' -- invalid option specified for pool "
                 "type 'log'"},
        PoolArgs{PoolType::Blk,
                 {{Option::BlockNumber, OptType::Short, "512"},
                  {Option::Layout, OptType::Long, "TEST_LAYOUT"}},
                 "error: '--layout|-l' -- invalid option specified for pool "
                 "type 'blk'"},
        PoolArgs{PoolType::Obj,
                 {{Option::WriteLayout, OptType::Long, ""}},
                 "error: '--write-layout|-w' -- invalid option specified for "
                 "pool type 'obj'"},
        PoolArgs{PoolType::Log,
                 {{Option::WriteLayout, OptType::Long, ""}},
                 "error: '--write-layout|-w' -- invalid option specified for "
                 "pool type 'log'"},
        PoolArgs{PoolType::Blk,
                 {{Option::BlockNumber, OptType::Short, "512"},
                  {Option::Mode, OptType::Long, "999"}},
                 "error: invalid mode value specified '999'"},
        PoolArgs{PoolType::Log,
                 {{Option::Size, OptType::ShortNoSpace, "1M"}},
                 "error: size must be >= " + std::to_string(PMEMLOG_MIN_POOL) +
                     " bytes"},
        PoolArgs{PoolType::Obj,
                 {{Option::Size, OptType::ShortNoSpace, "7M"}},
                 "error: size must be >= " + std::to_string(PMEMOBJ_MIN_POOL) +
                     " bytes"},
        PoolArgs{PoolType::Blk,
                 {{Option::BlockNumber, OptType::Short, "512"},
                  {Option::Size, OptType::Long, "15M"}},
                 "error: size must be >= " + std::to_string(PMEMBLK_MIN_POOL) +
                     " bytes"}));

/**
 * PmempoolCreateInvalidInheritTests.PMEMPOOL_INHERIT_PROPERTIES
 * Inheriting settings from other existing pools described by other pool file:
 * \test
 *          \li \c Step1. Inherit settings from existing pool / FAIL
 *          \li \c Step2. Make sure that pool file does not exists
 *          \li \c Step3. Make sure that output does not differ from expected
 */
TEST_P(PmempoolCreateInvalidInheritTests, PMEMPOOL_INHERIT_PROPERTIES) {
  /* Step 1 */
  EXPECT_EQ(1, CreatePool(GetParam().pool_inherited, inherit_file_path_))
      << GetStdOut();
  /* Step 2 */
  EXPECT_EQ(0, ValidateFile(inherit_file_path_)) << GetErrMsg();
  /* Step 3 */
  EXPECT_TRUE(string_utils::IsSubstrFound(GetParam().pool_inherited.err_msg,
                                          GetStdOut()))
      << "Output is different than expected\nExpected: "
      << GetParam().pool_inherited.err_msg << "\nReceived: " << GetStdOut();
}

INSTANTIATE_TEST_CASE_P(PmempoolCreateParam, PmempoolCreateInvalidInheritTests,
                        ::testing::Values(PoolInherit{
                            {PoolType::Log},
                            {PoolType::Obj,
                             {{Option::Inherit, OptType::Long,
                               local_config->GetWorkingDir() + "pool.file"}},
                             "error: size must be >= " +
                                 std::to_string(PMEMOBJ_MIN_POOL) +
                                 " bytes"}}));

/**
 * PmempoolCreateInvalidArgumentsPoolsetTests.PMEMPOOL_POOLSET
 * Creating different pool types described by poolset file with arguments:
 * - valid poolset and using --size/--max-size
 * - invalid poolsets
 * arguments
 * \test
 *          \li \c Step1. Check that it is impossible to create pool decribed by
 * poolset with given argument
 *          \li \c Step2. Make sure that pool does not exist
 *          \li \c Step3. Make sure that output does not differ from expected
 */
TEST_P(PmempoolCreateInvalidArgumentsPoolsetTests, PMEMPOOL_POOLSET) {
  /* Step 1 */
  EXPECT_EQ(1,
            CreatePool(GetParam().pool_args, GetParam().poolset.GetFullPath()))
      << GetStdOut();
  /* Step 2 */
  EXPECT_EQ(0, ValidateFile(GetParam().poolset)) << GetErrMsg();
  /* Step 3 */
  EXPECT_TRUE(
      string_utils::IsSubstrFound(GetParam().pool_args.err_msg, GetStdOut()))
      << "Output is different than expected\nExpected: "
      << GetParam().pool_args.err_msg << "\nReceived: " << GetStdOut();
}

INSTANTIATE_TEST_CASE_P(
    PmempoolCreatePoolset, PmempoolCreateInvalidArgumentsPoolsetTests,
    ::testing::Values(
        PoolsetArgs{{PoolType::Obj,
                     {{Option::Size, OptType::Long, "30M"}},
                     "error: -s|--size cannot be used with poolset file"},
                    Poolset{{"PMEMPOOLSET", "20M"}}},
        PoolsetArgs{{PoolType::Obj,
                     {{Option::MaxSize, OptType::Long, ""}},
                     "error: -M|--max-size cannot be used with poolset file"},
                    Poolset{{"PMEMPOOLSET", "20M"}}},
        PoolsetArgs{{PoolType::Obj,
                     {{Option::Size, OptType::Long, "30M"}},
                     "error: -s|--size cannot be used with poolset file"},
                    Poolset{{"PMEMPOOLSET", "20M"}}},
        PoolsetArgs{{PoolType::Obj, "net pool size 7340032 smaller than " +
                                        std::to_string(PMEMOBJ_MIN_POOL) +
                                        "\nerror: creating pool file failed"},
                    Poolset{{"PMEMPOOLSET", "7M"}}},
        PoolsetArgs{{PoolType::Obj, "net pool size 7340032 smaller than " +
                                        std::to_string(PMEMOBJ_MIN_POOL) +
                                        "\nerror: creating pool file failed"},
                    Poolset{{"PMEMPOOLSET", "20M"}, {"REPLICA", "7M"}}},
        PoolsetArgs{
            {PoolType::Log,
             "replication not supported\nerror: creating pool file failed"},
            Poolset{{"PMEMPOOLSET", "20M"}, {"REPLICA", "20M"}}},
        PoolsetArgs{
            {PoolType::Blk,
             {{Option::BlockNumber, OptType::Long, "512"}},
             " -- replication not supported\nerror: creating pool file failed"},
            Poolset{{"PMEMPOOLSET", "20M"}, {"REPLICA", "20M"}}}));
