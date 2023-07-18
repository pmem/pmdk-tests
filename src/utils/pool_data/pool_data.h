/*
 * Copyright 2018-2023, Intel Corporation
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

#ifndef POOL_DATA_H
#define POOL_DATA_H

#include <libpmemobj.h>
#include <iostream>
#include <vector>

template <typename T>
class ObjData {
 public:
  ObjData(PMEMobjpool *pop) : pop_(pop) {
  }

  int Write(std::vector<T> &data) {
    for (auto &v : data) {
      PMEMoid oid;
      if (pmemobj_alloc(pop_, &oid, sizeof(struct elem), type_num_,
                        elem_constructor, &v) != 0) {
        std::cerr << "Data allocation failed. Errno: " << errno;
        return -1;
      }
      ++type_num_;
    }
    return 0;
  }

  std::vector<T> Read() {
    std::vector<T> values;
    PMEMoid oid;
    int type_num = 0;

    while (!OID_IS_NULL(oid = POBJ_FIRST_TYPE_NUM(pop_, type_num))) {
      elem *e = static_cast<struct elem *>(pmemobj_direct(oid));
      values.emplace_back(e->value);
      ++type_num;
    }

    return values;
  }

 private:
  struct elem {
    T value;
  };

  static int elem_constructor(PMEMobjpool *pop, void *ptr, void *arg) {
    struct elem *element = static_cast<struct elem *>(ptr);
    element->value = *static_cast<T *>(arg);
    pmemobj_persist(pop, element, sizeof(struct elem));
    return 0;
  }
  PMEMobjpool *pop_;
  int type_num_ = 0;
};

#endif  // POOL_DATA_H
