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

#ifndef RAS_TEST_PHASE_H
#define RAS_TEST_PHASE_H

#include "gtest/gtest.h"
#include "inject_manager/inject_manager.h"
#include "non_copyable/non_copyable.h"

enum class ExecutionAction { begin, check_usc, inject, end, none };

template <class T>
class TestPhase : public NonCopyable {
 public:
  static T& GetInstance() {
    static T test_phase;
    return test_phase;
  }

  int RunPreTestAction() const;
  int RunPostTestAction() const;

  void ParseCmdArgs(int argc, char** argv);

  bool HasInjectAtEnd() const {
    return post_test_action_ == ExecutionAction::inject;
  }
  const std::string GetPhaseName() const {
    return this->phase_name_;
  }

 protected:
  InjectPolicy policy_;
  std::string policy_arg_;

  int Begin() const {
    return static_cast<const T*>(this)->Begin();
  }
  int Inject() const {
    return static_cast<const T*>(this)->Inject();
  }
  int CheckUSC() const {
    return static_cast<const T*>(this)->CheckUSC();
  }
  int End() const {
    return static_cast<const T*>(this)->End();
  }

 private:
  enum class ExecutionAction { none, begin, check_usc, inject, end };

  ExecutionAction pre_test_action_;
  ExecutionAction post_test_action_;
  std::string phase_name_;
};

template <class T>
int TestPhase<T>::RunPreTestAction() const {
  switch (pre_test_action_) {
    case ExecutionAction::begin:
      return Begin();
    case ExecutionAction::check_usc:
      return CheckUSC();
    case ExecutionAction::none:
      return 0;
    default:
      throw std::invalid_argument("Invalid pre execution action");
  }
}

template <class T>
int TestPhase<T>::RunPostTestAction() const {
  switch (post_test_action_) {
    case ExecutionAction::inject:
      return Inject();
    case ExecutionAction::end:
      return End();
    default:
      throw std::invalid_argument("Invalid post execution action");
  }
}

template <class T>
void TestPhase<T>::ParseCmdArgs(int argc, char** argv) {
  const std::string usage =
      "./" + std::string{argv[0]} +
      " <phase_number> <inject|cleanup> <inject_policy{all|first|last}>";
  if (argc != 4) {
    throw std::invalid_argument(usage);
  }

  int phase_number = std::atoi(argv[1]);
  phase_name_ = std::string{"phase_"} + argv[1];

  if (phase_number < 1) {
    pre_test_action_ = ExecutionAction::none;
  } else if (phase_number == 1) {
    pre_test_action_ = ExecutionAction::begin;
  } else {
    pre_test_action_ = ExecutionAction::check_usc;
  }

  std::map<std::string, ExecutionAction> action_args = {
      {"cleanup", ExecutionAction::end}, {"inject", ExecutionAction::inject}};
  auto search_action = action_args.find(std::string{argv[2]});
  if (search_action == action_args.end()) {
    throw std::invalid_argument(usage);
  }
  post_test_action_ = search_action->second;

  std::map<std::string, InjectPolicy> policy_args = {
      {"all", InjectPolicy::all},
      {"first", InjectPolicy::first},
      {"last", InjectPolicy::last}};
  auto search_policy = policy_args.find(std::string{argv[3]});
  if (search_policy == policy_args.end()) {
    throw std::invalid_argument(usage);
  }
  policy_ = search_policy->second;
  policy_arg_ = search_policy->first;
}

#endif  // RAS_TEST_PHASE
