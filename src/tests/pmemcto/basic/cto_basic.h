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

#ifndef PMDK_TESTS_CTO_BASIC_H
#define PMDK_TESTS_CTO_BASIC_H

#include <libpmemcto.h>
#include <array>
#include <memory>
#include <string>
#include <utility>
#include "configXML/local_configuration.h"
#include "gtest/gtest.h"
#include "poolset/poolset.h"

#ifdef _WIN32
#define strdup _strdup
#endif  // _WIN32

extern std::unique_ptr<LocalConfiguration> local_config;

class CtoBasic : public ::testing::Test {
 protected:
  std::string test_dir = local_config->GetTestDir();
  std::string pool_path = test_dir + "pool";
  PMEMctopool *pcp = nullptr;
};

namespace CtoCreateTests {
enum class Scenario { FILE_EXISTS, EMPTY_FILE_EXISTS, POOLSET_FILE_EXISTS };

class CtoCreate : public CtoBasic,
                  public ::testing::WithParamInterface<Scenario> {};
}

namespace CtoBasicTests {
struct in_args {
  std::string layout;
  size_t size = PMEMCTO_MIN_POOL;
  in_args() = default;
  in_args(std::string);
  in_args(size_t);
};

class CtoBasicNegative
    : public CtoBasic,
      public ::testing::WithParamInterface<std::pair<in_args, int>> {};

class CtoBasicPositive : public CtoBasic,
                         public ::testing::WithParamInterface<in_args> {};
}

namespace CtoBasicTests2 {
enum class Scenario { ALREADY_OPENED, READ_ONLY, LAYOUT_MISMATCH };

class CtoNegative : public CtoBasic,
                    public ::testing::WithParamInterface<Scenario> {};
}

namespace CtoPoolsetTests {
class CtoPoolsetNegative
    : public CtoBasic,
      public ::testing::WithParamInterface<std::pair<Poolset, int>> {};

class CtoPoolsetPositive : public CtoBasic,
                           public ::testing::WithParamInterface<Poolset> {};
}

namespace CtoSetFuncsTest {
enum class Funcs { MALLOC, FREE, REALLOC, STRDUP, PRINT, Count };
static std::array<int, static_cast<int>(Funcs::Count)> arr;

class CtoBasicSetFuncs : public CtoBasic {
 protected:
  static void *my_malloc(size_t size) {
    arr[static_cast<int>(Funcs::MALLOC)]++;
    return malloc(size);
  }

  static void my_free(void *ptr) {
    arr[static_cast<int>(Funcs::FREE)]++;
    return free(ptr);
  }

  static void *my_realloc(void *ptr, size_t size) {
    arr[static_cast<int>(Funcs::REALLOC)]++;
    return realloc(ptr, size);
  }

  static char *my_strdup(const char *s) {
    arr[static_cast<int>(Funcs::STRDUP)]++;
    return strdup(s);
  }
#ifndef _WIN32
  static void my_print(__attribute__((unused)) const char *s) {
#else
  static void CtoSetFuncsTest::CtoBasicSetFuncs::my_print(const char *s) {
#endif
    arr[static_cast<int>(Funcs::PRINT)]++;
  }
  void Work();
};
}

#endif  // PMDK_TESTS_CTO_BASIC_H
