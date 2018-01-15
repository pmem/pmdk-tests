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

#include "pmempool_transform.h"

/**
 * PMEMPOOL_TRANSFORM_VERBOSE
 * Transforming obj pool with verbose option enabled
 * \test
 *          \li \c Step1. Create obj pool / SUCCESS
 *          \li \c Step2. Transform poolset with dry-run and verbose options
 * / SUCCESS
 *          \li \c Step3. Transform poolset with verbose option / SUCCESS
 *          \li \c Step4. Make sure that transformation was correct
 */
TEST_F(PmempoolTransform, PMEMPOOL_TRANSFORM_VERBOSE) {
  std::string verbose_message =
      local_config->GetTestDir() + SEPARATOR + "pool_src.set -> " +
      local_config->GetTestDir() + SEPARATOR + "pool_dst.set: transformed\n";
  Poolset poolset_src = {
      local_config->GetTestDir(), POOLSET_PATH_SRC, {{"PMEMPOOLSET", "20M"}},
      local_config->GetTestDir()};
  Poolset poolset_dst = {
      local_config->GetTestDir(),
      POOLSET_PATH_DST,
      {{"PMEMPOOLSET", "20M"}, {"REPLICA", "8M", "10M", "10M"}},
      local_config->GetTestDir()};
  ASSERT_EQ(0, p_mgmt_.CreatePoolsetFile(poolset_src));
  /* Step1 */
  ASSERT_EQ(0, obj_mgmt_.CreatePool({poolset_src.GetFullPath()}));
  ASSERT_EQ(0, file_utils::ValidatePoolset(poolset_src,
                                           struct_utils::GetDefaultMode()));
  ASSERT_EQ(0, p_mgmt_.CreatePoolsetFile(poolset_dst));
  /* Step2 */
  int ret = TransformPoolCLI(poolset_src.GetFullPath().c_str(),
                             poolset_dst.GetFullPath().c_str(),
                             {{Option::DryRun, OptionType::Long},
                              {Option::Verbose, OptionType::Long}});
  EXPECT_EQ(0, ret) << "Dry run with verbose mode failed: "
                    << GetOutputContent();
  ret = TransformPoolCLI(poolset_src.GetFullPath().c_str(),
                         poolset_dst.GetFullPath().c_str(),
                         {{Option::Verbose, OptionType::Short},
                          {Option::DryRun, OptionType::Long}});
  EXPECT_EQ(0, ret) << "Dry run with verbose mode failed: "
                    << GetOutputContent();
  /* Step3 */
  ret = TransformPoolCLI(poolset_src.GetFullPath().c_str(),
                         poolset_dst.GetFullPath().c_str(),
                         {{Option::Verbose, OptionType::Long}});

  ASSERT_EQ(0, ret) << "transformation failed " << GetOutputContent();
  /* Step4 */
  ASSERT_EQ(0, file_utils::ValidatePoolset(poolset_dst,
                                           struct_utils::GetDefaultMode()));
  ASSERT_EQ(verbose_message, GetOutputContent());
}

/**
 * PMEMPOOL_TRANSFORM_MISSING_PART
 * Transforming obj pool with the missing part
 * \test
 *          \li \c Step1. Create obj pool / SUCCESS
 *          \li \c Step2. Remove one part from poolset / SUCCESS
 *          \li \c Step3. Transform poolset with dry-run option and without
 * / FAIL
 */
TEST_F(PmempoolTransform, PMEMPOOL_TRANSFORM_MISSING_PART) {
  Poolset poolset_src = {
      local_config->GetTestDir(),
      POOLSET_PATH_SRC,
      {{"PMEMPOOLSET", "20M", "20M"}, {"REPLICA", "8M", "10M", "10M"}}};
  Poolset poolset_dst = {local_config->GetTestDir(),
                         POOLSET_PATH_DST,
                         {{"PMEMPOOLSET", "20M", "20M"},
                          {"REPLICA", "8M", "10M", "10M"},
                          {"REPLICA", "8M", "10M", "10M"}}};
  ASSERT_EQ(0, p_mgmt_.CreatePoolsetFile(poolset_src));
  /* Step1 */
  ASSERT_EQ(0, obj_mgmt_.CreatePool({poolset_src.GetFullPath()}));
  ASSERT_EQ(0, file_utils::ValidatePoolset(poolset_src,
                                           struct_utils::GetDefaultMode()));
  ASSERT_EQ(0, p_mgmt_.CreatePoolsetFile(poolset_dst));
  /* Step2 */
  ASSERT_EQ(0, p_mgmt_.RemovePart(poolset_src.GetReplica(1).GetPart(1)));
  /* Step3 */
  int ret = TransformPoolCLI(poolset_src.GetFullPath().c_str(),
                             poolset_dst.GetFullPath().c_str(),
                             {{Option::DryRun, OptionType::Long}});
  EXPECT_EQ(1, ret) << "Dry run with verbose mode passed but should not";
  ret = TransformPoolCLI(poolset_src.GetFullPath().c_str(),
                         poolset_dst.GetFullPath().c_str(),
                         {{Option::DryRun, OptionType::Short}});
  EXPECT_EQ(1, ret) << "Dry run with verbose mode passed but should not";

  ret = TransformPoolCLI(poolset_src.GetFullPath().c_str(),
                         poolset_dst.GetFullPath().c_str());

  ASSERT_EQ(1, ret) << "transformation passed but should not";
}
