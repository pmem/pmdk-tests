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

#ifdef _WIN32

#include <codecvt>
#include <locale>
#include "api_c.h"

int ApiC::AllocateFileSpace(const std::string &path, size_t length) const {
  if (length <= 0) {
    std::cerr << "length should be > 0" << std::endl;
    return -1;
  }

  HANDLE h = CreateFile(path.c_str(), GENERIC_READ | GENERIC_WRITE,
                        FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, CREATE_NEW,
                        FILE_ATTRIBUTE_NORMAL, nullptr);

  if (h == INVALID_HANDLE_VALUE) {
    std::cerr << "INVALID_HANDLE_VALUE occurs\nError message: "
              << GetLastError() << std::endl;
    return -1;
  }

  /* check if sparse files are supported */
  DWORD flags = 0;
  BOOL ret = GetVolumeInformationByHandleW(h, nullptr, 0, nullptr, nullptr,
                                           &flags, nullptr, 0);
  if (!ret) {
    std::cerr << "GetVolumeInformationByHandleW: ";
    goto err;
  } else if ((flags & FILE_SUPPORTS_SPARSE_FILES) == 0) {
    std::cerr << "Filesystem does not support sparse files" << std::endl;
    goto err;
  }

  /* mark file as sparse */
  DWORD nbytes;
  ret = DeviceIoControl(h, FSCTL_SET_SPARSE, nullptr, 0, nullptr, 0, &nbytes,
                        nullptr);
  if (!ret) {
    std::cerr << "DeviceIoControl: ";
    goto err;
  }

  /* set file length */
  LARGE_INTEGER llen;
  llen.QuadPart = length;

  DWORD ptr = SetFilePointerEx(h, llen, nullptr, FILE_BEGIN);
  if (ptr == INVALID_SET_FILE_POINTER) {
    std::cerr << "SetFilePointerEx: ";
    goto err;
  }

  ret = SetEndOfFile(h);
  if (!ret) {
    std::cerr << "SetEndOfFile: ";
    goto err;
  }

  return 0;

err:
  std::cerr << GetLastError() << std::endl;
  CloseHandle(h);
  std::remove(path.c_str());
  return -1;
}

int ApiC::GetExecutablePath(std::string &path) {
  char file_path[MAX_PATH + 1] = {0};
  auto count = GetModuleFileName(nullptr, file_path, MAX_PATH);

  if (count == 0 || count == MAX_PATH) {
    return -1;
  }

  file_path[count] = '\0';

  char dir[MAX_PATH + 1] = {0};
  _splitpath(file_path, nullptr, dir, nullptr, nullptr);
  path = std::string{dir} + "\\";

  return 0;
}

size_t ApiC::GetFreespace(const std::string &dir) const {
  __int64 free_bytes_available, total_number_of_bytes,
      total_number_of_free_bytes;
  LPCSTR drive = dir.c_str();
  int result =
      GetDiskFreeSpaceExA(drive, (PULARGE_INTEGER)&free_bytes_available,
                          (PULARGE_INTEGER)&total_number_of_bytes,
                          (PULARGE_INTEGER)&total_number_of_free_bytes);

  if (result == 0) {
    std::cerr << "Unable to get file system statistics" << std::endl;
    return -1;
  }

  return total_number_of_free_bytes;
}

int ApiC::CreateFileT(const std::wstring &path,
                      const std::vector<std::wstring> &content,
                      bool is_bom) const {
  std::wstring temp_content;

  for (const auto &line : content) {
    temp_content += line + L"\n";
  }

  return CreateFileT(path, temp_content, is_bom);
}

int ApiC::CreateDirectoryT(const std::string &dir) const {
  BOOL ret = CreateDirectory(dir.c_str(), nullptr);

  if (ret) {
    return 0;
  }

  std::cerr << "Unable to create directory " << GetLastError() << std::endl;
  return -1;
}

int ApiC::CleanDirectory(const std::string &dir) const {
  int ret = 0;
  HANDLE h = nullptr;
  WIN32_FIND_DATA f_d;

  h = FindFirstFile((dir + "*").c_str(), &f_d);

  if (h == INVALID_HANDLE_VALUE) {
    std::cerr << "INVALID_HANDLE_VALUE occurs\nError message: "
              << GetLastError() << std::endl;
    return -1;
  }

  do {
    const std::string file_name = f_d.cFileName;
    const std::string full_file_name = dir + file_name;
    if (!lstrcmp(f_d.cFileName, ".") || !lstrcmp(f_d.cFileName, "..")) {
      continue;
    }

    SetFilePermission(dir + f_d.cFileName, 0600);

    if (DirectoryExists(dir + f_d.cFileName) &&
        !RemoveDirectory((dir + f_d.cFileName).c_str())) {
      std::cerr << "Unable to remove directory " << GetLastError() << std::endl;
      ret = -1;
    } else if (RemoveFile(dir + f_d.cFileName) != 0) {
      std::cerr << "File " << f_d.cFileName << " removing failed" << std::endl;
      ret = -1;
    }
  } while (FindNextFile(h, &f_d));

  if (h != INVALID_HANDLE_VALUE && !FindClose(h)) {
    std::cerr << "Closing dir failed" << std::endl;
    return -1;
  }

  return ret;
}

int ApiC::RemoveDirectoryT(const std::string &dir) const {
  BOOL ret = RemoveDirectory(dir.c_str());

  if (!ret) {
    std::cerr << "Unable to remove directory: " << GetLastError() << std::endl;
    return -1;
  }

  return 0;
}

int ApiC::CreateFileT(const std::wstring &path, const std::wstring &content,
                      bool is_bom) const {
  std::locale utf8_locale;

  if (is_bom) {
    utf8_locale = std::locale(
        std::locale(),
        new std::codecvt_utf8<wchar_t, 0x10ffff,
                              std::codecvt_mode::generate_header>());
  } else {
    utf8_locale =
        std::locale(std::locale(),
                    new std::codecvt_utf8<wchar_t, 0x10ffff,
                                          std::codecvt_mode::consume_header>());
  }

  std::wofstream file{path, std::ios::out | std::ios::trunc};
  file.imbue(utf8_locale);

  if (!file.good()) {
    std::wcerr << "File opening failed" << std::endl;
    return -1;
  }

  file << content;

  return 0;
}

int ApiC::ReadFile(const std::wstring &path, std::wstring &content) {
  std::wifstream file{path, std::ios::binary | std::ios::ate};

  if (!file.good()) {
    std::wcerr << "File opening failed" << std::endl;
    return -1;
  }

  std::wstring line;

  while (std::getline(file, line)) {
    content += line;
  }

  return 0;
}

bool ApiC::RegularFileExists(const std::wstring &path) const {
  struct stat64 file_stat;

  return _wstat64(path.c_str(), &file_stat) == 0
             ? (file_stat.st_mode & S_IFREG) != 0
             : false;
}

unsigned short ApiC::GetFilePermission(const std::wstring &file) const {
  struct stat64 file_stat;

  int result = _wstat64(file.c_str(), &file_stat);

  if (result != 0) {
    std::wcerr << L"Unable to change file permission: " << _wcserror(errno)
               << std::endl;

    return 0;
  }

  file_stat.st_mode &= 0700;

  return file_stat.st_mode;
}

int ApiC::SetFilePermission(const std::wstring &path, int permissions) const {
  int ret = _wchmod(path.c_str(), permissions);

  if (ret != 0) {
    std::wcerr << L"Unable to change file permission: " << _wcserror(errno)
               << std::endl;
  }

  return ret;
}

int ApiC::RemoveFile(const std::wstring &path) const {
  int ret = _wremove(path.c_str());

  if (ret != 0) {
    std::wcerr << L"The file can not be deleted: " << _wcserror(errno);
  }

  return ret;
}

bool ApiC::DirectoryExists(const std::wstring &dir) const {
  struct stat64 file_stat;
  if (_wstat64(dir.c_str(), &file_stat) == 0) {
    return (file_stat.st_mode & S_IFDIR) != 0;
  }

  return false;
}

size_t ApiC::GetFreespace(const std::wstring &dir) const {
  __int64 free_bytes_available, total_number_of_bytes,
      total_number_of_free_bytes;
  LPCWSTR drive = dir.c_str();
  int result =
      GetDiskFreeSpaceExW(drive, (PULARGE_INTEGER)&free_bytes_available,
                          (PULARGE_INTEGER)&total_number_of_bytes,
                          (PULARGE_INTEGER)&total_number_of_free_bytes);

  if (result == 0) {
    std::wcerr << L"Unable to get file system statistics" << std::endl;
    return -1;
  }

  return total_number_of_free_bytes;
}

#endif  // !_WIN32
