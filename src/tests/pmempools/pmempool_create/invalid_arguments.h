/*
 * Copyright 2017-2019, Intel Corporation
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

#ifndef PMDK_TESTS_SRC_TESTS_PMEMPOOLS_PMEMPOOL_CREATE_INVALID_ARGUMENTS_H_
#define PMDK_TESTS_SRC_TESTS_PMEMPOOLS_PMEMPOOL_CREATE_INVALID_ARGUMENTS_H_

#include "pmempool_create.h"

namespace create {
  class InvalidArgumentsTests : public PmempoolCreate,
                                public ::testing::WithParamInterface<PoolArgs> {
   public:
    PoolArgs pool_args;
  
    void SetUp() override;
  };
  
  class InvalidInheritTests : public PmempoolCreate,
                              public ::testing::WithParamInterface<PoolInherit> {
   public:
    PoolInherit pool_inherit;
  
    void SetUp() override;
  };
  
  class InvalidArgumentsPoolsetTests
      : public PmempoolCreate,
        public ::testing::WithParamInterface<PoolsetArgs> {
   public:
    PoolsetArgs poolset_args;
  
    void SetUp() override;
  };
}

#endif  // !PMDK_TESTS_SRC_TESTS_PMEMPOOLS_PMEMPOOL_CREATE_PMEMPOOL_CREATE_INVALID_ARGUMENTS_H_
