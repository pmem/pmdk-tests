/*
 * Copyright (c) 2018, Intel Corporation
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in
 * the documentation and/or other materials provided with the
 * distribution.
 *
 * * Neither the name of Intel Corporation nor the names of its
 * contributors may be used to endorse or promote products derived
 * from this software without specific prior written permission.
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
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY LOG OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <fstream>
#include <functional>
#include <future>
#include <iostream>
#include "pool_extend.h"
#include "../../utils/structures.h"

auto ValidateFile = [](const std::string& path) -> void {
  file_utils::ValidateFile(path, PMEMOBJ_MIN_POOL, 0664);
};

static const ExtendTestDirs& GetDirs()
{
  static const ExtendTestDirs obj {};
  return obj;
}

/**
 * PMEMOBJ_POOL_EXTEND_01
 * Extending pool that is not described as poolset
 * \test
 *          \li \c Step1. Create local pool / SUCCESS
 *          \li \c Step3. Allocate object of size greater than pool size / FAIL
 *          \li \c Step4. Check if error message is non-empty
 *          \li \c Step5. Reopen and check the pool
 */
TEST_F(ObjCtlPoolExtendTest, PMEMOBJ_POOL_EXTEND_01) {
  pool_path = pool_dir1 + "pool";

  /* Step 1 */
  pop = pmemobj_create(pool_path.c_str(), layout, 30 * MEBIBYTE, 0644);
  ASSERT_NE(pop, nullptr);
  size_t growth = 10 * MEBIBYTE;
  pmemobj_ctl_set(pop, "heap.size.granularity", &growth);

  PMEMoid oid;
  /* Step 2 */
  while (pmemobj_alloc(pop, &oid, MEBIBYTE, 0, NULL, NULL) == 0) {
  }
  /* Step 3 */
  const char* err = pmemobj_errormsg();
  ASSERT_TRUE(strlen(err) > 0);

  /* Step 4 */
  ReopenAndCheckPool(pop, pool_path);
}

/**
 * PMEMOBJ_POOL_EXTEND_02
 * Extending pool when allocation size is greater than initial pool size
 * \test
 *          \li \c Step1. Create local pool / SUCCESS
 *          \li \c Step2. Set granularity of pool extension to PMEMOBJ_MIN_PART
 *          / SUCCESS
 *          \li \c Step3. Allocate object of size greater than initial pool
 *          size / SUCCESS
 *          \li \c Step4. Reopen and check the pool
 */

/**
 * PMEMOBJ_POOL_EXTEND_15
 * Extending pool when granulation equals PMEMOBJ_MIN_PART and
 * allocation size is 100 * PMEMOBJ_MIN_PART
 *  \test
 *          \li \c Step1. Create local pool / SUCCESS
 *          \li \c Step2. Set granularity to PMEMOBJ_MIN_PART / SUCCESS
 *          \li \c Step3. Allocate object of size 100 * PMEMOBJ_MIN_PART
 *          / SUCCESS
 *          \li \c Step5. Reopen and check the pool
 */
class ObjCtlPoolExtendTestA
  : public ObjCtlPoolExtendTest,
    public ::testing::WithParamInterface<PoolExtendParamA> {};

TEST_P(ObjCtlPoolExtendTestA, EXTEND) {
  const auto& param = GetParam();

  /* Step 1 */
  Poolset poolset{pool_dir1, "pool.set", param.poolset};
  p_mgmt->CreatePoolsetFile(poolset);
  pop = pmemobj_create(poolset.GetFullPath().c_str(), layout, 0, 0644);
  ASSERT_NE(pop, nullptr);

  /* Step 2 */
  auto growth = param.growth;
  pmemobj_ctl_set(pop, "heap.size.granularity", &growth);

  /* Step 3 */
  PMEMoid oid;
  int ret = pmemobj_alloc(pop, &oid, param.alloc_size, 0, NULL, NULL);
  ASSERT_NO_FATAL_FAILURE(param.assert_f(ret));

  /* Step 4 */
  ReopenAndCheckPool(pop, poolset.GetFullPath());
}

INSTANTIATE_TEST_CASE_P(
  PoolExtend, ObjCtlPoolExtendTestA,
    ::testing::Values(
      PoolExtendParamA{ {{ "PMEMPOOLSET" }, { "OPTION SINGLEHDR" },
                        { "30M " + GetDirs().pool_dir1 }},
                        PMEMOBJ_MIN_PART,
                        2 * PMEMOBJ_MIN_PART,
                        [](int ret){ ASSERT_EQ(0, ret); } },
      PoolExtendParamA{ {{ "PMEMPOOLSET" }, { "OPTION SINGLEHDR" },
                        { "250M " + GetDirs().pool_dir1 }},
                        PMEMOBJ_MIN_PART,
                        100 * PMEMOBJ_MIN_PART,
                        [](int ret){ ASSERT_EQ(0, ret); } }
));


/**
 * PMEMOBJ_POOL_EXTEND_03
 * Extending pool when allocation size is greater than address space
 * reservation for the pool
 * \test
 *          \li \c Step1. Create local pool / SUCCESS
 *          \li \c Step2. Set granularity of pool extension / SUCCESS
 *          \li \c Step3. Allocate object of size greater than address space
 *          reservation / FAIL
 *          \li \c Step4. Check if pool was extended by allocating smaller
 *          objects
 *          \li \c Step5. Reopen and check the pool
 */
TEST_F(ObjCtlPoolExtendTest, PMEMOBJ_POOL_EXTEND_03) {
  Poolset poolset{
      pool_dir1,
      "pool.set",
      {{"PMEMPOOLSET"}, {"OPTION SINGLEHDR"},
       {"30M " + pool_dir1}}
  };

  /* Step 1 */
  p_mgmt->CreatePoolsetFile(poolset);
  pop = pmemobj_create(poolset.GetFullPath().c_str(), layout, 0, 0644);
  ASSERT_NE(pop, nullptr);

  /* Step 2 */
  size_t growth = PMEMOBJ_MIN_POOL;
  pmemobj_ctl_set(pop, "heap.size.granularity", &growth);

  /* Step 3 */
  PMEMoid oid;
  int ret = pmemobj_alloc(pop, &oid, 31 * MEBIBYTE, 0, NULL, NULL);
  ASSERT_EQ(-1, ret);

  size_t allocated = 0;
  while (pmemobj_alloc(pop, &oid, MEBIBYTE, 0, NULL, NULL) == 0) {
    allocated += pmemobj_alloc_usable_size(oid);
  }
  ASSERT_GE(allocated, 15 * MEBIBYTE);

  /* Step 4 */
  ReopenAndCheckPool(pop, poolset.GetFullPath());
}


/**
 * PMEMOBJ_POOL_EXTEND_04
 * Creating pool that is described as poolset with both files and directories
 * \test
 *          \li \c Step1. Create local pool described as poolset / FAIL
 */
/**
 * PMEMOBJ_POOL_EXTEND_04'
 * Creating pool that is described as poolset with directories without SINGLEHDR option
 * \test
 *          \li \c Step1. Create local pool described as poolset / FAIL
 */
class ObjCtlPoolExtendTestB
  : public ObjCtlPoolExtendTest,
    public ::testing::WithParamInterface<std::initializer_list<replica> > {};

TEST_P(ObjCtlPoolExtendTestB, EXTEND) {
  const auto& param = GetParam();

  Poolset poolset{pool_dir1, "pool.set", param};

  /* Step 1 */
  p_mgmt->CreatePoolsetFile(poolset);
  pop = pmemobj_create(poolset.GetFullPath().c_str(), layout, 0, 0644);
  ASSERT_TRUE(nullptr == pop);

  const char* err = pmemobj_errormsg();
  ASSERT_TRUE(strlen(err) > 0);
}

INSTANTIATE_TEST_CASE_P(
  PoolExtend, ObjCtlPoolExtendTestB,
    ::testing::Values(
      std::initializer_list<replica>{ {{"PMEMPOOLSET"}, {"OPTION SINGLEHDR"},
                                     {"30M " + GetDirs().pool_dir1 },
                                     {"20M " + GetDirs().pool_dir1 + "myfile.part0"}} },
      std::initializer_list<replica>{ {{"PMEMPOOLSET" },
                                     {"30M " + GetDirs().test_dir}} }
));


/**
 * PMEMOBJ_POOL_EXTEND_05
 * Extending pool described as poolset consisting of two directories
 * \test
 *          \li \c Step1. Create local pool / SUCCESS
 *          \li \c Step2. Set granularity of pool extension / SUCCESS
 *          \li \c Step3. Allocate object of size greater than double
 *          granularity / SUCCESS
 *          \li \c Step4. Check if necessary files were created
 *          \li \c Step5. Reopen and check the pool
 */
TEST_F(ObjCtlPoolExtendTest, PMEMOBJ_POOL_EXTEND_05) {
  Poolset poolset{pool_dir1,
                  "pool.set",
                  {{"PMEMPOOLSET"},
                   {"OPTION SINGLEHDR"},
                   {"30M " + pool_dir1},
                   {"30M " + pool_dir2}}};

  /* Step 1 */
  p_mgmt->CreatePoolsetFile(poolset);
  pop = pmemobj_create(poolset.GetFullPath().c_str(), layout, 0, 0644);
  ASSERT_NE(pop, nullptr);

  /* Step 2 */
  size_t growth = PMEMOBJ_MIN_POOL;
  pmemobj_ctl_set(pop, "heap.size.granularity", &growth);

  /* Step 3 */
  PMEMoid oid;
  int ret = pmemobj_alloc(pop, &oid, 3 * PMEMOBJ_MIN_POOL, 0, NULL, NULL);
  ASSERT_EQ(0, ret);

  /* Step 4 */
  int file_count = 0;
  ApiC::CountFilesWithAction(pools_dir, ValidateFile, &file_count);
  ASSERT_EQ(4, file_count);

  /* Step 5 */
  ReopenAndCheckPool(pop, poolset.GetFullPath());
}


/**
 * PMEMOBJ_POOL_EXTEND_06
 * Extending pool described as poolset consisting of two replicas
 * \test
 *          \li \c Step1. Create local pool / SUCCESS
 *          \li \c Step2. Set granularity of pool extension to PMEMOBJ_MIN_PART
 *          / SUCCESS
 *          \li \c Step3. Allocate objects until the whole space is reserved
 *          / SUCCESS
 *          \li \c Step4. Check if both replicas have the same size allocated
 *          and the same amount of files
 *          \li \c Step5. Reopen and check the pool
 */
TEST_F(ObjCtlPoolExtendTest, PMEMOBJ_POOL_EXTEND_06) {
  Poolset poolset{pool_dir1,
                  "pool.set",
                  {{"PMEMPOOLSET"},
                   {"OPTION SINGLEHDR"},
                   {"30M " + pool_dir1},
                   {"REPLICA"},
                   {"30M " + pool_dir2}}};

  /* Step 1 */
  p_mgmt->CreatePoolsetFile(poolset);
  pop = pmemobj_create(poolset.GetFullPath().c_str(), layout, 0, 0644);
  ASSERT_NE(pop, nullptr);

  /* Step 2 */
  size_t growth = PMEMOBJ_MIN_PART;
  pmemobj_ctl_set(pop, "heap.size.granularity", &growth);

  /* Step 3 */
  PMEMoid oid;
  size_t allocated = 0;
  while (pmemobj_alloc(pop, &oid, MEBIBYTE, 0, NULL, NULL) == 0) {
    allocated += pmemobj_alloc_usable_size(oid);
  }

  /* Step 4 */
  int file_count_rep1 = 0, file_count_rep2 = 0;
  ApiC::CountFilesWithAction(pool_dir1, [](const std::string&) -> void {},
                             &file_count_rep1);
  ApiC::CountFilesWithAction(pool_dir2, [](const std::string&) -> void {},
                             &file_count_rep2);
  ASSERT_EQ(file_count_rep1, file_count_rep2);

  /* Step 5 */
  ReopenAndCheckPool(pop, poolset.GetFullPath());
}

/**
 * PMEMOBJ_POOL_EXTEND_07
 * Extending pool described as poolset consisting of two replicas where second
 * replica has lesser address space reservation
 * \test
 *          \li \c Step1. Create local pool / SUCCESS
 *          \li \c Step2. Allocate objects until the whole space is reserved
 *          / SUCCESS
 *          \li \c Step3. Check if usable size is smaller than size of second
 *          replica
 *          \li \c Step4. Reopen and check the pool
 */
/**
 * PMEMOBJ_POOL_EXTEND_08
 * Extending pool with granularity set to 0
 * \test
 *          \li \c Step1. Create local pool / SUCCESS
 *          \li \c Step2. Allocate objects until the whole space is reserved
 *          / SUCCESS
 *          \li \c Step3. Check if usable size is lesser than PMEMOBJ_MIN_POOL
 *          \li \c Step4. Reopen and check the pool
 */

class ObjCtlPoolExtendTestC
  : public ObjCtlPoolExtendTest,
    public ::testing::WithParamInterface<PoolExtendParamA> {};

TEST_P(ObjCtlPoolExtendTestC, EXTEND) {
  const auto& param = GetParam();

  /* Step 1 */
  Poolset poolset{pool_dir1, "pool.set", param.poolset};
  p_mgmt->CreatePoolsetFile(poolset);
  pop = pmemobj_create(poolset.GetFullPath().c_str(), layout, 0, 0644);
  ASSERT_NE(pop, nullptr);

  /* Step 2 */
  auto growth = param.growth;
  pmemobj_ctl_set(pop, "heap.size.granularity", &growth);

  /* Step 3 */
  PMEMoid oid;
  size_t allocated = 0;
  while (pmemobj_alloc(pop, &oid, param.alloc_size, 0, NULL, NULL) == 0) {
    allocated += pmemobj_alloc_usable_size(oid);
  }
  ASSERT_NO_FATAL_FAILURE(param.assert_f(allocated));

  /* Step 4 */
  ReopenAndCheckPool(pop, poolset.GetFullPath());
}

INSTANTIATE_TEST_CASE_P(
  PoolExtend, ObjCtlPoolExtendTestC,
    ::testing::Values(
      PoolExtendParamA{ {{"PMEMPOOLSET"},
                       {"OPTION SINGLEHDR"},
                       {"30M " + GetDirs().pool_dir1},
                       {"REPLICA"},
                       {"20M " + GetDirs().pool_dir2}},
                        PMEMOBJ_MIN_PART,
                        MEBIBYTE,
                        [](size_t allocated){ ASSERT_LE(allocated, 20 * MEBIBYTE); } },
      PoolExtendParamA{ {{"PMEMPOOLSET"},
                       {"OPTION SINGLEHDR"},
                       {"30M " + GetDirs().pool_dir1},
                       {"REPLICA"},
                       {"20M " + GetDirs().pool_dir2}},
                        0,
                        MEBIBYTE,
                        [](size_t allocated){ ASSERT_LE(allocated, PMEMOBJ_MIN_POOL); }}
));


/**
 * PMEMOBJ_POOL_EXTEND_09
 * Extending pool when single allocation size is greater than granularity
 * \test
 *          \li \c Step1. Create local pool / SUCCESS
 *          \li \c Step2. Set granularity of pool extension to PMEMOBJ_MIN_POOL
 *          / SUCCESS
 *          \li \c Step3. Allocate objects with size greater than granularity
 *          / SUCCESS
 *          \li \c Step4. Check if usable size is equal to requested allocation
 *          and if file number is equal to floor((address space reservation -
 *          PMEMOBJ_MIN_POOL) / granularity) + 1
 *          \li \c Step5. Reopen and check the pool
 */
TEST_F(ObjCtlPoolExtendTest, PMEMOBJ_POOL_EXTEND_09) {
  Poolset poolset{
      pool_dir1,
      "pool.set",
      {{"PMEMPOOLSET"}, {"OPTION SINGLEHDR"}, {"50M " + pool_dir1}}};

  /* Step 1 */
  p_mgmt->CreatePoolsetFile(poolset);
  pop = pmemobj_create(poolset.GetFullPath().c_str(), layout, 0, 0644);
  ASSERT_NE(pop, nullptr);

  /* Step 2 */
  size_t growth = PMEMOBJ_MIN_POOL;
  pmemobj_ctl_set(pop, "heap.size.granularity", &growth);
  ASSERT_EQ(0, errno);

  /* Step 3 */
  PMEMoid oid;
  size_t allocated = 0;
  while (pmemobj_alloc(pop, &oid, 5 * PMEMOBJ_MIN_POOL, 0, NULL, NULL) == 0) {
    allocated += pmemobj_alloc_usable_size(oid);
  }

  /* Step 4 */
  ASSERT_GE(allocated, 5 * PMEMOBJ_MIN_POOL);
  int file_count = 0;
  ApiC::CountFilesWithAction(pools_dir, ValidateFile, &file_count);
  ASSERT_EQ(floor((50 * MEBIBYTE) / growth) + 1, file_count);

  /* Step 5 */
  ReopenAndCheckPool(pop, poolset.GetFullPath());
}

/**
 * PMEMOBJ_POOL_EXTEND_10
 * Extending pool when granularity is less than PMEMOBJ_MIN_PART
 * \test
 *          \li \c Step1. Create local pool / SUCCESS
 *          \li \c Step2. Set granularity of pool extension to
 *          PMEMOBJ_MIN_PART -1 / FAIL
 *          \li \c Step3. Check if error message is non-empty
 *          \li \c Step4. Reopen and check the pool
 */
TEST_F(ObjCtlPoolExtendTest, PMEMOBJ_POOL_EXTEND_10) {
  Poolset poolset{
      pool_dir1,
      "pool.set",
      {{"PMEMPOOLSET"}, {"OPTION SINGLEHDR"}, {"30M " + pool_dir1}}};

  /* Step 1 */
  p_mgmt->CreatePoolsetFile(poolset);
  pop = pmemobj_create(poolset.GetFullPath().c_str(), layout, 0, 0644);
  ASSERT_NE(pop, nullptr);

  /* Step 2 */
  size_t growth = PMEMOBJ_MIN_PART - 1;
  pmemobj_ctl_set(pop, "heap.size.granularity", &growth);

  /* Step 3 */
  const char* err = pmemobj_errormsg();
  ASSERT_TRUE(strlen(err) > 0);

  /* Step 4 */
  ReopenAndCheckPool(pop, poolset.GetFullPath());
}

/**
 * PMEMOBJ_POOL_EXTEND_11
 * Extending pool when granularity is set to 0 and allocation size is
 * PMEMOBJ_MIN_POOL
 * \test
 *          \li \c Step1. Create local pool / SUCCESS
 *          \li \c Step2. Set granularity of pool extension to 0 / SUCCESS
 *          \li \c Step3. Enable collecting statistics
 *          \li \c Step4. Try to allocate objects until the whole space is reserved
 *          / SUCCESS
 *          \li \c Step5. Check if stats.heap.curr_allocated equals 0
 *          \li \c Step6. Reopen and check the pool
 */
TEST_F(ObjCtlPoolExtendTest, PMEMOBJ_POOL_EXTEND_11) {
  Poolset poolset{
       pool_dir1,
       "pool.set",
      {{"PMEMPOOLSET"}, {"OPTION SINGLEHDR"}, {"30M " + pool_dir1}}};

  /* Step 1 */
  p_mgmt->CreatePoolsetFile(poolset);
  pop = pmemobj_create(poolset.GetFullPath().c_str(), layout, 0, 0644);
  ASSERT_NE(pop, nullptr);

  /* Step 2 */
  size_t growth = 0;
  pmemobj_ctl_set(pop, "heap.size.granularity", &growth);

  /* Step 3 */
  int enabled = 1;
  pmemobj_ctl_set(pop, "stats.enabled", &enabled);

  /* Step 4 */
  PMEMoid oid;
  while (pmemobj_alloc(pop, &oid, PMEMOBJ_MIN_POOL, 0, NULL, NULL) == 0) {
  }

  /* Step 5 */
  size_t allocated = -1;
  pmemobj_ctl_get(pop, "stats.heap.curr_allocated", &allocated);
  ASSERT_EQ(0, allocated);

  /* Step 4 */
  ReopenAndCheckPool(pop, poolset.GetFullPath());
}

/**
 * PMEMOBJ_POOL_EXTEND_12
 * Decreasing address space reservation between application instances
 * \test
 *          \li \c Step1. Create local pool / SUCCESS
 *          \li \c Step2. Allocate objects until the whole space is reserved
 *          / SUCCESS
 *          \li \c Step3. Close the pool
 *          \li \c Step4. Decrease size in poolset file
 *          \li \c Step5. Restart application
 *          \li \c Step6. Open the pool / FAIL
 */
TEST_F(ObjCtlPoolExtendTest, PMEMOBJ_POOL_EXTEND_12) {
  Poolset poolset{
      pool_dir1,
      "pool.set",
      {{"PMEMPOOLSET"}, {"OPTION SINGLEHDR"}, {"50M " + pool_dir1}}};
  /* Step 1 */
  p_mgmt->CreatePoolsetFile(poolset);
  pop = pmemobj_create(poolset.GetFullPath().c_str(), layout, 0, 0644);
  ASSERT_NE(pop, nullptr);

  /* Step 2 */
  PMEMoid oid;
  while (pmemobj_alloc(pop, &oid, PMEMOBJ_MIN_POOL, 0, NULL, NULL) == 0) {
  }

  /* Step 3 */
  pmemobj_close(pop);

  /* Step 4,5,6 */
  auto output = shell.ExecuteCommand("POOL_EXTEND_TEST " + pool_dir1 + " " +
                                     layout + " 30M");
  ASSERT_EQ(0, output.GetExitCode());
}

/**
 * PMEMOBJ_POOL_EXTEND_13
 * Increasing address space reservation between application instances
 * \test
 *          \li \c Step1. Create local pool / SUCCESS
 *          \li \c Step2. Set granularity of pool extension to PMEMOBJ_MIN_PART
 *          / SUCCESS
 *          \li \c Step3. Allocate objects until the whole space is reserved
 *          / SUCCESS
 *          \li \c Step4. Close the pool
 *          \li \c Step5. Increase size in poolset file
 *          \li \c Step6. Restart application
 *          \li \c Step7. Allocate objects until the whole space is reserved
 *          / SUCCESS
 *          \li \c Step8. Check if usable size is in tolerance with
 *          address space reservation
 *          \li \c Step9. Reopen the pool / SUCCESS
 */
TEST_F(ObjCtlPoolExtendTest, PMEMOBJ_POOL_EXTEND_13) {
  Poolset poolset{
      test_dir,
      "pool.set",
      {{"PMEMPOOLSET"}, {"OPTION SINGLEHDR"}, {"30M " + pool_dir1}}};

  /* Step 1 */
  p_mgmt->CreatePoolsetFile(poolset);
  pop = pmemobj_create(poolset.GetFullPath().c_str(), layout, 0, 0644);
  ASSERT_NE(pop, nullptr);

  /* Step 2 */
  PMEMoid oid;
  size_t growth = PMEMOBJ_MIN_PART;
  pmemobj_ctl_set(pop, "heap.size.granularity", &growth);

  /* Step 3 */
  size_t allocated = 0;
  while (pmemobj_alloc(pop, &oid, PMEMOBJ_MIN_POOL, 0, NULL, NULL) == 0) {
    allocated += pmemobj_alloc_usable_size(oid);
  }

  /* Step 4 */
  pmemobj_close(pop);

  /* Step 5,6 */
  auto output = shell.ExecuteCommand("./POOL_EXTEND_TEST " + test_dir + " " +
                                     pool_dir1 + " " + layout + " 50M 0");
  ASSERT_EQ(0, output.GetExitCode());
  pop = pmemobj_open(poolset.GetFullPath().c_str(), layout);
  ASSERT_NE(pop, nullptr);

  /* Step 7 */
  while (pmemobj_alloc(pop, &oid, PMEMOBJ_MIN_POOL, 0, NULL, NULL) == 0) {
    allocated += pmemobj_alloc_usable_size(oid);
  }

  /* Step 8 */
  ASSERT_GE(allocated, 0.8 * 50 * MEBIBYTE);

  /* Step 9 */
  ReopenAndCheckPool(pop, poolset.GetFullPath());
}

/**
 * PMEMOBJ_POOL_EXTEND_14
 * Extending pool when address space reservation is greater than volume size
 * \test
 *          \li \c Step1. Create local pool / SUCCESS
 *          \li \c Step2. Allocate objects until the whole space is reserved
 *          / SUCCESS
 *          \li \c Step3. Check if disk space is filled up
 *          \li \c Step4. Reopen and check the pool
 */
TEST_F(ObjCtlPoolExtendTest, PMEMOBJ_POOL_EXTEND_14) {
  long long pool_size = cmd.GetFreeSpaceT(pool_dir1);
  Poolset poolset{pool_dir1,
                  "pool.set",
                  {{"PMEMPOOLSET"},
                   {"OPTION SINGLEHDR"},
                   {std::to_string(pool_size) + " " + pool_dir1}}};
  /* Step 1 */
  p_mgmt->CreatePoolsetFile(poolset);
  pop = pmemobj_create(poolset.GetFullPath().c_str(), layout, 0, 0644);
  ASSERT_NE(pop, nullptr);

  /* Step 2 */
  PMEMoid oid;
  while (pmemobj_alloc(pop, &oid, 64 * MEBIBYTE, 0, NULL, NULL) == 0) {
  }

  /* Step 3 */
  unsigned long long free_space = cmd.GetFreeSpaceT(pool_dir1);
  ASSERT_LE(free_space, 128 * MEBIBYTE);

  /* Step 4 */
  ReopenAndCheckPool(pop, poolset.GetFullPath());
}




/**
 * PMEMOBJ_POOL_MANUAL_EXTEND_01
 * Manually extending pool that is not described as poolset
 * \test
 *          \li \c Step1. Create local pool / SUCCESS
 *          \li \c Step2. Try to extend the pool / FAIL
 *          \li \c Step3. Reopen and check the pool
 */

TEST_F(ObjCtlPoolExtendTest, PMEMOBJ_POOL_MANUAL_EXTEND_01) {
  pool_path = pool_dir1 + "pool";

  /* Step 1 */
  pop = pmemobj_create(pool_path.c_str(), layout, 30 * MEBIBYTE, 0644);
  ASSERT_NE(pop, nullptr);

  /* Step 2 */
  size_t growth = MEBIBYTE;
  int ret = pmemobj_ctl_exec(pop, "heap.size.extend", &growth);
  ASSERT_EQ(-1, ret);
  const char* err = pmemobj_errormsg();
  ASSERT_TRUE(strlen(err) > 0);

  /* Step 3 */
  ReopenAndCheckPool(pop, pool_path);
}


/**
 * PMEMOBJ_POOL_MANUAL_EXTEND_02
 * Checking if automatic extending pool respects size provided in poolset if
 * pool is also manually extended
 * \test
 *          \li \c Step1. Create local pool / SUCCESS
 *          \li \c Step2. Set granularity of pool extension / SUCCESS
 *          \li \c Step3. Manually extend the pool / SUCCESS
 *          \li \c Step4. Allocate objects until the whole space is reserved
 *          / SUCCESS
 *          \li \c Step5. Check if usable size is lesser than addres
 *          space reservation set in poolset
 *          \li \c Step6. Reopen and check the pool
 */
TEST_F(ObjCtlPoolExtendTest, PMEMOBJ_POOL_MANUAL_EXTEND_02) {
  Poolset poolset{
      test_dir,
      "pool.set",
      {{"PMEMPOOLSET"}, {"OPTION SINGLEHDR"}, {"30M " + pool_dir1}}};

  /* Step 1 */
  p_mgmt->CreatePoolsetFile(poolset);
  pop = pmemobj_create(poolset.GetFullPath().c_str(), layout, 0, 0644);
  ASSERT_NE(pop, nullptr);

  /* Step 2 */
  size_t growth = 2 * PMEMOBJ_MIN_PART;
  pmemobj_ctl_set(pop, "heap.size.granularity", &growth);

  /* Step 3 */
  int ret = pmemobj_ctl_exec(pop, "heap.size.extend", &growth);
  ASSERT_EQ(0, ret);

  /* Step 4 */
  PMEMoid oid;
  size_t allocated = 0;
  while (pmemobj_alloc(pop, &oid, 4 * MEBIBYTE, 0, NULL, NULL) == 0) {
    allocated += pmemobj_alloc_usable_size(oid);
  }

  /* Step 5 */
  ASSERT_LE(allocated + growth, 30 * MEBIBYTE);

  /* Step 6 */
  ReopenAndCheckPool(pop, poolset.GetFullPath());
}

/**
 * PMEMOBJ_POOL_MANUAL_EXTEND_03
 * Manually extending pool with size less than PMEMOBJ_MIN_PART
 * \test
 *          \li \c Step1. Create local pool / SUCCESS
 *          \li \c Step2. Try to extend the pool / FAIL
 *          \li \c Step3. Check if error message is non-empty
 *          \li \c Step4. Reopen and check the pool
 */
TEST_F(ObjCtlPoolExtendTest, PMEMOBJ_POOL_MANUAL_EXTEND_03) {
  Poolset poolset{
      test_dir,
      "pool.set",
      {{"PMEMPOOLSET"}, {"OPTION SINGLEHDR"}, {"30M " + pool_dir1}}};

  /* Step 1 */
  p_mgmt->CreatePoolsetFile(poolset);
  pop = pmemobj_create(poolset.GetFullPath().c_str(), layout, 0, 0644);
  ASSERT_NE(pop, nullptr);

  /* Step 2 */
  size_t growth = PMEMOBJ_MIN_PART - 1;
  int ret = pmemobj_ctl_exec(pop, "heap.size.extend", &growth);
  ASSERT_EQ(-1, ret);

  /* Step 3 */
  const char* err = pmemobj_errormsg();
  ASSERT_TRUE(strlen(err) > 0);

  /* Step 4 */
  ReopenAndCheckPool(pop, poolset.GetFullPath());
}

/**
 * PMEMOBJ_POOL_MANUAL_EXTEND_04
 * Manually extending pool described as poolset with two replicas with valid size
 * \test
 *          \li \c Step1. Create local pool / SUCCESS
 *          \li \c Step2. Try to extend the pool / SUCCESS
 *          \li \c Step3. Allocate objects until it is possible
 *          / SUCCESS
 *          \li \c Step4. Check if files were created for both replicas
 *          \li \c Step5. Reopen and check the pool
 */
TEST_F(ObjCtlPoolExtendTest, PMEMOBJ_POOL_MANUAL_EXTEND_04) {
  Poolset poolset{test_dir,
                  "pool.set",
                  {{"PMEMPOOLSET"},
                   {"OPTION SINGLEHDR"},
                   {"30M " + pool_dir1},
                   {"REPLICA"},
                   {"30M " + pool_dir2}}};

  /* Step 1 */
  p_mgmt->CreatePoolsetFile(poolset);
  pop = pmemobj_create(poolset.GetFullPath().c_str(), layout, 0, 0644);
  ASSERT_NE(pop, nullptr);

  /* Step 2 */
  size_t growth = 20 * MEBIBYTE;
  int ret = pmemobj_ctl_exec(pop, "heap.size.extend", &growth);
  ASSERT_EQ(0, ret);

  /* Step 3 */
  PMEMoid oid;
  while (pmemobj_alloc(pop, &oid, 4 * MEBIBYTE, 0, NULL, NULL) == 0) {
  }

  /* Step 4 */
  int file_count_rep1 = 0, file_count_rep2 = 0;
  ApiC::CountFilesWithAction(pool_dir1, [](const std::string&) -> void {},
                             &file_count_rep1);
  ApiC::CountFilesWithAction(pool_dir2, [](const std::string&) -> void {},
                             &file_count_rep2);
  ASSERT_EQ(file_count_rep1, file_count_rep2);

  /* Step 5 */
  ReopenAndCheckPool(pop, poolset.GetFullPath());
}

/**
 * PMEMOBJ_POOL_MANUAL_EXTEND_05
 * Manually extending pool that is described as poolset with files
 * \test
 *          \li \c Step1. Create local pool / SUCCESS
 *          \li \c Step2. Try to extend the pool / FAIL
 *          \li \c Step3. Reopen and check the pool
 */
TEST_F(ObjCtlPoolExtendTest, PMEMOBJ_POOL_MANUAL_EXTEND_08) {
  Poolset poolset{pool_dir1,
                  "pool.set",
                  {{"PMEMPOOLSET"},
                   {"OPTION SINGLEHDR"},
                   {"30M " + pool_dir1 + "myfile.part0"},
                   {"20M " + pool_dir1 + "myfile.part1"}}};

  /* Step 1 */
  p_mgmt->CreatePoolsetFile(poolset);
  pop = pmemobj_create(poolset.GetFullPath().c_str(), layout, 0, 0644);
  ASSERT_NE(pop, nullptr);

  size_t growth = 31 * MEBIBYTE;

  /* Step 2 */
  int ret = pmemobj_ctl_exec(pop, "heap.size.extend", &growth);
  ASSERT_EQ(-1, ret);
  const char* err = pmemobj_errormsg();
  ASSERT_TRUE(strlen(err) > 0);

  /* Step 3 */
  ReopenAndCheckPool(pop, poolset.GetFullPath());
}

/**
 * PMEMOBJ_POOL_MULTITHREAD_ALLOC_01
 * Checking if automatic pool extending is thread safe
 * \test
 *          \li \c Step1. Create local pool / SUCCESS
 *          \li \c Step2. Set minimal granularity of pool extension
 *          / SUCCESS
 *          \li \c Step3. Allocate 10 objects of size PMEMOBJ_MIN_PART
 *          in each of 5 separate threads / SUCCESS
 *          \li \c Step4. Reopen and check the pool
 */
/**
 * PMEMOBJ_POOL_MULTITHREAD_ALLOC_02
 * Checking if manual pool extending is thread safe
 * \test
 *          \li \c Step1. Create local pool / SUCCESS
 *          \li \c Step2. Set minimal granularity of pool extension
 *          / SUCCESS
 *          \li \c Step3. Perform manual pool extension (extend by
 *          PMEMOBJ_MIN_PART size) 10 times in each of 5 separate threads
 *          / SUCCESS
 *          \li \c Step4. Reopen and check the pool
 */
/**
 * PMEMOBJ_POOL_MULTITHREAD_ALLOC_03
 * Checking if parallel single manual and multiple auto pool extending
 * is thread safe
 * \test
 *          \li \c Step1. Create local pool / SUCCESS
 *          \li \c Step2. Set minimal granularity of pool extension
 *          / SUCCESS
 *          \li \c Step3. Allocate 10 objects of size PMEMOBJ_MIN_PART
 *          in each of 5 separate threads and extend pool manually
 *          (1 x by PMEMOBJ_MIN_PART) in 1 separate thread / SUCCESS
 *          \li \c Step4. Reopen and check the pool
 */
/**
 * PMEMOBJ_POOL_MULTITHREAD_ALLOC_04
 * Checking if parallel multiple manual and multiple auto pool extending
 * is thread safe
 * \test
 *          \li \c Step1. Create local pool / SUCCESS
 *          \li \c Step2. Set minimal granularity of pool extension
 *          / SUCCESS
 *          \li \c Step3. Allocate 10 objects of size PMEMOBJ_MIN_PART
 *          in each of 5 separate threads and extend pool manually
 *          (10 x by PMEMOBJ_MIN_PART) in another 5 separate threads
 *          / SUCCESS
 *          \li \c Step4. Reopen and check the pool
 */
class ObjCtlPoolMTExtendTest
  : public ObjCtlPoolExtendTest,
    public ::testing::WithParamInterface<PoolExtendMultiThreadParam> {};

TEST_P(ObjCtlPoolMTExtendTest, EXTEND) {
  auto& param = GetParam();

  Poolset poolset{pool_dir1, "pool.set", param.poolset};

  /* Step 1 */
  p_mgmt->CreatePoolsetFile(poolset);
  pop = pmemobj_create(poolset.GetFullPath().c_str(), layout, 0, 0644);
  ASSERT_NE(pop, nullptr);

  /* Step 2 */
  size_t growth = PMEMOBJ_MIN_PART;
  pmemobj_ctl_set(pop, "heap.size.granularity", &growth);

  /* Step 3 */
  PMEMoid oid;
  auto function_runner = [](int no_of_runs,
                            std::function<int()> function_to_run) -> bool {
  for (int i = 0; i < no_of_runs; ++i) {
    if (function_to_run())
      return false;
    }
    return true;
  };

  auto alloc = [this, &oid]() {
    return pmemobj_alloc(this->pop, &oid, PMEMOBJ_MIN_PART, 0, NULL, NULL);
  };

  auto extend = [this, &growth]() {
    return pmemobj_ctl_exec(this->pop, "heap.size.extend", &growth);
  };

  std::vector<std::future<bool>> fut_allocs, fut_extends;
  // Perform allocations
  for (size_t i = 0; i < param.no_of_alloc_threads; ++i) {
    fut_allocs.emplace_back(std::async(std::launch::async, function_runner,
                                       param.no_allocs_per_thread, alloc));
  }

  // Perform extensions
  for (size_t i = 0; i < param.no_of_extend_threads; ++i) {
    fut_extends.emplace_back(std::async(std::launch::async, function_runner,
                                        param.no_extends_per_thread, extend));
  }

  bool result = true;
  std::for_each(fut_allocs.begin(), fut_allocs.end(),
                [&result](std::future<bool>& fut) { result &= fut.get(); });
  std::for_each(fut_extends.begin(), fut_extends.end(),
                [&result](std::future<bool>& fut) { result &= fut.get(); });
  ASSERT_EQ(result, true);

  /* Step 4 */
  ReopenAndCheckPool(pop, poolset.GetFullPath());
}

INSTANTIATE_TEST_CASE_P(
  PoolExtend, ObjCtlPoolMTExtendTest,
  ::testing::Values(
    PoolExtendMultiThreadParam{
        { {"PMEMPOOLSET"}, {"OPTION SINGLEHDR"}, {"1G " + GetDirs().pool_dir1} },
        5, 10,
        0, 0},
    PoolExtendMultiThreadParam{
        { {"PMEMPOOLSET"}, {"OPTION SINGLEHDR"}, {"1G " + GetDirs().pool_dir1} },
        0, 0,
        5, 10},
    PoolExtendMultiThreadParam{
        { {"PMEMPOOLSET"}, {"OPTION SINGLEHDR"}, {"1G " + GetDirs().pool_dir1} },
        5, 10,
        1, 1},
    PoolExtendMultiThreadParam{
        { {"PMEMPOOLSET"}, {"OPTION SINGLEHDR"}, {"1G " + GetDirs().pool_dir1} },
        5, 10,
        5, 10}
));
