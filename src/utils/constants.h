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

#ifndef PMDK_TESTS_SRC_UTILS_CONSTANTS_H_
#define PMDK_TESTS_SRC_UTILS_CONSTANTS_H_

#include <map>
#include <string>

#ifdef _WIN32
/* On Windows there is no permission for execution. Also windows doesn't
* support groups of users in UNIX way */
const int PERMISSION_MASK = 0600;
#else
const int PERMISSION_MASK = 0777;
#endif  // _WIN32

#ifdef _WIN32
const std::string SEPARATOR = "\\";
#else
const std::string SEPARATOR = "/";
#endif  // _WIN32

static const size_t KIBIBYTE = 1 << 10;
static const size_t MEBIBYTE = KIBIBYTE << 10;
static const size_t GIGIBYTE = MEBIBYTE << 10;
static const size_t KILOBYTE = 1000;
static const size_t MEGABYTE = KILOBYTE * 1000;
static const size_t GIGABYTE = MEGABYTE * 1000;

static const std::map<std::string, size_t> SIZES{
    {"KiB", KIBIBYTE}, {"MiB", MEBIBYTE}, {"GiB", GIGIBYTE},
    {"KB", KILOBYTE},  {"MB", MEGABYTE},  {"GB", GIGABYTE},
    {"K", KIBIBYTE},   {"M", MEBIBYTE},   {"G", GIGIBYTE}};

static const int CONSISTENT = 1;

#endif  // !PMDK_TESTS_SRC_UTILS_CONSTANTS_H_
