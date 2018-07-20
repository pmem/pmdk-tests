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

#ifdef _WIN32

#include "dimm.h"
#include "string_utils.h"

#define GUID_SIZE sizeof("XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX")

DimmNamespace::DimmNamespace(const std::string& mountpoint) {
  if (mountpoint.empty()) {
    throw std::invalid_argument(
        "mountPoint field is empty. Please set mountPoint value.");
  }
  this->mountpoint_ = mountpoint;

  if (!ApiC::DirectoryExists(mountpoint)) {
    throw std::invalid_argument("Directory " + mountpoint + " does not exist.");
  }
  test_dir_ = mountpoint + SEPARATOR + "pmdk_tests" + SEPARATOR;
  if (!ApiC::DirectoryExists(test_dir_) &&
      ApiC::CreateDirectoryT(test_dir_) != 0) {
    throw std::invalid_argument("Could not create: " + test_dir_);
  }
  if (AcquireUid(mountpoint) != 0) {
    throw std::invalid_argument("Could not get UID of mountpoint: " +
                                mountpoint);
  }
}

HANDLE DimmNamespace::AcquireVolumeHandle(std::string mountpoint) const {
  std::wstring mountpointW = string_utils::Convert<wchar_t, char>(mountpoint);

  wchar_t mount[MAX_PATH];
  wchar_t volume[MAX_PATH];

  if (!GetVolumePathNameW(mountpointW.c_str(), mount, MAX_PATH) ||
      /* get volume name - "\\?\Volume{VOLUME_GUID}\" */
      !GetVolumeNameForVolumeMountPointW(mount, volume, MAX_PATH) ||
      wcslen(volume) == 0 || volume[wcslen(volume) - 1] != '\\') {
    return INVALID_HANDLE_VALUE;
  }

  /*
   * Remove trailing \\ as "CreateFile processes a volume GUID path with
   * an appended backslash as the root directory of the volume."
   */
  volume[wcslen(volume) - 1] = '\0';

  return CreateFileW(volume, 0, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr,
                     OPEN_EXISTING, 0, nullptr);
}

int DimmNamespace::AcquireUid(std::string mountpoint) {
  HANDLE vHandle = AcquireVolumeHandle(mountpoint);
  if (vHandle == INVALID_HANDLE_VALUE)
    return -1;

  STORAGE_DEVICE_NUMBER_EX sdn;
  sdn.DeviceNumber = -1;
  DWORD dwBytesReturned = 0;
  if (!DeviceIoControl(vHandle, IOCTL_STORAGE_GET_DEVICE_NUMBER_EX, nullptr, 0,
                       &sdn, sizeof(sdn), &dwBytesReturned, nullptr)) {
    /*
     * IOCTL_STORAGE_GET_DEVICE_NUMBER_EX is not supported
     * on this server
     */
    CloseHandle(vHandle);
    return -1;
  }
  GUID guid = sdn.DeviceGuid;

  char uid[GUID_SIZE];
  snprintf(
      uid, GUID_SIZE,
      "%08lX-%04hX-%04hX-%02hhX%02hhX-%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX",
      guid.Data1, guid.Data2, guid.Data3, guid.Data4[0], guid.Data4[1],
      guid.Data4[2], guid.Data4[3], guid.Data4[4], guid.Data4[5], guid.Data4[6],
      guid.Data4[7]);
  CloseHandle(vHandle);

  uid_ = std::string{uid};
  return 0;
}

int DimmNamespace::GetShutdownCount() const {
  HANDLE vHandle = AcquireVolumeHandle(mountpoint_);
  if (vHandle == INVALID_HANDLE_VALUE)
    return -1;

  STORAGE_PROPERTY_QUERY prop;
  DWORD dwSize;
  prop.PropertyId = StorageDeviceUnsafeShutdownCount;
  prop.QueryType = PropertyExistsQuery;
  STORAGE_DEVICE_UNSAFE_SHUTDOWN_COUNT ret;

  BOOL bResult = DeviceIoControl(vHandle, IOCTL_STORAGE_QUERY_PROPERTY, &prop,
                                 sizeof(prop), &ret, sizeof(ret),
                                 (LPDWORD)&dwSize, (LPOVERLAPPED) nullptr);

  if (!bResult) {
    CloseHandle(vHandle);
    return 0; /* STORAGE_PROPERTY_QUERY not supported */
  }
  prop.QueryType = PropertyStandardQuery;
  bResult = DeviceIoControl(vHandle, IOCTL_STORAGE_QUERY_PROPERTY, &prop,
                            sizeof(prop), &ret, sizeof(ret), (LPDWORD)&dwSize,
                            (LPOVERLAPPED) nullptr);

  CloseHandle(vHandle);
  if (!bResult)
    return -1;

  return ret.UnsafeShutdownCount;
}

#endif  // _WIN32
