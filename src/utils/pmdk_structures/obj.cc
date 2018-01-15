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

#include "obj.h"

int ObjManagement::CreatePool(const ObjPool &obj_pool) {
  const char *layout =
      obj_pool.layout.empty() ? nullptr : obj_pool.layout.c_str();
  PMEMobjpool *pop = pmemobj_create(obj_pool.path.c_str(), layout,
                                    obj_pool.size, obj_pool.mode);

  if (pop == nullptr) {
    std::cout << "Cannot create pool of obj type.\n" << strerror(errno)
              << std::endl;

    return -1;
  }

  pmemobj_close(pop);

  return 0;
}

PMEMobjpool *ObjManagement::OpenPool(const ObjPool &obj_pool) {
  const char *layout =
      obj_pool.layout.empty() ? nullptr : obj_pool.layout.c_str();
  PMEMobjpool *pop = pmemobj_open(obj_pool.path.c_str(), layout);

  if (pop == nullptr) {
    std::cout << "Cannot open pool.\n" << strerror(errno) << std::endl;
    return nullptr;
  }

  return pop;
}

int ObjManagement::CheckPool(const ObjPool &obj_pool) {
  const char *layout =
      obj_pool.layout.empty() ? nullptr : obj_pool.layout.c_str();

  return pmemobj_check(obj_pool.path.c_str(), layout);
}
