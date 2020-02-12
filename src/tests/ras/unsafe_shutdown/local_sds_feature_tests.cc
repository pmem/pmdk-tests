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

#include "local_sds_feature_tests.h"

void PmempoolSDSFeature::SetUp() {
  us_dimm_pool_path_ = test_phase_.GetUnsafeDimmNamespaces()[0].GetTestDir() +
                       GetNormalizedTestName() + "_pool";

  int disabled = 0;
  ASSERT_EQ(0, pmemobj_ctl_set(nullptr, "sds.at_create", &disabled))
      << "disabling SDS failed:  " << pmemobj_errormsg()
      << "; errno: " << errno;

  ASSERT_EQ(0, pmemlog_ctl_set(nullptr, "sds.at_create", &disabled))
      << "disabling SDS failed:  " << pmemlog_errormsg()
      << "; errno: " << errno;

  ASSERT_EQ(0, pmemblk_ctl_set(nullptr, "sds.at_create", &disabled))
      << "disabling SDS failed:  " << pmemblk_errormsg()
      << "; errno: " << errno;
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
TEST_F(PmempoolSDSFeature, TRY_OPEN_OBJ_phase_1) {
  /* Step1 */
  ASSERT_EQ(0, ObjCreateHelper(us_dimm_pool_path_, PMEMOBJ_MIN_POOL));

  /* Step2 */
  ASSERT_EQ(0, ObjEnableShutdownState(us_dimm_pool_path_));

  /* Step3 */
  ObjData<int> pd{pop_};
  ASSERT_EQ(0, pd.Write(obj_data_)) << "Writing to pool failed";
}

/* Step3. outside of test macros */

TEST_F(PmempoolSDSFeature, TRY_OPEN_OBJ_phase_2) {
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

TEST_F(PmempoolSDSFeature, TRY_OPEN_BLK_SDS_FEATURE_phase_1) {
  /* Step1 */
  pbp_ = pmemblk_create(us_dimm_pool_path_.c_str(), blk_size_, PMEMBLK_MIN_POOL,
                        0644 & PERMISSION_MASK);
  ASSERT_TRUE(pbp_ != nullptr) << "Pool creating failed. Errno: " << errno
                               << std::endl
                               << pmemblk_errormsg();
  /* Step2 */
  ASSERT_EQ(0, BlkEnableShutdownState(us_dimm_pool_path_));

  /* Step3 */
  BlkData<int> pd{pbp_};
  ASSERT_EQ(0, pd.Write(blk_data_)) << "Writing to pool failed";
}

/* Step4. outside of test macros */

TEST_F(PmempoolSDSFeature, TRY_OPEN_BLK_SDS_FEATURE_phase_2) {
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
 * TRY_OPEN_LOG_SDS_FEATURE
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
TEST_F(PmempoolSDSFeature, TRY_OPEN_LOG_SDS_FEATURE_phase_1) {
  /* Step1 */
  plp_ = pmemlog_create(us_dimm_pool_path_.c_str(), PMEMLOG_MIN_POOL,
                        0644 & PERMISSION_MASK);
  ASSERT_TRUE(plp_ != nullptr) << "Pool creating failed. Errno: " << errno
                               << std::endl
                               << pmemlog_errormsg();
  /* Step2 */
  ASSERT_EQ(0, LogEnableShutdownState(us_dimm_pool_path_));

  /* Step3 */
  LogData pd{plp_};
  ASSERT_EQ(0, pd.Write(log_data_)) << "Writing to pool failed";
}

/* Step4. outside of test macros */

TEST_F(PmempoolSDSFeature, TRY_OPEN_LOG_SDS_FEATURE_phase_2) {
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
 * SDS_DISABLED
 * Create obj pool on DIMM with SDS disabled, trigger unsafe shutdown,
 * open the pool
 * \test
 *          \li \c Step1. Create an obj pool on DIMM / SUCCESS
 *          \li \c Step2. Disable SDS on pool / SUCCESS
 *          \li \c Step3. Check that SDS feature is disabled / SUCCESS
 *          \li \c Step4. Write pattern to pool / SUCCESS
 *          \li \c Step5. Trigger US, run power cycle, check USC values / SUCCESS
 *          \li \c Step6. Open the pool / SUCCESS
 *          \li \c Step7. Verify written pattern / SUCCESS
 *          \li \c Step8. Close the pool / SUCCESS
 */
TEST_F(PmempoolSDSFeature, SDS_DISABLED_phase_1) {
  /* Step1 */
  ASSERT_EQ(0, ObjCreateHelper(us_dimm_pool_path_, PMEMOBJ_MIN_POOL));
  ;
  /* Step2 */
  ASSERT_EQ(0, ObjDisableShutdownState(us_dimm_pool_path_));

  /* Step4 */
  ObjData<int> pd{pop_};
  ASSERT_EQ(0, pd.Write(obj_data_)) << "Writing to pool failed";
}

/* Step5. outside of test macros */

TEST_F(PmempoolSDSFeature, SDS_DISABLED_phase_2) {
  ASSERT_TRUE(PassedOnPreviousPhase()) << "Part of test before shutdown failed";

  /* Step6 */
  ASSERT_EQ(0, ObjOpenSuccessHelper(us_dimm_pool_path_))
      << "Pool with SDS disabled was not opened after unsafe shutdown";

  /* Step7 */
  ObjData<int> pd{pop_};
  ASSERT_EQ(obj_data_, pd.Read()) << "Data read from pool differs from written";
}

/**
 * SDS_DISABLE_AFTER_US
 * Create obj pool on DIMM, enable SDS, trigger unsafe shutdown, disable SDS,
 * open the pool
 * \test
 *          \li \c Step1. Create an obj pool on DIMM / SUCCESS
 *          \li \c Step2. Enable SDS / SUCCESS
 *          \li \c Step3. Write pattern to pool / SUCCESS
 *          \li \c Step4. Trigger US, run power cycle, check USC values / SUCCESS
 *          \li \c Step5. Disable SDS on pool / SUCCESS
 *          \li \c Step6. Open the pool / SUCCESS
 *          \li \c Step7. Verify written pattern / SUCCESS
 *          \li \c Step8. Close the pool / SUCCESS
 */
TEST_F(PmempoolSDSFeature, SDS_DISABLE_AFTER_US_phase_1) {
  /* Step1 */
  ASSERT_EQ(0, ObjCreateHelper(us_dimm_pool_path_, PMEMOBJ_MIN_POOL));

  /* Step2 */
  ASSERT_EQ(0, ObjEnableShutdownState(us_dimm_pool_path_));

  /* Step3 */
  ObjData<int> pd{pop_};
  ASSERT_EQ(0, pd.Write(obj_data_)) << "Writing to pool failed";
}

/* Step4. outside of test macros */

TEST_F(PmempoolSDSFeature, SDS_DISABLE_AFTER_US_phase_2) {
  ASSERT_TRUE(PassedOnPreviousPhase()) << "Part of test before shutdown failed";

  /* Step5 */
  ASSERT_EQ(0, pmempool_feature_disable(us_dimm_pool_path_.c_str(),
                                        PMEMPOOL_FEAT_SHUTDOWN_STATE, 0))
      << "Could not disable SDS. Errno: " << errno;

  /* Step6 */
  ASSERT_EQ(0, ObjOpenSuccessHelper(us_dimm_pool_path_))
      << "Pool with SDS disabled was not opened after unsafe shutdown";

  /* Step7 */
  ObjData<int> pd{pop_};
  ASSERT_EQ(obj_data_, pd.Read()) << "Data read from pool differs from written";
}

/**
 * SDS_REENABLE_AFTER_US
 * Create obj pool on DIMM, enable SDS, trigger unsafe shutdown, disable SDS,
 * enable SDS, open the pool
 * \test
 *          \li \c Step1. Create an obj pool on DIMM / SUCCESS
 *          \li \c Step2. Enable SDS / SUCCESS
 *          \li \c Step3. Write pattern to pool / SUCCESS
 *          \li \c Step4. Trigger US, run power cycle, check USC values / SUCCESS
 *          \li \c Step5. Disable SDS on pool / SUCCESS
 *          \li \c Step6. Enable SDS on pool / SUCCESS
 *          \li \c Step7. Open the pool / SUCCESS
 *          \li \c Step8. Verify written pattern / SUCCESS
 *          \li \c Step9. Close the pool / SUCCESS
 */
TEST_F(PmempoolSDSFeature, SDS_REENABLE_AFTER_US_phase_1) {
  /* Step1 */
  ASSERT_EQ(0, ObjCreateHelper(us_dimm_pool_path_, PMEMOBJ_MIN_POOL));

  /* Step2 */
  ASSERT_EQ(0, ObjEnableShutdownState(us_dimm_pool_path_));

  /* Step3 */
  ObjData<int> pd{pop_};
  ASSERT_EQ(0, pd.Write(obj_data_)) << "Writing to pool failed";
}

/* Step4. outside of test macros */

TEST_F(PmempoolSDSFeature, SDS_REENABLE_AFTER_US_phase_2) {
  ASSERT_TRUE(PassedOnPreviousPhase()) << "Part of test before shutdown failed";

  /* Step5 */
  ASSERT_EQ(0, pmempool_feature_disable(us_dimm_pool_path_.c_str(),
                                        PMEMPOOL_FEAT_SHUTDOWN_STATE, 0))
      << "Could not disable SDS. Errno: " << errno;

  /* Step6 */
  ASSERT_EQ(0, pmempool_feature_enable(us_dimm_pool_path_.c_str(),
                                       PMEMPOOL_FEAT_SHUTDOWN_STATE, 0))
      << "Could not reenable SDS. Errno: " << errno;

  /* Step7 */
  ASSERT_EQ(0, ObjOpenSuccessHelper(us_dimm_pool_path_));

  /* Step8 */
  ObjData<int> pd{pop_};
  ASSERT_EQ(obj_data_, pd.Read()) << "Data read from pool differs from written";
}

void PmempoolSDSFeature::TearDown() {
  int enabled = 1;
  ASSERT_EQ(0, pmemobj_ctl_set(nullptr, "sds.at_create", &enabled))
      << "enabling SDS failed:  " << pmemobj_errormsg() << "; errno: " << errno;

  ASSERT_EQ(0, pmemlog_ctl_set(nullptr, "sds.at_create", &enabled))
      << "enabling SDS failed:  " << pmemlog_errormsg() << "; errno: " << errno;

  ASSERT_EQ(0, pmemblk_ctl_set(nullptr, "sds.at_create", &enabled))
      << "enabling SDS failed:  " << pmemblk_errormsg() << "; errno: " << errno;
}

int PmempoolSDSFeature::EnableAndQuerySDS(const std::string &path) {
  if (pmempool_feature_enable(path.c_str(), PMEMPOOL_FEAT_SHUTDOWN_STATE, 0) !=
      0) {
    std::cerr << "Enabling shutdown state failed" << std::endl;
    return -1;
  }

  if (pmempool_feature_query(path.c_str(), PMEMPOOL_FEAT_SHUTDOWN_STATE, 0) !=
      1) {
    std::cerr << "SDS is not enabled according to pmempool_feature_query"
              << std::endl;
    return -1;
  }
  return 0;
}

int PmempoolSDSFeature::DisableAndQuerySDS(const std::string &path) {
  if (pmempool_feature_disable(path.c_str(), PMEMPOOL_FEAT_SHUTDOWN_STATE, 0) !=
      0) {
    std::cerr << "Disabling shutdown state failed" << std::endl;
    return -1;
  }

  if (pmempool_feature_query(path.c_str(), PMEMPOOL_FEAT_SHUTDOWN_STATE, 0) !=
      0) {
    std::cerr << "SDS is not disabled according to pmempool_feature_query"
              << std::endl;
    return -1;
  }
  return 0;
}

int PmempoolSDSFeature::ObjEnableShutdownState(const std::string &path) {
  pmemobj_close(pop_);
  if (EnableAndQuerySDS(path) != 0) {
    return -1;
  }
  pop_ = pmemobj_open(path.c_str(), nullptr);
  if (pop_ == nullptr) {
    std::cerr << "Opening the pool after enabling SDS failed. Errno: " << errno
              << std::endl;
    return -1;
  }
  return 0;
}

int PmempoolSDSFeature::ObjDisableShutdownState(const std::string &path) {
  pmemobj_close(pop_);
  if (DisableAndQuerySDS(path) != 0) {
    return -1;
  }
  pop_ = pmemobj_open(path.c_str(), nullptr);
  if (pop_ == nullptr) {
    std::cerr << "Opening the pool after disabling SDS failed. Errno: " << errno
              << std::endl;
    return -1;
  }
  return 0;
}

int PmempoolSDSFeature::BlkEnableShutdownState(const std::string &path) {
  pmemblk_close(pbp_);
  if (EnableAndQuerySDS(path) != 0) {
    return -1;
  }
  pbp_ = pmemblk_open(path.c_str(), blk_size_);
  if (pbp_ == nullptr) {
    std::cerr << "Opening the pool after enabling SDS failed. Errno: " << errno
              << std::endl;
    return -1;
  }
  return 0;
}

int PmempoolSDSFeature::LogEnableShutdownState(const std::string &path) {
  pmemlog_close(plp_);
  if (EnableAndQuerySDS(path) != 0) {
    return -1;
  }

  plp_ = pmemlog_open(path.c_str());
  if (plp_ == nullptr) {
    std::cerr << "Opening the pool after enabling SDS failed. Errno: " << errno
              << std::endl
              << pmemlog_errormsg();
    return -1;
  }
  return 0;
}
