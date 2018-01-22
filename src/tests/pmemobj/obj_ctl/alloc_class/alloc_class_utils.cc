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

#include "alloc_class_utils.h"
#include <iostream>
#include <map>
#include <string>

namespace AllocClassUtils {
namespace {
const std::map<pobj_header_type, std::string> hdr_type = {
    {pobj_header_type::POBJ_HEADER_LEGACY, "POBJ_HEADER_LEGACY"},
    {pobj_header_type::POBJ_HEADER_COMPACT, "POBJ_HEADER_COMPACT"},
    {pobj_header_type::POBJ_HEADER_NONE, "POBJ_HEADER_NONE"},
    {pobj_header_type::MAX_POBJ_HEADER_TYPES, "MAX_POBJ_HEADER_TYPES"}};

void printArg(const pobj_alloc_class_desc &arg) {
  std::cout << "Unit size: " << arg.unit_size << std::endl;
  std::cout << "Units per block: " << arg.units_per_block << std::endl;
  std::cout << "Header type: " << hdr_type.at(arg.header_type) << std::endl;
  std::cout << "Class ID: " << arg.class_id << std::endl;
}
}
bool IsAllocClassValid(const pobj_alloc_class_desc &write,
                       const pobj_alloc_class_desc &read) {
  const unsigned bitmap_size = 2432;
  bool ret = write.class_id == read.class_id &&
             write.header_type == read.header_type &&
             write.unit_size == read.unit_size &&
             (write.units_per_block > bitmap_size
                  ? write.units_per_block >= 2048
                  : write.units_per_block <= read.units_per_block);
  if (!ret) {
    std::cerr << "---[SET]---\n";
    printArg(write);
    std::cerr << "---[GET]---\n";
    printArg(read);
  }
  return ret;
}

unsigned GetHeaderMetadata(const enum pobj_header_type &hdr) {
  unsigned metadata;
  switch (hdr) {
  case POBJ_HEADER_LEGACY:
    metadata = 64;
    break;
  case POBJ_HEADER_COMPACT:
    metadata = 16;
    break;
  case POBJ_HEADER_NONE:
    metadata = 0;
    break;
  default:
    throw std::out_of_range("Header type not supported");
  }
  return metadata;
}
}