#include <string>
#include <algorithm>
#include <memory>
#include <vector>
#include <cstring>

#include "c++/algorithm.h"
#include "c++/array_ref.h"
#include "DesktopHooks.h"
#include "windows/Hook.h"
#include "windows/ConfigFile.h"
#include "windows/utilwindow.h"
#include "plugins/ActionsPlugin.h"
#include "plugins/ScrollPlugin.h"
#include "plugins/FullscreenPlugin.h"
#include "plugins/SystemMenuPlugin.h"
#include "windows/debug.h"
#include "macros.h"
#include "windows/application.h"
#include "windows/trayicon.h"

namespace {

HINSTANCE dllinstance_ = nullptr;

// /////////////////////////////////////////////////////////////////////////////

class PowerWin {
public:
  PowerWin();

  static int run();

  static PowerWin* get() {
    return instance_;
  }

  HWND getWindow() {
    return window_;
  }

private:
  std::vector<std::unique_ptr<Plugin>> plugins_;

  Windows::TrayIcon tray_icon_;
  HWND window_;


  static PowerWin* instance_;

  static ATOM RegisterWinClass(HINSTANCE hInstance);
  void start(HWND hwnd);
  void onDestroy();

  static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
};

PowerWin* PowerWin::instance_ = nullptr;

PowerWin::PowerWin() :
  plugins_(),
  tray_icon_(),
  window_(nullptr)
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

ATOM PowerWin::RegisterWinClass(HINSTANCE hInstance) {
  WNDCLASSEX wcex;

  wcex.cbSize = sizeof(WNDCLASSEX);

  wcex.style		= CS_HREDRAW | CS_VREDRAW;
  wcex.lpfnWndProc	= WndProc;
  wcex.cbClsExtra	= 0;
  wcex.cbWndExtra	= 0;
  wcex.hInstance	= hInstance;
  wcex.hIcon		= 0;
  wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
  wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
  wcex.lpszMenuName	= 0;
  wcex.lpszClassName	= L"PowerWin";
  wcex.hIconSm		= 0;

  return RegisterClassEx(&wcex);
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
#ifdef ENV32BIT
  print(L"app_entry 32bit %d\n", GetCurrentThreadId());
#else
  print(L"app_entry 64bit %d\n", GetCurrentThreadId());
#endif

  PowerWin powerwin;
  instance_ = &powerwin;

  RegisterWinClass(Windows::Application::getInstance());

  powerwin.window_ = CreateWindowW(L"PowerWin",
                           L"PowerWin",
                           WS_OVERLAPPEDWINDOW,
                           CW_USEDEFAULT,
                           0,
                           CW_USEDEFAULT,
                           0,
                           NULL,
                           NULL,
                           Windows::Application::getInstance(),
                           NULL);

  // main message loop
  MSG msg;
  while (GetMessage(&msg, NULL, 0, 0)) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }

  print(L"the end\n");

  return 0;
}

bool StartProgram(cpp::wstring_ref exe_path, std::wstring args) {
  STARTUPINFO si;  
  PROCESS_INFORMATION pi;
  
  std::memset(&si, sizeof(si), 0);
  std::memset(&pi, sizeof(pi), 0);
  
  si.cb = sizeof(si);
  
  BOOL success = CreateProcess(exe_path.begin(), &(*args.begin()), nullptr,
	                             nullptr, false, 0, nullptr, nullptr, &si, &pi);
	if (success) {
	  CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
	}
	return success;
} 

bool RunDll64Bit(cpp::wstring_ref dll_name, cpp::wstring_ref entry, cpp::wstring_ref cmdln_args)
{
  std::wstring entry_args;
  entry_args += dll_name;
  entry_args += L',';
  entry_args += entry;
  entry_args += L' ';
  entry_args += cmdln_args;

  print(L"%ls\n", entry_args.c_str());
  
  return StartProgram(L"C:\\Windows\\Sysnative\\rundll32.exe", std::move(entry_args));
}

void PowerWin::start(HWND hwnd) {
  window_ = hwnd;

  print(L"PowerWin::start\n");

  ConfigFile config;
  config.loadFromFile(Windows::Application::getExecutablePath() + L"\\config.ini");

  for (auto& plugin : plugins_) {
    Plugin::Options opts;
    for (const std::wstring& key : config.getKeys(plugin->getName())) {
      opts[key] = config.getString(plugin->getName(), key.c_str(), nullptr);
    }
    plugin->setOptions(std::move(opts));

    if (plugin->getBooleanOption(L"active", true)) {
      print(L"activate plugin (0x%p) %s\n", plugin.get(), plugin->getName());
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
    print(L"start 64-dll\n");
    RunDll64Bit(L"libpower64.dll", L"KeepTheCarRunning", L"");
  }
#endif
}

void PowerWin::onDestroy() {
  try {
    for (auto& plugin : plugins_) {
      print(L"deactivate plugin %s\n", plugin->getName());
      plugin->deactivate();
    }
  } catch (...) {
    MessageBox(NULL, L"Ups", L"!!!", MB_ICONERROR | MB_OK);
  }

  tray_icon_.remove();

  PostQuitMessage(0);
}

LRESULT CALLBACK PowerWin::WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
  switch (msg) {
  case WM_CREATE:
    PowerWin::get()->start(hwnd);
    break;

  case WM_DESTROY:
    PowerWin::get()->onDestroy();
    break;

  default:
    return DefWindowProc(hwnd, msg, wparam, lparam);
  }
  return 0;
}

} // namespace

extern "C" {

HINSTANCE win_getDllInstance() {
  if (dllinstance_ == 0) {
    print(L"too early access of dll instance!");
  }

  return dllinstance_;
}

void win_updateDllInstance(HINSTANCE instance) {
  if (dllinstance_ == NULL) {
    dllinstance_ = instance;
  }
}

HWND win_getMainWindow() {
  PowerWin* instance = PowerWin::get();
  if (instance == nullptr) {
    print(L"not in powerwin process!");
  }

  HWND window = instance->getWindow();
  if (window == nullptr) {
    print(L"too early access of main window!");
  }

  return window;
}

void CALLBACK KeepTheCarRunning(HINSTANCE hInstance,
                                HINSTANCE hPrevInstance,
                                LPSTR lpCmdLine,
                                int nCmdShow)
{
#ifdef ENV32BIT
  Windows::Application app(L"PowerWin32", hInstance);
#else
  Windows::Application app(L"PowerWin64", hInstance);
#endif
  app.run(PowerWin::run);
}

void win_destroy() {
  DestroyWindow(win_getMainWindow());
}

} // extern "C"

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

