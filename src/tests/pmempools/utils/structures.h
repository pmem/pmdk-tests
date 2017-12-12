/*
 * Copyright 2017, Intel Corporation
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

#ifndef PMDK_TESTS_SRC_TESTS_PMEMPOOLS_PMEMPOOL_CREATE_STRUCTURES_H_
#define PMDK_TESTS_SRC_TESTS_PMEMPOOLS_PMEMPOOL_CREATE_STRUCTURES_H_

#include "constants.h"
#include "poolset/poolset.h"
#include <array>
#include <iostream>
#include <libpmemblk.h>
#include <libpmemlog.h>
#include <libpmemobj.h>

enum class PoolType { Obj, Blk, Log, None, MaxPoolTypes };

enum class OptType { Long, Short, ShortNoSpace };

enum class Option {
  Size,
  MaxSize,
  Mode,
  Inherit,
  Force,
  Verbose,
  Help,
  WriteLayout,
  BlockNumber,
  Layout,

  MaxOption
};

struct Arg {
  Option option;
  OptType arg_type;
  std::string value;

  Arg(Option option, OptType arg_type, std::string value)
      : option(option), arg_type(arg_type), value(value){};

  Arg(Option option, OptType arg_type) : option(option), arg_type(arg_type){};
};

struct PoolArgs {
  PoolType pool_type;
  std::vector<Arg> args;
  std::string err_msg;

  PoolArgs(PoolType pool_type_, std::initializer_list<Arg> args_)
      : pool_type(pool_type_), args(args_){};

  PoolArgs(PoolType pool_type) : pool_type(pool_type){};

  PoolArgs(PoolType pool_type_, std::initializer_list<Arg> args_,
           std::string err_msg_)
      : pool_type(pool_type_), args(args_), err_msg(err_msg_){};

  PoolArgs(PoolType pool_type, std::string err_msg_)
      : pool_type(pool_type), err_msg(err_msg_){};
};

struct PoolInherit {
  PoolArgs pool_base;
  PoolArgs pool_inherited;
};

struct PoolsetArgs {
  PoolArgs pool_args;
  Poolset poolset;
};

namespace struct_utils {
const std::array<std::string, static_cast<int>(PoolType::MaxPoolTypes)>
    POOL_TYPES{{"obj ", "blk ", "log ", ""}};

const std::array<std::string, static_cast<int>(Option::MaxOption)> LONG_OPTIONS{
    {"--size ", "--max-size ", "--mode ", "--inherit ", "--force ",
     "--verbose ", "--help ", "--write-layout", "", "--layout "}};

const std::array<std::string, static_cast<int>(Option::MaxOption)>
    SHORT_OPTIONS{{"-s", "-M", "-m", "-i", "-f", "-v", "-h", "-w", "", "-l"}};

const std::array<size_t, static_cast<int>(PoolType::MaxPoolTypes)>
    POOL_MIN_SIZES{{static_cast<size_t>(PMEMOBJ_MIN_POOL),
                    static_cast<size_t>(PMEMBLK_MIN_POOL),
                    static_cast<size_t>(PMEMLOG_MIN_POOL), 0}};

static inline std::string ParseArg(Option option, OptType arg_type,
                                   const std::string &value) {
  switch (arg_type) {
  case OptType::Long:
    return LONG_OPTIONS[static_cast<int>(option)] + value;
    break;
  case OptType::Short:
    return SHORT_OPTIONS[static_cast<int>(option)] + " " + value;
    break;
  case OptType::ShortNoSpace:
    return SHORT_OPTIONS[static_cast<int>(option)] + value;
    break;
  }

  return "";
}

static inline int GetDefaultMode() { return 0664 & PERMISSION_MASK; }

static inline std::string CombineArguments(const std::vector<Arg> &args) {
  std::string arguments;

  for (const auto &arg : args) {
    arguments += ParseArg(arg.option, arg.arg_type, arg.value) + " ";
  }

  return arguments;
}

static inline size_t GetPoolSize(const PoolArgs &pool_args) {
  size_t size;

  auto GetSize = [&size](Arg arg) {
    if (arg.option == Option::Size) {
      size = SIZES_MiB.at(arg.value);
      return true;
    }
    return false;
  };

  if (std::any_of(std::begin(pool_args.args), std::end(pool_args.args),
                  GetSize)) {
    return size;
  }

  return POOL_MIN_SIZES[static_cast<int>(pool_args.pool_type)];
}

static inline int GetPoolMode(const PoolArgs &pool_args) {
  int mode;

  auto ContainsMode = [&mode](Arg arg) {
    if (arg.option == Option::Mode) {
      mode = static_cast<int>(std::strtoul(arg.value.c_str(), nullptr, 8));
      mode &= PERMISSION_MASK;
      return true;
    }
    return false;
  };

  if (std::any_of(std::begin(pool_args.args), std::end(pool_args.args),
                  ContainsMode)) {
    return mode;
  }

  return GetDefaultMode();
}
} // namespace struct_utils

#endif // !PMDK_TESTS_SRC_TESTS_PMEMPOOLS_PMEMPOOL_CREATE_STRUCTURES_H_
