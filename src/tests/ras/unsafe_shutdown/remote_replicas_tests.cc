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

#include "remote_replicas_tests.h"

void SyncRemoteReplica::SetUp() {
  RemotePoolsetTC param = GetParam();
  ASSERT_TRUE(param.enough_dimms_) << "Not enough dimms specified to run test.";
}

int RemotePoolset::CreateRemotePoolsetFile() const {
  IShell shell{host_};
  for (const auto& line : poolset_.GetContent()) {
    std::string cmd = "echo " + line + " >> " + poolset_.GetFullPath();
    auto out = shell.ExecuteCommand(cmd);

    if (out.GetExitCode() != 0) {
      std::cerr << cmd << ": " << out.GetContent() << std::endl;
      return out.GetExitCode();
    }
  }
  return 0;
}

std::string RemotePoolset::GetReplicaLine() const {
  /* Remove port argument from host address representation */
  std::string host_repr = host_;
  size_t found = host_repr.find(" ");
  if (found != std::string::npos) {
    host_repr = host_repr.substr(0, found);
  }
  return "REPLICA " + host_repr + " " + poolset_.GetFullPath();
}

std::ostream& operator<<(std::ostream& stream, RemotePoolsetTC const& p) {
  stream << p.description_;
  return stream;
}

/* Currently there is no way to directly enable SDS on a poolset with remote
 replicas. A workaround involving pmempool_transform must be used for this
 purpose. */
PMEMobjpool* RemotePoolsetTC::CreatePoolWithSDS() {
  // Create pool without remote replicas
  PMEMobjpool* pop = pmemobj_create(local_poolset_.GetFullPath().c_str(),
                                    nullptr, 0, 0644 & PERMISSION_MASK);
  if (pop == nullptr) {
    std::cerr << "Pool creating failed. Errno: " << strerror(errno) << std::endl
              << pmemobj_errormsg() << std::endl;
    return pop;
  }
  pmemobj_close(pop);

  // Enable SDS
  if (pmempool_feature_enable(local_poolset_.GetFullPath().c_str(),
                              PMEMPOOL_FEAT_SHUTDOWN_STATE, 0) != 0) {
    std::cerr << "Enabling shutdown state failed" << std::endl;
    return nullptr;
  }

  // Create poolest file with remote replicas
  std::vector<std::string> content{local_poolset_.GetContent()};
  for (const auto& rp : remote_poolsets_) {
    content.emplace_back(rp.GetReplicaLine());
  }

  ApiC api;
  if (api.CreateFileT(GetPathTransformed(), content) != 0) {
    std::cerr << "Error while creating poolset file: " << GetPathTransformed()
              << std::endl;
    return nullptr;
  }

  // Add remote replicas to pool with enabled SDS with pmempool_transform
  if (pmempool_transform(local_poolset_.GetFullPath().c_str(),
                         GetPathTransformed().c_str(), 0) != 0) {
    std::cerr << "transform failed" << std::endl;
    return nullptr;
  }

  return pmemobj_open(GetPathTransformed().c_str(), nullptr);
}

int RemotePoolsetTC::CreatePoolsetFiles() const {
  PoolsetManagement psm;
  if (psm.CreatePoolsetFile(local_poolset_) != 0) {
    std::cerr << "error while creating local poolset file: "
              << local_poolset_.GetFullPath() << std::endl;
    return -1;
  }

  for (const auto& rp : remote_poolsets_) {
    if (rp.CreateRemotePoolsetFile() != 0) {
      std::cerr << "Error while creating remote poolset: "
                << rp.poolset_.GetFullPath() << std::endl;
      return -1;
    }
  }
  return 0;
}

/**
 * TC_SYNC_WITH_REMOTE_REPLICAS
 * Create poolset with remote replicas specified by parameter write data,
 * trigger US.
 * If syncable: restore pool from replica and confirm written data correctness
 * else: confirm syncing not possible
 * \test
 *          \li \c Step1. Create pool from poolset with primary pool on US DIMM
 * and replicas according to given parameter / SUCCESS
 *          \li \c Step2. Write pattern to primary pool persistently / SUCCESS
 *          \li \c Step3. Trigger US on specified dimms, reboot, confirm USC
 * incremented / SUCCESS
 *          \li \c Step4. Open the pool / FAIL
 *          \li \c Step5. Try syncing the pool / SUCCESS
 *          \li \c Step6. If syncable: open the pool, confirm data correctness,
 * else: try opening the pool, try repairing / SUCCESS
 */
TEST_P(SyncRemoteReplica, TC_SYNC_REMOTE_REPLICA_phase_1) {
  RemotePoolsetTC param = GetParam();

  /* Step1 */
  ASSERT_EQ(0, param.CreatePoolsetFiles());
  pop_ = param.CreatePoolWithSDS();
  ASSERT_TRUE(pop_ != nullptr) << pmemobj_errormsg();

  /* Step2 */
  ObjData<int> pd{pop_};
  ASSERT_EQ(0, pd.Write(obj_data_)) << "Writing data to pool failed";
}

TEST_P(SyncRemoteReplica, TC_SYNC_REMOTE_REPLICA_phase_2) {
  RemotePoolsetTC param = GetParam();
  ASSERT_TRUE(PassedOnPreviousPhase());

  /* Step4 */
  pop_ = pmemobj_open(param.GetPathTransformed().c_str(), nullptr);
  ASSERT_EQ(nullptr, pop_) << "Pool : " << param.GetPath().c_str()
                           << " after US was opened but should be not";
  ASSERT_EQ(EINVAL, errno) << param.GetPathTransformed();

  /* Step5 */
  int expected_sync_exit = (param.is_syncable_ ? 0 : -1);
  ASSERT_EQ(expected_sync_exit,
            pmempool_sync(param.GetPathTransformed().c_str(), 0));

  /* Step6 */
  pop_ = pmemobj_open(param.GetPathTransformed().c_str(), nullptr);
  if (param.is_syncable_) {
    ASSERT_TRUE(pop_ != nullptr) << "Syncable pool was not opened after sync";
    ObjData<int> pd{pop_};
    ASSERT_EQ(obj_data_, pd.Read()) << "Reading data from pool failed";
  } else {
    ASSERT_EQ(nullptr, pop_)
        << "Pool was unexpectedly opened after failed sync";
    ASSERT_EQ(EINVAL, errno);
    ASSERT_EQ(-1, PmempoolRepair(param.GetPathTransformed()))
        << "Pool repair should not be possible";
  }
}

std::vector<RemotePoolsetTC> GetPoolsetsWithRemoteReplicaParams() {
  std::vector<RemotePoolsetTC> ret;

  LocalTestPhase& loc_tp = LocalTestPhase::GetInstance();
  RemoteTestPhase& rem_tp = RemoteTestPhase::GetInstance();
  const auto& node = rem_tp.GetNode(0);

  const std::string rps_dir = rem_tp.GetRemotePoolsetsDir();

  std::vector<std::string> rem_safe_mnts = rem_tp.GetSafeMountpoints(node);
  std::vector<std::string> rem_unsafe_mnts = rem_tp.GetUnsafeMountpoints(node);
  const auto& loc_unsafe_dn = loc_tp.GetUnsafeDimmNamespaces();
  const auto& loc_safe_dn = loc_tp.GetSafeDimmNamespaces();

  {
    RemotePoolsetTC tc{
        "master: unsafe DIMM 0, local: unsafe DIMM 1, remote: safe DIMM 0"};
    if (loc_unsafe_dn.size() >= 2 && loc_safe_dn.size() >= 1 &&
        rem_safe_mnts.size() >= 1) {
      tc.enough_dimms_ = true;
      tc.is_syncable_ = true;

      std::string local_master_part_path =
          loc_unsafe_dn[0].GetTestDir() + SEPARATOR + "master7";
      std::string local_replica_part_path =
          loc_unsafe_dn[1].GetTestDir() + SEPARATOR + "replica7";
      std::string rp_part_path = rem_safe_mnts[0] + SEPARATOR + "remote1";

      tc.AddRemotePoolset(
          node.GetAddress(),
          Poolset{rps_dir,
                  "remote_pool1.set",
                  {{"PMEMPOOLSET", "18MB " + rp_part_path + ".part0",
                    "9MB " + rp_part_path + ".part1"}}});

      tc.local_poolset_ =
          Poolset{loc_unsafe_dn[0].GetTestDir(),
                  "pool7.set",
                  {{"PMEMPOOLSET", "9MB " + local_master_part_path + ".part0",
                    "9MB " + local_master_part_path + ".part1",
                    "9MB " + local_master_part_path + ".part2"},
                   {"REPLICA", "9MB " + local_replica_part_path + ".part0",
                    "18MB " + local_replica_part_path + ".part1"}}};
    }
    ret.emplace_back(tc);
  }

  {
    RemotePoolsetTC tc{
        "master: safe DIMM 0, local: safe DIMM 0, remote: unsafe DIMM 0"};
    if (rem_unsafe_mnts.size() >= 1 && loc_safe_dn.size() >= 1) {
      tc.enough_dimms_ = true;
      tc.is_syncable_ = true;

      std::string local_master_part_path =
          loc_safe_dn[0].GetTestDir() + SEPARATOR + "master8";
      std::string local_replica_part_path =
          loc_safe_dn[0].GetTestDir() + SEPARATOR + "replica8";
      std::string rp_part_path = rem_unsafe_mnts[0] + SEPARATOR + "remote2";

      tc.AddRemotePoolset(
          node.GetAddress(),
          Poolset{rps_dir,
                  "remote_pool2.set",
                  {{"PMEMPOOLSET", "18MB " + rp_part_path + ".part0",
                    "9MB " + rp_part_path + ".part1"}}});

      tc.local_poolset_ =
          Poolset{loc_safe_dn[0].GetTestDir(),
                  "pool_8.set",
                  {{"PMEMPOOLSET", "9MB " + local_master_part_path + ".part0",
                    "9MB " + local_master_part_path + ".part1",
                    "9MB " + local_master_part_path + ".part2"},
                   {"REPLICA", "9MB " + local_replica_part_path + ".part0",
                    "18MB " + local_replica_part_path + ".part1"}}};
    }
    ret.emplace_back(tc);
  }

  {
    RemotePoolsetTC tc{
        "Master: unsafe DIMM 0, local: safe DIMM 0, remote: unsafe DIMM 0"};

    if (rem_unsafe_mnts.size() >= 1 && loc_safe_dn.size() >= 1 &&
        loc_unsafe_dn.size() >= 1) {
      tc.enough_dimms_ = true;
      tc.is_syncable_ = true;

      std::string local_master_part_path =
          loc_unsafe_dn[0].GetTestDir() + SEPARATOR + "master9";
      std::string local_replica_part_path =
          loc_safe_dn[0].GetTestDir() + SEPARATOR + "replica9";
      std::string rp1_part_path = rem_unsafe_mnts[0] + SEPARATOR + "remote3";

      tc.AddRemotePoolset(
          node.GetAddress(),
          Poolset{rps_dir,
                  "remote_pool3.set",
                  {{"PMEMPOOLSET", "18MB " + rp1_part_path + ".part0",
                    "9MB " + rp1_part_path + ".part1"}}});
      tc.local_poolset_ =
          Poolset{loc_unsafe_dn[0].GetTestDir(),
                  "pool_9.set",
                  {{"PMEMPOOLSET", "9MB " + local_master_part_path + ".part0",
                    "9MB " + local_master_part_path + ".part1",
                    "9MB " + local_master_part_path + ".part2"},
                   {"REPLICA", "9MB " + local_replica_part_path + ".part0",
                    "18MB " + local_replica_part_path + ".part1"}}};
    }
    ret.emplace_back(tc);
  }

  {
    RemotePoolsetTC tc{
        "Master: unsafe DIMM 0, remote1: unsafe DIMM 0, remote2: non-DIMM"};
    if (rem_unsafe_mnts.size() >= 1 && loc_unsafe_dn.size() >= 1) {
      tc.enough_dimms_ = true;
      tc.is_syncable_ = true;

      std::string local_master_path =
          loc_unsafe_dn[0].GetTestDir() + SEPARATOR + "master10";
      std::string rp1_part_path = rem_unsafe_mnts[0] + SEPARATOR + "remote4";
      std::string rp2_part_path = node.GetTestDir() + SEPARATOR + "remote5";

      tc.AddRemotePoolset(
          node.GetAddress(),
          Poolset{rps_dir,
                  "remote_pool4.set",
                  {{"PMEMPOOLSET", "18MB " + rp1_part_path + ".part0",
                    "9MB " + rp1_part_path + ".part1"}}});

      tc.AddRemotePoolset(
          node.GetAddress(),
          Poolset{rps_dir,
                  "remote_pool5.set",
                  {{"PMEMPOOLSET", "18MB " + rp2_part_path + ".part0",
                    "9MB " + rp2_part_path + ".part1"}}});
      tc.local_poolset_ =
          Poolset{loc_unsafe_dn[0].GetTestDir(),
                  "pool_10.set",
                  {{"PMEMPOOLSET", "9MB " + local_master_path + ".part0",
                    "9MB " + local_master_path + ".part1",
                    "9MB " + local_master_path + ".part2"}}};
    }
    ret.emplace_back(tc);
  }

  {
    RemotePoolsetTC tc{
        "Master: unsafe DIMM 0, remote1: unsafe DIMM 0, remote2: unsafe DIMM "
        "0"};
    if (rem_unsafe_mnts.size() >= 1 && loc_unsafe_dn.size() >= 1) {
      tc.enough_dimms_ = true;
      tc.is_syncable_ = false;

      std::string local_master_part_path =
          loc_unsafe_dn[0].GetTestDir() + SEPARATOR + "master11";
      std::string rp1_part_path = rem_unsafe_mnts[0] + SEPARATOR + "remote6";
      std::string rp2_part_part = rem_unsafe_mnts[0] + SEPARATOR + "remote7";

      tc.AddRemotePoolset(
          node.GetAddress(),
          Poolset{rps_dir,
                  "remote_pool6.set",
                  {{"PMEMPOOLSET", "18MB " + rp1_part_path + ".part0",
                    "9MB " + rp1_part_path + ".part1"}}});

      tc.AddRemotePoolset(
          node.GetAddress(),
          Poolset{rps_dir,
                  "remote_pool7.set",
                  {{"PMEMPOOLSET", "18MB " + rp2_part_part + ".part0",
                    "9MB " + rp2_part_part + ".part1"}}});

      tc.local_poolset_ =
          Poolset{loc_unsafe_dn[0].GetTestDir(),
                  "pool_11.set",
                  {{"PMEMPOOLSET", "9MB " + local_master_part_path + ".part0",
                    "9MB " + local_master_part_path + ".part1",
                    "9MB " + local_master_part_path + ".part2"}}};
    }
    ret.emplace_back(tc);
  }

  {
    RemotePoolsetTC tc{
        "Master: unsafe DIMM 0, local: safe  DIMM 0, remote: unsafe DIMM "
        "0"};
    if (rem_unsafe_mnts.size() >= 1 && loc_unsafe_dn.size() >= 1) {
      tc.enough_dimms_ = true;
      tc.is_syncable_ = true;

      std::string local_master_part_path =
          loc_unsafe_dn[0].GetTestDir() + SEPARATOR + "master12";
      std::string local_replica_part_path =
          loc_safe_dn[0].GetTestDir() + SEPARATOR + "replica10";
      std::string rp_part_path = rem_unsafe_mnts[0] + SEPARATOR + "remote8";

      tc.AddRemotePoolset(
          node.GetAddress(),
          Poolset{rps_dir,
                  "remote_pool8.set",
                  {{"PMEMPOOLSET", "18MB " + rp_part_path + ".part0",
                    "9MB " + rp_part_path + ".part1"}}});

      tc.local_poolset_ =
          Poolset{loc_unsafe_dn[0].GetTestDir(),
                  "pool_12.set",
                  {{"PMEMPOOLSET", "9MB " + local_master_part_path + ".part0",
                    "9MB " + local_master_part_path + ".part1",
                    "9MB " + local_master_part_path + ".part2"},
                   {"REPLICA", "9MB " + local_replica_part_path + ".part0",
                    "18MB " + local_replica_part_path + ".part1"}}};
    }
    ret.emplace_back(tc);
  }

  return ret;
}

INSTANTIATE_TEST_CASE_P(
    UnsafeShutdown, SyncRemoteReplica,
    ::testing::ValuesIn(GetPoolsetsWithRemoteReplicaParams()));
