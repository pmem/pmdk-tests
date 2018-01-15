/*
 * Copyright 2018-2019, Intel Corporation
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

namespace transform {

/**
 * TransformInvalidTests.PMEMPOOL_TRANSFORM_API
 * Execution of pmempool_transform API on source/destination pool sets:
 * - by adding a replica twice, second time with usable (net) size smaller than
 * master replica
 * - remove secondary replica and add secondary replica with one part smaller
 * than PMEMOBJ_MIN_PART
 * - pool set file format is invalid (no PMEMPOOLSET string at the beginning)
 * - remove secondary replica and add secondary replica with less size than
 * poolset
 * \test
 *          \li \c Step1. Transform poolset with and without dry-run option
 * / SUCCESS
 *          \li \c Step2. Make sure that transformation was correct
 *          \li \c Step3. Transform poolset with and without dry-run option
 * / FAIL
 *          \li \c Step4. Make sure that the transition has not changed source
 * poolset
 */
TEST_P(TransformInvalidTests, PMEMPOOL_TRANSFORM_API) {
  /* Step1 */
  int ret = pmempool_transform(GetParam().poolset_src.GetFullPath().c_str(),
                               GetParam().poolset_temp.GetFullPath().c_str(),
                               PMEMPOOL_TRANSFORM_DRY_RUN);
  EXPECT_EQ(0, ret) << "PMEMPOOL_TRANSFORM_DRY_RUN failed: " << strerror(errno);

  ret = pmempool_transform(GetParam().poolset_src.GetFullPath().c_str(),
                           GetParam().poolset_temp.GetFullPath().c_str(), 0);

  ASSERT_EQ(0, ret) << "transformation failed " << strerror(errno);
  /* Step2 */
  ASSERT_EQ(0, file_utils::ValidatePoolset(GetParam().poolset_temp,
                                           GetParam().obj_pool.mode));
  /* Step3 */
  ret = pmempool_transform(GetParam().poolset_temp.GetFullPath().c_str(),
                           GetParam().poolset_dst.GetFullPath().c_str(),
                           PMEMPOOL_TRANSFORM_DRY_RUN);
  EXPECT_EQ(-1, ret) << "PMEMPOOL_TRANSFORM_DRY_RUN passed but should not";

  ret = pmempool_transform(GetParam().poolset_temp.GetFullPath().c_str(),
                           GetParam().poolset_dst.GetFullPath().c_str(), 0);

  ASSERT_EQ(-1, ret) << "transformation passed but should not";
  /* Step4 */
  ASSERT_EQ(0, file_utils::ValidatePoolset(GetParam().poolset_temp,
                                           GetParam().obj_pool.mode));
}

/**
 * TransformInvalidTests.PMEMPOOL_TRANSFORM_CLI
 * Execution of pmempool_transform CLI on source/destination pool sets:
 * - by adding a replica twice, second time with usable (net) size smaller than
 * master replica
 * - remove secondary replica and add secondary replica with one part smaller
 * than PMEMOBJ_MIN_PART
 * - pool set file format is invalid (no PMEMPOOLSET string at the beginning)
 * - remove secondary replica and add secondary replica with less size than
 * poolset
 * \test
 *          \li \c Step1. Transform poolset with and without dry-run option
 * / SUCCESS
 *          \li \c Step2. Make sure that transformation was correct
 *          \li \c Step3. Transform poolset with and without dry-run option
 * / FAIL
 *          \li \c Step4. Make sure that the transition has not changed source
 * poolset
 */
TEST_P(TransformInvalidTests, PMEMPOOL_TRANSFORM_CLI) {
  /* Step1 */
  int ret = TransformPoolCLI(GetParam().poolset_src.GetFullPath().c_str(),
                             GetParam().poolset_temp.GetFullPath().c_str(),
                             {{Option::DryRun, OptionType::Long}});
  EXPECT_EQ(0, ret) << "PMEMPOOL_TRANSFORM_DRY_RUN failed: " << GetOutputContent();
  ret = TransformPoolCLI(GetParam().poolset_src.GetFullPath().c_str(),
                         GetParam().poolset_temp.GetFullPath().c_str(),
                         {{Option::DryRun, OptionType::Short}});
  EXPECT_EQ(0, ret) << "PMEMPOOL_TRANSFORM_DRY_RUN failed: " << GetOutputContent();

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
  EXPECT_EQ(1, ret) << "PMEMPOOL_TRANSFORM_DRY_RUN passed but should not";
  ret = TransformPoolCLI(GetParam().poolset_temp.GetFullPath().c_str(),
                         GetParam().poolset_dst.GetFullPath().c_str(),
                         {{Option::DryRun, OptionType::Short}});
  EXPECT_EQ(1, ret) << "PMEMPOOL_TRANSFORM_DRY_RUN passed but should not";

  ret = TransformPoolCLI(GetParam().poolset_temp.GetFullPath().c_str(),
                         GetParam().poolset_dst.GetFullPath().c_str());

  ASSERT_EQ(1, ret) << "transformation passed but should not";
  /* Step4 */
  ASSERT_EQ(0, file_utils::ValidatePoolset(GetParam().poolset_temp,
                                           GetParam().obj_pool.mode));
}

INSTANTIATE_TEST_CASE_P(
    PmempoolTransformParam, TransformInvalidTests,
    ::testing::Values(
        /* Adding a replica twice, second time with usable (net) size smaller
           than master replica*/
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
                    {{"PMEMPOOLSET", "20M"},
                     {"REPLICA", "10M", "20M"},
                     {"REPLICA", "10M", "10M"}}}},
        /* remove secondary replica and add secondary replica with one part
           smaller than PMEMOBJ_MIN_PART */
        TransformArgs{ObjPool{local_config->GetTestDir() + POOLSET_PATH_SRC},
                      Poolset{local_config->GetTestDir(),
                              POOLSET_PATH_SRC,
                              {{"PMEMPOOLSET", "20M"},
                               {"REPLICA", std::to_string(PMEMOBJ_MIN_POOL)}}},
                      Poolset{local_config->GetTestDir(),
                              POOLSET_PATH_TEMP,
                              {{"PMEMPOOLSET", "20M"}}},
                      Poolset{local_config->GetTestDir(),
                              POOLSET_PATH_DST,
                              {{"PMEMPOOLSET", "20M"},
                               {"REPLICA", "10M", "10M",
                                std::to_string(PMEMOBJ_MIN_PART / 2)}}}},
        /* pool set file format is invalid (no PMEMPOOLSET string at the
           beginning) */
        TransformArgs{
            ObjPool{local_config->GetTestDir() + POOLSET_PATH_SRC},
            Poolset{local_config->GetTestDir(),
                    POOLSET_PATH_SRC,
                    {{"PMEMPOOLSET", "20M", "20M"}, {"REPLICA", "10M"}}},
            Poolset{local_config->GetTestDir(),
                    POOLSET_PATH_TEMP,
                    {{"PMEMPOOLSET", "20M", "20M"}}},
            Poolset{local_config->GetTestDir(),
                    POOLSET_PATH_DST,
                    {{"REPLICA",
                      "10M " + local_config->GetTestDir() + "replica1.part0"},
                     {"PMEMPOOLSET", "20M", "20M"}}}},
        /* remove secondary replica and add secondary replica with less size
           than poolset */
        TransformArgs{
            ObjPool{local_config->GetTestDir() + POOLSET_PATH_SRC},
            Poolset{local_config->GetTestDir(),
                    POOLSET_PATH_SRC,
                    {{"PMEMPOOLSET", "20M", "20M"}, {"REPLICA", "20M"}}},
            Poolset{local_config->GetTestDir(),
                    POOLSET_PATH_TEMP,
                    {{"PMEMPOOLSET", "20M", "20M"}}},
            Poolset{local_config->GetTestDir(),
                    POOLSET_PATH_DST,
                    {{"PMEMPOOLSET", "20M", "20M"}, {"REPLICA", "19M"}}}}));

/**
 * TransformInvalidBasicTests.PMEMPOOL_TRANSFORM_API
 * Execution of pmempool_transform API on source/destination pool sets:
 * - transforming the same poolset file
 * - adding secondary replica containing part from master replica
 * - remove secondary replica and add replica as primary replica
 * - remove and add replica in one step
 * - adding secondary replica with wrong content
 * - adding secondary replica containing two parts with the same name
 * - reordering parts in primary replica
 * - reordering replicas
 * \test
 *          \li \c Step1. Transform poolset with and without dry-run option
 * / FAIL
 *          \li \c Step2. Make sure that the transition has not changed source
 * poolset
 */
TEST_P(TransformInvalidBasicTests, PMEMPOOL_TRANSFORM_API) {
  /* Step1 */
  int ret = pmempool_transform(GetParam().poolset_src.GetFullPath().c_str(),
                               GetParam().poolset_dst.GetFullPath().c_str(),
                               PMEMPOOL_TRANSFORM_DRY_RUN);
  EXPECT_EQ(-1, ret) << "PMEMPOOL_TRANSFORM_DRY_RUN failed: " << strerror(errno);

  ret = pmempool_transform(GetParam().poolset_src.GetFullPath().c_str(),
                           GetParam().poolset_dst.GetFullPath().c_str(), 0);

  ASSERT_EQ(-1, ret) << "transformation passed but should not";
  /* Step2 */
  ASSERT_EQ(0, file_utils::ValidatePoolset(GetParam().poolset_src,
                                           GetParam().obj_pool.mode));
}

/**
 * TransformInvalidBasicTests.PMEMPOOL_TRANSFORM_CLI
 * Execution of pmempool_transform CLI on source/destination pool sets:
 * - transformaing the same poolset file
 * - adding secondary replica containing part from master replica
 * - remove and add replica in one step
 * - adding secondary replica with wrong content
 * - adding secondary replica containing two parts with the same name
 * - reordering parts in primary replica
 * - reordering replicas
 * \test
 *          \li \c Step1. Transform poolset with and without dry-run option
 * / FAIL
 *          \li \c Step2. Make sure that the transition has not changed source
 * poolset
 */
TEST_P(TransformInvalidBasicTests, PMEMPOOL_TRANSFORM_CLI) {
  /* Step1 */
  int ret = TransformPoolCLI(GetParam().poolset_src.GetFullPath().c_str(),
                             GetParam().poolset_dst.GetFullPath().c_str(),
                             {{Option::DryRun, OptionType::Long}});
  EXPECT_EQ(1, ret) << "PMEMPOOL_TRANSFORM_DRY_RUN failed: " << GetOutputContent();
  ret = TransformPoolCLI(GetParam().poolset_src.GetFullPath().c_str(),
                         GetParam().poolset_dst.GetFullPath().c_str(),
                         {{Option::DryRun, OptionType::Short}});
  EXPECT_EQ(1, ret) << "PMEMPOOL_TRANSFORM_DRY_RUN failed: " << GetOutputContent();

  ret = TransformPoolCLI(GetParam().poolset_src.GetFullPath().c_str(),
                         GetParam().poolset_dst.GetFullPath().c_str());

  ASSERT_EQ(1, ret) << "transformation failed " << GetOutputContent();
  /* Step2 */
  ASSERT_EQ(0, file_utils::ValidatePoolset(GetParam().poolset_src,
                                           GetParam().obj_pool.mode));
}

INSTANTIATE_TEST_CASE_P(
    PmempoolTransformParamBasic, TransformInvalidBasicTests,
    ::testing::Values(
        /* Transformaing the same poolset file */
        TransformArgs{ObjPool{local_config->GetTestDir() + POOLSET_PATH_SRC},
                      Poolset{local_config->GetTestDir(),
                              POOLSET_PATH_SRC,
                              {{"PMEMPOOLSET", "20M"}}},
                      Poolset{local_config->GetTestDir(),
                              POOLSET_PATH_DST,
                              {{"PMEMPOOLSET", "20M"}}}},
        /* Adding secondary replica containing part from master replica */
        TransformArgs{
            ObjPool{local_config->GetTestDir() + POOLSET_PATH_SRC},
            Poolset{local_config->GetTestDir(),
                    POOLSET_PATH_SRC,
                    {{"PMEMPOOLSET", "20M", "20M"}}},
            Poolset{local_config->GetTestDir(),
                    POOLSET_PATH_DST,
                    {{"PMEMPOOLSET", "20M", "20M"},
                     {"REPLICA", "20M",
                      "20M " + local_config->GetTestDir() + "pool.part1"}}}},
        /* Remove and add replica in one step */
        TransformArgs{ObjPool{local_config->GetTestDir() + POOLSET_PATH_SRC},
                      Poolset{local_config->GetTestDir(),
                              POOLSET_PATH_SRC,
                              {{"PMEMPOOLSET", "20M", "20M", "20M"},
                               {"REPLICA", "20M", "20M"}}},
                      Poolset{local_config->GetTestDir(),
                              POOLSET_PATH_DST,
                              {{"PMEMPOOLSET", "20M", "20M"},
                               {"REPLICA", "20M", "20M"}}}},
        /* Adding secondary replica with wrong content */
        TransformArgs{ObjPool{local_config->GetTestDir() + POOLSET_PATH_SRC},
                      Poolset{local_config->GetTestDir(),
                              POOLSET_PATH_SRC,
                              {{"PMEMPOOLSET", "20M", "20M"}}},
                      Poolset{local_config->GetTestDir(),
                              POOLSET_PATH_DST,
                              {{"PMEMPOOLSET", "20M", "20M"},
                               {"REPLICA", "20M", "20Mb"}}}},
        /* Adding secondary replica containing two parts with the same name */
        TransformArgs{
            ObjPool{local_config->GetTestDir() + POOLSET_PATH_SRC},
            Poolset{local_config->GetTestDir(),
                    POOLSET_PATH_SRC,
                    {{"PMEMPOOLSET", "20M", "20M"}}},
            Poolset{local_config->GetTestDir(),
                    POOLSET_PATH_DST,
                    {{"PMEMPOOLSET", "20M", "20M"},
                     {"REPLICA", "20M", "22M " + local_config->GetTestDir() +
                                            "replica1.part0"}}}},
        /* Reordering parts in primary replica */
        TransformArgs{
            ObjPool{local_config->GetTestDir() + POOLSET_PATH_SRC},
            Poolset{local_config->GetTestDir(),
                    POOLSET_PATH_SRC,
                    {{"PMEMPOOLSET", "20M", "20M"}, {"REPLICA", "20M", "20M"}}},
            Poolset{local_config->GetTestDir(),
                    POOLSET_PATH_DST,
                    {{"PMEMPOOLSET",
                      "20M " + local_config->GetTestDir() + "pool.part1",
                      "20M " + local_config->GetTestDir() + "pool.part0"},
                     {"REPLICA", "20M", "20M"}}}},
        /* Reordering replicas */
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
                      "20M " + local_config->GetTestDir() + "replica2.part0"},
                     {"REPLICA",
                      "20M " + local_config->GetTestDir() + "pool.part0"}}}}));
}
