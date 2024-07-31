/*
 * Copyright 2018-2023, Intel Corporation
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

#include "local_basic_tests.h"

void UnsafeShutdownBasic::SetUp() {
  ASSERT_LE(1, test_phase_.GetUnsafeDimmNamespaces().size())
      << "Insufficient number of dimms to run this test";
  us_dimm_pool_path_ = test_phase_.GetUnsafeDimmNamespaces()[0].GetTestDir() +
                       GetNormalizedTestName() + "_pool";
}
bool sds_state;

/**
 * TRY_OPEN_OBJ
 * Create obj pool on DIMM, trigger unsafe shutdown, try opening the pool
 * \test
 *          \li \c Step1. Create an obj pool on DIMM / SUCCESS
 *          \li \c Step2. Write pattern to pool / SUCCESS
 *          \li \c Step3. Trigger US, run power cycle, check USC values / SUCCESS
 *          \li \c Step4. Open the pool / FAIL: pop = NULL, errno = EINVAL
 *          \li \c Step5. Repair and open the pool / SUCCESS
 *          \li \c Step6. Verify written pattern / SUCCESS
 *          \li \c Step7. Close the pool / SUCCESS
 */
TEST_F(UnsafeShutdownBasic, TRY_OPEN_OBJ_phase_1) {
  /* Step1 */
  pop_ = pmemobj_create(us_dimm_pool_path_.c_str(), nullptr, PMEMOBJ_MIN_POOL,
                        0644);
  ASSERT_TRUE(pop_ != nullptr) << "Pool creating failed. Errno: " << errno
                               << std::endl
                               << pmemobj_errormsg();

  /* first SDS */
  std::cout << "Befre get: " << sds_state << std::endl;
  /* check SDS state */
  pmemobj_ctl_get(pop_ , "sds.at_create", &sds_state);
  std::cout << "1| SDS after create: " << sds_state << std::endl;

  /* Step2 */
  ObjData<int> pd{pop_};
  ASSERT_EQ(0, pd.Write(obj_data_)) << "Writing to pool failed";
}

/* Step3. outside of test macros */

TEST_F(UnsafeShutdownBasic, TRY_OPEN_OBJ_phase_2) {
  ASSERT_TRUE(PassedOnPreviousPhase()) << "Part of test before shutdown failed";

  /* Step4 */
  /* check SDS state */
  pmemobj_ctl_get(NULL , "sds.at_create", &sds_state);
  std::cout << "SDS before open: " << sds_state << std::endl;

  pop_ = pmemobj_open(us_dimm_pool_path_.c_str(), nullptr);
  ASSERT_EQ(nullptr, pop_)
      << "Pool was opened after unsafe shutdown but should be not";
  ASSERT_EQ(EINVAL, errno);

  /* Step5 */
  ASSERT_EQ(PMEMPOOL_CHECK_RESULT_REPAIRED, PmempoolRepair(us_dimm_pool_path_))
      << "Pool was not repaired";
  pop_ = pmemobj_open(us_dimm_pool_path_.c_str(), nullptr);
  ASSERT_TRUE(pop_ != nullptr) << "Pool opening failed. Errno: " << errno
                               << std::endl
                               << pmemobj_errormsg();

  /* Step6 */
  ObjData<int> pd{pop_};
  ASSERT_EQ(obj_data_, pd.Read()) << "Data read from pool differs from written";
}
