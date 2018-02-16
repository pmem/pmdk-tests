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

#include "ext_cfg.h"
#include "alloc_class_utils.h"

using namespace std;

/**
 * PMEMOBJ_CTL_ALLOC_CLASS_FROM_EXT_CFG
 * Creating allocation classes from external configurations via:
 * - PMEMOBJ_CONF environment variable
 * - configuration file pointed by PMEMOBJ_CONF_FILE environment variable
 * Following scenarios are considered:
 * - class of ids: 128, 254, retrieved automatically
 * - header types: POBJ_HEADER_NONE, POBJ_HEADER_LEGACY, POBJ_HEADER_COMPACT
 * \test
 *          \li \c Step1. Create valid ctl query / SUCCESS
 *          \li \c Step2. Set selected environment variable / SUCCESS
 *          \li \c Step3. Create configuration file if PMEMOBJ_CONF_FILE is set
 *          / SUCCESS
 *          \li \c Step4. Create pmemobj pool / SUCCESS
 *          \li \c Step5. Create allocation class / SUCCESS
 *          \li \c Step6. Validate allocation class / SUCCESS
 *          \li \c Step7. Allocate an object from created allocation class
 *          / SUCCESS
 *          \li \c Step8. Make sure object is allocated
 *          \li \c Step9. Close pool / SUCCESS
 */
TEST_P(ObjCtlExtCfgPosTest, PMEMOBJ_CTL_ALLOC_CLASS_FROM_EXT_CFG) {
  /* Step 4 */
  PMEMobjpool *pop =
      pmemobj_create(pool_path_.c_str(), nullptr, PMEMOBJ_MIN_POOL, 0666);
  ASSERT_TRUE(pop != nullptr) << pmemobj_errormsg();
  pobj_alloc_class_desc read_arg;
  PMEMoid oid = OID_NULL;
  if (write_arg_.class_id == auto_class_id) {
    int id =
        GetAllocClassId(pop, write_arg_.unit_size, write_arg_.units_per_block,
                        write_arg_.header_type);
    EXPECT_NE(-1, id);
    write_arg_.class_id = id;
  }
  string entry_point =
      "heap.alloc_class." + to_string(write_arg_.class_id) + ".desc";
  /* Step 5 */
  EXPECT_EQ(0, pmemobj_ctl_get(pop, entry_point.c_str(), &read_arg));
  /* Step 6 */
  EXPECT_TRUE(AllocClassUtils::IsAllocClassValid(write_arg_, read_arg));
  /* Step 7 */
  EXPECT_EQ(
      0, pmemobj_xalloc(
             pop, &oid, read_arg.unit_size -
                            AllocClassUtils::hdrs[write_arg_.header_type].size,
             0, POBJ_CLASS_ID(write_arg_.class_id), nullptr, nullptr))
      << pmemobj_errormsg();
  /* Step 8 */
  EXPECT_FALSE(OID_IS_NULL(oid));
  pmemobj_free(&oid);
  /* Step 9 */
  pmemobj_close(pop);
}

INSTANTIATE_TEST_CASE_P(
    ParamTest, ObjCtlExtCfgPosTest,
    ::testing::Combine(
        ::testing::Values(
            pobj_alloc_class_desc{512, 1024, POBJ_HEADER_COMPACT, 128},
            pobj_alloc_class_desc{128, 128, POBJ_HEADER_LEGACY, 254},
            pobj_alloc_class_desc{8, 1024, POBJ_HEADER_NONE, 254},
            pobj_alloc_class_desc{512, 1023, POBJ_HEADER_NONE, auto_class_id},
            pobj_alloc_class_desc{512, 1023, POBJ_HEADER_LEGACY, auto_class_id},
            pobj_alloc_class_desc{512, 1023, POBJ_HEADER_COMPACT,
                                  auto_class_id}),
        ::testing::Values(ExternalCfg::FROM_ENV_VAR,
                          ExternalCfg::FROM_CFG_FILE)));

/**
 * PMEMOBJ_CTL_ALLOC_CLASS_FROM_EXT_CFG
 * Creating allocation classes from external configurations via:
 * - PMEMOBJ_CONF environment variable
 * - configuration file pointed by PMEMOBJ_CONF_FILE environment variable
 * Following scenarios are considered:
 * - unit sizes: 0, PMEMOBJ_MAX_ALLOC_SIZE + 1
 * - class of ids: 0, 255
 * - units per block: 0, greater than internal bitmap size(i.e. 2432)
 * - header types: invalid
 * \test
 *          \li \c Step1. Create valid ctl query / SUCCESS
 *          \li \c Step2. Set selected environment variable / SUCCESS
 *          \li \c Step3. Create configuration file if PMEMOBJ_CONF_FILE is set
 *          / SUCCESS
 *          \li \c Step4. Create pmemobj pool / FAIL: ret = -1, errno = EINVAL
 */
TEST_P(ObjCtlExtCfgNegTest, PMEMOBJ_CTL_ALLOC_CLASS_FROM_EXT_CFG) {
  errno = 0;
  PMEMobjpool *pop =
      pmemobj_create(pool_path_.c_str(), nullptr, PMEMOBJ_MIN_POOL, 0666);
  EXPECT_EQ(nullptr, pop);
  EXPECT_EQ(errno, EINVAL);
}

INSTANTIATE_TEST_CASE_P(
    ParamTest, ObjCtlExtCfgNegTest,
    ::testing::Combine(
        ::testing::Values(
            pobj_alloc_class_desc{0, 1024, POBJ_HEADER_COMPACT, 128},
            pobj_alloc_class_desc{PMEMOBJ_MAX_ALLOC_SIZE + 1, 1024,
                                  POBJ_HEADER_COMPACT, 128},
            pobj_alloc_class_desc{1024, 1024, POBJ_HEADER_COMPACT, 0},
            pobj_alloc_class_desc{1024, 1024, POBJ_HEADER_COMPACT, 255},
            pobj_alloc_class_desc{1024, 0, POBJ_HEADER_COMPACT, auto_class_id},
            pobj_alloc_class_desc{1024, 0, POBJ_HEADER_COMPACT, 128},
            pobj_alloc_class_desc{128, 4096, POBJ_HEADER_COMPACT, 128},
            pobj_alloc_class_desc{1024, 1024, MAX_POBJ_HEADER_TYPES, 128}),
        ::testing::Values(ExternalCfg::FROM_ENV_VAR,
                          ExternalCfg::FROM_CFG_FILE)));
