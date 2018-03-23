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

std::ostream& operator<<(std::ostream& stream, poolset_tc const& p) {
  stream << p.description;
  return stream;
}

void SyncLocalReplica::SetUp() {
  poolset_tc param = GetParam();
  ASSERT_TRUE(param.enough_dimms)
      << "Test needs more dimms than are specified.";
  us_dimm_colls_in_test_ = param.us_dimms;
}

/**
 * TC_SYNC_LOCAL_REPLICA
 * Create poolset with local replicas specified by parameter write data,
 * trigger US.
 * If syncable: restore pool from replica and confirm written data correctness
 * else: confirm syncing not possible
 * \test
 *         \li \c Step1. Create and open pool from poolset with primary pool on US DIMM
 * and replicas according to given parameter. / SUCCESS
 *          \li \c Step2. Write pattern to pool persistently.
 *          \li \c Step3. Trigger US on specified dimms, reboot
 *          \li \c Step4. Confirm USC incremented / SUCCESS
 *          \li \c Step5. Open the pool / FAIL
 *          \li \c Step6. Try syncing pool, repair if not syncable / SUCCESS
 *          \li \c Step7. Open the pool / SUCCESS
 *          \li \c Step8. Read and confirm data from pool / SUCCESS
 */
TEST_P(SyncLocalReplica, TC_SYNC_LOCAL_REPLICA_before_us) {
  poolset_tc param = GetParam();
  Poolset ps = param.poolset;

  /* Step1. */
  PoolsetManagement p_mgmt;
  ASSERT_EQ(p_mgmt.CreatePoolsetFile(ps), 0)
      << "error while creating poolset file";
  ASSERT_TRUE(p_mgmt.PoolsetFileExists(ps))
      << "Poolset file " << ps.GetFullPath() << " doesnt exist";

  std::string cmd = "pmempool create obj " + ps.GetFullPath();
  auto out = shell_.ExecuteCommand(cmd);
  ASSERT_EQ(out.GetExitCode(), 0) << cmd << std::endl
                                  << "errno: " << errno << std::endl
                                  << out.GetContent();

  pop_ = pmemobj_open(ps.GetFullPath().c_str(), nullptr);
  ASSERT_NE(pop_, nullptr) << "Error while pool opening" << std::endl
                           << pmemobj_errormsg();

  /* Step2. */
  ObjData<int> pd{pop_};
  ASSERT_EQ(pd.Write(obj_data_), 0) << "Writing to pool failed";

}

TEST_P(SyncLocalReplica, TC_SYNC_LOCAL_REPLICA_after_first_us) {
  close_pools_at_end_ = true;
  ASSERT_TRUE(PassedOnPreviousPhase())
      << "Part of test before shutdown failed.";

  /* Step4. */
  ASSERT_NO_FATAL_FAILURE(ConfirmRebootedWithUS());

  poolset_tc tc = GetParam();
  Poolset ps = tc.poolset;

  /* Step5. */
  ASSERT_EQ(pmemobj_open(ps.GetFullPath().c_str(), nullptr), nullptr)
      << "Pool after US was opened but should be not";

  std::string cmd = "pmempool sync " + ps.GetFullPath();
  auto out = shell_.ExecuteCommand(cmd);

  int sync_exit_code = (tc.is_syncable ? 0 : 1);
  ASSERT_EQ(out.GetExitCode(), sync_exit_code)
      << cmd << " result was other than expected." << std::endl
      << out.GetContent();

  pop_ = pmemobj_open(ps.GetFullPath().c_str(), nullptr);

  /* Step6. */
  if (!tc.is_syncable) {
    ASSERT_EQ(pop_, nullptr) << "Unsyncable pool was opened but should be not";
    Repair(ps.GetFullPath());
    pop_ = pmemobj_open(ps.GetFullPath().c_str(), nullptr);
  }

  /* Step7. */
  ASSERT_NE(pop_, nullptr) << "Syncable pool could not be opened after sync";

  /* Step8. */
  ObjData<int> pd{pop_};
  ASSERT_EQ(pd.Write(obj_data_), 0) << "Reading data from pool failed";
}

std::vector<poolset_tc> GetPoolsetLocalReplicaParams() {
  std::vector<poolset_tc> ret;

  /* Healthy replica on another dimm. */
  {
    poolset_tc tc;
    tc.description = "Healthy replica on non-us dimm.";
    if (us_dimm_colls.size() > 0 && non_us_dimm_colls.size() > 0) {
      tc.enough_dimms = true;
      tc.poolset =
          Poolset{non_us_dimm_colls[0].GetMountpoint(),
                  "pool_tc1.set",
                  {{"PMEMPOOLSET", "9MB " + us_dimm_colls[0].GetMountpoint() +
                                       SEPARATOR + "tc1_master.part0",
                    "9MB " + non_us_dimm_colls[0].GetMountpoint() + SEPARATOR +
                        "tc1_master.part1"},
                   {"REPLICA", "9MB " + non_us_dimm_colls[0].GetMountpoint() +
                                   SEPARATOR + "tc1_replica.part0",
                    "9MB " + non_us_dimm_colls[0].GetMountpoint() + SEPARATOR +
                        "tc1_replica.part1"}}};
      tc.us_dimms = {us_dimm_colls[0]};
      tc.enough_dimms = true;
      tc.is_syncable = true;
    } else {
      tc.enough_dimms = false;
    }
    ret.emplace_back(tc);
  }

  /* Healthy replica on non dimm device. */
  {
    poolset_tc tc;
    tc.description = "Healthy replica on non-dimm device.";
    if (us_dimm_colls.size() > 0) {
      tc.enough_dimms = true;
      tc.poolset =
          Poolset{us_dimm_colls[0].GetMountpoint(),
                  "pool_tc2.set",
                  {{"PMEMPOOLSET", "9MB " + us_dimm_colls[0].GetMountpoint() +
                                       SEPARATOR + "tc2_master.part0",
                    "9MB " + us_dimm_colls[0].GetMountpoint() + SEPARATOR +
                        "tc2_master.part1"},
                   {"REPLICA", "9MB " + local_dimm_config->GetTestDir() +
                                   SEPARATOR + "tc2_replica.part0",
                    "9MB " + local_dimm_config->GetTestDir() + SEPARATOR +
                        "tc2_replica.part1"}}};
      tc.us_dimms = {us_dimm_colls[0]};
      tc.is_syncable = true;
    } else {
      tc.enough_dimms = false;
    }
    ret.emplace_back(tc);
  }

  /* Two local replicas, one partially on US DIMM, other on another DIMM
     */
  {
    poolset_tc tc;
    tc.description =
        "Healthy replica on non-us dimm; replica partially on us dimm";
    if (us_dimm_colls.size() > 0 && non_us_dimm_colls.size() > 0) {
      tc.enough_dimms = true;
      tc.poolset =
          Poolset{non_us_dimm_colls[0].GetMountpoint(),
                  "pool_tc3.set",
                  {{"PMEMPOOLSET", "9MB " + us_dimm_colls[0].GetMountpoint() +
                                       SEPARATOR + "tc3_master.part0",
                    "9MB " + non_us_dimm_colls[0].GetMountpoint() + SEPARATOR +
                        "tc3_master.part1"},
                   {"REPLICA", "9MB " + us_dimm_colls[0].GetMountpoint() +
                                   SEPARATOR + "tc3_replica1.part0",
                    "9MB " + non_us_dimm_colls[0].GetMountpoint() + SEPARATOR +
                        "tc3_replica1.part1"},
                   {"REPLICA", "9MB " + non_us_dimm_colls[0].GetMountpoint() +
                                   SEPARATOR + "tc3_replica2.part0",
                    "9MB " + non_us_dimm_colls[0].GetMountpoint() + SEPARATOR +
                        "tc3_replica2.part1"}}};
      tc.us_dimms = {us_dimm_colls[0]};
      tc.is_syncable = true;
    } else {
      tc.enough_dimms = false;
    }
    ret.emplace_back(tc);
  }

  /* Replica and master replica on same US dimm */
  {
    poolset_tc tc;
    tc.description = "Master and replica on same us dimm.";
    if (us_dimm_colls.size() > 0) {
      tc.enough_dimms = true;
      tc.poolset = Poolset{
          us_dimm_colls[0].GetMountpoint(),
          "pool1.set",
          {{"PMEMPOOLSET",
            "9MB " + us_dimm_colls[0].GetMountpoint() + SEPARATOR + "master1.part0",
            "9MB " + us_dimm_colls[0].GetMountpoint() + SEPARATOR + "master1.part1",
            "9MB " + us_dimm_colls[0].GetMountpoint() + SEPARATOR + "master1.part2"},
           {"REPLICA",
            "9MB " + us_dimm_colls[0].GetMountpoint() + SEPARATOR + "replica1.part0",
            "18MB " + us_dimm_colls[0].GetMountpoint() + SEPARATOR +
                "replica1.part1"}}};
      tc.us_dimms = {us_dimm_colls[0]};
      tc.is_syncable = false;
    } else {
      tc.enough_dimms = false;
    }
    ret.emplace_back(tc);
  }

  /* Replica and master replica partially on US and non-US dimms. */
  {
    poolset_tc tc;
    tc.description = "Master and replica partially on same us and non-us dimm.";
    if (us_dimm_colls.size() > 0 && non_us_dimm_colls.size() > 0) {
      tc.enough_dimms = true;
      tc.poolset = Poolset{
          us_dimm_colls[0].GetMountpoint(),
          "pool2.set",
          {{"PMEMPOOLSET",
            "9MB " + us_dimm_colls[0].GetMountpoint() + SEPARATOR + "master2.part0",
            "9MB " + non_us_dimm_colls[0].GetMountpoint() + SEPARATOR +
                "master2.part1"},
           {"REPLICA", "9MB " + non_us_dimm_colls[0].GetMountpoint() + SEPARATOR +
                           "replica2.part0",
            "18MB " + us_dimm_colls[0].GetMountpoint() + SEPARATOR +
                "replica2.part1"}}};
      tc.us_dimms = {us_dimm_colls[0]};
      tc.is_syncable = false;
    } else {
      tc.enough_dimms = false;
    }
    ret.emplace_back(tc);
  }

  /* Replica and master replica on different US dimms */
  {
    poolset_tc tc;
    tc.description = "Master and replica on different us dimms.";
    if (us_dimm_colls.size() > 1) {
      tc.enough_dimms = true;
      tc.poolset = Poolset{
          us_dimm_colls[0].GetMountpoint(),
          "pool3.set",
          {{"PMEMPOOLSET",
            "9MB " + us_dimm_colls[0].GetMountpoint() + SEPARATOR + "master3.part0",
            "9MB " + us_dimm_colls[0].GetMountpoint() + SEPARATOR + "master3.part1",
            "9MB " + us_dimm_colls[0].GetMountpoint() + SEPARATOR + "master3.part2"},
           {"REPLICA",
            "9MB " + us_dimm_colls[1].GetMountpoint() + SEPARATOR + "replica3.part0",
            "18MB " + us_dimm_colls[1].GetMountpoint() + SEPARATOR +
                "replica3.part1"}}};
      tc.us_dimms = {us_dimm_colls[0], us_dimm_colls[1]};
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
    poolset_tc tc;
    tc.description = "Master and replica partially on two us dimms.";
    if (us_dimm_colls.size() >= 2) {
      tc.enough_dimms = true;
      tc.poolset = Poolset{
          us_dimm_colls[0].GetMountpoint(),
          "pool4.set",
          {{"PMEMPOOLSET",
            "9MB " + us_dimm_colls[0].GetMountpoint() + SEPARATOR + "master4.part0",
            "9MB " + us_dimm_colls[0].GetMountpoint() + SEPARATOR + "master4.part1",
            "9MB " + us_dimm_colls[1].GetMountpoint() + SEPARATOR + "master4.part2"},
           {"REPLICA",
            "9MB " + us_dimm_colls[1].GetMountpoint() + SEPARATOR + "replica4.part0",
            "18MB " + us_dimm_colls[0].GetMountpoint() + SEPARATOR +
                "replica4.part1"}}};
      tc.us_dimms = {us_dimm_colls[0], us_dimm_colls[1]};
      tc.is_syncable = false;
    } else {
      tc.enough_dimms = false;
    }
    ret.emplace_back(tc);
  }

  /* Master and Replica partially on non-dimm, partially on two us-dimms. */
  {
    poolset_tc tc;
    tc.description =
        "Master and replica partially on non-dimm, partially on two us dimms.";
    if (us_dimm_colls.size() >= 2) {
      tc.enough_dimms = true;
      tc.poolset = Poolset{
          us_dimm_colls[0].GetMountpoint(),
          "pool5.set",
          {{"PMEMPOOLSET",
            "9MB " + us_dimm_colls[0].GetMountpoint() + SEPARATOR + "master5.part0",
            "9MB " + local_dimm_config->GetTestDir() + SEPARATOR +
                "master5.part1",
            "9MB " + us_dimm_colls[0].GetMountpoint() + SEPARATOR + "master5.part2"},
           {"REPLICA",
            "9MB " + us_dimm_colls[1].GetMountpoint() + SEPARATOR + "replica5.part0",
            "18MB " + local_dimm_config->GetTestDir() + SEPARATOR +
                "replica5.part1"}}};
      tc.us_dimms = {us_dimm_colls[0], us_dimm_colls[1]};
      tc.is_syncable = false;
    } else {
      tc.enough_dimms = false;
    }
    ret.emplace_back(tc);
  }

  /* Master and two Replicas on US dimms */
  {
    poolset_tc tc;
    tc.description =
        "Master on us dimm 1, replica on us_dimm 1, replica on us_dimm 2";
    if (us_dimm_colls.size() >= 2) {
      tc.enough_dimms = true;
      tc.poolset = Poolset{
          us_dimm_colls[0].GetMountpoint(),
          "pool6.set",
          {{"PMEMPOOLSET",
            "9MB " + us_dimm_colls[0].GetMountpoint() + SEPARATOR + "master6.part0",
            "9MB " + us_dimm_colls[0].GetMountpoint() + SEPARATOR + "master6.part1",
            "9MB " + us_dimm_colls[0].GetMountpoint() + SEPARATOR + "master6.part2"},
           {"REPLICA", "9MB " + us_dimm_colls[1].GetMountpoint() + SEPARATOR +
                           "replica6a.part0",
            "18MB " + us_dimm_colls[1].GetMountpoint() + SEPARATOR +
                "replica6a.part1"},
           {"REPLICA", "9MB " + us_dimm_colls[0].GetMountpoint() + SEPARATOR +
                           "replica6b.part0",
            "18MB " + us_dimm_colls[0].GetMountpoint() + SEPARATOR +
                "replica6b.part1"}}};
      tc.us_dimms = {us_dimm_colls[0], us_dimm_colls[1]};
      tc.is_syncable = false;
    } else {
      tc.enough_dimms = false;
    }
    ret.emplace_back(tc);
  }
  return ret;
}

INSTANTIATE_TEST_CASE_P(UnsafeShutdown, SyncLocalReplica,
                        ::testing::ValuesIn(GetPoolsetLocalReplicaParams()));

void UnsafeShutdownTransform::SetUp() {
  ASSERT_GT(us_dimm_colls.size(), 0)
      << "Test needs more us dimms to run than was specified.";
  ASSERT_GT(non_us_dimm_colls.size(), 0)
      << "Test needs more non-us dimms to run than was specified.";

  std::string us_dimm0_master11_path =
      us_dimm_colls[0].GetMountpoint() + SEPARATOR + "master11";
  std::string non_us_dimm_replica11_path =
      non_us_dimm_colls[0].GetMountpoint() + SEPARATOR + "replica11";
  std::string us_dimm0_replica11_path =
      us_dimm_colls[0].GetMountpoint() + SEPARATOR + "replica11";

  origin_ =
      Poolset(us_dimm_colls[0].GetMountpoint(), "pool11_origin.set",
              {
                  {"PMEMPOOLSET", "9MB " + us_dimm0_master11_path + ".part0",
                   "9MB " + us_dimm0_master11_path + ".part1",
                   "9MB " + us_dimm0_master11_path + ".part2"},
                  {"REPLICA", "9MB " + non_us_dimm_replica11_path + ".part0",
                   "18MB " + non_us_dimm_replica11_path + ".part1"},
              });
  added_ =
      Poolset(us_dimm_colls[0].GetMountpoint(), "pool11_added.set",
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
      Poolset(us_dimm_colls[0].GetMountpoint(), "pool11_final.set",
              {
                  {"PMEMPOOLSET", "9MB " + us_dimm0_master11_path + ".part0",
                   "9MB " + us_dimm0_master11_path + ".part1",
                   "9MB " + us_dimm0_master11_path + ".part2"},
                  {"REPLICA", "9MB " + us_dimm0_replica11_path + ".part0",
                   "18MB " + us_dimm0_replica11_path + ".part1"},
              });

  us_dimm_colls_in_test_ = {us_dimm_colls.front()};
}

/**
 * TC_RESTORE_AFTER_TRANSFORMING_HEALTHY_REPLICA_TO_US_DIMM
 * Create pool from poolset with primary pool on US DIMM and healthy replica,
 * transform replica to US DIMM, pool should be valid.
 * \test
 *          \li \c Step1. Create pool from poolset with primary pool on US DIMM
 * and healthy replica., open pool. / SUCCESS
 *          \li \c Step2. Write data to pool / SUCCESS
 *          \li \c Step3. Increment USC on DIMM with primary pool, reboot,
 * confirm USC incremented. / SUCCESS
 *          \li \c Step4. Confirm USC incremented / SUCCESS
 *          \li \c Step5. Transform healthy replica to US DIMM. / SUCCESS
 *          \li \c Step6. Open the pool. / FAIL
 *          \li \c Step7. Restore the pool from replica / SUCCESS
 *          \li \c Step8. Open the pool. / SUCCESS
 *          \li \c Step9. Read and confirm data from pool / SUCCESS
 */

TEST_F(UnsafeShutdownTransform,
       TC_TRANSFORM_HEALTHY_REPLICA_TO_US_DIMM_before_us) {
  PoolsetManagement p_mgmt;

  /* Step1. */
  ASSERT_EQ(0, p_mgmt.CreatePoolsetFile(origin_))
      << "Creating poolset file " + origin_.GetFullPath() + " failed";

  std::string cmd = "pmempool create obj " + origin_.GetFullPath();
  auto out = shell_.ExecuteCommand(cmd);
  ASSERT_EQ(out.GetExitCode(), 0) << cmd << std::endl
                                  << "errno: " << errno << std::endl
                                  << out.GetContent();

  pop_ = pmemobj_open(origin_.GetFullPath().c_str(), nullptr);
  ASSERT_NE(pop_, nullptr) << "Error while pool opening" << std::endl
                           << pmemobj_errormsg();

  /* Step2. */
  ObjData<int> pd{pop_};
  ASSERT_EQ(pd.Write(obj_data_), 0) << "Writing to pool failed";
}

TEST_F(UnsafeShutdownTransform,
       TC_TRANSFORM_HEALTHY_REPLICA_TO_US_DIMM_after_first_us) {
  close_pools_at_end_ = true;
  ASSERT_TRUE(PassedOnPreviousPhase())
      << "Part of test before shutdown failed.";

  /* Step4 */
  ASSERT_NO_FATAL_FAILURE(ConfirmRebootedWithUS());

  /* Step5. */
  PoolsetManagement p_mgmt;
  ASSERT_EQ(0, p_mgmt.CreatePoolsetFile(added_))
      << "Creating poolset file " + origin_.GetFullPath() + " failed";
  ASSERT_EQ(0, p_mgmt.CreatePoolsetFile(final_))
      << "Creating poolset file " + final_.GetFullPath() + " failed";

  std::string cmd = "pmempool transform " + origin_.GetFullPath() + " " +
                    added_.GetFullPath();
  auto out = shell_.ExecuteCommand(cmd);
  ASSERT_EQ(out.GetExitCode(), 0) << cmd << std::endl << out.GetContent();

  cmd =
      "pmempool transform " + added_.GetFullPath() + " " + final_.GetFullPath();
  out = shell_.ExecuteCommand(cmd);
  ASSERT_EQ(out.GetExitCode(), 0) << cmd << std::endl << out.GetContent();

  /* Step6. */
  ASSERT_EQ(pmemobj_open(final_.GetFullPath().c_str(), nullptr), nullptr)
      << "Pool was opened but it should be not";
  /* Step7. */
  ASSERT_NO_FATAL_FAILURE(Repair(final_.GetFullPath().c_str()));

  /* Step8. */
  pop_ = pmemobj_open(final_.GetFullPath().c_str(), nullptr);

  /* Step9 */
  ObjData<int> pd{pop_};
  ASSERT_EQ(pd.Write(obj_data_), 0) << "Writing to pool failed";
}
