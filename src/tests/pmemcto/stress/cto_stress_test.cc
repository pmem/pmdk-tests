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

#include "cto_stress.h"

/**
 * PMEMCTO_STRESS
 * Single-threaded stress test for pmemcto library
 * \test
 *          \li \c Step1. Create pmemcto pool / SUCCESS
 *          \li \c Step2. Allocate memory of random size until pool is full
 *          \li \c Step3. Free random number of allocations
 *          \li \c Step4. Reallocate all allocations
 *          \li \c Repeat steps 2-4 given number of times
 *          \li \c Step5. Free whole allocated memory
 *          \li \c Step6. Close pmemcto pool / SUCCESS
 */
TEST_F(CtoStressTest, PMEMCTO_STRESS) {
  /* Step 1 */
  pcp = pmemcto_create(pool_path.c_str(), nullptr, pool_size, 0666);
  EXPECT_TRUE(pcp != nullptr) << pmemcto_errormsg();

  std::default_random_engine generator;
  int i = 1024;
  while (i--) {
    /* Step 2 */
    Malloc(allocs);
    /* Step 3 */
    std::uniform_int_distribution<size_t> start_dist(0, allocs.size());
    auto start = start_dist(generator);
    std::uniform_int_distribution<size_t> end_dist(start, allocs.size());
    auto end = end_dist(generator);
    Free(allocs, allocs.begin() + start, allocs.begin() + end);
    /* Step 4 */
    Realloc(allocs);
  }
  /* Step 5 */
  Free(allocs, allocs.begin(), allocs.end());
  /* Step 6 */
  pmemcto_close(pcp);
  /* Clean up */
  ApiC::RemoveFile(pool_path);
}
