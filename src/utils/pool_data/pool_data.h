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

#ifndef POOL_DATA_H
#define POOL_DATA_H

#include <libpmemblk.h>
#include <libpmemlog.h>
#include <libpmemobj.h>
#include <vector>
#include "gtest/gtest.h"

template <typename T>
class ObjData {
 public:
  ObjData(PMEMobjpool *pop) : pop_(pop) {
  }

  int WriteData() {
    for (auto v : test_data_) {
      PMEMoid oid;
      pmemobj_alloc(pop_, &oid, sizeof(struct elem), type_num_,
                    elem_constructor, &v);
      type_num_++;

      if (OID_IS_NULL(oid)) {
        std::cerr << "Data allocation failed. Errno: " << errno;
        return 1;
      }
    }
    return 0;
  }

  int AssertDataCorrect() {
    std::vector<T> values;
    int tn = 0;

    while (true) {
      PMEMoid oid = POBJ_FIRST_TYPE_NUM(pop_, tn);
      tn++;

      if (OID_IS_NULL(oid)) {
        break;
      }

      elem *e = (struct elem *)pmemobj_direct(oid);
      values.emplace_back(e->value);
    }

    if (values != test_data_) {
      std::cerr << "Data read from obj pool does not match data written to it"
                << std::endl;
      return 1;
    }
    return 0;
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

  std::vector<int> test_data_ = {-2, 0, 12345, 1412, 1231, 23, 432, 34, 3};
};

class BlkData {
 public:
  BlkData(PMEMblkpool *pbp) : pbp_(pbp) {
  }
  int WriteData();
  int AssertDataCorrect();

 private:
  PMEMblkpool *pbp_;
  int writes_number_ = 75;
  size_t blk_size_;
};

class LogData {
 public:
  LogData(PMEMlogpool *plp) : plp_(plp) {
  }
  int WriteData();
  int AssertDataCorrect();

 private:
  static int ReadLog(const void *buf, size_t len, void *arg);
  static std::string const log_text_;

  const int chunks_ = 10;
  PMEMlogpool *plp_;
};

#endif  // POOL_DATA_H
