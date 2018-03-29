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

#include "alloc_class.h"
#include <limits>
#include "alloc_class_utils.h"

using namespace std;

/**
 * PMEMOBJ_CTL_CLASS_WITH_SAME_DESC
 * Creating two allocation classes with the same description
 * \test
 *          \li \c Step1. Create pmemobj pool / SUCCESS
 *          \li \c Step2. Create allocation class automatically / SUCCESS
 *          \li \c Step3. Create allocation class automatically with the same
 *          size as above / FAIL: ret = -1, errno = EINVAL
 *          \li \c Step4. Create allocation class with an id of 128 / SUCCESS
 *          \li \c Step5. Create allocation class with an id of 129 with the
 *          same size as above / FAIL: ret = -1, errno = EINVAL
 *          \li \c Step6. Create allocation class automatically / SUCCESS
 *          \li \c Step7. Create allocation class with an id of 130 with the
 *          same size as above / FAIL: ret = -1, errno = EINVAL
 *          \li \c Step8. Close pool / SUCCESS
 */
TEST_F(ObjCtlAllocClassTest, PMEMOBJ_CTL_CLASS_WITH_SAME_DESC) {
  /* Step 1 */
  PMEMobjpool *pop =
      pmemobj_create(pool_path_.c_str(), nullptr, PMEMOBJ_MIN_POOL, 0666);
  ASSERT_TRUE(pop != nullptr) << pmemobj_errormsg();
  /* Step 2 */
  pobj_alloc_class_desc write_arg;
  write_arg.unit_size = 512;
  write_arg.alignment = 0;
  write_arg.units_per_block = 1024;
  write_arg.header_type = POBJ_HEADER_COMPACT;
  EXPECT_EQ(0, pmemobj_ctl_set(pop, "heap.alloc_class.new.desc", &write_arg))
      << pmemobj_errormsg();
  /* Step 3 */
  EXPECT_EQ(-1, pmemobj_ctl_set(pop, "heap.alloc_class.new.desc", &write_arg));
  EXPECT_EQ(EINVAL, errno);
  errno = 0;
  /* Step 4 */
  write_arg.unit_size = 1024;
  EXPECT_EQ(0, pmemobj_ctl_set(pop, "heap.alloc_class.128.desc", &write_arg))
      << pmemobj_errormsg();
  /* Step 5 */
  EXPECT_EQ(-1, pmemobj_ctl_set(pop, "heap.alloc_class.129.desc", &write_arg));
  EXPECT_EQ(EINVAL, errno);
  errno = 0;
  /* Step 6 */
  write_arg.unit_size = 2048;
  EXPECT_EQ(0, pmemobj_ctl_set(pop, "heap.alloc_class.new.desc", &write_arg))
      << pmemobj_errormsg();
  /* Step 7 */
  EXPECT_EQ(-1, pmemobj_ctl_set(pop, "heap.alloc_class.130.desc", &write_arg));
  EXPECT_EQ(EINVAL, errno);
  /* Step 8 */
  pmemobj_close(pop);
}

/**
 * PMEMOBJ_CTL_OVERWRITE_CUSTOM_CLASS
 * Overwriting custom allocation class
 * \test
 *          \li \c Step1. Create pmemobj pool / SUCCESS
 *          \li \c Step2. Create allocation class with an id of 128 / SUCCESS
 *          \li \c Step3. Create allocation class with an id of 128 /
 *          FAIL: ret = -1, errno = EEXIST
 *          \li \c Step4. Close pool / SUCCESS
 */
TEST_F(ObjCtlAllocClassTest, PMEMOBJ_CTL_OVERWRITE_CUSTOM_CLASS) {
  /* Step 1 */
  PMEMobjpool *pop =
      pmemobj_create(pool_path_.c_str(), nullptr, PMEMOBJ_MIN_POOL, 0666);
  ASSERT_TRUE(pop != nullptr) << pmemobj_errormsg();
  /* Step 2 */
  pobj_alloc_class_desc write_arg;
  write_arg.unit_size = 512;
  write_arg.alignment = 0;
  write_arg.units_per_block = 1024;
  write_arg.header_type = POBJ_HEADER_COMPACT;
  EXPECT_EQ(0, pmemobj_ctl_set(pop, "heap.alloc_class.128.desc", &write_arg))
      << pmemobj_errormsg();
  /* Step 3 */
  write_arg.unit_size = 1024;
  EXPECT_EQ(-1, pmemobj_ctl_set(pop, "heap.alloc_class.128.desc", &write_arg));
  EXPECT_EQ(EEXIST, errno);
  /* Step 4 */
  pmemobj_close(pop);
}

/**
 * PMEMOBJ_CTL_SET_ALL_ALLOCATION_CLASSES
 * Creating maximum number of allocation classes
 * \test
 *          \li \c Step1. Create pmemobj pool / SUCCESS
 *          \li \c Step2. Count number of available class ids
 *          \li \c Step3. Create all available allocation classes automatically
 *          / SUCCESS
 *          \li \c Step4. Retrieve information about all allocation classes
 *          / SUCCESS
 *          \li \c Step5. Close pool / SUCCESS
 */
TEST_F(ObjCtlAllocClassTest, PMEMOBJ_CTL_SET_ALL_ALLOCATION_CLASSES) {
  /* Step 1 */
  PMEMobjpool *pop =
      pmemobj_create(pool_path_.c_str(), nullptr, PMEMOBJ_MIN_POOL, 0666);
  ASSERT_TRUE(pop != nullptr) << pmemobj_errormsg();
  /* Step 2 */
  int free_ids = 0;
  string entry_point;
  pobj_alloc_class_desc read_arg;
  for (int i = 0; i <= 127; ++i) {
    entry_point = "heap.alloc_class." + to_string(i) + ".desc";
    if (pmemobj_ctl_get(pop, entry_point.c_str(), &read_arg) == -1) {
      ++free_ids;
    }
  }
  /* as man page states: Only values between 0-254 are valid (...)
   * values between 0-127 are reserved (...) and can be used only for reading */
  free_ids += 127;
  /* Step 3 */
  pobj_alloc_class_desc write_arg;
  write_arg.header_type = POBJ_HEADER_COMPACT;
  write_arg.alignment = 0;
  write_arg.units_per_block = 512;
  for (int i = 1; i <= free_ids; ++i) {
    write_arg.unit_size = i * 512;
    EXPECT_EQ(0, pmemobj_ctl_set(pop, "heap.alloc_class.new.desc", &write_arg))
        << pmemobj_errormsg();
  }
  /* Step 4 */
  for (int i = 0; i <= 254; ++i) {
    entry_point = "heap.alloc_class." + to_string(i) + ".desc";
    EXPECT_EQ(0, pmemobj_ctl_get(pop, entry_point.c_str(), &read_arg))
        << pmemobj_errormsg();
  }
  /* Step 5 */
  pmemobj_close(pop);
}

/**
 * PMEMOBJ_CTL_ALLOCATE_FROM_UNEXISTING_CLASS
 * Allocating objects from unexisting allocation classes with ids: 128, 254
 * \test
 *          \li \c Step1. Create pmemobj pool / SUCCESS
 *          \li \c Step2. Allocate an object from the allocation class
 *          / FAIL: ret = -1, errno = EINVAL
 *          \li \c Step3. Make sure object is not allocated
 *          \li \c Step4. Close pool / SUCCESS
 */
TEST_F(ObjCtlAllocClassTest, PMEMOBJ_CTL_ALLOCATE_FROM_UNEXISTING_CLASS) {
  /* Step 1 */
  PMEMobjpool *pop =
      pmemobj_create(pool_path_.c_str(), nullptr, PMEMOBJ_MIN_POOL, 0666);
  ASSERT_TRUE(pop != nullptr) << pmemobj_errormsg();
  /* Step 2 */
  PMEMoid oid = OID_NULL;
  std::vector<int> class_id{128, 254};
  for (const auto id : class_id) {
    EXPECT_EQ(-1, pmemobj_xalloc(pop, &oid, 1024, 0, POBJ_CLASS_ID(id), nullptr,
                                 nullptr));
    EXPECT_EQ(EINVAL, errno);
    errno = 0;
    /* Step 3 */
    EXPECT_TRUE(OID_IS_NULL(oid));
  }
  /* Step 4 */
  pmemobj_close(pop);
}

/**
 * PMEMOBJ_CTL_ALLOCATE_FROM_CLASS
 * Allocating objects from allocation classes
 * \test
 *          \li \c Step1. Create pmemobj pool / SUCCESS
 *          \li \c Step2. Retrieve information about class of id = 1 / SUCCESS
 *          \li \c Step3. Allocate an object from the allocation class with an
 *          id equal to 0 / SUCCESS
 *          \li \c Step4. Make sure object is allocated
 *          \li \c Step5. Create allocation class automatically / SUCCESS
 *          \li \c Step6. Retrieve information about allocation class / SUCCESS
 *          \li \c Step7. Allocate an object from created allocation class
 *          / SUCCESS
 *          \li \c Step8. Make sure object is allocated
 *          \li \c Step9. Close pool / SUCCESS
 */
TEST_F(ObjCtlAllocClassTest, PMEMOBJ_CTL_ALLOCATE_FROM_CLASS) {
  /* Step 1 */
  PMEMobjpool *pop =
      pmemobj_create(pool_path_.c_str(), nullptr, PMEMOBJ_MIN_POOL, 0666);
  ASSERT_TRUE(pop != nullptr) << pmemobj_errormsg();
  /* Step 2 */
  PMEMoid oid = OID_NULL;
  pobj_alloc_class_desc read_arg;
  EXPECT_EQ(0, pmemobj_ctl_get(pop, "heap.alloc_class.1.desc", &read_arg))
      << pmemobj_errormsg();
  /* Step 3 */
  EXPECT_EQ(0, pmemobj_xalloc(pop, &oid, read_arg.unit_size, 0,
                              POBJ_CLASS_ID(0), nullptr, nullptr))
      << pmemobj_errormsg();
  /* Step 4 */
  EXPECT_FALSE(OID_IS_NULL(oid));
  pmemobj_free(&oid);
  /* Step 5 */
  pobj_alloc_class_desc write_arg;
  write_arg.unit_size = 1024;
  write_arg.alignment = 0;
  write_arg.units_per_block = 1024;
  write_arg.header_type = POBJ_HEADER_COMPACT;
  EXPECT_EQ(0, pmemobj_ctl_set(pop, "heap.alloc_class.new.desc", &write_arg))
      << pmemobj_errormsg();
  /* Step 6 */
  string entry_point =
      "heap.alloc_class." + to_string(write_arg.class_id) + ".desc";
  EXPECT_EQ(0, pmemobj_ctl_get(pop, entry_point.c_str(), &read_arg));
  /* Step 7 */
  EXPECT_EQ(
      0, pmemobj_xalloc(pop, &oid,
                        read_arg.unit_size -
                            AllocClassUtils::hdrs[write_arg.header_type].size,
                        0, POBJ_CLASS_ID(write_arg.class_id), nullptr, nullptr))
      << pmemobj_errormsg();
  /* Step 8 */
  EXPECT_FALSE(OID_IS_NULL(oid));
  pmemobj_free(&oid);
  /* Step 9 */
  pmemobj_close(pop);
}

/**
 * PMEMOBJ_CTL_CHECK_HDR_TYPE_NUM_SUPPORT
 * Checking support of type numbers when allocating objects from allocation
 * classes with headers:
 * - POBJ_HEADER_NONE / type numbers not supported, always set to 0
 * - POBJ_HEADER_LEGACY / type numbers supported
 * - POBJ_HEADER_COMPACT / type numbers supported
 * \test
 *          \li \c Step1. Create pmemobj pool / SUCCESS
 *          \li \c Step2. Create allocation class / SUCCESS
 *          \li \c Step3. Allocate an object from the allocation class with type
 *          number equal to 1 / SUCCESS
 *          \li \c Step4. Make sure object is allocated
 *          \li \c Step5. Get type number of the allocated object
 *          \li \c Step6. Make sure valid type number is returned
 *          \li \c Step7. Close pool / SUCCESS
 */
TEST_P(ObjCtlAllocateFromCustomAllocClassParamTest,
       PMEMOBJ_CTL_CHECK_HDR_TYPE_NUM_SUPPORT) {
  /* Step 1 */
  PMEMobjpool *pop =
      pmemobj_create(pool_path_.c_str(), nullptr, PMEMOBJ_MIN_POOL, 0666);
  ASSERT_TRUE(pop != nullptr) << pmemobj_errormsg();
  /* Step 2 */
  pobj_alloc_class_desc write_arg;
  write_arg.unit_size = 512;
  write_arg.alignment = 0;
  write_arg.units_per_block = 1024;
  write_arg.class_id = 254;
  write_arg.header_type = GetParam();
  string entry_point =
      "heap.alloc_class." + to_string(write_arg.class_id) + ".desc";
  EXPECT_EQ(0, pmemobj_ctl_set(pop, entry_point.c_str(), &write_arg));
  /* Step 3 */
  PMEMoid oid = OID_NULL;
  uint64_t type_num = 1;
  EXPECT_EQ(0,
            pmemobj_xalloc(pop, &oid, write_arg.unit_size, type_num,
                           POBJ_CLASS_ID(write_arg.class_id), nullptr, nullptr))
      << pmemobj_errormsg();
  /* Step 4 */
  EXPECT_FALSE(OID_IS_NULL(oid));
  /* Step 5 */
  uint64_t ret = pmemobj_type_num(oid);
  /* Step 6 */
  if (write_arg.header_type == POBJ_HEADER_NONE) {
    EXPECT_EQ(0, ret);
  } else {
    EXPECT_EQ(type_num, ret);
  }
  pmemobj_free(&oid);
  /* Step 7 */
  pmemobj_close(pop);
}

INSTANTIATE_TEST_CASE_P(DifferentHdrType,
                        ObjCtlAllocateFromCustomAllocClassParamTest,
                        ::testing::Values(POBJ_HEADER_COMPACT,
                                          POBJ_HEADER_LEGACY,
                                          POBJ_HEADER_NONE));

/**
 * PMEMOBJ_CTL_CHECK_HDR_METADATA
 * Checking usable size of single allocation from allocation classes considering
 * different header types,
 * unit sizes and units per block
 * Properties of different header types:
 * - POBJ_HEADER_NONE / no header metadata, allocation can only span single unit
 * - POBJ_HEADER_LEGACY / 64-byte header, allocation can span up to 64 units
 * - POBJ_HEADER_COMPACT / 16-byte header, allocation can span up to 64 units
 * \test
 *          \li \c Step1. Create pmemobj pool / SUCCESS
 *          \li \c Step2. Create allocation class / SUCCESS
 *          \li \c Step3. Retrieve information about allocation class
 *          / SUCCESS
 *          \li \c Step4. Allocate an object of size 1 from the allocation class
 *          / SUCCESS
 *          \li \c Step5. Make sure object is allocated
 *          \li \c Step6. Make sure number of usable bytes in the object is
 *          valid
 *          \li \c Step7. Allocate an object of max possible size from the
 *          allocation class / SUCCESS
 *          \li \c Step8. Make sure object is allocated
 *          \li \c Step9. Make sure number of usable bytes in the object is
 *          valid
 *          \li \c Step10. Allocate an object of size greater than max possible
 *          size from the allocation class / FAIL: ret = -1, errno = EINVAL
 *          \li \c Step11. Make sure object is not allocated
 *          \li \c Step12. Close pool / SUCCESS
 */
TEST_P(ObjCtlAllocateFromCustomAllocClassParamTest2,
       PMEMOBJ_CTL_CHECK_HDR_METADATA) {
  /* Step 1 */
  PMEMobjpool *pop =
      pmemobj_create(pool_path_.c_str(), nullptr, PMEMOBJ_MIN_POOL, 0666);
  ASSERT_TRUE(pop != nullptr) << pmemobj_errormsg();
  /* Step 2 */
  PMEMoid oid = OID_NULL;
  pobj_alloc_class_desc write_arg;
  alloc_class_size arg;
  tie(arg, write_arg.header_type) = GetParam();
  write_arg.unit_size = arg.unit_size;
  write_arg.alignment = 0;
  write_arg.units_per_block = arg.units_per_block;
  write_arg.class_id = 254;
  string entry_point =
      "heap.alloc_class." + to_string(write_arg.class_id) + ".desc";
  EXPECT_EQ(0, pmemobj_ctl_set(pop, entry_point.c_str(), &write_arg))
      << pmemobj_errormsg();
  /* Step 3 */
  pobj_alloc_class_desc read_arg;
  EXPECT_EQ(0, pmemobj_ctl_get(pop, entry_point.c_str(), &read_arg))
      << pmemobj_errormsg();
  /* Step 4 */
  EXPECT_EQ(0,
            pmemobj_xalloc(pop, &oid, 1, 0, POBJ_CLASS_ID(write_arg.class_id),
                           nullptr, nullptr))
      << pmemobj_errormsg();
  /* Step 5 */
  EXPECT_FALSE(OID_IS_NULL(oid));
  /* Step 6 */
  size_t usable_size = pmemobj_alloc_usable_size(oid);
  size_t header_metadata = AllocClassUtils::hdrs[write_arg.header_type].size;
  EXPECT_EQ(usable_size, write_arg.unit_size - header_metadata);
  pmemobj_free(&oid);
  /* Step 7 */
  size_t max_valid;
  if (write_arg.header_type == POBJ_HEADER_NONE) {
    max_valid = write_arg.unit_size;
  } else {
    int max_blk = read_arg.units_per_block > 64 ? 64 : read_arg.units_per_block;
    max_valid = max_blk * read_arg.unit_size - header_metadata;
  }
  EXPECT_EQ(0,
            pmemobj_xalloc(pop, &oid, max_valid, 0,
                           POBJ_CLASS_ID(write_arg.class_id), nullptr, nullptr))
      << pmemobj_errormsg();
  /* Step 8 */
  EXPECT_FALSE(OID_IS_NULL(oid));
  /* Step 9 */
  usable_size = pmemobj_alloc_usable_size(oid);
  EXPECT_EQ(usable_size, max_valid);
  pmemobj_free(&oid);
  /* Step 10 */
  EXPECT_EQ(
      -1, pmemobj_xalloc(pop, &oid, max_valid + 1, 0,
                         POBJ_CLASS_ID(write_arg.class_id), nullptr, nullptr));
  EXPECT_EQ(EINVAL, errno);
  /* Step 11 */
  EXPECT_TRUE(OID_IS_NULL(oid));
  /* Step 12 */
  pmemobj_close(pop);
}

INSTANTIATE_TEST_CASE_P(
    DifferentUnitSizeBlocks, ObjCtlAllocateFromCustomAllocClassParamTest2,
    ::testing::Combine(::testing::Values(alloc_class_size{16384, 32},
                                         alloc_class_size{512, 64},
                                         alloc_class_size{512, 1024}),
                       ::testing::Values(POBJ_HEADER_COMPACT,
                                         POBJ_HEADER_LEGACY,
                                         POBJ_HEADER_NONE)));

/**
 * PMEMOBJ_CTL_CUSTOM_ALLOCATION_CLASS
 * Creating and/or retrieving information from allocation classes.
 * Following scenarios are considered:
 * - SET - creating allocation class
 * - GET - retrieving information about allocation class
 * - SET & GET - steps above combined
 * Following conditions checked:
 * Positive:
 * - unit sizes (1, 8, PMEMOBJ_MAX_ALLOC_SIZE) SET&GET / SUCCESS
 * - units per block (1024, max value of unsigned) / SUCCESS
 * - class ids (128, 254) SET&GET / SUCCESS
 * - header types (POBJ_HEADER_NONE, POBJ_HEADER_LEGACY, POBJ_HEADER_COMPACT)
 *   SET&GET / SUCCESS
 * Negative:
 * - Class id out of range (255, max value of unsigned) - SET&GET
 *   / FAIL: ret = -1, errno = ERANGE
 * - Invalid alignment (1, max value of size_t) - SET
 * - Invalid unit size (0, PMEMOBJ_MAX_ALLOC_SIZE + 1) - SET / FAIL: ret = -1,
 *   errno = EINVAL
 * - Invalid units per block (0) - SET / FAIL: ret = -1, errno = EINVAL
 * - Invalid header type (MAX_POBJ_HEADER_TYPES) - SET / FAIL: ret = -1, errno =
 *   EINVAL
 * - Overwrite default allocation class (class of id = 1) - SET / FAIL: ret =
 *   -1, errno = EEXIST
 * - Retrieve information from unexisting class (class of id = 128) - GET
 *   / FAIL: ret = -1, errno = ENOENT
 * \test
 *          \li \c Step1. Create pmemobj pool / SUCCESS
 *          \li \c Step2. Execute scenario
 *          \li \c Step3. Close pool
 */
TEST_P(ObjCtlAllocClassParamTest, PMEMOBJ_CTL_CUSTOM_ALLOCATION_CLASS) {
  /* Step 1 */
  PMEMobjpool *pop =
      pmemobj_create(pool_path_.c_str(), nullptr, PMEMOBJ_MIN_POOL, 0666);
  ASSERT_TRUE(pop != nullptr) << pmemobj_errormsg();
  /* Step 2 */
  in_args i_args;
  out_args o_args;
  tie(i_args, o_args) = GetParam();

  string entry_point =
      "heap.alloc_class." + to_string(i_args.write_args.class_id) + ".desc";
  if (i_args.scenario != Scenario::GET) {
    EXPECT_EQ(o_args.ret,
              pmemobj_ctl_set(pop, entry_point.c_str(), &i_args.write_args));
    if (o_args.ret != 0) {
      EXPECT_EQ(o_args.err, errno);
      errno = 0;
    }
  }

  if (i_args.scenario != Scenario::SET) {
    pobj_alloc_class_desc read_args;
    EXPECT_EQ(o_args.ret,
              pmemobj_ctl_get(pop, entry_point.c_str(), &read_args));
    if (o_args.ret != 0) {
      EXPECT_EQ(o_args.err, errno);
      errno = 0;
    }
    if (o_args.ret == 0 && i_args.scenario == Scenario::SET_GET) {
      EXPECT_TRUE(
          AllocClassUtils::IsAllocClassValid(i_args.write_args, read_args));
    }
  }
  /* Step 3 */
  pmemobj_close(pop);
}

INSTANTIATE_TEST_CASE_P(
    ClassIdOutOfRange, ObjCtlAllocClassParamTest,
    ::testing::Values(make_pair(in_args{{512, 0, 1024, POBJ_HEADER_COMPACT,
                                         255},
                                        Scenario::SET_GET},
                                out_args{-1, ERANGE}),
                      make_pair(in_args{{512, 0, 1024, POBJ_HEADER_COMPACT,
                                         (numeric_limits<unsigned>::max)()},
                                        Scenario::SET_GET},
                                out_args{-1, ERANGE})));
INSTANTIATE_TEST_CASE_P(
    InvalidUnitSize, ObjCtlAllocClassParamTest,
    ::testing::Values(make_pair(in_args{{0, 0, 1024, POBJ_HEADER_COMPACT, 128},
                                        Scenario::SET},
                                out_args{-1, EINVAL}),
                      make_pair(in_args{{PMEMOBJ_MAX_ALLOC_SIZE + 1, 0, 1024,
                                         POBJ_HEADER_COMPACT, 128},
                                        Scenario::SET},
                                out_args{-1, EINVAL})));
INSTANTIATE_TEST_CASE_P(
    InvalidAlignment, ObjCtlAllocClassParamTest,
    ::testing::Values(
        make_pair(in_args{{512, 1, 1024, POBJ_HEADER_COMPACT, 128},
                          Scenario::SET},
                  out_args{-1, ENOTSUP}),
        make_pair(in_args{{512, (std::numeric_limits<size_t>::max)(), 1024,
                           POBJ_HEADER_COMPACT, 128},
                          Scenario::SET},
                  out_args{-1, ENOTSUP})));
INSTANTIATE_TEST_CASE_P(
    InvalidUnitsPerBlock, ObjCtlAllocClassParamTest,
    ::testing::Values(make_pair(in_args{{512, 0, 0, POBJ_HEADER_COMPACT, 128},
                                        Scenario::SET},
                                out_args{-1, EINVAL})));
INSTANTIATE_TEST_CASE_P(InvalidHeaderType, ObjCtlAllocClassParamTest,
                        ::testing::Values(make_pair(
                            in_args{{512, 0, 1024, MAX_POBJ_HEADER_TYPES, 128},
                                    Scenario::SET},
                            out_args{-1, EINVAL})));
INSTANTIATE_TEST_CASE_P(
    OverwriteDefaultAllocationClass, ObjCtlAllocClassParamTest,
    ::testing::Values(make_pair(in_args{{512, 0, 1024, POBJ_HEADER_COMPACT, 1},
                                        Scenario::SET},
                                out_args{-1, EEXIST})));
INSTANTIATE_TEST_CASE_P(
    RetrieveInfoFromUnexistingClass, ObjCtlAllocClassParamTest,
    ::testing::Values(make_pair(
        in_args{{512, 0, 1024, POBJ_HEADER_COMPACT, 128}, Scenario::GET},
        out_args{-1, ENOENT})));
INSTANTIATE_TEST_CASE_P(
    CreateCustomAllocationClass, ObjCtlAllocClassParamTest,
    ::testing::Values(
        make_pair(in_args{{1, 0, 1000, POBJ_HEADER_COMPACT, 128},
                          Scenario::SET_GET},
                  out_args{0, 0}),
        make_pair(in_args{{8, 0, 1024, POBJ_HEADER_COMPACT, 128},
                          Scenario::SET_GET},
                  out_args{0, 0}),
        make_pair(in_args{{512, 0, 1024, POBJ_HEADER_COMPACT, 254},
                          Scenario::SET_GET},
                  out_args{0, 0}),
        make_pair(in_args{{PMEMOBJ_MAX_ALLOC_SIZE, 0, 1024, POBJ_HEADER_COMPACT,
                           128},
                          Scenario::SET_GET},
                  out_args{0, 0}),
        make_pair(in_args{{512, 0, 1024, POBJ_HEADER_NONE, 128},
                          Scenario::SET_GET},
                  out_args{0, 0}),
        make_pair(in_args{{1024, 0, (numeric_limits<unsigned>::max)(),
                           POBJ_HEADER_COMPACT, 128},
                          Scenario::SET_GET},
                  out_args{0, 0}),
        make_pair(in_args{{512, 0, 1024, POBJ_HEADER_LEGACY, 128},
                          Scenario::SET_GET},
                  out_args{0, 0})));
