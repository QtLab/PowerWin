#include "Plugin.h"

#include <algorithm>
#include <boost/algorithm/string.hpp>

#include "windows/debug.h"

Plugin::Plugin(cpp::wstring_view name) :
  active_(false), options_(), name_(name.to_string())
{
}

Plugin::~Plugin() {
  active_ = false;
}

void Plugin::activate() {
  if (active_) {
    deactivate();
  }

  onActivate(options_);
  active_ = true;
}

void Plugin::deactivate() {
  if (active_) {
    onDeactivate();
    active_ = false;
  }
}

bool Plugin::getBooleanOption(cpp::wstring_view key, bool default_) {
  auto opt = options_.find(key);
  if (opt != options_.end()) {
    // Schlüssel vorhanden

    std::wstring value = opt->second;
    boost::to_lower(value);
    if (boost::equal(value, L"true")) {
      return true;
    } else if (boost::equal(value, L"false")) {
      return false;
    }

    print(L"Cannot read config key %s\n", key.c_str());
  }

  return default_;
}
