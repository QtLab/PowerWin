#pragma once

#include <windows.h>
#include <functional>
#include <map>

#include "macros.h"
#include "controls/messagesink.h"

namespace Windows {

struct ShortCut {
  UINT modifiers;
  UINT key;
};

typedef std::function<void()> HotkeyHandler;

class HotkeyManager {
  HotkeyManager();

public:
  HotkeyManager& get();

  static void registerHotkey() { get().registerHotkey(); }
  void registerHotkey(ShortCut shortcut, HotkeyHandler handler);
  static void unregisterHotkey() { get().unregisterHotkey(); }
  void unregisterHotkey(const Hotkey& hotkey);

private:
  MessageSink message_sink_;

  std::map<unsigned, HotkeyHandler> handlers_;
  unsigned counter_;

  LRESULT HotkeyManager::WndProc(UINT msg, WPARAM wparam, LPARAM lparam);
};

class Hotkey {
  DISALLOW_COPY_AND_ASSIGN(Hotkey);

public:
  Hotkey(unsigned fsModifiers, unsigned vk, HotkeyHandler handler);
  Hotkey(Hotkey&& other) noexcept;
  ~Hotkey();

  bool activate();
  bool deactivate();

  bool isActive() const { return active_; }

private:
  const unsigned modifiers_;
  const unsigned vk_;
  const HotkeyHandler handler_;
  const unsigned id_;

  bool active_;

  friend class HotkeyManager;
};

} // namespace Windows

