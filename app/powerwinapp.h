#pragma once

#include <vector>

#include <lightports/controls/messagesink.h>
#include <lightports/extra/trayicon.h>
#include <lightports/extra/menu.h>
#include <windows.h>

#include "module.h"
#include "modulemanager.h"
#include "hotkeymanager.h"
#include "configuration.h"
#include "hooklibmanager.h"
#include "globalevents.h"

namespace PowerWin {

class PowerWinApp : public Windows::MessageSink {
  PowerWinApp();
  ~PowerWinApp();

public:
  static int run();

  HWND getWindow() {
    return getNativeHandle();
  }

private:
  enum
  {
    InfoMenu,
    InfoEntry,

    AutostartEntry,
    QuitEntry
  };

  Windows::TrayIcon tray_icon_;
  Windows::Menu popup_menu_;
  Windows::Menu info_menu_;

  Configuration configuration_;
  HotkeyManager hotkeys_;
  GlobalEvents global_events_;

  ModuleManager modules_;
  HookLibManager hooklibs_;

  Hotkey quit_shortcut_;

  static ATOM RegisterWinClass(HINSTANCE hInstance);

  LRESULT onMessage(UINT msg, WPARAM wparam, LPARAM lparam) override;
  void onCreate() override;
  void onDestroy() override;

  void onContextMenu(Windows::Point pt);
  void onAutostartSet(bool value);
};

} // namespace PowerWin

