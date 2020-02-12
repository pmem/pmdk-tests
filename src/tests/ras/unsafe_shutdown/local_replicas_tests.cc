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

#include "local_replicas_tests.h"

std::ostream& operator<<(std::ostream& stream, sync_local_replica_tc const& p) {
  stream << p.description;
  return stream;
}

void SyncLocalReplica::SetUp() {
  sync_local_replica_tc param = GetParam();
  ASSERT_TRUE(param.enough_dimms)
      << "Insufficient number of DIMMs to run this test";
}

/**
 * TC_SYNC_LOCAL_REPLICA
 * Create poolset with local replicas specified by parameter write data,
 * trigger US.
 * If syncable: restore pool from replica and confirm written data correctness
 * else: repair, sync, confirm written data.
 * \test
 *          \li \c Step1. Create a pool from poolset with primary pool on unsafely shutdown DIMM
 * and replicas according to given parameter. / SUCCESS
 *          \li \c Step2. Enable SDS on poolset
 *          \li \c Step3. Write pattern to pool persistently.
 *          \li \c Step4. Trigger unsafely shutdown on specified dimms, power cycle,
 *          confirm USC is incremented / SUCCESS
 *          \li \c Step5. Open the pool / FAIL: pop = NULL, errno = EINVAL
 *          \li \c Step6. If pool syncable: sync, else: try sync, repair, sync / SUCCESS
 *          \li \c Step7. Open the pool / SUCCESS
 *          \li \c Step8. Read and confirm data from pool / SUCCESS
 *          \li \c Step9. Close the pool / SUCCESS
 */
TEST_P(SyncLocalReplica, TC_SYNC_LOCAL_REPLICA_phase_1) {
  Poolset ps = GetParam().poolset;

  /* Step1 */
  PoolsetManagement p_mgmt;
  ASSERT_EQ(0, p_mgmt.CreatePoolsetFile(ps))
      << "error while creating poolset file";
  ASSERT_TRUE(p_mgmt.PoolsetFileExists(ps))
      << "Poolset file " << ps.GetFullPath() << " does not exist";

  ASSERT_EQ(0, ObjCreateHelper(ps.GetFullPath(), 0));

  /* Step3 */
  ObjData<int> pd{pop_};
  ASSERT_EQ(0, pd.Write(obj_data_)) << "Writing to pool failed";
}

/* Step4 - outside of test macros */

TEST_P(SyncLocalReplica, TC_SYNC_LOCAL_REPLICA_phase_2) {
  ASSERT_TRUE(PassedOnPreviousPhase()) << "Part of test before shutdown failed";

  sync_local_replica_tc param = GetParam();

  /* Step5 */
  ASSERT_EQ(0, ObjOpenFailureHelper(param.poolset.GetFullPath(), EINVAL))
      << "Pool after unexpectedly opened after US";

  /* Step6 */
  int expected_sync_exit = (param.is_syncable ? 0 : -1);
  ASSERT_EQ(expected_sync_exit,
            pmempool_sync(param.poolset.GetFullPath().c_str(), 0));

  if (param.is_syncable) {
    ASSERT_EQ(0, ObjOpenSuccessHelper(param.poolset.GetFullPath()));
  } else {
    ASSERT_EQ(0, ObjOpenFailureHelper(param.poolset.GetFullPath(), EINVAL));
    ASSERT_EQ(PMEMPOOL_CHECK_RESULT_SYNC_REQ,
              PmempoolRepair(param.poolset.GetFullPath()))
        << "Pool was not repaired";
    ASSERT_EQ(pmempool_sync(param.poolset.GetFullPath().c_str(), 0), 0);
    ASSERT_EQ(0, ObjOpenSuccessHelper(param.poolset.GetFullPath()));
  }

  /* Step8 */
  ObjData<int> pd{pop_};
  ASSERT_EQ(obj_data_, pd.Read()) << "Reading data from pool failed";
}

std::vector<sync_local_replica_tc> GetSyncLocalReplicaParams() {
  LocalTestPhase& test_phase = LocalTestPhase::GetInstance();
  const auto& unsafe_dn = test_phase.GetUnsafeDimmNamespaces();
  const auto& safe_dn = test_phase.GetSafeDimmNamespaces();

  std::vector<sync_local_replica_tc> ret_vec;

  /* Master replica on unsafely shutdown DIMM, healthy secondary replica on
   * another DIMM. */
  {
    sync_local_replica_tc tc;
    tc.description =
        "Master replica on unsafely shutdown DIMM, secondary replica "
        "on safely shutdown DIMM.";
    if (unsafe_dn.size() > 0 && safe_dn.size() > 0) {
      tc.enough_dimms = true;
      tc.poolset = Poolset{
          safe_dn[0].GetTestDir(),
          "pool_tc1.set",
          {{"PMEMPOOLSET",
            "9MB " + unsafe_dn[0].GetTestDir() + "tc1_master.part0",
            "9MB " + safe_dn[0].GetTestDir() + "tc1_master.part1"},
           {"REPLICA", "9MB " + safe_dn[0].GetTestDir() + "tc1_replica.part0",
            "9MB " + safe_dn[0].GetTestDir() + "tc1_replica.part1"}}};
      tc.is_syncable = true;
    } else {
      tc.enough_dimms = false;
    }
    ret_vec.emplace_back(tc);
  }

  /* Master replica on unsafely shutdown DIMM, healthy secondary replica on
   * non-pmem device. */
  {
    sync_local_replica_tc tc;
    tc.description =
        "Master replica on unsafely shutdown DIMM, healthy secondary replica "
        "on non-pmem device.";
    if (unsafe_dn.size() > 0) {
      tc.enough_dimms = true;
      tc.poolset = Poolset{
          unsafe_dn[0].GetTestDir(),
          "pool_tc2.set",
          {{"PMEMPOOLSET",
            "9MB " + unsafe_dn[0].GetTestDir() + "tc2_master.part0",
            "9MB " + unsafe_dn[0].GetTestDir() + "tc2_master.part1"},
           {"REPLICA", "9MB " + test_phase.GetTestDir() + "tc2_replica.part0",
            "9MB " + test_phase.GetTestDir() + "tc2_replica.part1"}}};
      tc.is_syncable = true;
    } else {
      tc.enough_dimms = false;
    }
    ret_vec.emplace_back(tc);
  }

  /* Two local secodary replicas, one partially on unsafely shutdown DIMM, other
   * on safely shutdown DIMM.
     */
  {
    sync_local_replica_tc tc;
    tc.description =
        "Healthy secondary replica on safely shutdown DIMM; replica partially "
        "on unsafely shutdown DIMM ";
    if (unsafe_dn.size() > 0 && safe_dn.size() > 0) {
      tc.enough_dimms = true;
      tc.poolset = Poolset{
          safe_dn[0].GetTestDir(),
          "pool_tc3.set",
          {{"PMEMPOOLSET",
            "9MB " + unsafe_dn[0].GetTestDir() + "tc3_master.part0",
            "9MB " + safe_dn[0].GetTestDir() + "tc3_master.part1"},
           {"REPLICA",
            "9MB " + unsafe_dn[0].GetTestDir() + "tc3_replica1.part0",
            "9MB " + safe_dn[0].GetTestDir() + "tc3_replica1.part1"},
           {"REPLICA", "9MB " + safe_dn[0].GetTestDir() + "tc3_replica2.part0",
            "9MB " + safe_dn[0].GetTestDir() + "tc3_replica2.part1"}}};
      tc.is_syncable = true;
    } else {
      tc.enough_dimms = false;
    }
    ret_vec.emplace_back(tc);
  }

  /* Master replica and secondary replica on the same unsafely shutdown DIMM */
  {
    sync_local_replica_tc tc;
    tc.description =
        "Master and secondary replicas on same unsafely shutdown DIMM.";
    if (unsafe_dn.size() > 0) {
      tc.enough_dimms = true;
      tc.poolset = Poolset{
          unsafe_dn[0].GetTestDir(),
          "pool1.set",
          {{"PMEMPOOLSET", "9MB " + unsafe_dn[0].GetTestDir() + "master1.part0",
            "9MB " + unsafe_dn[0].GetTestDir() + "master1.part1",
            "9MB " + unsafe_dn[0].GetTestDir() + "master1.part2"},
           {"REPLICA", "9MB " + unsafe_dn[0].GetTestDir() + "replica1.part0",
            "18MB " + unsafe_dn[0].GetTestDir() + "replica1.part1"}}};
      tc.is_syncable = false;
    } else {
      tc.enough_dimms = false;
    }
    ret_vec.emplace_back(tc);
  }

  /* Master and secondary replica partially on unsafely shutdown and safely
   * shutdown dimms. */
  {
    sync_local_replica_tc tc;
    tc.description =
        "Master and secondary replica partially interleaved on unsafely and "
        "safely shutdown DIMMs.";
    if (unsafe_dn.size() > 0 && safe_dn.size() > 0) {
      tc.enough_dimms = true;
      tc.poolset = Poolset{
          unsafe_dn[0].GetTestDir(),
          "pool2.set",
          {{"PMEMPOOLSET", "9MB " + unsafe_dn[0].GetTestDir() + "master2.part0",
            "9MB " + safe_dn[0].GetTestDir() + "master2.part1"},
           {"REPLICA", "9MB " + safe_dn[0].GetTestDir() + "replica2.part0",
            "18MB " + unsafe_dn[0].GetTestDir() + "replica2.part1"}}};
      tc.is_syncable = false;
    } else {
      tc.enough_dimms = false;
    }
    ret_vec.emplace_back(tc);
  }

  /* Master and secondary replica on different unsafely shutdown DIMMs
   */
  {
    sync_local_replica_tc tc;
    tc.description =
        "Master and secondary replica on different unsafely shutdown DIMMs.";
    if (unsafe_dn.size() > 1) {
      tc.enough_dimms = true;
      tc.poolset = Poolset{
          unsafe_dn[0].GetTestDir(),
          "pool3.set",
          {{"PMEMPOOLSET", "9MB " + unsafe_dn[0].GetTestDir() + "master3.part0",
            "9MB " + unsafe_dn[0].GetTestDir() + "master3.part1",
            "9MB " + unsafe_dn[0].GetTestDir() + "master3.part2"},
           {"REPLICA", "9MB " + unsafe_dn[1].GetTestDir() + "replica3.part0",
            "18MB " + unsafe_dn[1].GetTestDir() + "replica3.part1"}}};
      tc.is_syncable = false;
    } else {
      tc.enough_dimms = false;
    }
    ret_vec.emplace_back(tc);
  }

  /* Master and secondary replicas with parts interleaved between two unsafely
   * shutdown DIMMs.
     */
  {
    sync_local_replica_tc tc;
    tc.description =
        "Master and secondary replica partially on two unsafely shutdown "
        "dimms.";
    if (unsafe_dn.size() >= 2) {
      tc.enough_dimms = true;
      tc.poolset = Poolset{
          unsafe_dn[0].GetTestDir(),
          "pool4.set",
          {{"PMEMPOOLSET", "9MB " + unsafe_dn[0].GetTestDir() + "master4.part0",
            "9MB " + unsafe_dn[0].GetTestDir() + "master4.part1",
            "9MB " + unsafe_dn[1].GetTestDir() + "master4.part2"},
           {"REPLICA", "9MB " + unsafe_dn[1].GetTestDir() + "replica4.part0",
            "18MB " + unsafe_dn[0].GetTestDir() + "replica4.part1"}}};
      tc.is_syncable = false;
    } else {
      tc.enough_dimms = false;
    }
    ret_vec.emplace_back(tc);
  }

  /* Master replica and secondary replica partially on non-pmem,
   * partially on two us-dimms. */
  {
    sync_local_replica_tc tc;
    tc.description =
        "Master and secondary replica partially on non-pmem, partially on two "
        "unsafely shutdown dimms.";
    if (unsafe_dn.size() >= 2) {
      tc.enough_dimms = true;
      tc.poolset = Poolset{
          unsafe_dn[0].GetTestDir(),
          "pool5.set",
          {{"PMEMPOOLSET", "9MB " + unsafe_dn[0].GetTestDir() + "master5.part0",
            "9MB " + test_phase.GetTestDir() + "master5.part1",
            "9MB " + unsafe_dn[0].GetTestDir() + "master5.part2"},
           {"REPLICA", "9MB " + unsafe_dn[1].GetTestDir() + "replica5.part0",
            "18MB " + test_phase.GetTestDir() + "replica5.part1"}}};
      tc.is_syncable = false;
    } else {
      tc.enough_dimms = false;
    }
    ret_vec.emplace_back(tc);
  }

  /* Master and two secondary replicas on unsafely shutdown DIMMs */
  {
    sync_local_replica_tc tc;
    tc.description =
        "Master replica on unsafely shutdown DIMM 1, secondary replica on "
        "unsafely shutdown DIMM 1, secondary replica on unsafely shutdown DIMM "
        "2";
    if (unsafe_dn.size() >= 2) {
      tc.enough_dimms = true;
      tc.poolset = Poolset{
          unsafe_dn[0].GetTestDir(),
          "pool6.set",
          {{"PMEMPOOLSET", "9MB " + unsafe_dn[0].GetTestDir() + "master6.part0",
            "9MB " + unsafe_dn[0].GetTestDir() + "master6.part1",
            "9MB " + unsafe_dn[0].GetTestDir() + "master6.part2"},
           {"REPLICA", "9MB " + unsafe_dn[1].GetTestDir() + "replica6a.part0",
            "18MB " + unsafe_dn[1].GetTestDir() + "replica6a.part1"},
           {"REPLICA", "9MB " + unsafe_dn[0].GetTestDir() + "replica6b.part0",
            "18MB " + unsafe_dn[0].GetTestDir() + "replica6b.part1"}}};
      tc.is_syncable = false;
    } else {
      tc.enough_dimms = false;
    }
    ret_vec.emplace_back(tc);
  }
  return ret_vec;
}

INSTANTIATE_TEST_CASE_P(UnsafeShutdown, SyncLocalReplica,
                        ::testing::ValuesIn(GetSyncLocalReplicaParams()));

void UnsafeShutdownTransform::SetUp() {
  const auto& us_dn = test_phase_.GetUnsafeDimmNamespaces();
  const auto& non_us_dn = test_phase_.GetSafeDimmNamespaces();

  ASSERT_LE(1, us_dn.size())
      << "Insufficient number of unsafely shutdown DIMMs to run this test";
  ASSERT_LE(1, non_us_dn.size())
      << "Insufficient number of safely shutdown DIMMs to run this test";

  std::string us_dimm0_master11_path = us_dn[0].GetTestDir() + "master11";
  std::string non_us_dimm_replica11_path =
      non_us_dn[0].GetTestDir() + "replica11";
  std::string us_dimm0_replica11_path = us_dn[0].GetTestDir() + "replica11";

  origin_ =
      Poolset(us_dn[0].GetTestDir(), "pool11_origin.set",
              {
                  {"PMEMPOOLSET", "9MB " + us_dimm0_master11_path + ".part0",
                   "9MB " + us_dimm0_master11_path + ".part1",
                   "9MB " + us_dimm0_master11_path + ".part2"},
                  {"REPLICA", "9MB " + non_us_dimm_replica11_path + ".part0",
                   "18MB " + non_us_dimm_replica11_path + ".part1"},
              });
  added_ =
      Poolset(us_dn[0].GetTestDir(), "pool11_added.set",
              {
                  {"PMEMPOOLSET", "9MB " + us_dimm0_master11_path + ".part0",
                   "9MB " + us_dimm0_master11_path + ".part1",
                   "9MB " + us_dimm0_master11_path + ".part2"},
                  {"REPLICA", "9MB " + non_us_dimm_replica11_path + ".part0",
                   "18MB " + non_us_dimm_replica11_path + ".part1"},
                  {"REPLICA", "9MB " + us_dimm0_replica11_path + ".part0",
                   "18MB " + us_dimm0_replica11_path + ".part1"},
              });
  final_ =
      Poolset(us_dn[0].GetTestDir(), "pool11_final.set",
              {
                  {"PMEMPOOLSET", "9MB " + us_dimm0_master11_path + ".part0",
                   "9MB " + us_dimm0_master11_path + ".part1",
                   "9MB " + us_dimm0_master11_path + ".part2"},
                  {"REPLICA", "9MB " + us_dimm0_replica11_path + ".part0",
                   "18MB " + us_dimm0_replica11_path + ".part1"},
              });
}

/**
 * TC_TRANSFORM_POOLSET_TO_US_DIMM
 * Create pool from poolset with primary pool on unsafely shutdown DIMM and
 * healthy replica, transform replica to unsafely shutdown DIMM,
 * pool should be valid.
 * \test
 *          \li \c Step1. Create pool from poolset with primary pool on
 * unsafely shutdown DIMM and healthy replica, open pool. / SUCCESS
 *          \li \c Step2. Enable SDS on pool / SUCCESS
 *          \li \c Step3. Write data to pool / SUCCESS
 *          \li \c Step4. Increment USC on DIMM with primary pool, power cycle,
 * confirm USC is incremented. / SUCCESS
 *          \li \c Step5. Create poolset files to be transformed to / SUCCESS
 *          \li \c Step6. Try transforming the pool / FAIL: ret != 0, errno = EINVAL
 *          \li \c Step7. Sync pool / SUCCESS
 *          \li \c Step8. Transform healthy replica to unsafely shutdown DIMM. / SUCCESS
 *          \li \c Step9. Open the pool. / SUCCESS
 *          \li \c Step10. Read and confirm data from pool / SUCCESS
 *          \li \c Step11. Close the pool / SUCCESS
 */
TEST_F(UnsafeShutdownTransform, TC_TRANSFORM_POOLSET_TO_US_DIMM_phase_1) {
  PoolsetManagement p_mgmt;

  /* Step1 */
  ASSERT_EQ(0, p_mgmt.CreatePoolsetFile(origin_))
      << "Creating poolset file " + origin_.GetFullPath() + " failed";

  ASSERT_EQ(0, ObjCreateHelper(origin_.GetFullPath(), 0));

  /* Step3 */
  ObjData<int> pd{pop_};
  ASSERT_EQ(0, pd.Write(obj_data_)) << "Writing to pool failed";
}

/* Step4 - oustide test macros */

TEST_F(UnsafeShutdownTransform, TC_TRANSFORM_POOLSET_TO_US_DIMM_phase_2) {
  ASSERT_TRUE(PassedOnPreviousPhase()) << "Part of test before shutdown failed";

  /* Step5 */
  PoolsetManagement p_mgmt;
  ASSERT_EQ(0, p_mgmt.CreatePoolsetFile(added_))
      << "Creating poolset file " + origin_.GetFullPath() + " failed";
  ASSERT_EQ(0, p_mgmt.CreatePoolsetFile(final_))
      << "Creating poolset file " + final_.GetFullPath() + " failed";

  /* Step6 */
  ASSERT_NE(pmempool_transform(origin_.GetFullPath().c_str(),
                               added_.GetFullPath().c_str(), 0),
            0);
  ASSERT_EQ(EINVAL, errno);

  /* Step7 */
  ASSERT_EQ(pmempool_sync(origin_.GetFullPath().c_str(), 0), 0);

  /* Step8 */
  ASSERT_EQ(pmempool_transform(origin_.GetFullPath().c_str(),
                               added_.GetFullPath().c_str(), 0),
            0);
  ASSERT_EQ(pmempool_transform(added_.GetFullPath().c_str(),
                               final_.GetFullPath().c_str(), 0),
            0);

  /* Step9 */
  ASSERT_EQ(0, ObjOpenSuccessHelper(final_.GetFullPath()));

  /* Step10 */
  ObjData<int> pd{pop_};
  ASSERT_EQ(0, pd.Write(obj_data_)) << "Writing to pool failed";
}
