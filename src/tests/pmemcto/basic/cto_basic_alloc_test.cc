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

#include "../../../utils/api_c/api_c.h"
#include "cto_basic.h"

/**
 * PMEMCTO_ALLOCATE_SIZE_ZERO
 * Allocating memory of size 0
 * \test
 *          \li \c Step1. Create pmemcto pool / SUCCESS
 *          \li \c Step2. Allocate memory with size 0 / NULL or unique ptr is returned
 *          \li \c Step3. Free the memory / SUCCESS, no signal is raised
 *          \li \c Step4. Allocate aligned memory with size 0 / NULL or unique ptr is returned
 *          \li \c Step5. Free the memory / SUCCESS, no signal is raised
 *          \li \c Step6. Allocate an array of 0 elements / NULL or unique ptr is returned
 *          \li \c Step7. Free the memory / SUCCESS, no signal is raised
 *          \li \c Step8. Reallocate memory pointed with NULL as ptr and size of 0 / NULL or unique ptr is returned
 *          \li \c Step9. Free the memory / SUCCESS, no signal is raised
 *          \li \c Step10. Free memory pointed by nullptr / No signal is raised
 *          \li \c Step11. Close pmecto pool / SUCCESS
 */
TEST_F(CtoBasic, PMEMCTO_ALLOCATE_SIZE_ZERO) {
  /* Step 1 */
  pcp = pmemcto_create(pool_path.c_str(), nullptr, PMEMCTO_MIN_POOL, 0666);
  EXPECT_TRUE(pcp != nullptr) << pmemcto_errormsg();
  /* Step 2 */
  void *ptr = pmemcto_malloc(pcp, 0);
  /* Step 3 */
  pmemcto_free(pcp, ptr);
  /* Step 4 */
  ptr = pmemcto_aligned_alloc(pcp, 2, 0);
  /* Step 5 */
  pmemcto_free(pcp, ptr);
  /* Step 6 */
  ptr = pmemcto_calloc(pcp, 0, 1024);
  /* Step 7 */
  pmemcto_free(pcp, ptr);
  /* Step 8 */
  ptr = pmemcto_realloc(pcp, nullptr, 0);
  /* Step 9 */
  pmemcto_free(pcp, ptr);
  /* Step 10 */
  pmemcto_free(pcp, nullptr);
  /* Step 11 */
  pmemcto_close(pcp);
  /* Cleanup */
  ApiC::RemoveFile(pool_path);
}

/**
 * PMEMCTO_ALIGNED_ALLOC
 * Allocating memory with memory address aligned
 * \test
 *          \li \c Step1. Create pmemcto pool / SUCCESS
 *          \li \c Step2. Allocate memory with alignment of 0 / FAIL: ptr = NULL, errno = EINVAL
 *          \li \c Step3. Allocate memory with alignment that is not power of 2 / FAIL: ptr = NULL, errno = EINVAL
 *          \li \c Step4. Allocate memory with alignment that is power of 2 / SUCCESS
 *          \li \c Step5. Make sure memory address is aligned / SUCCESS
 *          \li \c Step6. Free memory / SUCCESS
 *          \li \c Step7. Close pmemecto pool / SUCCESS
 */
TEST_F(CtoBasic, PMEMCTO_ALIGNED_ALLOC) {
  /* Step 1 */
  pcp = pmemcto_create(pool_path.c_str(), nullptr, PMEMCTO_MIN_POOL, 0666);
  EXPECT_TRUE(pcp != nullptr) << pmemcto_errormsg();
  /* Step 2 */
  void *ptr;
  ptr = pmemcto_aligned_alloc(pcp, 0, MEBIBYTE);
  EXPECT_EQ(nullptr, ptr);
  EXPECT_EQ(EINVAL, errno);
  /* Step 3 */
  errno = 0;
  ptr = pmemcto_aligned_alloc(pcp, 3, MEBIBYTE);
  EXPECT_EQ(nullptr, ptr);
  EXPECT_EQ(EINVAL, errno);
  /* Step 4 */
  for (unsigned alignment = 1; alignment < PMEMCTO_MIN_POOL / 2;
       alignment *= 2) {
    ptr = pmemcto_aligned_alloc(pcp, alignment, MEBIBYTE);
    EXPECT_TRUE(ptr != nullptr) << pmemcto_errormsg();
    /* Step 5 */
    EXPECT_TRUE((reinterpret_cast<uintptr_t>(ptr) % alignment) == 0);
    /* Step 6 */
    pmemcto_free(pcp, ptr);
  }
  /* Step 7 */
  pmemcto_close(pcp);
  /* Cleanup */
  ApiC::RemoveFile(pool_path);
}

/**
 * PMEMCTO_CALLOC
 * Allocating memory for an array of elements
 * \test
 *          \li \c Step1. Create pmemcto pool / SUCCESS
 *          \li \c Step2. Allocate memory / SUCCESS
 *          \li \c Step3. Set memory with pattern
 *          \li \c Step4. Free memory / SUCCESS
 *          \li \c Step5. Allocate memory for an array of elements / SUCCESS
 *          \li \c Step6. Make sure memory is zeroed
 *          \li \c Step7. Free memory / SUCCESS
 *          \li \c Step8. Close pmemecto pool / SUCCESS
 */
TEST_F(CtoBasic, PMEMCTO_CALLOC) {
  /* Step 1 */
  pcp = pmemcto_create(pool_path.c_str(), nullptr, PMEMCTO_MIN_POOL, 0666);
  EXPECT_TRUE(pcp != nullptr) << pmemcto_errormsg();
  /* Step 2 */
  void *ptr;
  size_t size;
  size = PMEMCTO_MIN_POOL / 2;
  ptr = pmemcto_malloc(pcp, size);
  EXPECT_TRUE(ptr != nullptr) << pmemcto_errormsg();
  /* Step 3 */
  memset(ptr, 0x41, size);
  /* Step 4 */
  pmemcto_free(pcp, ptr);
  /* Step 5 */
  size_t nmemb = size / 1024;
  ptr = pmemcto_calloc(pcp, nmemb, 1024);
  EXPECT_TRUE(ptr != nullptr) << pmemcto_errormsg();
  /* Step 6 */
  void *zeroed_buf = calloc(nmemb, 1024);
  int ret = memcmp(ptr, zeroed_buf, size);
  EXPECT_EQ(0, ret);
  /* Step 7 */
  pmemcto_free(pcp, ptr);
  /* Step 8 */
  pmemcto_close(pcp);
  /* Cleanup */
  free(zeroed_buf);
  ApiC::RemoveFile(pool_path);
}

/**
 * PMEMCTO_ROOT_PTR
 * Setting abd obtaing the root object pointer
 * \test
 *          \li \c Step1. Create pmemcto pool / SUCCESS
 *          \li \c Step2. Obtain pointer to the root object that is not set / SUCCESS: pcp = NULL
 *          \li \c Step3. Allocate memory / SUCCESS
 *          \li \c Step4. Set the root object pointer
 *          \li \c Step5. Close pmemcto pool / SUCCESS
 *          \li \c Step6. Open pmemcto pool / SUCCESS
 *          \li \c Step7. Obtain pointer to the root object / SUCCESS
 *          \li \c Step8. Free memory / SUCCESS
 *          \li \c Step9. Set to root object pointer to nullptr
 *          \li \c Step10. Obtain pointer to the root object / SUCCESS: pcp = NULL
 *          \li \c Step11. Close pmemcto pool / SUCCESS
 */
TEST_F(CtoBasic, PMEMCTO_ROOT_PTR) {
  /* Step 1 */
  pcp = pmemcto_create(pool_path.c_str(), nullptr, PMEMCTO_MIN_POOL, 0666);
  EXPECT_TRUE(pcp != nullptr) << pmemcto_errormsg();
  /* Step 2 */
  void *ptr = pmemcto_get_root_pointer(pcp);
  EXPECT_EQ(nullptr, ptr);
  /* Step 3 */
  ptr = pmemcto_malloc(pcp, 64);
  EXPECT_TRUE(ptr != nullptr) << pmemcto_errormsg();
  /* Step 4 */
  pmemcto_set_root_pointer(pcp, ptr);
  /* Step 5 */
  pmemcto_close(pcp);
  /* Step 6 */
  pcp = pmemcto_open(pool_path.c_str(), nullptr);
  EXPECT_TRUE(pcp != nullptr) << pmemcto_errormsg();
  /* Step 7 */
  void *root_ptr = pmemcto_get_root_pointer(pcp);
  EXPECT_TRUE(root_ptr != nullptr);
  /* Step 8 */
  pmemcto_free(pcp, ptr);
  /* Step 9 */
  pmemcto_set_root_pointer(pcp, nullptr);
  /* Step 10 */
  root_ptr = pmemcto_get_root_pointer(pcp);
  EXPECT_EQ(nullptr, root_ptr);
  /* Step 11 */
  pmemcto_close(pcp);
  /* Cleanup */
  ApiC::RemoveFile(pool_path);
}

/**
 * PMEMCTO_MALLOC_USABLE_SIZE
 * Obtaining size of block of memory allocated from pool
 * \test
 *          \li \c Step1. Create pmemcto pool / SUCCESS
 *          \li \c Step2. Obtain the number of usable bytes in the block of memory pointed by nullptr / SUCCESS: usable bytes = 0
 *          \li \c Step3. Allocate memory of size equal to 0 / SUCCESS
 *          \li \c Step4. Obtain the number of usable bytes in the block of memory / SUCCESS: usable bytes > 0
 *          \li \c Step5. Free memory / SUCCESS
 *          \li \c Step6. Allocate memory of size equal to mebibyte / SUCCESS
 *          \li \c Step7. Obtain the number of usable bytes in the block of memory / SUCCESS: usable bytes = mebibyte
 *          \li \c Step8. Free memory / SUCCESS
 *          \li \c Step9. Close pmemcto pool / SUCCESS
 */
TEST_F(CtoBasic, PMEMCTO_MALLOC_USABLE_SIZE) {
  /* Step 1 */
  pcp = pmemcto_create(pool_path.c_str(), nullptr, PMEMCTO_MIN_POOL, 0666);
  EXPECT_TRUE(pcp != nullptr) << pmemcto_errormsg();
  /* Step 2 */
  size_t usable_size = pmemcto_malloc_usable_size(pcp, nullptr);
  EXPECT_EQ(0, usable_size);
  /* Step 3 */
  void *ptr = pmemcto_malloc(pcp, 0);
  EXPECT_TRUE(ptr != nullptr);
  /* Step 4 */
  usable_size = pmemcto_malloc_usable_size(pcp, ptr);
  EXPECT_GT(usable_size, 0);
  /* Step 5 */
  pmemcto_free(pcp, ptr);
  /* Step 6 */
  ptr = pmemcto_malloc(pcp, MEBIBYTE);
  EXPECT_TRUE(ptr != nullptr);
  /* Step 7 */
  usable_size = pmemcto_malloc_usable_size(pcp, ptr);
  EXPECT_EQ(MEBIBYTE, usable_size);
  /* Step 8 */
  pmemcto_free(pcp, ptr);
  /* Step 9 */
  pmemcto_close(pcp);
  /* Cleanup */
  ApiC::RemoveFile(pool_path);
}

/**
 * PMEMCTO_REALLOC
 * Changing the size of the memory block
 * \test
 *          \li \c Step1. Create pmemcto pool / SUCCESS
 *          \li \c Step2. Allocate 1 MiB of memory with pmemcto_realloc() passing NULL as ptr / SUCCESS
 *          \li \c Step3. Obtain the number of usable bytes in the block of memory / SUCCESS: usable bytes = 1 MiB
 *          \li \c Step4. Set memory with pattern
 *          \li \c Step5. Reallocate memory to size of 2 MiB / SUCCESS
 *          \li \c Step6. Obtain the number of usable bytes in the block of memory / SUCCESS: usable bytes = 2 MiB
 *          \li \c Step7. Make sure pattern is written
 *          \li \c Step8. Reallocate memory to size of 512 KiB / SUCCESS
 *          \li \c Step9. Obtain the number of usable bytes in the block of memory / SUCCESS: usable bytes = 512 KiB
 *          \li \c Step10. Make sure pattern is written / SUCCESS
 *          \li \c Step11. Free memory with pmemcto_realloc() passing 0 as size / SUCCESS
 *          \li \c Step12. Close pmemcto pool / SUCCESS
 */
TEST_F(CtoBasic, PMEMCTO_REALLOC) {
  /* Step 1 */
  pcp = pmemcto_create(pool_path.c_str(), nullptr, PMEMCTO_MIN_POOL, 0666);
  EXPECT_TRUE(pcp != nullptr) << pmemcto_errormsg();
  /* Step 2 */
  size_t size = MEBIBYTE;
  void *ptr = pmemcto_realloc(pcp, nullptr, size);
  EXPECT_TRUE(ptr != nullptr) << pmemcto_errormsg();
  /* Step 3 */
  size_t usable_size = pmemcto_malloc_usable_size(pcp, ptr);
  EXPECT_EQ(size, usable_size);
  /* Step 4 */
  memset(ptr, 0x41, usable_size);
  /* Step 5 */
  ptr = pmemcto_realloc(pcp, ptr, 2 * size);
  EXPECT_TRUE(ptr != nullptr) << pmemcto_errormsg();
  /* Step 6 */
  usable_size = pmemcto_malloc_usable_size(pcp, ptr);
  EXPECT_EQ(2 * size, usable_size);
  /* Step 7 */
  void *buf = malloc(size);
  memset(buf, 0x41, size);
  int ret = memcmp(ptr, buf, size);
  EXPECT_EQ(0, ret);
  /* Step 8 */
  ptr = pmemcto_realloc(pcp, ptr, size / 2);
  EXPECT_TRUE(ptr != nullptr) << pmemcto_errormsg();
  /* Step 9 */
  usable_size = pmemcto_malloc_usable_size(pcp, ptr);
  EXPECT_EQ(size / 2, usable_size);
  /* Step 10 */
  ret = memcmp(ptr, buf, size / 2);
  EXPECT_EQ(0, ret);
  /* Step 11 */
  pmemcto_realloc(pcp, ptr, 0);
  /* Step 12 */
  pmemcto_close(pcp);
  /* Cleanup */
  free(buf);
  ApiC::RemoveFile(pool_path);
}

/**
 * PMEMCTO_STRDUP
 * Duplicating a string
 * \test
 *          \li \c Step1. Create pmemcto pool / SUCCESS
 *          \li \c Step2. Duplicate a string of the size greater than pool size / FAIL: ptr = NULL, errno = ENOMEM
 *          \li \c Step3. Duplicate a string of the size of 1 KiB / SUCCESS
 *          \li \c Step4. Make sure string content is duplicated
 *          \li \c Step5. Free string
 *          \li \c Step6. Duplicate a wide char string of the size greater than pool size / FAIL: ptr = NULL, errno = ENOMEM
 *          \li \c Step7. Duplicate a wide char string of the size of 1 KiB / SUCCESS
 *          \li \c Step8. Make sure string content is duplicated
 *          \li \c Step9. Free string
 *          \li \c Step10. Close pmemcto pool / SUCCESS
 */
TEST_F(CtoBasic, PMEMCTO_STRDUP) {
  /* Step 1 */
  pcp = pmemcto_create(pool_path.c_str(), nullptr, PMEMCTO_MIN_POOL, 0666);
  EXPECT_TRUE(pcp != nullptr) << pmemcto_errormsg();
  /* Step 2 */
  std::string s(2 * PMEMCTO_MIN_POOL, 0x41);
  const char *str = pmemcto_strdup(pcp, s.c_str());
  EXPECT_EQ(nullptr, str);
  EXPECT_EQ(ENOMEM, errno);
  errno = 0;
  /* Step 3 */
  std::string s2(KIBIBYTE, 0x42);
  char *str2 = pmemcto_strdup(pcp, s2.c_str());
  EXPECT_TRUE(str2 != nullptr);
  /* Step 4 */
  int ret = memcmp(s2.c_str(), str2, KIBIBYTE);
  EXPECT_EQ(0, ret);
  /* Step 5 */
  pmemcto_free(pcp, str2);
  /* Step 6 */
  std::wstring w_s(2 * PMEMCTO_MIN_POOL, 0x43);
  const wchar_t *w_str = pmemcto_wcsdup(pcp, w_s.c_str());
  EXPECT_EQ(nullptr, w_str);
  EXPECT_EQ(ENOMEM, errno);
  errno = 0;
  /* Step 7 */
  std::wstring w_s2(KIBIBYTE, 0x44);
  wchar_t *w_str2 = pmemcto_wcsdup(pcp, w_s2.c_str());
  EXPECT_TRUE(w_str2 != nullptr);
  /* Step 8 */
  ret = memcmp(w_s2.c_str(), w_str2, KIBIBYTE);
  EXPECT_EQ(0, ret);
  /* Step 9 */
  pmemcto_free(pcp, w_str2);
  /* Step 10 */
  pmemcto_close(pcp);
  /* Clean up */
  ApiC::RemoveFile(pool_path);
}
