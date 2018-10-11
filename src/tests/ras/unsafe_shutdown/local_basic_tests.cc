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

#include "local_basic_tests.h"

void UnsafeShutdownBasic::SetUp() {
  ASSERT_LE(1, test_phase_.GetUnsafeDimmNamespaces().size())
      << "Insufficient number of dimms to run this test";
  us_dimm_pool_path_ = test_phase_.GetUnsafeDimmNamespaces()[0].GetTestDir() +
                       GetNormalizedTestName() + "_pool";
}

/**
 * TRY_OPEN_OBJ
 * Create obj pool on DIMM, trigger unsafe shutdown, try opening the pool
 * \test
 *          \li \c Step1. Create an obj pool on DIMM / SUCCESS
 *          \li \c Step2. Enable SDS feature on pool / SUCCESS
 *          \li \c Step3. Write pattern to pool / SUCCESS
 *          \li \c Step4. Trigger US, run power cycle, check USC values / SUCCESS
 *          \li \c Step5. Open the pool / FAIL: pop = NULL, errno = EINVAL
 *          \li \c Step6. Repair and open the pool / SUCCESS
 *          \li \c Step7. Verify written pattern / SUCCESS
 *          \li \c Step8. Close the pool / SUCCESS
 */
TEST_F(UnsafeShutdownBasic, TRY_OPEN_OBJ_phase_1) {
  /* Step1 */
  ASSERT_EQ(0, ObjCreateHelper(us_dimm_pool_path_, PMEMOBJ_MIN_POOL));
  /* Step2 */
  ObjData<int> pd{pop_};
  ASSERT_EQ(0, pd.Write(obj_data_)) << "Writing to pool failed";
}

/* Step3. outside of test macros */

TEST_F(UnsafeShutdownBasic, TRY_OPEN_OBJ_phase_2) {
  ASSERT_TRUE(PassedOnPreviousPhase()) << "Part of test before shutdown failed";

  /* Step4 */
  ASSERT_EQ(0, ObjOpenFailureHelper(us_dimm_pool_path_, EINVAL))
      << "Pool unexpectedly opened after unsafe shutdown";

  /* Step6 */
  ASSERT_EQ(PMEMPOOL_CHECK_RESULT_REPAIRED, PmempoolRepair(us_dimm_pool_path_))
      << "Pool was not repaired";

  ASSERT_EQ(0, ObjOpenSuccessHelper(us_dimm_pool_path_))
      << "Pool could not be opened after succesful repair";

  /* Step7 */
  ObjData<int> pd{pop_};
  ASSERT_EQ(obj_data_, pd.Read()) << "Data read from pool differs from written";
}

/**
 * TRY_OPEN_BLK
 * Create blk pool on DIMM, trigger unsafe shutdown, try opening the pool
 * \test
 *          \li \c Step1. Create a blk pool on DIMM / SUCCESS
 *          \li \c Step2. Enable SDS feature on pool / SUCCESS
 *          \li \c Step3. Write pattern to pool / SUCCESS
 *          \li \c Step4. Trigger US, run power cycle, check USC values / SUCCESS
 *          \li \c Step5. Open the pool / FAIL: pbp = NULL, errno = EINVAL
 *          \li \c Step6. Repair and open the pool / SUCCESS
 *          \li \c Step7. Verify written pattern / SUCCESS
 *          \li \c Step8. Close the pool / SUCCESS
 */
TEST_F(UnsafeShutdownBasic, TRY_OPEN_BLK_phase_1) {
  /* Step1 */
  pbp_ = pmemblk_create(us_dimm_pool_path_.c_str(), blk_size_, PMEMBLK_MIN_POOL,
                        0644 & PERMISSION_MASK);
  ASSERT_TRUE(pbp_ != nullptr) << "Pool creating failed. Errno: " << errno
                               << std::endl
                               << pmemblk_errormsg();
  /* Step3 */
  BlkData<int> pd{pbp_};
  ASSERT_EQ(0, pd.Write(blk_data_)) << "Writing to pool failed";
}

/* Step4. outside of test macros */

TEST_F(UnsafeShutdownBasic, TRY_OPEN_BLK_phase_2) {
  ASSERT_TRUE(PassedOnPreviousPhase());

  /* Step5 */
  pbp_ = pmemblk_open(us_dimm_pool_path_.c_str(), blk_size_);
  ASSERT_EQ(nullptr, pbp_)
      << "Pool was opened after unsafe shutdown but should be not";
  ASSERT_EQ(EINVAL, errno);

  /* Step6 */
  ASSERT_EQ(PMEMPOOL_CHECK_RESULT_REPAIRED, PmempoolRepair(us_dimm_pool_path_))
      << "Pool was not repaired";
  pbp_ = pmemblk_open(us_dimm_pool_path_.c_str(), blk_size_);
  ASSERT_TRUE(pbp_ != nullptr) << "Pool opening failed. Errno: " << errno
                               << std::endl
                               << pmemobj_errormsg();

  /* Step7 */
  BlkData<int> pd{pbp_};
  ASSERT_EQ(blk_data_, pd.Read(blk_data_.size()))
      << "Data read from pool differs from written";
}

/**
 * TRY_OPEN_LOG
 * Create log pool on DIMM, trigger unsafe shutdown, try opening the pool
 * \test
 *          \li \c Step1. Create a log pool on DIMM / SUCCESS
 *          \li \c Step2. Enable SDS feature on pool / SUCCESS
 *          \li \c Step3. Write pattern to pool / SUCCESS
 *          \li \c Step4. Trigger US, run power cycle, check USC values / SUCCESS
 *          \li \c Step5. Open the pool / FAIL: plp = NULL, errno = EINVAL
 *          \li \c Step6. Repair and open the pool / SUCCESS
 *          \li \c Step7. Verify written pattern / SUCCESS
 *          \li \c Step8. Close the pool / SUCCESS
 */
TEST_F(UnsafeShutdownBasic, TRY_OPEN_LOG_phase_1) {
  /* Step1 */
  plp_ = pmemlog_create(us_dimm_pool_path_.c_str(), PMEMLOG_MIN_POOL,
                        0644 & PERMISSION_MASK);
  ASSERT_TRUE(plp_ != nullptr) << "Pool creating failed. Errno: " << errno
                               << std::endl
                               << pmemlog_errormsg();

  /* Step3 */
  LogData pd{plp_};
  ASSERT_EQ(0, pd.Write(log_data_)) << "Writing to pool failed";
}

/* Step4. outside of test macros */

TEST_F(UnsafeShutdownBasic, TRY_OPEN_LOG_phase_2) {
  ASSERT_TRUE(PassedOnPreviousPhase()) << "Part of test before shutdown failed";

  /* Step5 */
  plp_ = pmemlog_open(us_dimm_pool_path_.c_str());
  ASSERT_EQ(nullptr, plp_)
      << "Pool was opened after unsafe shutdown but should be not";
  ASSERT_EQ(EINVAL, errno);

  /* Step6 */
  ASSERT_EQ(PMEMPOOL_CHECK_RESULT_REPAIRED, PmempoolRepair(us_dimm_pool_path_))
      << "Pool was not repaired";
  plp_ = pmemlog_open(us_dimm_pool_path_.c_str());
  ASSERT_TRUE(plp_ != nullptr) << "Pool opening failed. Errno: " << errno
                               << std::endl
                               << pmemobj_errormsg();

  /* Step7 */
  LogData pd{plp_};
  ASSERT_EQ(log_data_, pd.Read()) << "Data read from pool differs from written";
}

/**
 * TC_TRY_OPEN_AFTER_DOUBLE_US
 * Create pool on DIMM, trigger unsafe shutdown twice, try opening the
 * pool
 * \test
 *          \li \c Step1. Create a pool on DIMM / SUCCESS
 *          \li \c Step2. Enable SDS / SUCCESS
 *          \li \c Step3. Write pattern to pool / SUCCESS
 *          \li \c Step4. Trigger US, power cycle, confirm USC is incremented -
 * repeat twice / SUCCESS
 *          \li \c Step5. Open the pool / FAIL: pop = NULL, errno = EINVAL
 *          \li \c Step6. Repair and open the pool / SUCCESS
 *          \li \c Step7. Verify pattern / SUCCESS
 *          \li \c Step8. Close the pool / SUCCESS
 */
TEST_F(UnsafeShutdownBasic, TC_TRY_OPEN_AFTER_DOUBLE_US_phase_1) {
  /* Step1 */
  ASSERT_EQ(ObjCreateHelper(us_dimm_pool_path_, PMEMOBJ_MIN_POOL), 0);

  /* Step3 */
  ObjData<int> pd{pop_};
  ASSERT_EQ(0, pd.Write(obj_data_)) << "Writing to pool failed";
}

TEST_F(UnsafeShutdownBasic, TC_TRY_OPEN_AFTER_DOUBLE_US_phase_2) {
  ASSERT_TRUE(PassedOnPreviousPhase()) << "Part of test before shutdown failed";
}

/* Step4. - outside of test macros */

TEST_F(UnsafeShutdownBasic, TC_TRY_OPEN_AFTER_DOUBLE_US_phase_3) {
  ASSERT_TRUE(PassedOnPreviousPhase()) << "Part of test before shutdown failed";

  /* step5 */
  ASSERT_EQ(0, ObjOpenFailureHelper(us_dimm_pool_path_, EINVAL))
      << "Pool after USC unexpectedly opened";

  /* Step6 */
  ASSERT_EQ(PMEMPOOL_CHECK_RESULT_REPAIRED,
            PmempoolRepair(us_dimm_pool_path_.c_str()))
      << "Pool was not repaired";
  ASSERT_EQ(0, ObjOpenSuccessHelper(us_dimm_pool_path_))
      << "Pool not opened after succesful repair";

  /* Step7 */
  ObjData<int> pd{pop_};
  ASSERT_EQ(obj_data_, pd.Read()) << "Data read from pool differs from written";
}

/**
 * TC_OPEN_CLEAN
 * Create pool on DIMM, close, trigger unsafe shutdown, open the pool
 * \test
 *          \li \c Step1. Create a pool on DIMM. \ SUCCESS
 *          \li \c Step2. Enable SDS / SUCCESS
 *          \li \c Step3. Write data to pool. \ SUCCESS
 *          \li \c Step4. Close pool. \ SUCCESS
 *          \li \c Step5. Increment USC, power cycle, confirm USC is incremented /
 *          SUCCESS
 *          \li \c Step6. Open the pool. / SUCCESS
 *          \li \c Step7. Verify written pattern / SUCCESS
 *          \li \c Step8. Close the pool / SUCCESS
 */
TEST_F(UnsafeShutdownBasicClean, TC_OPEN_CLEAN_phase_1) {
  /* Step1 */
  ASSERT_EQ(0, ObjCreateHelper(us_dimm_pool_path_, PMEMOBJ_MIN_POOL));

  /* Step3 */
  ObjData<int> pd{pop_};
  ASSERT_EQ(0, pd.Write(obj_data_)) << "Writing to pool failed";
}

/* Step4, 5 outside of test macros */

TEST_F(UnsafeShutdownBasicClean, TC_OPEN_CLEAN_phase_2) {
  ASSERT_TRUE(PassedOnPreviousPhase()) << "Part of test before shutdown failed";

  /* Step6 */
  ASSERT_EQ(0, ObjOpenSuccessHelper(us_dimm_pool_path_));

  /* Step7 */
  ObjData<int> pd{pop_};
  ASSERT_EQ(obj_data_, pd.Read()) << "Data read from pool differs from written";
}

void UnsafeShutdownBasicWithoutUS::SetUp() {
  ASSERT_LE(1, test_phase_.GetSafeDimmNamespaces().size())
      << "Insufficient number of dimms to run this test";
  non_us_dimm_pool_path_ = test_phase_.GetSafeDimmNamespaces()[0].GetTestDir() +
                           GetNormalizedTestName() + "_pool";
}

/*
* TC_OPEN_DIRTY_NO_US
* Create pool on DIMM, write data, end process without closing the pool, open
* the pool, confirm data.
* \test
*          \li \c Step1. Create a pool on DIMM / SUCCESS
*          \li \c Step2. Enable SDS / SUCCESS
*          \li \c Step3. Write pattern to pool / SUCCESS
*          \li \c Step4. Open the pool / SUCCESS
*          \li \c Step5. Verify pattern / SUCCESS
*/
TEST_F(UnsafeShutdownBasicWithoutUS, TC_OPEN_DIRTY_NO_US_phase_1) {
  /* Step1 */
  ASSERT_EQ(0, ObjCreateHelper(non_us_dimm_pool_path_, PMEMOBJ_MIN_POOL));

  /* Step3 */
  ObjData<int> pd{pop_};
  ASSERT_EQ(0, pd.Write(obj_data_)) << "Writing to pool failed";
}

TEST_F(UnsafeShutdownBasicWithoutUS, TC_OPEN_DIRTY_NO_US_phase_2) {
  ASSERT_TRUE(PassedOnPreviousPhase()) << "Part of test before shutdown failed";

  /* Step3 */
  ASSERT_EQ(0, ObjOpenSuccessHelper(non_us_dimm_pool_path_))
      << "Opening pool after safe shutdown failed";

  /* Step4 */
  ObjData<int> pd{pop_};
  ASSERT_EQ(obj_data_, pd.Read()) << "Data read from pool differs from written";
}
