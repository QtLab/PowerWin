#include "autostart.h"

#include "debug.h"
#include "application.h"
#include <windows.h>

namespace Windows {

bool setProgramToAutostart(bool value) {
  HKEY key;
  LONG error = RegCreateKeyExW (
    HKEY_CURRENT_USER,
    L"Software\\Microsoft\\Windows\\CurrentVersion\\Run",
    0, NULL, REG_OPTION_NON_VOLATILE,
    KEY_ALL_ACCESS, NULL, &key, 0);
  if (error != ERROR_SUCCESS) {
    WIN_WARNING(L"cannot open autostart registry key: %s", GetLastWindowsError().c_str());
    return false;
  }

  if (value) {
    error = RegSetValueExW(key, Application::getName().c_str(), 0, REG_SZ,
      (LPBYTE)(Application::getExecutablePath().toString().c_str()), (DWORD)((Application::getExecutablePath().toString().size() + 1) * sizeof(wchar_t)));
  } else {
    if (RegQueryValueExW(key, Application::getName().c_str(),0,0,0,0) != ERROR_FILE_NOT_FOUND)
      error = RegDeleteValueW(key, Application::getName().c_str());
  }

  RegCloseKey(key);
  if (error != ERROR_SUCCESS) {
    WIN_WARNING(L"cannot set autostart registry value: %s", GetLastWindowsError().c_str());
    return false;
  }
  return true;
}

bool isProgramInAutostart() {
  HKEY key;
  LONG error;

  error = RegCreateKeyExW(
    HKEY_CURRENT_USER,
    L"Software\\Microsoft\\Windows\\CurrentVersion\\Run",
    0, NULL, REG_OPTION_NON_VOLATILE,
    KEY_ALL_ACCESS, NULL, &key, 0);
  if (error != ERROR_SUCCESS) {
    WIN_WARNING(L"cannot open autostart registry key: %s", GetLastWindowsError().c_str());
    return false;
  }

  wchar_t *buffer;
  DWORD buffersize = 0;

#if (WINVER < _WIN32_WINNT_VISTA)
  DWORD type;
  error = RegQueryValueEx(key, Application::getName().c_str(), NULL,  &type, NULL, &buffersize);
  if (type != REG_SZ) {
    WIN_WARNING(L"%s", L"autostart registry value has wrong type.");
    RegCloseKey(key);
    return false;
  }
#else
  error = RegGetValue(key, NULL, Application::getName().c_str(), RRF_RT_REG_SZ, NULL, NULL, &buffersize);
#endif
  if (error == ERROR_SUCCESS) {
    buffer = (wchar_t*)malloc(sizeof(wchar_t) * (buffersize+1));
#if (WINVER < _WIN32_WINNT_VISTA)
    error = RegQueryValueEx(key, Application::getName().c_str(), NULL,  &type, (BYTE*)buffer, &buffersize);
#else
    error = RegGetValue(key, NULL, Application::getName().c_str(), RRF_RT_REG_SZ, NULL, buffer, &buffersize);
#endif
  }
  bool result;
  if (error != ERROR_SUCCESS) {
    if (error != ERROR_FILE_NOT_FOUND) {
      WIN_WARNING(L"cannot read autostart registry value: %s", GetLastWindowsError().c_str());
    }
    result = false;
  } else {
    result = (Application::getExecutablePath().toString().compare(buffer) == 0);
  }

  RegCloseKey(key);

  return result;
}

} // namespace Windows
