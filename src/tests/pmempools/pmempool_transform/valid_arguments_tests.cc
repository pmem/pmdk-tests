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

#include "valid_arguments.h"

namespace transform {
/**
 * TransformValidTests.PMEMPOOL_TRANSFORM_API
 * Transforming obj pool using valid arguments:
 * - add and remove secondary replica
 * - remove secondary replica and add replica as primary replica with
 * PMEMOBJ_MIN_SIZE
 * - remove secondary replica and add replica as primary replica
 * - remove and add the same primary replica
 * - remove primary replica, add primary and ternary replica
 * \test
 *          \li \c Step1. Transform poolset with and without dry-run option
 * / SUCCESS
 *          \li \c Step2. Make sure that transformation was correct
 *          \li \c Step3. Transform poolset with and without dry-run option
 * / SUCCESS
 *          \li \c Step4. Make sure that transformation was correct
 */
TEST_P(TransformValidTests, PMEMPOOL_TRANSFORM_API) {
  /* Step1 */
  int ret = pmempool_transform(GetParam().poolset_src.GetFullPath().c_str(),
                               GetParam().poolset_temp.GetFullPath().c_str(),
                               PMEMPOOL_DRY_RUN);
  EXPECT_EQ(0, ret) << "PMEMPOOL_DRY_RUN failed: " << strerror(errno);

  ret = pmempool_transform(GetParam().poolset_src.GetFullPath().c_str(),
                           GetParam().poolset_temp.GetFullPath().c_str(), 0);

  ASSERT_EQ(0, ret) << "transformation failed " << strerror(errno);
  /* Step2 */
  ASSERT_EQ(0, file_utils::ValidatePoolset(GetParam().poolset_temp,
                                           GetParam().obj_pool.mode));
  /* Step3 */
  ret = pmempool_transform(GetParam().poolset_temp.GetFullPath().c_str(),
                           GetParam().poolset_dst.GetFullPath().c_str(),
                           PMEMPOOL_DRY_RUN);
  EXPECT_EQ(0, ret) << "PMEMPOOL_DRY_RUN failed: " << strerror(errno);

  ret = pmempool_transform(GetParam().poolset_temp.GetFullPath().c_str(),
                           GetParam().poolset_dst.GetFullPath().c_str(), 0);

  ASSERT_EQ(0, ret) << "transformation failed " << strerror(errno);
  /* Step4 */
  ASSERT_EQ(0, file_utils::ValidatePoolset(GetParam().poolset_dst,
                                           GetParam().obj_pool.mode));
}

/**
 * TransformValidTests.PMEMPOOL_TRANSFORM_CLI
 * Transforming obj pool using valid arguments:
 * - add and remove secondary replica
 * - remove secondary replica and add replica as primary replica with
 * PMEMOBJ_MIN_SIZE
 * - remove secondary replica and add replica as primary replica
 * - remove and add the same primary replica
 * - remove primary replica, add primary and ternary replica
 * \test
 *          \li \c Step1. Transform poolset with and without dry-run option
 * / SUCCESS
 *          \li \c Step2. Make sure that transformation was correct
 *          \li \c Step3. Transform poolset with and without dry-run option
 * / SUCCESS
 *          \li \c Step4. Make sure that transformation was correct
 */
TEST_P(TransformValidTests, PMEMPOOL_TRANSFORM_CLI) {
  /* Step1 */
  int ret = TransformPoolCLI(GetParam().poolset_src.GetFullPath().c_str(),
                             GetParam().poolset_temp.GetFullPath().c_str(),
                             {{Option::DryRun, OptionType::Long}});
  EXPECT_EQ(0, ret) << "PMEMPOOL_DRY_RUN failed: " << GetOutputContent();
  ret = TransformPoolCLI(GetParam().poolset_src.GetFullPath().c_str(),
                         GetParam().poolset_temp.GetFullPath().c_str(),
                         {{Option::DryRun, OptionType::Short}});
  EXPECT_EQ(0, ret) << "PMEMPOOL_DRY_RUN failed: " << GetOutputContent();

  ret = TransformPoolCLI(GetParam().poolset_src.GetFullPath().c_str(),
                         GetParam().poolset_temp.GetFullPath().c_str());

  ASSERT_EQ(0, ret) << "transformation failed " << GetOutputContent();
  /* Step2 */
  ASSERT_EQ(0, file_utils::ValidatePoolset(GetParam().poolset_temp,
                                           GetParam().obj_pool.mode));
  /* Step3 */
  ret = TransformPoolCLI(GetParam().poolset_temp.GetFullPath().c_str(),
                         GetParam().poolset_dst.GetFullPath().c_str(),
                         {{Option::DryRun, OptionType::Long}});
  EXPECT_EQ(0, ret) << "PMEMPOOL_DRY_RUN failed: " << GetOutputContent();
  ret = TransformPoolCLI(GetParam().poolset_temp.GetFullPath().c_str(),
                         GetParam().poolset_dst.GetFullPath().c_str(),
                         {{Option::DryRun, OptionType::Short}});
  EXPECT_EQ(0, ret) << "PMEMPOOL_DRY_RUN failed: " << GetOutputContent();

  ret = TransformPoolCLI(GetParam().poolset_temp.GetFullPath().c_str(),
                         GetParam().poolset_dst.GetFullPath().c_str());

  ASSERT_EQ(0, ret) << "transformation failed " << GetOutputContent();
  /* Step4 */
  ASSERT_EQ(0, file_utils::ValidatePoolset(GetParam().poolset_dst,
                                           GetParam().obj_pool.mode));
}

INSTANTIATE_TEST_CASE_P(
    TransformParamValidTests, TransformValidTests,
    ::testing::Values(
        TransformArgs{
            ObjPool{local_config->GetTestDir() + POOLSET_PATH_SRC},
            Poolset{local_config->GetTestDir(),
                    POOLSET_PATH_SRC,
                    {{"PMEMPOOLSET", "20M"}}},
            Poolset{local_config->GetTestDir(),
                    POOLSET_PATH_TEMP,
                    {{"PMEMPOOLSET", "20M"}, {"REPLICA", "10M", "20M"}}},
            Poolset{local_config->GetTestDir(),
                    POOLSET_PATH_DST,
                    {{"PMEMPOOLSET", "20M"}}}},
        TransformArgs{
            ObjPool{local_config->GetTestDir() + POOLSET_PATH_SRC},
            Poolset{local_config->GetTestDir(),
                    POOLSET_PATH_SRC,
                    {{"PMEMPOOLSET", "20M"},
                     {"REPLICA", std::to_string(PMEMOBJ_MIN_POOL)}}},
            Poolset{local_config->GetTestDir(),
                    POOLSET_PATH_TEMP,
                    {{"PMEMPOOLSET", "20M"}}},
            Poolset{local_config->GetTestDir(),
                    POOLSET_PATH_DST,
                    {{"PMEMPOOLSET", std::to_string(PMEMOBJ_MIN_POOL) + " " +
                                         local_config->GetTestDir() +
                                         "replica1.part0"},
                     {"REPLICA",
                      "20M " + local_config->GetTestDir() + "pool.part0"}}}},
        TransformArgs{
            ObjPool{local_config->GetTestDir() + POOLSET_PATH_SRC},
            Poolset{local_config->GetTestDir(),
                    POOLSET_PATH_SRC,
                    {{"PMEMPOOLSET", "20M", "20M"},
                     {"REPLICA", std::to_string(PMEMOBJ_MIN_POOL)}}},
            Poolset{local_config->GetTestDir(),
                    POOLSET_PATH_TEMP,
                    {{"PMEMPOOLSET", std::to_string(PMEMOBJ_MIN_POOL) + " " +
                                         local_config->GetTestDir() +
                                         "replica1.part0"}}},
            Poolset{local_config->GetTestDir(),
                    POOLSET_PATH_DST,
                    {{"PMEMPOOLSET", "20M", "20M"},
                     {"REPLICA", std::to_string(PMEMOBJ_MIN_POOL)}}}},
        TransformArgs{
            ObjPool{local_config->GetTestDir() + POOLSET_PATH_SRC},
            Poolset{local_config->GetTestDir(),
                    POOLSET_PATH_SRC,
                    {{"PMEMPOOLSET", "20M", "20M"},
                     {"REPLICA", std::to_string(PMEMOBJ_MIN_POOL)}}},
            Poolset{local_config->GetTestDir(),
                    POOLSET_PATH_TEMP,
                    {{"PMEMPOOLSET", std::to_string(PMEMOBJ_MIN_POOL) + " " +
                                         local_config->GetTestDir() +
                                         "replica1.part0"}}},
            Poolset{local_config->GetTestDir(),
                    POOLSET_PATH_DST,
                    {{"PMEMPOOLSET", "20M", "20M"},
                     {"REPLICA", std::to_string(PMEMOBJ_MIN_POOL)}}}},
        TransformArgs{
            ObjPool{local_config->GetTestDir() + POOLSET_PATH_SRC},
            Poolset{local_config->GetTestDir(),
                    POOLSET_PATH_SRC,
                    {{"PMEMPOOLSET", "20M", "20M"},
                     {"REPLICA", std::to_string(PMEMOBJ_MIN_POOL)}}},
            Poolset{local_config->GetTestDir(),
                    POOLSET_PATH_TEMP,
                    {{"PMEMPOOLSET", std::to_string(PMEMOBJ_MIN_POOL) + " " +
                                         local_config->GetTestDir() +
                                         "replica1.part0"}}},
            Poolset{local_config->GetTestDir(),
                    POOLSET_PATH_DST,
                    {{"PMEMPOOLSET", "20M", "20M"},
                     {"REPLICA", std::to_string(PMEMOBJ_MIN_POOL)},
                     {"REPLICA", std::to_string(PMEMOBJ_MIN_POOL)}}}}));

/**
 * TransformValidBasicTests.PMEMPOOL_TRANSFORM_API
 * Transforming obj pool using valid arguments:
 * - adding replica with the same size as primary replica
 * - adding replica with bigger size as primary replica
 * - removing primary and ternary replicas
 * - removing ternary replica and reorder replicas
 * \test
 *          \li \c Step1. Transform poolset with and without dry-run option
 * / SUCCESS
 *          \li \c Step2. Make sure that transformation was correct
 */
TEST_P(TransformValidBasicTests, PMEMPOOL_TRANSFORM_API) {
  /* Step1 */
  int ret = pmempool_transform(GetParam().poolset_src.GetFullPath().c_str(),
                               GetParam().poolset_dst.GetFullPath().c_str(),
                               PMEMPOOL_DRY_RUN);
  EXPECT_EQ(0, ret) << "PMEMPOOL_DRY_RUN failed: " << strerror(errno);

  ret = pmempool_transform(GetParam().poolset_src.GetFullPath().c_str(),
                           GetParam().poolset_dst.GetFullPath().c_str(), 0);

  ASSERT_EQ(0, ret) << "transformation failed " << strerror(errno);
  /* Step2 */
  ASSERT_EQ(0, file_utils::ValidatePoolset(GetParam().poolset_dst,
                                           GetParam().obj_pool.mode));
}

/**
 * TransformValidBasicTests.PMEMPOOL_TRANSFORM_CLI
 * Transforming obj pool using valid arguments:
 * - adding replica with the same size as primary replica
 * - adding replica with bigger size as primary replica
 * - removing primary and ternary replicas
 * - removing ternary replica and reorder replicas
 * \test
 *          \li \c Step1. Transform poolset with and without dry-run option
 * / SUCCESS
 *          \li \c Step2. Make sure that transformation was correct
 */
TEST_P(TransformValidBasicTests, PMEMPOOL_TRANSFORM_CLI) {
  int ret = TransformPoolCLI(GetParam().poolset_src.GetFullPath().c_str(),
                             GetParam().poolset_dst.GetFullPath().c_str(),
                             {{Option::DryRun, OptionType::Long}});
  EXPECT_EQ(0, ret) << "PMEMPOOL_DRY_RUN failed: " << GetOutputContent();
  ret = TransformPoolCLI(GetParam().poolset_src.GetFullPath().c_str(),
                         GetParam().poolset_dst.GetFullPath().c_str(),
                         {{Option::DryRun, OptionType::Short}});
  EXPECT_EQ(0, ret) << "PMEMPOOL_DRY_RUN failed: " << GetOutputContent();

  ret = TransformPoolCLI(GetParam().poolset_src.GetFullPath().c_str(),
                         GetParam().poolset_dst.GetFullPath().c_str());

  ASSERT_EQ(0, ret) << "transformation failed " << GetOutputContent();
  ASSERT_EQ(0, file_utils::ValidatePoolset(GetParam().poolset_dst,
                                           GetParam().obj_pool.mode));
}

INSTANTIATE_TEST_CASE_P(
    TransformParamValidTests, TransformValidBasicTests,
    ::testing::Values(
        TransformArgs{ObjPool{local_config->GetTestDir() + POOLSET_PATH_SRC},
                      Poolset{local_config->GetTestDir(),
                              POOLSET_PATH_SRC,
                              {{"PMEMPOOLSET", "20M", "30M"}}},
                      Poolset{local_config->GetTestDir(),
                              POOLSET_PATH_DST,
                              {{"PMEMPOOLSET", "20M", "30M"},
                               {"REPLICA", "20M", "30M"}}}},
        TransformArgs{ObjPool{local_config->GetTestDir() + POOLSET_PATH_SRC},
                      Poolset{local_config->GetTestDir(),
                              POOLSET_PATH_SRC,
                              {{"PMEMPOOLSET", "20M", "30M"}}},
                      Poolset{local_config->GetTestDir(),
                              POOLSET_PATH_DST,
                              {{"PMEMPOOLSET", "20M", "30M"},
                               {"REPLICA", "21M", "30M"}}}},
        TransformArgs{
            ObjPool{local_config->GetTestDir() + POOLSET_PATH_SRC},
            Poolset{local_config->GetTestDir(),
                    POOLSET_PATH_SRC,
                    {{"PMEMPOOLSET", "20M"},
                     {"REPLICA", "20M"},
                     {"REPLICA", "20M"}}},
            Poolset{local_config->GetTestDir(),
                    POOLSET_PATH_DST,
                    {{"PMEMPOOLSET", "20M " + local_config->GetTestDir() +
                                         "replica1.part0"}}}},
        TransformArgs{
            ObjPool{local_config->GetTestDir() + POOLSET_PATH_SRC},
            Poolset{local_config->GetTestDir(),
                    POOLSET_PATH_SRC,
                    {{"PMEMPOOLSET", "20M"},
                     {"REPLICA", "20M"},
                     {"REPLICA", "20M"}}},
            Poolset{local_config->GetTestDir(),
                    POOLSET_PATH_DST,
                    {{"PMEMPOOLSET",
                      "20M " + local_config->GetTestDir() + "replica1.part0"},
                     {"REPLICA",
                      "20M " + local_config->GetTestDir() + "pool.part0"}}}}));
}
