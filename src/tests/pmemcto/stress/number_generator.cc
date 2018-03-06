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

#include "number_generator.h"
#include <iomanip>
#include <iostream>

NumberGenerator::NumberGenerator()
    : seed{static_cast<long>(
          std::chrono::system_clock::now().time_since_epoch().count())},
      gen(seed),
      dist{} {
  std::cout << "Seed: " << seed << std::endl;
}

NumberGenerator::NumberGenerator(double m, double s)
    : seed{static_cast<long>(
          std::chrono::system_clock::now().time_since_epoch().count())},
      gen(seed),
      dist{m, s} {
  std::cout << "Seed: " << seed << std::endl;
}

size_t NumberGenerator::Rand(double base, double max_exp) {
  double exp = std::round(dist(gen));
  do {
    exp = std::round(dist(gen));
  } while (exp > max_exp);
  size_t number = static_cast<size_t>(pow(base, exp));
  ++hist[number];
  return number;
}

void NumberGenerator::Print() {
  std::cout << "Distribution of persist values: " << std::endl;
  std::cout << std::fixed << std::setw(12) << "Size" << ' ' << "Count"
            << std::endl;
  for (auto p : hist) {
    std::cout << std::fixed << std::setw(12) << p.first << ' ' << p.second
              << std::endl;
  }
}
