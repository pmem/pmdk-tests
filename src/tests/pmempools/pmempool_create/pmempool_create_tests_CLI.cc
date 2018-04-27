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

#include "pmempool_create.h"

/**
 * PMEMPOOL_CREATE_INHERIT
 * Inheriting settings from the same pool
 * \test
 *          \li \c Step1. Create log pool with 10M size / SUCCESS
 *          \li \c Step2. Inherit settings from existing pool / FAIL: ret = 1
 *          \li \c Step3. Make sure that expected failure message is returned
 */
TEST_F(PmempoolCreate, PMEMPOOL_CREATE_INVALID_INHERIT) {
  /* Step 1 */
  EXPECT_EQ(0, CreatePool(PoolArgs{PoolType::Log,
                                   {{Option::Size, OptionType::Short, "10M"}}},
                          pool_path_))
      << GetOutputContent();
  /* Step 2 */
  EXPECT_EQ(
      1, CreatePool(PoolArgs{PoolType::Log,
                             {{Option::Inherit, OptionType::Long, pool_path_}}},
                    pool_path_))
      << GetOutputContent();
  /* Step 3 */
  EXPECT_TRUE(string_utils::IsSubstrFound<char>(
      "error: creating pool file failed", GetOutputContent()))
      << "Expected: error: creating pool file failed\nActual: "
      << GetOutputContent();
}

/**
 * PMEMPOOL_CREATE_MAX_SIZE
 * Creating log pool pool with maximal size
 * \test
 *          \li \c Step1. Create log pool with maximal size / SUCCESS
 *          \li \c Step2. Check the size of the created pool
 */
TEST_F(PmempoolCreate, PMEMPOOL_CREATE_MAX_SIZE) {
  size_t free_space = api_c_.GetFreeSpaceT(local_config->GetTestDir());
  /* Step 1 */
  EXPECT_EQ(0, CreatePool(PoolArgs{PoolType::Log,
                                   {{Option::MaxSize, OptionType::Long}}},
                          pool_path_))
      << GetOutputContent();
  /* Step 2 */
  EXPECT_EQ(free_space, api_c_.GetFileSize(pool_path_));
}

/**
 * PMEMPOOL_CREATE_VERBOSE
 * Creating obj pool with verbose option enabled
 * \test
 *          \li \c Step1. Create obj pool with verbose option / SUCCESS
 *          \li \c Step2. Make sure that expected output is returned
 */
TEST_F(PmempoolCreate, PMEMPOOL_CREATE_VERBOSE) {
  const std::string out_msg = "Creating pool: " + pool_path_ +
                              "\n\ttype  : obj"
                              "\n\tsize  : 20.0M [20971520]"
                              "\n\tmode  : 0664"
                              "\n\tlayout: ''";
  /* Step 1 */
  EXPECT_EQ(0, CreatePool(PoolArgs{PoolType::Obj,
                                   {{Option::Verbose, OptionType::Long},
                                    {Option::Size, OptionType::Long, "20M"}}},
                          pool_path_))
      << GetOutputContent();
  /* Step 2 */
  EXPECT_TRUE(string_utils::IsSubstrFound(out_msg, GetOutputContent()))
      << "Expected: " << out_msg << "\nActual: " << GetOutputContent();
}

/**
 * PMEMPOOL_CREATE_HELP_MESSAGES
 * Displaying help message using valid short and long options
 * \test
 *          \li \c Step1. Display help message for pmempool create command using
 * long option / SUCCESS
 *          \li \c Step2. Display help message for pmempool create command using
 * short option / SUCCESS
 */
TEST_F(PmempoolCreate, PMEMPOOL_CREATE_HELP_MESSAGES) {
  /* Step 1 */
  EXPECT_EQ(0, shell_.ExecuteCommand("pmempool create -h").GetExitCode());
  /* Step 2 */
  EXPECT_EQ(0, shell_.ExecuteCommand("pmempool create --help").GetExitCode());
}
