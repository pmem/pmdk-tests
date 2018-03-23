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

#include "us_basic_tests.h"

void UnsafeShutdownBasic::SetUp() {
  ASSERT_GE(us_dimms.size(), 1)
      << "Test needs more dimms to run than was specified.";
  us_dimm_pool_path_ = us_dimms.front().GetMountpoint() + SEPARATOR +
                       GetNormalizedTestName() + "_pool";
  us_dimm_collections_ = {us_dimms.front()};
}

/**
 * TRY_OPEN_OBJ
 * Create obj pool on DIMM, trigger unsafe shutdown, try opening the pool
 * \test
 *          \li \c Step1. Create a pool on DIMM / SUCCESS
 *          \li \c Step2. Write pattern to pool / SUCCESS
 *          \li \c Step3. Trigger US, reboot, /SUCCESS
 *          \li \c Step4. Confirm USC incremented /SUCCESS
 *          \li \c Step5. Open the pool / FAIL
 *          \li \c Step6. Force open the pool / SUCCESS
 *          \li \c Step7. Verify written pattern / SUCCESS
 */
TEST_F(UnsafeShutdownBasic, TRY_OPEN_OBJ_before_us) {
  /* Step1. */
  pop_ = pmemobj_create(us_dimm_pool_path_.c_str(), nullptr, PMEMOBJ_MIN_POOL,
                        0644);
  ASSERT_NE(pop_, nullptr) << "Pool creating failed" << std::endl
                           << pmemobj_errormsg();

  /* Step2. */
  ObjData<int> pd{pop_};
  ASSERT_EQ(pd.WriteData(), 0) << "Writing to pool failed";

  /* Step3. */
  ASSERT_NO_FATAL_FAILURE(Inject());
}

TEST_F(UnsafeShutdownBasic, TRY_OPEN_OBJ_after_first_us) {
  close_pools_at_end_ = true;
  ASSERT_TRUE(PassedOnPreviousPhase())
      << "Part of test before shutdown failed.";

  /* Step4. */
  ASSERT_NO_FATAL_FAILURE(ConfirmRebootedWithUS());

  /* Step5. */
  ASSERT_EQ(pmemobj_open(us_dimm_pool_path_.c_str(), nullptr), nullptr)
      << "Pool was opened after unsafe shutdown, but should not.";

  /* Step6. */
  ASSERT_NO_FATAL_FAILURE(Repair(us_dimm_pool_path_));
  pop_ = pmemobj_open(us_dimm_pool_path_.c_str(), nullptr);
  ASSERT_NE(pop_, nullptr) << "Pool opening failed. Errno: " << errno
                           << std::endl
                           << pmemobj_errormsg();

  /* Step7 */
  ObjData<int> pd{pop_};
  ASSERT_EQ(pd.AssertDataCorrect(), 0) << "Reading data from pool failed";
}

/**
 * TRY_OPEN_BLK
 * Create blk pool on DIMM, trigger unsafe shutdown, try opening the pool
 * \test
 *          \li \c Step1. Create a pool on DIMM / SUCCESS
 *          \li \c Step2. Write pattern to pool / SUCCESS
 *          \li \c Step3. Trigger US, reboot, /SUCCESS
 *          \li \c Step4. Confirm USC incremented /SUCCESS
 *          \li \c Step5. Open the pool / FAIL
 *          \li \c Step6. Force open the pool / SUCCESS
 *          \li \c Step7. Verify written pattern / SUCCESS
 */
TEST_F(UnsafeShutdownBasic, TRY_OPEN_BLK_before_us) {
  /* Step1. */
  pbp_ = pmemblk_create(us_dimm_pool_path_.c_str(), blk_size_, PMEMBLK_MIN_POOL,
                        0666);
  ASSERT_NE(pbp_, nullptr) << "Pool creating failed" << std::endl
                           << pmemblk_errormsg();

  /* Step2. */
  BlkData pd{pbp_};
  ASSERT_EQ(pd.WriteData(), 0) << "Writing to pool failed";

  /* Step3. */
  ASSERT_NO_FATAL_FAILURE(Inject());
}

TEST_F(UnsafeShutdownBasic, TRY_OPEN_BLK_after_first_us) {
  close_pools_at_end_ = true;
  ASSERT_TRUE(PassedOnPreviousPhase())
      << "Part of test before shutdown failed.";

  /* Step4. */
  ASSERT_NO_FATAL_FAILURE(ConfirmRebootedWithUS());

  /* Step5. */
  ASSERT_EQ(pmemblk_open(us_dimm_pool_path_.c_str(), blk_size_), nullptr)
      << "Pool was opened after unsafe shutdown, but should not.";

  /* Step6. */
  ASSERT_NO_FATAL_FAILURE(Repair(us_dimm_pool_path_));
  pbp_ = pmemblk_open(us_dimm_pool_path_.c_str(), blk_size_);
  ASSERT_NE(pbp_, nullptr) << "Pool opening failed. Errno: " << errno
                           << std::endl
                           << pmemobj_errormsg();

  /* Step7 */
  BlkData pd{pbp_};
  ASSERT_EQ(pd.AssertDataCorrect(), 0) << "Reading data from pool failed";
}

/**
 * TRY_OPEN_LOG
 * Create log pool on DIMM, trigger unsafe shutdown, try opening the pool
 * \test
 *          \li \c Step1. Create a pool on DIMM / SUCCESS
 *          \li \c Step2. Write pattern to pool / SUCCESS
 *          \li \c Step3. Trigger US, reboot, /SUCCESS
 *          \li \c Step4. Confirm USC incremented /SUCCESS
 *          \li \c Step5. Open the pool / FAIL
 *          \li \c Step6. Force open the pool / SUCCESS
 *          \li \c Step7. Verify written pattern / SUCCESS
 */
TEST_F(UnsafeShutdownBasic, TRY_OPEN_LOG_before_us) {
  /* Step1. */
  plp_ = pmemlog_create(us_dimm_pool_path_.c_str(), PMEMLOG_MIN_POOL, 0666);
  ASSERT_NE(plp_, nullptr) << "Pool creating failed" << std::endl
                           << pmemlog_errormsg();

  /* Step2. */
  LogData pd{plp_};
  ASSERT_EQ(pd.WriteData(), 0) << "Writing to pool failed";

  /* Step3. */
  ASSERT_NO_FATAL_FAILURE(Inject());
}

TEST_F(UnsafeShutdownBasic, TRY_OPEN_LOG_after_first_us) {
  close_pools_at_end_ = true;
  ASSERT_TRUE(PassedOnPreviousPhase())
      << "Part of test before shutdown failed.";

  /* Step4. */
  ASSERT_NO_FATAL_FAILURE(ConfirmRebootedWithUS());

  /* Step5. */
  ASSERT_EQ(pmemlog_open(us_dimm_pool_path_.c_str()), nullptr)
      << "Pool was opened after unsafe shutdown, but should not.";

  /* Step6. */
  ASSERT_NO_FATAL_FAILURE(Repair(us_dimm_pool_path_));
  plp_ = pmemlog_open(us_dimm_pool_path_.c_str());
  ASSERT_NE(plp_, nullptr) << "Pool opening failed. Errno: " << errno
                           << std::endl
                           << pmemobj_errormsg();

  /* Step7 */
  LogData pd{plp_};
  ASSERT_EQ(pd.AssertDataCorrect(), 0) << "Reading data from pool failed";
}

/*
* TC_OPEN_DIRTY_NO_USC
* Create pool on DIMM, write data, end process without closing the pool, open
* the pool, confirm data.
* \test
*          \li \c Step1. Create a pool on DIMM / SUCCESS
*          \li \c Step2. Write pattern to pool / SUCCESS
*          \li \c Step3. Open the pool / SUCCESS
*          \li \c Step4. Verify pattern / SUCCESS
*/
TEST_F(UnsafeShutdownBasic, TC_OPEN_DIRTY_NO_USC_before_us) {
  /* Step1. */
  pop_ = pmemobj_create(us_dimm_pool_path_.c_str(), nullptr, PMEMOBJ_MIN_POOL,
                        0644);
  ASSERT_NE(pop_, nullptr) << "Pool creating failed. Errno: " << errno
                           << std::endl
                           << pmemobj_errormsg();
  /* Step2. */
  ObjData<int> pd{pop_};
  ASSERT_EQ(pd.WriteData(), 0) << "Writing to pool failed";
}

TEST_F(UnsafeShutdownBasic, TC_OPEN_DIRTY_NO_USC_after_first_us) {
  ASSERT_TRUE(PassedOnPreviousPhase())
      << "Part of test before shutdown failed.";

  /* Step3. */
  pop_ = pmemobj_open(us_dimm_pool_path_.c_str(), nullptr);
  ASSERT_NE(pop_, nullptr) << "Opening pool after shutdown failed. Errno: "
                           << errno << std::endl
                           << pmemobj_errormsg();

  /* Step4. */
  ObjData<int> pd{pop_};
  ASSERT_EQ(pd.AssertDataCorrect(), 0) << "Reading data from pool failed";
}

/**
 * TC_TRY_OPEN_AFTER_DOUBLE_US
 * Create pool on DIMM, trigger unsafe shutdown twice, try opening the
 * pool
 * \test
 *          \li \c Step1. Create a pool on DIMM / SUCCESS
 *	    \li \c Step2. Write pattern to pool / SUCCESS
 *          \li \c Step3. Trigger US, reboot, confirm USC incremented -
 * repeat twice / SUCCESS
 *          \li \c Step4. Open the pool / FAIL
 *          \li \c Step5. Force open the pool / SUCCESS
 *          \li \c Step6. Verify pattern / SUCCESS
 */
TEST_F(UnsafeShutdownBasic, TC_TRY_OPEN_AFTER_DOUBLE_US_before_us) {
  /* Step1. */
  pop_ = pmemobj_create(us_dimm_pool_path_.c_str(), nullptr, PMEMOBJ_MIN_POOL,
                        0644);
  ASSERT_NE(pop_, nullptr) << "Opening pool after shutdown failed. Errno: "
                           << errno << std::endl
                           << pmemobj_errormsg();
  /* Step2. */
  ObjData<int> pd{pop_};
  ASSERT_EQ(pd.WriteData(), 0) << "Writing to pool failed";

  /* Step3. */
  ASSERT_NO_FATAL_FAILURE(Inject());
}

TEST_F(UnsafeShutdownBasic, TC_TRY_OPEN_AFTER_DOUBLE_US_after_first_us) {
  ASSERT_TRUE(PassedOnPreviousPhase())
      << "Part of test before shutdown failed.";
  ASSERT_NO_FATAL_FAILURE(ConfirmRebootedWithUS());
  ASSERT_NO_FATAL_FAILURE(Inject());
}

TEST_F(UnsafeShutdownBasic, TC_TRY_OPEN_AFTER_DOUBLE_US_after_second_us) {
  close_pools_at_end_ = true;
  ASSERT_TRUE(PassedOnPreviousPhase())
      << "Part of test before shutdown failed.";
  ASSERT_NO_FATAL_FAILURE(ConfirmRebootedWithUS());

  /* step4 */
  ASSERT_EQ(pmemobj_open(us_dimm_pool_path_.c_str(), nullptr), nullptr)
      << "Pool was opened after unsafe shutdown, but should not.";

  /* Step5 */
  Repair(us_dimm_pool_path_.c_str());
  pop_ = pmemobj_open(us_dimm_pool_path_.c_str(), nullptr);
  ASSERT_NE(pop_, nullptr) << "Opening pool after shutdown failed. Errno: "
                           << errno << std::endl
                           << pmemobj_errormsg();

  /* Step6. */
  ObjData<int> pd{pop_};
  ASSERT_EQ(pd.AssertDataCorrect(), 0) << "Reading data from pool failed";
}

/**
 * TC_OPEN_CLEAN
 * Create pool on DIMM, close, trigger unsafe shutdown, open the pool
 * \test
 *          \li \c Step1. Create a pool on DIMM. \ SUCCESS
 *          \li \c Step2. Write data to pool. \ SUCCESS
 *          \li \c Step3. Close pool. \ SUCCESS
 *          \li \c Step4. Increment USC, reboot,
 *          \li \c Step5. Confirm USC incremented /
 * SUCCESS
 *          \li \c Step6. Open the pool. / SUCCESS
 *  	    \li \c Step7. Verify written pattern / SUCCESS
 */
TEST_F(UnsafeShutdownBasic, TC_OPEN_CLEAN_AFTER_US_before_us) {
  /* Step1 */
  pop_ = pmemobj_create(us_dimm_pool_path_.c_str(), nullptr, PMEMOBJ_MIN_POOL,
                        0644);
  ASSERT_NE(pop_, nullptr) << "Pool creating failed. Errno: " << errno
                           << std::endl
                           << pmemobj_errormsg();
  /* Step2. */
  ObjData<int> pd{pop_};
  ASSERT_EQ(pd.WriteData(), 0) << "Writing to pool failed";

  /* Step3 */
  pmemobj_close(pop_);

  /* Step4. */
  ASSERT_NO_FATAL_FAILURE(Inject());
}

TEST_F(UnsafeShutdownBasic, TC_OPEN_CLEAN_AFTER_US_after_first_us) {
  close_pools_at_end_ = true;
  ASSERT_TRUE(PassedOnPreviousPhase())
      << "Part of test before shutdown failed.";

  /* Step5. */
  ASSERT_NO_FATAL_FAILURE(ConfirmRebootedWithUS());

  /* Step6. */
  pop_ = pmemobj_open(us_dimm_pool_path_.c_str(), nullptr);
  ASSERT_NE(pop_, nullptr) << pmemobj_errormsg() << "errno:" << errno;

  /* Step7 */
  ObjData<int> pd{pop_};
  ASSERT_EQ(pd.AssertDataCorrect(), 0) << "Reading data from pool failed";
}
