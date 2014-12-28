#include <string>
#include <algorithm>
#include <memory>
#include <vector>
#include <cstring>

#include "c++/algorithm.h"
#include "c++/arrayref.h"
#include "c++/boolean.h"
#include "powerwin.h"
#include "powerwin-private.h"
#include "windows/configfile.h"
#include "windows/utilwindow.h"
#include "windows/path.h"
#include "plugins/ActionsPlugin.h"
#include "plugins/ScrollPlugin.h"
#include "plugins/FullscreenPlugin.h"
#include "plugins/SystemMenuPlugin.h"
#include "windows/debug.h"
#include "macros.h"
#include "windows/application.h"
#include "windows/trayicon.h"
#include "windows/controls.h"

#if CPUBITSET == 32
# define POWERWIN_APP_NAME L"PowerWin32"
# define POWERWIN_64BIT_NAME L"PowerWin64"
#elif CPUBITSET == 64
# define POWERWIN_APP_NAME L"PowerWin64"
#endif

namespace {

HINSTANCE dllinstance_ = nullptr;

class PowerWin : Windows::Window {
  PowerWin();

public:
  static int run();

  static PowerWin* get() {
    return instance_;
  }

  HWND getWindow() {
    return getNativeHandle();
  }

  LRESULT onMessage(UINT msg, WPARAM wparam, LPARAM lparam) override;
  void onCreate() override;
  void onDestroy() override;

private:
  std::vector<std::unique_ptr<Plugin>> plugins_;

  Windows::TrayIcon tray_icon_;


  static PowerWin* instance_;

  static ATOM RegisterWinClass(HINSTANCE hInstance);
  void start();
  void onDestroy();
};

PowerWin* PowerWin::instance_ = nullptr;

PowerWin::PowerWin() :
  plugins_(),
  tray_icon_()
{
#ifdef MAIN_MODULE
  plugins_.push_back(std::unique_ptr<Plugin>(new ActionsPlugin()));
  plugins_.push_back(std::unique_ptr<Plugin>(new ScrollPlugin()));
  //plugins_.push_back(std::unique_ptr<Plugin>(new FullscreenPlugin()));
  plugins_.push_back(std::unique_ptr<Plugin>(new SystemMenuPlugin()));
#else
  plugins_.push_back(std::unique_ptr<Plugin>(new SystemMenuPlugin()));
#endif
}

int PowerWin::run() {
  //  init Comctl32.dll
  /*const INITCOMMONCONTROLSEX icce = {
    sizeof(INITCOMMONCONTROLSEX),
    ICC_STANDARD_CLASSES
  };
  if (!InitCommonControlsEx(&icce)) {
    ERROR(L"%s\n", L"Kann 'common controls' nicht initailsieren!");
  }*/
  PowerWin powerwin;
  instance_ = &powerwin;

  powerwin.create(nullptr, lit(POWERWIN_APP_NAME));

  print(L"%ls hwnd: %d\n", POWERWIN_APP_NAME, powerwin.getNativeHandle());

  processMessages();

  print(L"%ls: The end\n", POWERWIN_APP_NAME);

  return 0;
}

void PowerWin::onCreate() {
  print(L"PowerWin::start\n");

  ConfigFile config;
  config.loadFromFile(Windows::Application::getExecutablePath() + L"\\config.ini");

  for (auto&& plugin : plugins_) {
    Plugin::Options opts;
    for (const std::wstring& key : config.getKeys(plugin->getName())) {
      opts[key] = config.getString(plugin->getName(), key.c_str(), nullptr);
    }
    plugin->setOptions(std::move(opts));

    if (plugin->getBooleanOption(L"active", true)) {
      print(L"activate plugin (0x%p) %ls\n", plugin.get(), plugin->getName());
      plugin->activate();
    }
  }


  /*tray_icon.add(WinExtra::getMainWindow(),
                ExtractIcon(
                  Windows::Application::getInstance(),
                  L"C:\\Windows\\system32\\shell32.dll",
                  -26));*/
#ifdef MAIN_MODULE
  tray_icon_.add(window_, LoadIcon(NULL, IDI_APPLICATION));

  // start 64Bit-DLL
  if (Windows::Application::Is64BitWindows()) {
    print(L"Start 64-bit process\n");
    Windows::RunDll::execute64BitDll(Windows::Application::getExecutablePath() + L"\\libpowerwin64.dll", L"FixWindows", L"");
    // TODO: error logging
  }
#endif
}

void PowerWin::onDestroy() {
#ifdef MAIN_MODULE
  // exit 64Bit-DLL
  if (Windows::Application::Is64BitWindows()) {
    SetLastError(0);
    HWND window_64bit = FindWindowW(POWERWIN_64BIT_NAME, nullptr);
    if (window_64bit == nullptr) {
      print(L"Cannot find window to 64Bit-Process: %ls\n", Windows::GetLastWindowsError().c_str());
    } else {
      print(L"Exit 64-bit process. window: %d\n", window_64bit);
      if (!PostMessage(window_64bit, WM_CLOSE, 0, 0)) {
        print(L"PostMessage to 64-bit process window failed: %ls\n", Windows::GetLastWindowsError().c_str());
      }
    }
  }
#endif

  // deactivate all plugins
  try {
    for (auto&& plugin : plugins_) {
      print(L"Deactivate plugin %ls\n", plugin->getName());
      plugin->deactivate();
    }
  } catch (...) {
    MessageBox(NULL, L"Ups", L"!!!", MB_ICONERROR | MB_OK);
  }

#ifdef MAIN_MODULE
  // remove tray icon
  tray_icon_.remove();
#endif

  // exit process
  PostQuitMessage(0);
}

} // namespace

extern "C"
void CALLBACK FixWindows(HINSTANCE hInstance,
                         HINSTANCE hPrevInstance,
                         LPSTR lpCmdLine,
                         int nCmdShow)
{
  Windows::Application app(POWERWIN_APP_NAME, hInstance);
  app.run(PowerWin::run);
}

///////////////////////////////////////////////////////////////////////////////
// Windowpicker

//auto windowpicker = MouseClickHook([](unsigned button, POINT pt) -> bool {
//  if (button == WM_LBUTTONDOWN) return true;
//  if (button != WM_LBUTTONUP) return false;
  
//  OutputDebugString(L"windowpicker\n");

//  HWND window = WindowFromPoint(pt);
//  if (window == NULL) {
//    OutputDebugString(L"Kein Fenster ausgewählt!\n");
//    return false;
//  } else {
//    wchar_t class_name[255];
//    GetClassNameW(window, class_name, sizeof(class_name));

//    OutputDebugString(L"Fensterklassenname: '");
//    OutputDebugString(class_name);
//    OutputDebugString(L"'\n");
//  }

//  return true;
//});

