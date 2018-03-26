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

#ifndef PMDK_TESTS_SRC_UTILS_PMDK_STRUCTURES_OBJ_H_
#define PMDK_TESTS_SRC_UTILS_PMDK_STRUCTURES_OBJ_H_

#include "constants.h"
#include <iostream>
#include <libpmemobj.h>
#include <string>

class ObjPool {
public:
  std::string path;
  size_t size = 0;
  int mode = 0664 & PERMISSION_MASK;
  std::string layout;

  ObjPool() {}

  ObjPool(std::string path_) : path(path_) {}

  ObjPool(std::string path_, std::string layout_)
      : path(path_), layout(layout_) {}

  ObjPool(std::string path_, size_t size_) : path(path_), size(size_) {}

  ObjPool(std::string path_, int mode_) : path(path_), mode(mode_) {}

  ObjPool(std::string path_, size_t size_, int mode_)
      : path(path_), size(size_), mode(mode_) {}

  ObjPool(std::string path_, size_t size_, int mode_, std::string layout_)
      : path(path_), size(size_), mode(mode_), layout(layout_) {}
};

class ObjManagement {
public:
  int CreatePool(const ObjPool &obj_pool);
  PMEMobjpool *OpenPool(const ObjPool &obj_pool);
  int CheckPool(const ObjPool &obj_pool);
  void ClosePool(PMEMobjpool *pop) { pmemobj_close(pop); }
};

#endif // !PMDK_TESTS_SRC_UTILS_PMDK_STRUCTURES_OBJ_H_
