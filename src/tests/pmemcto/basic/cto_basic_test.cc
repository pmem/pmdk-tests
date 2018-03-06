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

#include "cto_basic.h"
#include <poolset/poolset_management.h>
#include <array>
#include <limits>
#include <memory>
#include <numeric>
#include "../../../utils/api_c/api_c.h"

namespace CtoCreateTests {
/**
 * PMEMCTO_INVALID_CREATE
 * Creating pmemcto pool whilst pool or poolset file already exists
 * Following scenarios are considered:
 * - pmemcto_create() points to a pool set file, but poolsize is not zero
 * - pmemcto_create() points to an existing file, which is not empty
 * - pmemcto_create() points to an existing file, which is empty
 * \test
 *          \li \c Step1. Depending on test scenario create empty/not-empty file or poolset file
 *          \li \c Step2. Create pmemcto pool pointing to an existing file / FAIL: pcp = NULL, errno = EEXIST
 */
TEST_P(CtoCreate, PMEMCTO_INVALID_CREATE) {
  Scenario s = GetParam();
  Poolset p{{"PMEMPOOLSET", "20M"}};
  /* Step 1 */
  switch (s) {
    case Scenario::EMPTY_FILE_EXISTS:
      ApiC::CreateFileT(pool_path, "");
      break;
    case Scenario::FILE_EXISTS:
      ApiC::CreateFileT(pool_path, "data");
      break;
    case Scenario::POOLSET_FILE_EXISTS:
      PoolsetManagement p_mgmt;
      p_mgmt.CreatePoolsetFile(p);
      pool_path = p.GetFullPath();
      break;
  }
  pcp = pmemcto_create(pool_path.c_str(), nullptr, PMEMCTO_MIN_POOL, 0666);
  EXPECT_EQ(nullptr, pcp);
  EXPECT_EQ(EEXIST, errno);
  errno = 0;
  /* Step 2 */
  ApiC::RemoveFile(pool_path);
}

INSTANTIATE_TEST_CASE_P(Invalid, CtoCreate,
                        ::testing::Values(Scenario::EMPTY_FILE_EXISTS,
                                          Scenario::FILE_EXISTS,
                                          Scenario::POOLSET_FILE_EXISTS));
}
namespace CtoBasicTests2 {
/**
 * PMEMCTO_INVALID_OPEN_CHECK
 * Opening and checking consistency of pmemcto pool with invalid arguments.
 * Following scenarios are considered:
 * - opening the pool twice / FAIL: pcp = NULL, errno = EAGAIN
 * - checking consistency of opened pool / FAIL: ret = -1, errno = EAGAIN
 * - opening the pool with invalid layout / FAIL: pcp = NULL, errno = EINVAL
 * - checking consistency of the pool with invalid layout / FAIL: ret = -1, errno = EINVAL
 * - opening read only pool / FAIL: pcp = NULL, errno = EACCESS
 * - checking consistency of read only pool / FAIL: ret = -1, errno = EACCESS
 * \test
 *          \li \c Step1. Create pmemcto pool / SUCCESS
 *          \li \c Step2. Close pmemcto pool / SUCCESS
 *          \li \c Step3. Run test scenario
 */
TEST_P(CtoNegative, PMEMCTO_INVALID_OPEN_CHECK) {
  /* Step 1 */
  pcp = pmemcto_create(pool_path.c_str(), "valid", PMEMCTO_MIN_POOL, 0666);
  EXPECT_TRUE(pcp != nullptr) << pmemcto_errormsg();
  /* Step 2 */
  pmemcto_close(pcp);

  Scenario s;
  s = GetParam();
  int err;
  std::string layout = "valid";
  /* Step 3 */
  switch (s) {
    case Scenario::ALREADY_OPENED:
      pcp = pmemcto_open(pool_path.c_str(), layout.c_str());
#ifdef _WIN32
      err = EWOULDBLOCK;
#else
      err = EAGAIN;
#endif
      break;
    case Scenario::READ_ONLY:
      ApiC::SetFilePermission(pool_path, 0444);
      err = EACCES;
      break;
    case Scenario::LAYOUT_MISMATCH:
      layout = "invalid";
      err = EINVAL;
      break;
  }

  int ret = pmemcto_check(pool_path.c_str(), layout.c_str());
  EXPECT_EQ(-1, ret);
  EXPECT_EQ(err, errno);
  errno = 0;

  PMEMctopool *pcp2 = pmemcto_open(pool_path.c_str(), layout.c_str());
  EXPECT_EQ(nullptr, pcp2);
  EXPECT_EQ(err, errno);
  errno = 0;

  if (s == Scenario::ALREADY_OPENED) {
    pmemcto_close(pcp);
  }
  if (s == Scenario::READ_ONLY) {
    ApiC::SetFilePermission(pool_path, 0666);
  }
  ApiC::RemoveFile(pool_path);
}

INSTANTIATE_TEST_CASE_P(Invalid, CtoNegative,
                        ::testing::Values((Scenario{Scenario::ALREADY_OPENED}),
                                          (Scenario{Scenario::LAYOUT_MISMATCH}),
                                          (Scenario{Scenario::READ_ONLY})));
}
namespace CtoBasicTests {
/**
 * PMEMCTO_VALID_LAYOUT
 * Creating opening and checking consistency of pmemcto pool
 * Following scenarios are considered:
 * - layout is empty
 * - layout is equal to PMEMCTO_MAX_LAYOUT
 * \test
 *          \li \c Step1. Create pmemcto pool / SUCCESS
 *          \li \c Step2. Close pmemecto pool / SUCCESS
 *          \li \c Step3. Open pmemcto pool / SUCCESS
 *          \li \c Step4. Close pmemecto pool / SUCCESS
 *          \li \c Step5. Check consistency of pmemcto pool / SUCCESS: ret = 1
 */
TEST_P(CtoBasicPositive, PMEMCTO_VALID_LAYOUT) {
  in_args i_args = GetParam();
  /* Step 1 */
  pcp = pmemcto_create(pool_path.c_str(), i_args.layout.c_str(), i_args.size,
                       0666);
  EXPECT_TRUE(pcp != nullptr) << pmemcto_errormsg();
  /* Step 2 */
  pmemcto_close(pcp);
  /* Step 3 */
  pcp = pmemcto_open(pool_path.c_str(), i_args.layout.c_str());
  EXPECT_TRUE(pcp != nullptr) << pmemcto_errormsg();
  /* Step 4 */
  pmemcto_close(pcp);
  /* Step 5 */
  int ret = pmemcto_check(pool_path.c_str(), i_args.layout.c_str());
  EXPECT_EQ(1, ret);
  /* Cleanup */
  ApiC::RemoveFile(pool_path);
}

INSTANTIATE_TEST_CASE_P(
    Valid, CtoBasicPositive,
    ::testing::Values(in_args{},
                      in_args{{std::string(PMEMCTO_MAX_LAYOUT - 1, 'x')}}));

/**
 * PMEMCTO_BASIC_CREATE
 * Creating pmemcto pool with invalid arguments
 * Following scenarios are considered:
 * Invalid pool size:
 * - 0 / errno = ENOENT
 * - 1 / errno = EINVAL
 * - PMEMCTO_MIN_POOL - 1 / errno = EINVAL
 * - max value of size_t type / errno = EFBIG
 * Invalid layout:
 * - longer than PMEMCTO_MAX_LAYOUT / errno = EINVAL
 * \test
 *          \li \c Step1. Create pmemcto pool / FAIL: pcp = NULL, expected errno is set
 */
TEST_P(CtoBasicNegative, PMEMCTO_BASIC_CREATE) {
  in_args i_args;
  int err;
  std::tie(i_args, err) = GetParam();
  /* Step 1 */
  pcp = pmemcto_create(pool_path.c_str(), i_args.layout.c_str(), i_args.size,
                       0666);
  EXPECT_EQ(nullptr, pcp);
  EXPECT_EQ(err, errno);
  errno = 0;
}

INSTANTIATE_TEST_CASE_P(
    InvalidPoolSize, CtoBasicNegative,
    ::testing::Values(
        std::make_pair(in_args{0}, ENOENT), std::make_pair(in_args{1}, EINVAL),
        std::make_pair(in_args{PMEMCTO_MIN_POOL - 1}, EINVAL),
        std::make_pair(in_args{(std::numeric_limits<size_t>::max)()}, EFBIG)));
INSTANTIATE_TEST_CASE_P(InvalidLayout, CtoBasicNegative,
                        ::testing::Values(std::make_pair(
                            in_args{/* len is PMEMCT_MAX_LAYOUT + '\0'
                                     * after conversion to C-string */
                                    std::string(PMEMCTO_MAX_LAYOUT, 'x')},
                            EINVAL)));
}

namespace CtoPoolsetTests {
/**
 * PMEMCTO_VALID_POOLSET
 * Creating pmemcto pool based on poolset file
 * Following scenarios are considered:
 * - size of the master replica is equal to PMEMCTO_MIN_POOL
 * - size of the part is equal to PMEMCTO_MIN_PART
 * \test
 *          \li \c Step1. Create poolset file
 *          \li \c Step2. Create pmemcto pool based on poolset file / SUCCESS
 *          \li \c Step3. Make sure all files are created
 *          \li \c Step4. Close pmemcto pool
 */
TEST_P(CtoPoolsetPositive, PMEMCTO_VALID_POOLSET) {
  Poolset p = GetParam();
  /* Step 1 */
  PoolsetManagement p_mgmt;
  p_mgmt.CreatePoolsetFile(p);
  /* Step 2 */
  pcp = pmemcto_create(p.GetFullPath().c_str(), nullptr, 0, 0666);
  EXPECT_TRUE(pcp != nullptr) << pmemcto_errormsg();
  /* Step 3 */
  bool allExist = p_mgmt.AllFilesExist(p);
  EXPECT_TRUE(allExist);
  /* Step 4 */
  pmemcto_close(pcp);
  /* Cleanup */
  p_mgmt.RemovePartsFromPoolset(p);
  p_mgmt.RemovePoolsetFile(p);
}

INSTANTIATE_TEST_CASE_P(
    Valid, CtoPoolsetPositive,
    ::testing::Values(
        Poolset{{"PMEMPOOLSET", std::to_string(PMEMCTO_MIN_POOL)}},
        Poolset{{"PMEMPOOLSET", "20M", std::to_string(PMEMCTO_MIN_PART)}}));

/**
 * PMEMCTO_INVALID_POOLSET
 * Creating pmemcto pool based on poolset file
 * Following scenarios are considered:
 * - size of the master replica is lower than PMEMCTO_MIN_POOL / errno = EINVAL
 * - size of the part is lower than PMEMCTO_MIN_PART / errno = EINVAL
 * - secondary replica is specified in poolset file / errno = ENOTSUP
 * \test
 *          \li \c Step1. Create poolset file
 *          \li \c Step2. Create pmemcto pool based on poolset file / FAIL: pcp = NULL, expected errno is set
 *          \li \c Step3. Make sure no files are created
 */
TEST_P(CtoPoolsetNegative, PMEMCTO_INVALID_POOLSET) {
  int err;
  Poolset p;
  std::tie(p, err) = GetParam();
  /* Step 1 */
  PoolsetManagement p_mgmt;
  p_mgmt.CreatePoolsetFile(p);
  /* Step 2 */
  pcp = pmemcto_create(p.GetFullPath().c_str(), nullptr, 0, 0666);
  EXPECT_EQ(nullptr, pcp);
  EXPECT_EQ(err, errno);
  /* Step 3 */
  bool noneExist = p_mgmt.NoFilesExist(p);
  EXPECT_TRUE(noneExist);
  /* Cleanup */
  p_mgmt.RemovePoolsetFile(p);
}

INSTANTIATE_TEST_CASE_P(
    Invalid, CtoPoolsetNegative,
    ::testing::Values(
        std::make_pair(Poolset{{"PMEMPOOLSET",
                                std::to_string(PMEMCTO_MIN_POOL - 1)}},
                       EINVAL),
        std::make_pair(Poolset{{"PMEMPOOLSET", std::to_string(PMEMCTO_MIN_POOL),
                                std::to_string(PMEMCTO_MIN_PART - 1)}},
                       EINVAL),
        std::make_pair(Poolset{{"PMEMPOOLSET", "20M"}, {"REPLICA", "20M"}},
                       ENOTSUP)));
}
namespace CtoSetFuncsTest {
/**
 * PMEMCTO_SET_FUNCS
 * Overriding default memory allocation calls used internally by libpmemcto
 * \test
 *          \li \c Step1. Override all possible memory allocations calls
 *          \li \c Step2. Run application based on libpmemcto_* functions
 *          \li \c Step3. Make sure malloc, free and stdup calls are overridden
 *          \li \c Step4. Set all memory allocation to default ones
 *          \li \c Step5. Run application based on libpmemcto_* functions
 *          \li \c Step6. Make sure calls to overriden functions are not made
 */
TEST_F(CtoBasicSetFuncs, PMEMCTO_SET_FUNCS) {
  /* Step 1 */
  pmemcto_set_funcs(my_malloc, my_free, my_realloc, my_strdup, my_print);
  /* Step 2 */
  Work();
  /* Step 3 */
  EXPECT_GT(arr[static_cast<int>(Funcs::MALLOC)], 0);
  EXPECT_GT(arr[static_cast<int>(Funcs::FREE)], 0);
  EXPECT_GT(arr[static_cast<int>(Funcs::STRDUP)], 0);
  /* Step 4 */
  pmemcto_set_funcs(nullptr, nullptr, nullptr, nullptr, nullptr);
  /* Step 5 */
  std::array<int, static_cast<int>(Funcs::Count)> arr2(arr);
  Work();
  /* Step 6 */
  for (int i = 0; i < (int)Funcs::Count; ++i) {
    EXPECT_EQ(arr[i], arr2[i]);
  }
}
}

/**
 * PMEMCTO_CHECK_VERSION
 * Checking version of installed libpmemcto library
 */
TEST(PmemctoCheckVersion, PMEMCTO_CHECK_VERSION) {
  EXPECT_TRUE(pmemcto_check_version(0, 0) != nullptr);
  EXPECT_TRUE(pmemcto_check_version(PMEMCTO_MAJOR_VERSION - 1,
                                    PMEMCTO_MINOR_VERSION) != nullptr);
  EXPECT_TRUE(pmemcto_check_version(PMEMCTO_MAJOR_VERSION + 1,
                                    PMEMCTO_MINOR_VERSION) != nullptr);
  EXPECT_TRUE(pmemcto_check_version(PMEMCTO_MAJOR_VERSION,
                                    PMEMCTO_MINOR_VERSION + 1) != nullptr);
  EXPECT_TRUE(pmemcto_check_version(PMEMCTO_MAJOR_VERSION,
                                    (std::numeric_limits<unsigned>::max)()) !=
              nullptr);
  EXPECT_TRUE(pmemcto_check_version((std::numeric_limits<unsigned>::max)(),
                                    PMEMCTO_MINOR_VERSION) != nullptr);
  EXPECT_EQ(nullptr, pmemcto_check_version(PMEMCTO_MAJOR_VERSION,
                                           PMEMCTO_MINOR_VERSION));
  if (PMEMCTO_MINOR_VERSION > 0) {
    EXPECT_EQ(nullptr, pmemcto_check_version(PMEMCTO_MAJOR_VERSION,
                                             PMEMCTO_MINOR_VERSION - 1));
  }
}
