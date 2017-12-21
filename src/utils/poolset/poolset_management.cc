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

#include "poolset_management.h"

bool PoolsetManagement::AllFilesExist(const Poolset &p) {
  for (const auto &part : p.GetParts()) {
    if (!PartExists(part)) {
      return false;
    }
  }
  return true;
}

bool PoolsetManagement::NoFilesExist(const Poolset &p) {
  for (const auto &part : p.GetParts()) {
    if (PartExists(part)) {
      return false;
    }
  }
  return true;
}

bool PoolsetManagement::ReplicaExists(const Replica &r) {
  for (const auto &part : r.GetParts()) {
    if (!api_c_.RegularFileExists(part.GetPath())) {
      return false;
    }
  }
  return true;
}

bool PoolsetManagement::PartExists(const Part &p) {
  return api_c_.RegularFileExists(p.GetPath());
}

bool PoolsetManagement::PoolsetFileExists(const Poolset &p) {
  return api_c_.RegularFileExists(p.GetFullPath());
}

int PoolsetManagement::CreatePoolsetFile(const Poolset &p) {
  return api_c_.CreateFileT(p.GetFullPath(), p.GetContent());
}

int PoolsetManagement::RemovePoolsetFile(const Poolset &p) {
  return api_c_.RemoveFile(p.GetFullPath());
}

int PoolsetManagement::RemovePartsFromPoolset(const Poolset &p) {
  int ret = 0;
  for (const auto &part : p.GetParts()) {
    ret |= api_c_.RemoveFile(part.GetPath());
  }
  return ret;
}

int PoolsetManagement::RemovePart(const Part &p) {
  return api_c_.RemoveFile(p.GetPath());
}
