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

#include "us_move_tests.h"

std::ostream& operator<<(std::ostream& stream, move_param const& m) {
  stream << m.description;
  return stream;
}

std::vector<move_param> GetMoveParams() {
  std::vector<move_param> ret_vec;
  LocalTestPhase& test_phase = LocalTestPhase::GetInstance();
  const auto& unsafe_dn = test_phase.GetUnsafeDimmNamespaces();
  const auto& safe_dn = test_phase.GetSafeDimmNamespaces();

  /* Move pool from unsafely shutdown DIMM to non-DIMM */
  {
    move_param tc;
    tc.description = "from unsafely shutdown DIMM to non-pmem";
    if (unsafe_dn.size() >= 1) {
      tc.enough_dimms = true;
      tc.src_pool_dir = unsafe_dn[0].GetTestDir();
      tc.dest_pool_dir = test_phase.GetTestDir();
    } else {
      tc.enough_dimms = false;
    }
    ret_vec.emplace_back(tc);
  }

  /* Move pool from non-DIMM to unsafely shutdown DIMM */
  {
    move_param tc;
    tc.description = "from non-pmem to unsafely shutdown DIMM";
    if (unsafe_dn.size() >= 1) {
      tc.enough_dimms = true;
      tc.src_pool_dir = test_phase.GetTestDir();
      tc.dest_pool_dir = unsafe_dn[0].GetTestDir();
    } else {
      tc.enough_dimms = false;
    }
    ret_vec.emplace_back(tc);
  }

  /* Move pool from unsafely shutdown DIMM to safely shutdown DIMM */
  {
    move_param tc;
    tc.description = "from unsafely shutdown DIMM to safely shutdown DIMM";
    if (unsafe_dn.size() >= 1 && safe_dn.size() >= 1) {
      tc.enough_dimms = true;
      tc.src_pool_dir = unsafe_dn[0].GetTestDir();
      tc.dest_pool_dir = safe_dn[0].GetTestDir();
    } else {
      tc.enough_dimms = false;
    }
    ret_vec.emplace_back(tc);
  }

  /* Move pool from unsafely shutdown DIMM to unsafely shutdown DIMM */
  {
    move_param tc;
    tc.description = "from unsafely shutdown DIMM to unsafely shutdown DIMM";
    if (unsafe_dn.size() >= 2) {
      tc.enough_dimms = true;
      tc.src_pool_dir = unsafe_dn[1].GetTestDir();
      tc.dest_pool_dir = unsafe_dn[0].GetTestDir();
    } else {
      tc.enough_dimms = false;
    }
    ret_vec.emplace_back(tc);
  }

  /* Move pool from safely shutdown Dimm to non-Dimm */
  {
    move_param tc;
    tc.description = "From safely shutdown DIMM to non-pmem";
    if (safe_dn.size() >= 1) {
      tc.enough_dimms = true;
      tc.src_pool_dir = safe_dn[0].GetTestDir();
      tc.dest_pool_dir = test_phase.GetTestDir();
    } else {
      tc.enough_dimms = false;
    }
    ret_vec.emplace_back(tc);
  }

  /* Move pool from non-Dimm to safely shutdown Dimm */
  {
    move_param tc;
    tc.description = "From non-pmem to safely shutdown DIMM";
    if (safe_dn.size() >= 1) {
      tc.enough_dimms = true;
      tc.src_pool_dir = test_phase.GetTestDir();
      tc.dest_pool_dir = safe_dn[0].GetTestDir();
    } else {
      tc.enough_dimms = false;
    }
    ret_vec.emplace_back(tc);
  }
  return ret_vec;
}

void MovePoolClean::SetUp() {
  move_param param = GetParam();
  close_pools_at_end_ = true;
  ASSERT_TRUE(param.enough_dimms)
      << "Insufficient number of DIMMs to run this test";
  src_pool_path_ = param.src_pool_dir + GetNormalizedTestName() + "_pool";
  dest_pool_path_ = param.dest_pool_dir + GetNormalizedTestName() + "_pool";
}

/**
 * TC_MOVE_POOL_CLEAN
 * Check if pool moved between devices in a manner specified by test parameters
 * and closed properly can be reopened.
 * Trigger unsafe shutdown after closing the pool.
 * \test
 *          \li \c Step1. Create pool on device. / SUCCESS
 *          \li \c Step2. Write pattern persistently to pool, close the pool
 *          / SUCCESS
 *          \li \c Step3. Increment USC on DIMM specified by parameter, power cycle
 *          \li \c Step4. Confirm USC incremented / SUCCESS
 *          \li \c Step4. Move the pool to different device. / SUCCESS
 *          \li \c Step5. Open the pool. / SUCCESS
 *          \li \c Step6. Verify written pattern. / SUCCESS
 *          \li \c Step7. Close the pool / SUCCESS
 */
TEST_P(MovePoolClean, TC_MOVE_POOL_CLEAN_phase_1) {
  /* Step1 */
  pop_ = pmemobj_create(src_pool_path_.c_str(), nullptr, PMEMOBJ_MIN_POOL,
                        0644 & PERMISSION_MASK);
  ASSERT_TRUE(pop_ != nullptr) << "Pool creating failed. Errno: " << errno
                               << std::endl
                               << pmemobj_errormsg();
  /* Step2 */
  ObjData<int> pd{pop_};
  ASSERT_EQ(0, pd.Write(obj_data_)) << "Writing to pool failed";
}

/* Step3. outside of test macros */

TEST_P(MovePoolClean, TC_MOVE_POOL_CLEAN_phase_2) {
  ASSERT_TRUE(PassedOnPreviousPhase()) << "Part of test before shutdown failed";

  /* Step4 */
  auto out =
      shell_.ExecuteCommand("mv " + src_pool_path_ + " " + dest_pool_path_);
  ASSERT_EQ(0, out.GetExitCode()) << out.GetContent() << std::endl;

  /* Step5 */
  pop_ = pmemobj_open(dest_pool_path_.c_str(), nullptr);
  ASSERT_TRUE(pop_ != nullptr) << "Pool opening failed. Errno:" << errno
                               << std::endl
                               << pmemobj_errormsg();

  /* Step6 */
  ObjData<int> pd{pop_};
  ASSERT_EQ(obj_data_, pd.Read()) << "Reading data from pool failed";
}

INSTANTIATE_TEST_CASE_P(UnsafeShutdown, MovePoolClean,
                        ::testing::ValuesIn(GetMoveParams()));

void MovePoolDirty::SetUp() {
  move_param param = GetParam();
  ASSERT_TRUE(param.enough_dimms)
      << "Insufficient number of DIMMs to run this test";
  src_pool_path_ = param.src_pool_dir + GetNormalizedTestName() + "_pool";
  dest_pool_path_ = param.dest_pool_dir + GetNormalizedTestName() + "_pool";
}

/**
 * TC_MOVE_POOL_DIRTY
 * Check if pool moved between devices in a manner specified by test parameters
 * can be opened.
 * \test
 *          \li \c Step1. Create pool on device. / SUCCESS
 *          \li \c Step2. Write pattern persistently to pool / SUCCESS
 *          \li \c Step3. Increment USC on DIMM specified by parameter, power cycle,
 *          confirm USC is incremented / SUCCESS
 *          \li \c Step4. Move the pool to different device. / SUCCESS
 *          \li \c Step5. Try opening the pool / FAIL: pop = NULL,
 *          errno = EINVAL
 *          \li \c Step6. Repair pool and open / SUCCESS
 *          \li \c Step7. Verify written pattern / SUCCESS
 *          \li \c Step8. Close the pool / SUCCESS
 */
TEST_P(MovePoolDirty, TC_MOVE_POOL_DIRTY_phase_1) {
  /* Step1 */
  pop_ = pmemobj_create(src_pool_path_.c_str(), nullptr, PMEMOBJ_MIN_POOL,
                        0644 & PERMISSION_MASK);
  ASSERT_TRUE(pop_ != nullptr) << "Pool creating failed" << std::endl
                               << pmemobj_errormsg();

  /* Step2 */
  ObjData<int> pd{pop_};
  ASSERT_EQ(0, pd.Write(obj_data_)) << "Writing to pool failed";
}

/* Step3 - outside of test macros */

TEST_P(MovePoolDirty, TC_MOVE_POOL_DIRTY_phase_2) {
  ASSERT_TRUE(PassedOnPreviousPhase()) << "Part of test before shutdown failed";

  /* Step4 */
  auto out =
      shell_.ExecuteCommand("mv " + src_pool_path_ + " " + dest_pool_path_);
  ASSERT_EQ(0, out.GetExitCode()) << "Moving operation failed" << std::endl
                                  << out.GetContent() << std::endl;

  /* Step5 */
  pop_ = pmemobj_open(dest_pool_path_.c_str(), nullptr);
  ASSERT_EQ(nullptr, pop_)
      << "Dirty pool after moving was opened but should be not";
  ASSERT_EQ(EINVAL, errno);

  /* Step6 */
  ASSERT_EQ(PMEMPOOL_CHECK_RESULT_REPAIRED, PmempoolRepair(dest_pool_path_))
      << "Pool was not repaired";
  pop_ = pmemobj_open(dest_pool_path_.c_str(), nullptr);
  ASSERT_TRUE(pop_ != nullptr)
      << "Pool after repair was not opened. Errno: " << errno << std::endl
      << pmemobj_errormsg();

  /* Step7 */
  ObjData<int> pd{pop_};
  ASSERT_EQ(obj_data_, pd.Read()) << "Reading data from pool failed";
}

INSTANTIATE_TEST_CASE_P(UnsafeShutdown, MovePoolDirty,
                        ::testing::ValuesIn(GetMoveParams()));
