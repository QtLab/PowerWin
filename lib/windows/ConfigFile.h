#pragma once

#include <string>
#include <vector>

#include "macros.h"
#include "../c++/stringref.h"

////////////////////////////////////////////////////////////////////////////////
//     ConfigFile
////////////////////////////////////////////////////////////////////////////////
class ConfigFile {
  DISALLOW_COPY_AND_ASSIGN(ConfigFile);

public:  
  ConfigFile();

  void loadFromFile(const cpp::wstring_ref& filename);
  void save() { }

  bool existsKey(const wchar_t* section, const wchar_t* key) const;

  std::wstring getString(const wchar_t* section, const wchar_t* key, const wchar_t* default_value) const;
  int getInteger(const wchar_t* section, const wchar_t* key, int default_value) const;
  void setString(const wchar_t* section, const wchar_t* key, const std::wstring& value);
  void setInteger(const wchar_t* section, const wchar_t* key,  int value);

  std::vector<std::wstring> getSections() const;
  std::vector<std::wstring> getKeys(const wchar_t* section) const;

private:
  std::wstring filename_;

  static std::vector<std::wstring> splitStringArray(const wchar_t* strings);
};
