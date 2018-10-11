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

#include "unsafe_shutdown.h"

std::string UnsafeShutdown::GetNormalizedTestName() const {
  auto &test_info = GetTestInfo();
  std::string test_name{std::string{test_info.test_case_name()} + "_" +
                        std::string{test_info.name()}};
  string_utils::ReplaceAll(test_name, std::string{"/"}, std::string{"_"});
  string_utils::ReplaceAll(test_name, test_phase_.GetPhaseName(),
                           std::string{""});
  return test_name;
}

void UnsafeShutdown::StampPassedResult() const {
  if (GetTestInfo().result()->Passed()) {
    ApiC::CreateFileT(GetPassedStamp(), "");
  }
}

bool UnsafeShutdown::PassedOnPreviousPhase() const {
  bool ret = ApiC::RegularFileExists(GetPassedStamp());
  if (ret) {
    ApiC::RemoveFile(GetPassedStamp());
  } else {
    std::cerr << "Previous phase of the test failed" << std::endl;
  }
  return ret;
}

int UnsafeShutdown::PmempoolRepair(std::string pool_file_path) const {
  unsigned int flags = PMEMPOOL_CHECK_FORMAT_STR | PMEMPOOL_CHECK_REPAIR |
                       PMEMPOOL_CHECK_VERBOSE | PMEMPOOL_CHECK_ALWAYS_YES;
  struct pmempool_check_args args = {pool_file_path.c_str(), nullptr,
                                     PMEMPOOL_POOL_TYPE_DETECT, flags};

  PMEMpoolcheck *ppc = pmempool_check_init(&args, sizeof(args));
  if (!ppc) {
    std::cerr << "pmempool_check_init failed" << std::endl;
    return -1;
  }

  pmempool_check_status *status;
  std::string status_msg;
  while ((status = pmempool_check(ppc)) != nullptr) {
    switch (status->type) {
      case PMEMPOOL_CHECK_MSG_TYPE_ERROR:
      case PMEMPOOL_CHECK_MSG_TYPE_INFO:
        status_msg.append(status->str.msg + '\n');
        break;
      default:
        pmempool_check_end(ppc);
        std::cerr << "Status with incorrect type was returned" << std::endl;
        return -1;
    }
  }

  return pmempool_check_end(ppc);
}

int UnsafeShutdown::ObjCreateHelper(const std::string &path, size_t size) {
  pop_ = pmemobj_create(path.c_str(), nullptr, size, 0644 & PERMISSION_MASK);
  if (pop_ == nullptr) {
    std::cerr << "Pool creating failed. Errno: " << strerror(errno) << std::endl
              << pmemobj_errormsg() << std::endl;
    return -1;
  }
  return 0;
}

int UnsafeShutdown::ObjOpenSuccessHelper(const std::string &path) {
  pop_ = pmemobj_open(path.c_str(), nullptr);
  if (pop_ == nullptr) {
    std::cerr << "Pool opening failed. Errno: " << strerror(errno) << std::endl
              << pmemobj_errormsg();
    return -1;
  }
  return 0;
}

int UnsafeShutdown::ObjOpenFailureHelper(const std::string &path,
                                         int expected_errno) {
  pop_ = pmemobj_open(path.c_str(), nullptr);
  if (pop_ != nullptr) {
    std::cerr << "Pool was unexpectedly opened with success." << std::endl;
    return -1;
  }

  if (expected_errno != errno) {
    std::cerr << "Expected errno (" << expected_errno
              << ") is different than actual (" << errno << ")" << std::endl;
    return -1;
  }
  return 0;
}
