///
/// Copyright (c) 2016 R1tschY
/// 
/// Permission is hereby granted, free of charge, to any person obtaining a copy
/// of this software and associated documentation files (the "Software"), to 
/// deal in the Software without restriction, including without limitation the 
/// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or 
/// sell copies of the Software, and to permit persons to whom the Software is
/// furnished to do so, subject to the following conditions:
/// 
/// The above copyright notice and this permission notice shall be included in
/// all copies or substantial portions of the Software.
/// 
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
/// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
/// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
/// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
/// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
/// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
/// IN THE SOFTWARE.
///

#include "wkill.h"

#include <windows.h>
#include <QLabel>
#include <QHBoxLayout>
#include <QDebug>
#include <QMouseEvent>
#include <QGraphicsDropShadowEffect>
#include <QFont>
#include <QKeyEvent>
#include <QCoreApplication>

#include <app/configuration.h>
#include <app/hotkeymanager.h>
#include <app/hooklibmanager.h>
#include <lightports/os/process.h>

using namespace Windows;

namespace PowerWin {

WKillWindow::WKillWindow()
{
  resize(300, 150);
  // TODO: resize to text size
  // TODO: place to screen center

  // TODO: create overlay base class

  setWindowFlags(Qt::Tool
    | Qt::WindowCloseButtonHint
    | Qt::FramelessWindowHint);
  setAttribute(Qt::WA_TranslucentBackground);
  auto* effect = new QGraphicsDropShadowEffect();
  effect->setBlurRadius(10);
  effect->setColor(QColor(0, 0, 0, 0xcc));
  effect->setXOffset(3.0);
  effect->setYOffset(3.0);
  setGraphicsEffect(effect);

  // frame

  auto* frame = new QFrame(this);
  frame->setFrameShape(QFrame::NoFrame);
  frame->setObjectName("overlayWindowFrame");

  auto* frameLayout = new QVBoxLayout(this);
  frameLayout->addWidget(frame);
  frameLayout->setMargin(0);
  setLayout(frameLayout);

  // content

  auto* layout = new QHBoxLayout();
  frame->setLayout(layout);
  activateButton_ = new QLabel(tr("Drag on Window"), this);
  QFont font = activateButton_->font();
  font.setPointSize(24);
  activateButton_->setFont(font);

  layout->addWidget(activateButton_);

  // style

  setWindowOpacity(0.75);
  setStyleSheet(""
    "#overlayWindowFrame {"
    "  background-color: #ccc; "
    "  border-radius: 10px;"
    "  padding: 10px;"
    "  padding-top: 0px;"
    "  margin: 10px;"
    "}");
}

void WKillWindow::mousePressEvent(QMouseEvent* event)
{
  if (event->buttons() != Qt::LeftButton)
  {
    // exit through other mouse click
    choosing_ = false;
    setCursor(Qt::ArrowCursor);
    return;
  }

  setCursor(Qt::PointingHandCursor);
  choosing_ = true;
}

void WKillWindow::mouseReleaseEvent(QMouseEvent* event)
{
  if (!choosing_)
    return;

  cancelChoosing();

  try
  {
    // kill process of clicked window
    auto hwnd = Window::at(Point(event->globalX(), event->globalY()));
    if (!hwnd)
    {
      qCritical() << "wkill: no window at "
          << event->globalX() << "/" << event->globalY();
      return;
    }
    if (hwnd == ::GetShellWindow() || hwnd == ::GetDesktopWindow())
      return;

    auto pid = hwnd.getProcessId();
    if (QCoreApplication::applicationPid() == pid)
      return;

    auto process = Process::open(Process::AccessRights::Terminate, pid);

    qInfo() << "wkill: terminate HWND:" << hwnd.getHWND() << "PID:" << pid;

    // TODO: do not kill own process, desktop process, ...
    process.terminate(1);
  }
  catch(const Exception& exp)
  {
    qCritical() << "wkill: Terminate process failed:" << exp.what();
  }
}

void WKillWindow::keyPressEvent(QKeyEvent* event)
{
  switch (event->key())
  {
  case Qt::Key_Escape:
    cancelChoosing();
    return;
  }
}

void WKillWindow::cancelChoosing()
{
  setCursor(Qt::ArrowCursor);
  hide();
  choosing_ = false;
}

WKill::WKill(ModuleContext& context)
: hotkey_(context.getHotkeyManager())
{
  hotkey_.setCallback([=](){ onHotkey(); });
  hotkey_.setKey(context
    .getConfiguration()
    .readValue(L"wkill", L"hotkey", L"Ctrl+Alt+X"));
}

void WKill::onHotkey()
{
  window_.show();
  window_.activateWindow();
  window_.raise();
}

ModuleRegistry::element<WKill> XWKill(
  L"wkill", L"kill process with click on window (xkill clone)"
);

} // namespace PowerWin
