#ifndef APPLICATION_H
#define APPLICATION_H

#include <windows.h>
#include <functional>
#include <string>
#include <cassert>
#include <boost/filesystem.hpp>

#include "windows.h"
#include "macros.h"
#include "memory.h"
#include <c++/stringview.h>

namespace Windows {

class Application final {
  DISALLOW_COPY_AND_ASSIGN(Application);

public:
  // types
  typedef std::function<int()> Callback;

  // ctor
  Application(cpp::wstring_view name, HINSTANCE instance);

  // run
  static int run(Callback entry) { return self().run(entry); }
  int run(Callback entry);

  // application properties
  static HINSTANCE getInstance() { return self().appinstance_; }
  static cpp::wstring_view getName() { return self().name_; }

  // paths
  static boost::filesystem::path getExecutablePath();
  static boost::filesystem::path getConigPath();

  // system
  //static WindowsVersion getWindowsVersion() { return winversion; }
  // TODO: static string getWindowsVersionString();
  static bool Is64BitWindows();

  // messages
  static void processMessages();

private:
  Handle mutex_;
  bool is_running_;

  const HINSTANCE appinstance_;
  const std::wstring name_;

  static Application* instance_;

  static Application& self() { assert(instance_ == nullptr); return *instance_; }
};

} // namespace Windows

#endif // APPLICATION_H
