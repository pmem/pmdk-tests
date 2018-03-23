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

#include "us_local_replicas_tests.h"

std::ostream& operator<<(std::ostream& stream, sync_local_replica_tc const& p) {
  stream << p.description;
  return stream;
}

void SyncLocalReplica::SetUp() {
  sync_local_replica_tc param = GetParam();
  ASSERT_TRUE(param.enough_dimms)
      << "Test needs more dimms than are specified.";
}

/**
 * TC_SYNC_LOCAL_REPLICA
 * Create poolset with local replicas specified by parameter write data,
 * trigger US.
 * If syncable: restore pool from replica and confirm written data correctness
 * else: repair, sync, confirm written data.
 * \test
 *         \li \c Step1. Create and open pool from poolset with primary pool on US DIMM
 * and replicas according to given parameter. / SUCCESS
 *          \li \c Step2. Write pattern to pool persistently.
 *          \li \c Step3. Trigger US on specified dimms, power cycle,
 *          confirm USC incremented / SUCCESS
 *          \li \c Step4. Open the pool / FAIL
 *          \li \c Step5. If pool syncable: sync, else: try sync, repair, sync / SUCCESS
 *          \li \c Step6. Open the pool / SUCCESS
 *          \li \c Step7. Read and confirm data from pool / SUCCESS
 *          \li \c Step8. Close the pool / SUCCESS
 */
TEST_P(SyncLocalReplica, TC_SYNC_LOCAL_REPLICA_phase_1) {
  sync_local_replica_tc param = GetParam();

  /* Step1. */
  PoolsetManagement p_mgmt;
  ASSERT_EQ(0, p_mgmt.CreatePoolsetFile(param.poolset))
      << "error while creating poolset file";
  ASSERT_TRUE(p_mgmt.PoolsetFileExists(param.poolset))
      << "Poolset file " << param.poolset.GetFullPath() << " doesnt exist";

  std::string cmd = "pmempool create obj " + param.poolset.GetFullPath();
  auto out = shell_.ExecuteCommand(cmd);
  ASSERT_EQ(0, out.GetExitCode()) << cmd << std::endl
                                  << "errno: " << errno << std::endl
                                  << out.GetContent();

  pop_ = pmemobj_open(param.poolset.GetFullPath().c_str(), nullptr);
  ASSERT_NE(nullptr, pop_) << "Error while pool opening. Errno:" << errno
                           << std::endl
                           << pmemobj_errormsg();

  /* Step2. */
  ObjData<int> pd{pop_};
  ASSERT_EQ(0, pd.Write(obj_data_)) << "Writing to pool failed";
}

/* Step3 - outside of test macros */

TEST_P(SyncLocalReplica, TC_SYNC_LOCAL_REPLICA_phase_2) {
  ASSERT_TRUE(PassedOnPreviousPhase())
      << "Part of test before shutdown failed.";

  sync_local_replica_tc tc = GetParam();

  /* Step4. */
  pop_ = pmemobj_open(tc.poolset.GetFullPath().c_str(), nullptr);
  ASSERT_EQ(nullptr, pop_) << "Pool after US was opened but should be not";
  ASSERT_EQ(EINVAL, errno);

  /* Step5. */
  int expected_sync_exit = (tc.is_syncable ? 0 : 1);
  ASSERT_NO_FATAL_FAILURE(Sync(tc.poolset.GetFullPath(), expected_sync_exit));
  pop_ = pmemobj_open(tc.poolset.GetFullPath().c_str(), nullptr);

  if (!tc.is_syncable) {
    ASSERT_EQ(nullptr, pop_) << "Unsyncable pool was opened but should be not";
    ASSERT_EQ(EINVAL, errno);
    ASSERT_NO_FATAL_FAILURE(Repair(tc.poolset.GetFullPath()));
    ASSERT_NO_FATAL_FAILURE(Sync(tc.poolset.GetFullPath()));
    pop_ = pmemobj_open(tc.poolset.GetFullPath().c_str(), nullptr);
  }

  /* Step6. */
  ASSERT_NE(nullptr, pop_) << "Syncable pool could not be opened after sync";

  /* Step7. */
  ObjData<int> pd{pop_};
  ASSERT_EQ(obj_data_, pd.Read()) << "Reading data from pool failed";
}

std::vector<sync_local_replica_tc> GetSyncLocalReplicaParams() {
  LocalTestPhase& test_phase = LocalTestPhase::GetInstance();
  const auto unsafe_dn = test_phase.GetUnsafeDimmNamespaces();
  const auto safe_dn = test_phase.GetSafeDimmNamespaces();

  std::vector<sync_local_replica_tc> ret;

  /* Healthy replica on another dimm. */
  {
    sync_local_replica_tc tc;
    tc.description = "Healthy replica on non-us dimm.";
    if (unsafe_dn.size() > 0 && safe_dn.size() > 0) {
      tc.enough_dimms = true;
      tc.poolset =
          Poolset{safe_dn[0].GetMountpoint(),
                  "pool_tc1.set",
                  {{"PMEMPOOLSET", "9MB " + unsafe_dn[0].GetMountpoint() +
                                       SEPARATOR + "tc1_master.part0",
                    "9MB " + safe_dn[0].GetMountpoint() + SEPARATOR +
                        "tc1_master.part1"},
                   {"REPLICA", "9MB " + safe_dn[0].GetMountpoint() + SEPARATOR +
                                   "tc1_replica.part0",
                    "9MB " + safe_dn[0].GetMountpoint() + SEPARATOR +
                        "tc1_replica.part1"}}};
      tc.is_syncable = true;
    } else {
      tc.enough_dimms = false;
    }
    ret.emplace_back(tc);
  }

  /* Healthy replica on non dimm device. */
  {
    sync_local_replica_tc tc;
    tc.description = "Healthy replica on non-dimm device.";
    if (unsafe_dn.size() > 0) {
      tc.enough_dimms = true;
      tc.poolset = Poolset{
          unsafe_dn[0].GetMountpoint(),
          "pool_tc2.set",
          {{"PMEMPOOLSET", "9MB " + unsafe_dn[0].GetMountpoint() + SEPARATOR +
                               "tc2_master.part0",
            "9MB " + unsafe_dn[0].GetMountpoint() + SEPARATOR +
                "tc2_master.part1"},
           {"REPLICA",
            "9MB " + test_phase.GetTestDir() + SEPARATOR + "tc2_replica.part0",
            "9MB " + test_phase.GetTestDir() + SEPARATOR +
                "tc2_replica.part1"}}};
      tc.is_syncable = true;
    } else {
      tc.enough_dimms = false;
    }
    ret.emplace_back(tc);
  }

  /* Two local replicas, one partially on US DIMM, other on another DIMM
     */
  {
    sync_local_replica_tc tc;
    tc.description =
        "Healthy replica on non-us dimm; replica partially on us dimm";
    if (unsafe_dn.size() > 0 && safe_dn.size() > 0) {
      tc.enough_dimms = true;
      tc.poolset =
          Poolset{safe_dn[0].GetMountpoint(),
                  "pool_tc3.set",
                  {{"PMEMPOOLSET", "9MB " + unsafe_dn[0].GetMountpoint() +
                                       SEPARATOR + "tc3_master.part0",
                    "9MB " + safe_dn[0].GetMountpoint() + SEPARATOR +
                        "tc3_master.part1"},
                   {"REPLICA", "9MB " + unsafe_dn[0].GetMountpoint() +
                                   SEPARATOR + "tc3_replica1.part0",
                    "9MB " + safe_dn[0].GetMountpoint() + SEPARATOR +
                        "tc3_replica1.part1"},
                   {"REPLICA", "9MB " + safe_dn[0].GetMountpoint() + SEPARATOR +
                                   "tc3_replica2.part0",
                    "9MB " + safe_dn[0].GetMountpoint() + SEPARATOR +
                        "tc3_replica2.part1"}}};
      tc.is_syncable = true;
    } else {
      tc.enough_dimms = false;
    }
    ret.emplace_back(tc);
  }

  /* Replica and master replica on same US dimm */
  {
    sync_local_replica_tc tc;
    tc.description = "Master and replica on same us dimm.";
    if (unsafe_dn.size() > 0) {
      tc.enough_dimms = true;
      tc.poolset = Poolset{
          unsafe_dn[0].GetMountpoint(),
          "pool1.set",
          {{"PMEMPOOLSET",
            "9MB " + unsafe_dn[0].GetMountpoint() + SEPARATOR + "master1.part0",
            "9MB " + unsafe_dn[0].GetMountpoint() + SEPARATOR + "master1.part1",
            "9MB " + unsafe_dn[0].GetMountpoint() + SEPARATOR +
                "master1.part2"},
           {"REPLICA", "9MB " + unsafe_dn[0].GetMountpoint() + SEPARATOR +
                           "replica1.part0",
            "18MB " + unsafe_dn[0].GetMountpoint() + SEPARATOR +
                "replica1.part1"}}};
      tc.is_syncable = false;
    } else {
      tc.enough_dimms = false;
    }
    ret.emplace_back(tc);
  }

  /* Replica and master replica partially on US and non-US dimms. */
  {
    sync_local_replica_tc tc;
    tc.description = "Master and replica partially on same us and non-us dimm.";
    if (unsafe_dn.size() > 0 && safe_dn.size() > 0) {
      tc.enough_dimms = true;
      tc.poolset = Poolset{
          unsafe_dn[0].GetMountpoint(),
          "pool2.set",
          {{"PMEMPOOLSET",
            "9MB " + unsafe_dn[0].GetMountpoint() + SEPARATOR + "master2.part0",
            "9MB " + safe_dn[0].GetMountpoint() + SEPARATOR + "master2.part1"},
           {"REPLICA",
            "9MB " + safe_dn[0].GetMountpoint() + SEPARATOR + "replica2.part0",
            "18MB " + unsafe_dn[0].GetMountpoint() + SEPARATOR +
                "replica2.part1"}}};
      tc.is_syncable = false;
    } else {
      tc.enough_dimms = false;
    }
    ret.emplace_back(tc);
  }

  /* Replica and master replica on different US dimms */
  {
    sync_local_replica_tc tc;
    tc.description = "Master and replica on different us dimms.";
    if (unsafe_dn.size() > 1) {
      tc.enough_dimms = true;
      tc.poolset = Poolset{
          unsafe_dn[0].GetMountpoint(),
          "pool3.set",
          {{"PMEMPOOLSET",
            "9MB " + unsafe_dn[0].GetMountpoint() + SEPARATOR + "master3.part0",
            "9MB " + unsafe_dn[0].GetMountpoint() + SEPARATOR + "master3.part1",
            "9MB " + unsafe_dn[0].GetMountpoint() + SEPARATOR +
                "master3.part2"},
           {"REPLICA", "9MB " + unsafe_dn[1].GetMountpoint() + SEPARATOR +
                           "replica3.part0",
            "18MB " + unsafe_dn[1].GetMountpoint() + SEPARATOR +
                "replica3.part1"}}};
      tc.is_syncable = false;
    } else {
      tc.enough_dimms = false;
    }
    ret.emplace_back(tc);
  }

  /* Replica and master replica with parts distributed between two US
   * dimms.
     */
  {
    sync_local_replica_tc tc;
    tc.description = "Master and replica partially on two us dimms.";
    if (unsafe_dn.size() >= 2) {
      tc.enough_dimms = true;
      tc.poolset = Poolset{
          unsafe_dn[0].GetMountpoint(),
          "pool4.set",
          {{"PMEMPOOLSET",
            "9MB " + unsafe_dn[0].GetMountpoint() + SEPARATOR + "master4.part0",
            "9MB " + unsafe_dn[0].GetMountpoint() + SEPARATOR + "master4.part1",
            "9MB " + unsafe_dn[1].GetMountpoint() + SEPARATOR +
                "master4.part2"},
           {"REPLICA", "9MB " + unsafe_dn[1].GetMountpoint() + SEPARATOR +
                           "replica4.part0",
            "18MB " + unsafe_dn[0].GetMountpoint() + SEPARATOR +
                "replica4.part1"}}};
      tc.is_syncable = false;
    } else {
      tc.enough_dimms = false;
    }
    ret.emplace_back(tc);
  }

  /* Master and Replica partially on non-dimm, partially on two us-dimms. */
  {
    sync_local_replica_tc tc;
    tc.description =
        "Master and replica partially on non-dimm, partially on two us dimms.";
    if (unsafe_dn.size() >= 2) {
      tc.enough_dimms = true;
      tc.poolset = Poolset{
          unsafe_dn[0].GetMountpoint(),
          "pool5.set",
          {{"PMEMPOOLSET",
            "9MB " + unsafe_dn[0].GetMountpoint() + SEPARATOR + "master5.part0",
            "9MB " + test_phase.GetTestDir() + SEPARATOR + "master5.part1",
            "9MB " + unsafe_dn[0].GetMountpoint() + SEPARATOR +
                "master5.part2"},
           {"REPLICA", "9MB " + unsafe_dn[1].GetMountpoint() + SEPARATOR +
                           "replica5.part0",
            "18MB " + test_phase.GetTestDir() + SEPARATOR + "replica5.part1"}}};
      tc.is_syncable = false;
    } else {
      tc.enough_dimms = false;
    }
    ret.emplace_back(tc);
  }

  /* Master and two Replicas on US dimms */
  {
    sync_local_replica_tc tc;
    tc.description =
        "Master on us dimm 1, replica on us_dimm 1, replica on us_dimm 2";
    if (unsafe_dn.size() >= 2) {
      tc.enough_dimms = true;
      tc.poolset = Poolset{
          unsafe_dn[0].GetMountpoint(),
          "pool6.set",
          {{"PMEMPOOLSET",
            "9MB " + unsafe_dn[0].GetMountpoint() + SEPARATOR + "master6.part0",
            "9MB " + unsafe_dn[0].GetMountpoint() + SEPARATOR + "master6.part1",
            "9MB " + unsafe_dn[0].GetMountpoint() + SEPARATOR +
                "master6.part2"},
           {"REPLICA", "9MB " + unsafe_dn[1].GetMountpoint() + SEPARATOR +
                           "replica6a.part0",
            "18MB " + unsafe_dn[1].GetMountpoint() + SEPARATOR +
                "replica6a.part1"},
           {"REPLICA", "9MB " + unsafe_dn[0].GetMountpoint() + SEPARATOR +
                           "replica6b.part0",
            "18MB " + unsafe_dn[0].GetMountpoint() + SEPARATOR +
                "replica6b.part1"}}};
      tc.is_syncable = false;
    } else {
      tc.enough_dimms = false;
    }
    ret.emplace_back(tc);
  }
  return ret;
}

INSTANTIATE_TEST_CASE_P(UnsafeShutdown, SyncLocalReplica,
                        ::testing::ValuesIn(GetSyncLocalReplicaParams()));

void UnsafeShutdownTransform::SetUp() {
  const auto& us_dn = test_phase_.GetUnsafeDimmNamespaces();
  const auto& non_us_dn = test_phase_.GetSafeDimmNamespaces();

  ASSERT_LE(1, us_dn.size())
      << "Test needs more us dimms to run than was specified.";
  ASSERT_LE(1, non_us_dn.size())
      << "Test needs more non-us dimms to run than was specified.";

  std::string us_dimm0_master11_path =
      us_dn[0].GetMountpoint() + SEPARATOR + "master11";
  std::string non_us_dimm_replica11_path =
      non_us_dn[0].GetMountpoint() + SEPARATOR + "replica11";
  std::string us_dimm0_replica11_path =
      us_dn[0].GetMountpoint() + SEPARATOR + "replica11";

  origin_ =
      Poolset(us_dn[0].GetMountpoint(), "pool11_origin.set",
              {
                  {"PMEMPOOLSET", "9MB " + us_dimm0_master11_path + ".part0",
                   "9MB " + us_dimm0_master11_path + ".part1",
                   "9MB " + us_dimm0_master11_path + ".part2"},
                  {"REPLICA", "9MB " + non_us_dimm_replica11_path + ".part0",
                   "18MB " + non_us_dimm_replica11_path + ".part1"},
              });
  added_ =
      Poolset(us_dn[0].GetMountpoint(), "pool11_added.set",
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
      Poolset(us_dn[0].GetMountpoint(), "pool11_final.set",
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
 * Create pool from poolset with primary pool on US DIMM and healthy replica,
 * transform replica to US DIMM, pool should be valid.
 * \test
 *          \li \c Step1. Create pool from poolset with primary pool on US DIMM
 * and healthy replica., open pool. / SUCCESS
 *          \li \c Step2. Write data to pool / SUCCESS
 *          \li \c Step3. Increment USC on DIMM with primary pool, power cycle,
 * confirm USC incremented. / SUCCESS
 *          \li \c Step4. Create poolset files to be transformed to / SUCCESS
 *          \li \c Step5. Try transforming the pool / FAILURE
 *          \li \c Step6. Sync pool / SUCCESS
 *          \li \c Step7. Transform healthy replica to US DIMM. / SUCCESS
 *          \li \c Step8. Open the pool. / SUCCESS
 *          \li \c Step9. Read and confirm data from pool / SUCCESS
 *          \li \c Step10. Close the pool / SUCCESS
 */
TEST_F(UnsafeShutdownTransform, TC_TRANSFORM_POOLSET_TO_US_DIMM_phase_1) {
  PoolsetManagement p_mgmt;

  /* Step1. */
  ASSERT_EQ(0, p_mgmt.CreatePoolsetFile(origin_))
      << "Creating poolset file " + origin_.GetFullPath() + " failed";

  std::string cmd = "pmempool create obj " + origin_.GetFullPath();
  auto out = shell_.ExecuteCommand(cmd);
  ASSERT_EQ(0, out.GetExitCode()) << cmd << std::endl
                                  << "errno: " << errno << std::endl
                                  << out.GetContent();

  pop_ = pmemobj_open(origin_.GetFullPath().c_str(), nullptr);
  ASSERT_NE(nullptr, pop_) << "Error while pool opening. Errno: " << errno
                           << std::endl
                           << pmemobj_errormsg();

  /* Step2. */
  ObjData<int> pd{pop_};
  ASSERT_EQ(0, pd.Write(obj_data_)) << "Writing to pool failed";
}

/* Step3 - oustide test macros */

TEST_F(UnsafeShutdownTransform,
       TC_TRANSFORM_HEALTHY_REPLICA_TO_US_DIMM_phase_2) {
  ASSERT_TRUE(PassedOnPreviousPhase())
      << "Part of test before shutdown failed.";

  /* Step4. */
  PoolsetManagement p_mgmt;
  ASSERT_EQ(0, p_mgmt.CreatePoolsetFile(added_))
      << "Creating poolset file " + origin_.GetFullPath() + " failed";
  ASSERT_EQ(0, p_mgmt.CreatePoolsetFile(final_))
      << "Creating poolset file " + final_.GetFullPath() + " failed";

  /* Step5. */
  ASSERT_NO_FATAL_FAILURE(
      Transform(origin_.GetFullPath(), added_.GetFullPath(), 1));

  /* Step6. */
  ASSERT_NO_FATAL_FAILURE(Sync(origin_.GetFullPath()));

  /* Step7. */
  ASSERT_NO_FATAL_FAILURE(
      Transform(origin_.GetFullPath(), added_.GetFullPath()));
  ASSERT_NO_FATAL_FAILURE(
      Transform(added_.GetFullPath(), final_.GetFullPath()));

  /* Step8. */
  pop_ = pmemobj_open(final_.GetFullPath().c_str(), nullptr);
  ASSERT_NE(nullptr, pop_) << "Pool opening failed. Errno: " << errno
                           << std::endl
                           << pmemobj_errormsg();

  /* Step9 */
  ObjData<int> pd{pop_};
  ASSERT_EQ(0, pd.Write(obj_data_)) << "Writing to pool failed";
}
