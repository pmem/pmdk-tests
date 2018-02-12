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

#ifndef PMDK_ALLOC_CLASS_H
#define PMDK_ALLOC_CLASS_H

#include <libpmemobj.h>
#include <map>
#include <memory>
#include <string>
#include <tuple>
#include <utility>
#include "configXML/local_configuration.h"
#include "gtest/gtest.h"

extern std::unique_ptr<LocalConfiguration> local_config;

class ObjCtlAllocClassTest : public ::testing::Test {
 private:
  std::string test_dir_ = local_config->GetTestDir();

 public:
  std::string pool_path_ = test_dir_ + "pool";
  void SetUp();
  void TearDown();
};

enum class Scenario { SET, GET, SET_GET };

struct in_args {
  pobj_alloc_class_desc write_args;
  Scenario scenario;
  in_args() = default;
  in_args(pobj_alloc_class_desc desc, Scenario s)
      : write_args(desc), scenario(s) {
  }
};

struct out_args {
  int ret;
  int err;
};

class ObjCtlAllocClassParamTest
    : public ObjCtlAllocClassTest,
      public ::testing::WithParamInterface<std::pair<in_args, out_args>> {};

class ObjCtlAllocateFromCustomAllocClassParamTest
    : public ObjCtlAllocClassTest,
      public ::testing::WithParamInterface<enum pobj_header_type> {};

struct alloc_class_size {
  size_t unit_size;
  unsigned units_per_block;
};

class ObjCtlAllocateFromCustomAllocClassParamTest2
    : public ObjCtlAllocClassTest,
      public ::testing::WithParamInterface<
          std::tuple<alloc_class_size, enum pobj_header_type>> {};

#endif  // PMDK_ALLOC_CLASS_H
