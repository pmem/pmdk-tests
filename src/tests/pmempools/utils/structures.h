/*
 * Copyright (c) 2017, Intel Corporation
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

#ifndef PMDK_TESTS_SRC_TESTS_PMEMPOOLS_PMEMPOOL_CREATE_STRUCTURES_H_
#define PMDK_TESTS_SRC_TESTS_PMEMPOOLS_PMEMPOOL_CREATE_STRUCTURES_H_

#include <libpmemblk.h>
#include <libpmemlog.h>
#include <libpmemobj.h>
#include <iostream>
#include <map>
#include <memory>
#include "constants.h"
#include "poolset/poolset.h"

static std::map<std::string, size_t> SIZES_MiB = {
    {"PMEMOBJ_MIN_POOL", PMEMOBJ_MIN_POOL},
    {"PMEMLOG_MIN_POOL", PMEMLOG_MIN_POOL},
    {"PMEMBLK_MIN_POOL", PMEMBLK_MIN_POOL},
    {"7M", 7 * MEBIBYTE},
    {"8M", 8 * MEBIBYTE},
    {"20M", 20 * MEBIBYTE},
    {"64M", 64 * MEBIBYTE}};

enum class PoolType { Obj, Blk, Log, None };

enum class OptType { Short, Long, ShortNoSpace };

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
  Layout
};

struct Arg {
  Option option;
  OptType arg_type;
  std::string value;

  Arg(Option option, OptType arg_type, std::string value)
      : option(option), arg_type(arg_type), value(value){};
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
static inline std::string GetLongOption(Option option) {
  switch (option) {
    case Option::Size:
      return "--size ";
    case Option::MaxSize:
      return "--max-size ";
    case Option::Mode:
      return "--mode ";
    case Option::Inherit:
      return "--inherit ";
    case Option::Force:
      return "--force ";
    case Option::Verbose:
      return "--verbose ";
    case Option::Help:
      return "--help ";
    case Option::WriteLayout:
      return "--write-layout";
    case Option::BlockNumber:
      return "";
    case Option::Layout:
      return "--layout ";
    default:
      break;
  }

  return "";
}

static inline std::string GetShortOption(Option option) {
  switch (option) {
    case Option::Size:
      return "-s";
    case Option::MaxSize:
      return "-M";
    case Option::Mode:
      return "-m";
    case Option::Inherit:
      return "-i";
    case Option::Force:
      return "-f";
    case Option::Verbose:
      return "-v";
    case Option::Help:
      return "-h";
    case Option::WriteLayout:
      return "-w";
    case Option::BlockNumber:
      return "";
    case Option::Layout:
      return "-l";
    default:
      break;
  }

  return "";
}

static inline std::string ParseArg(Option option, OptType arg_type,
                                   const std::string& value) {
  switch (arg_type) {
    case OptType::Long:
      return GetLongOption(option) + value;
      break;
    case OptType::Short:
      return GetShortOption(option) + " " + value;
      break;
    case OptType::ShortNoSpace:
      return GetShortOption(option) + value;
      break;
    default:
      break;
  }

  return "";
}

static inline size_t GetMinSize(PoolType pool_type) {
  switch (pool_type) {
    case PoolType::Obj:
      return static_cast<size_t>(PMEMOBJ_MIN_POOL);
    case PoolType::Log:
      return static_cast<size_t>(PMEMLOG_MIN_POOL);
    case PoolType::Blk:
      return static_cast<size_t>(PMEMBLK_MIN_POOL);
    case PoolType::None:
      return 0;
    default:
      return 0;
  }

  return 0;
}

static inline int GetDefaultMode() {
  int mode = 0664;

#ifdef _WIN32
  /* On Windows there is no perrmition for execution. Also windows doesn't
  * support groups of users in UNIX way */
  mode &= 0600;
#else
  mode &= 0777;
#endif  // _WIN32

  return mode;
}

static inline std::string CombineArguments(const std::vector<Arg>& args) {
  std::string arguments;

  for (const auto& arg : args) {
    arguments += ParseArg(arg.option, arg.arg_type, arg.value) + " ";
  }

  return arguments;
}

static inline size_t GetPoolSize(const PoolArgs& pool_args) {
  size_t size;

  auto GetSize = [&size](Arg arg) {
    if (arg.option == Option::Size) {
      size = SIZES_MiB[arg.value];
      return true;
    }
    return false;
  };

  if (std::any_of(std::begin(pool_args.args), std::end(pool_args.args),
                  GetSize)) {
    return size;
  }

  return GetMinSize(pool_args.pool_type);
}

static inline int GetPoolMode(const PoolArgs& pool_args) {
  int mode;

  auto GetMode = [&mode](Arg arg) {
    if (arg.option == Option::Mode) {
      mode = static_cast<int>(std::strtoul(arg.value.c_str(), nullptr, 8));
#ifdef _WIN32
      /* On Windows there is no perrmition for execution. Also windows doesn't
      * support groups of users in UNIX way */
      mode &= 0600;
#else
      mode &= 0777;
#endif  // _WIN32
      return true;
    }
    return false;
  };

  if (std::any_of(std::begin(pool_args.args), std::end(pool_args.args),
                  GetMode)) {
    return mode;
  }

  return GetDefaultMode();
}
}

#endif