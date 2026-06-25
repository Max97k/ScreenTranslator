#include "globalaction.h"
#include "debug.h"

#include <QApplication>
#include <array>

namespace service
{
QHash<QPair<quint32, quint32>, QAction *> GlobalAction::actions_;

void GlobalAction::init()
{
  qApp->installNativeEventFilter(new GlobalAction);
}

bool GlobalAction::makeGlobal(QAction *action)
{
  QKeySequence hotKey = action->shortcut();
  if (hotKey.isEmpty())
    return true;
  Qt::KeyboardModifiers allMods = Qt::ShiftModifier | Qt::ControlModifier |
                                  Qt::AltModifier | Qt::MetaModifier;
  Qt::Key key = hotKey.isEmpty() ? Qt::Key(0)
                                 : Qt::Key((hotKey[0] ^ allMods) & hotKey[0]);
  Qt::KeyboardModifiers mods = hotKey.isEmpty()
                                   ? Qt::KeyboardModifiers(0)
                                   : Qt::KeyboardModifiers(hotKey[0] & allMods);
  const quint32 nativeKey = nativeKeycode(key);
  const quint32 nativeMods = nativeModifiers(mods);
  const bool res = registerHotKey(nativeKey, nativeMods);
  if (res)
    actions_.insert(qMakePair(nativeKey, nativeMods), action);
  else
    LERROR() << "Failed to register global hotkey:" << LARG(hotKey.toString());
  return res;
}

bool GlobalAction::removeGlobal(QAction *action)
{
  QKeySequence hotKey = action->shortcut();
  if (hotKey.isEmpty())
    return true;
  Qt::KeyboardModifiers allMods = Qt::ShiftModifier | Qt::ControlModifier |
                                  Qt::AltModifier | Qt::MetaModifier;
  Qt::Key key = hotKey.isEmpty() ? Qt::Key(0)
                                 : Qt::Key((hotKey[0] ^ allMods) & hotKey[0]);
  Qt::KeyboardModifiers mods = hotKey.isEmpty()
                                   ? Qt::KeyboardModifiers(0)
                                   : Qt::KeyboardModifiers(hotKey[0] & allMods);
  const quint32 nativeKey = nativeKeycode(key);
  const quint32 nativeMods = nativeModifiers(mods);
  if (!actions_.contains(qMakePair(nativeKey, nativeMods)))
    return true;
  const bool res = unregisterHotKey(nativeKey, nativeMods);
  if (res)
    actions_.remove(qMakePair(nativeKey, nativeMods));
  else
    LERROR() << "Failed to unregister global hotkey:" << (hotKey.toString());
  return res;
}

bool GlobalAction::update(QAction *action, const QKeySequence &newShortcut)
{
  if (!action->shortcut().isEmpty())
    removeGlobal(action);
  action->setShortcut(newShortcut);
  return newShortcut.isEmpty() ? true : makeGlobal(action);
}

void GlobalAction::triggerHotKey(quint32 nativeKey, quint32 nativeMods)
{
  QAction *action = actions_.value(qMakePair(nativeKey, nativeMods));
  if (action && action->isEnabled())
    action->activate(QAction::Trigger);
}
}  // namespace service

#ifdef Q_OS_LINUX
#include <X11/Xlib.h>
#include <xcb/xcb_event.h>
#include <QX11Info>

namespace service
{
static bool error = false;

static int customHandler(Display *display, XErrorEvent *event)
{
  Q_UNUSED(display);
  switch (event->error_code) {
    case BadAccess:
    case BadValue:
    case BadWindow:
      if (event->request_code == 33 /* X_GrabKey */ ||
          event->request_code == 34 /* X_UngrabKey */) {
        error = true;
      }
      [[fallthrough]];
    default: return 0;
  }
}

bool GlobalAction::registerHotKey(quint32 nativeKey, quint32 nativeMods)
{
  Display *display = QX11Info::display();
  Window window = QX11Info::appRootWindow();
  Bool owner = True;
  int pointer = GrabModeAsync;
  int keyboard = GrabModeAsync;
  error = false;
  int (*handler)(Display * display, XErrorEvent * event) =
      XSetErrorHandler(customHandler);
  XGrabKey(display, nativeKey, nativeMods, window, owner, pointer, keyboard);
  // allow numlock
  XGrabKey(display, nativeKey, nativeMods | Mod2Mask, window, owner, pointer,
           keyboard);
  XSync(display, False);
  XSetErrorHandler(handler);
  return !error;
}

bool GlobalAction::unregisterHotKey(quint32 nativeKey, quint32 nativeMods)
{
  Display *display = QX11Info::display();
  Window window = QX11Info::appRootWindow();
  error = false;
  int (*handler)(Display * display, XErrorEvent * event) =
      XSetErrorHandler(customHandler);
  XUngrabKey(display, nativeKey, nativeMods, window);
  // allow numlock
  XUngrabKey(display, nativeKey, nativeMods | Mod2Mask, window);
  XSync(display, False);
  XSetErrorHandler(handler);
  return !error;
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
bool GlobalAction::nativeEventFilter(const QByteArray &eventType, void *message,
                                     qintptr *result)
#else
bool GlobalAction::nativeEventFilter(const QByteArray &eventType, void *message,
                                     long *result)
#endif
{
  Q_UNUSED(eventType);
  Q_UNUSED(result);
  xcb_generic_event_t *event = static_cast<xcb_generic_event_t *>(message);
  if (event->response_type == XCB_KEY_PRESS) {
    xcb_key_press_event_t *keyEvent =
        static_cast<xcb_key_press_event_t *>(message);
    const quint32 keycode = keyEvent->detail;
    const quint32 modifiers = keyEvent->state & ~XCB_MOD_MASK_2;
    triggerHotKey(keycode, modifiers);
  }
  return false;
}

quint32 GlobalAction::nativeKeycode(Qt::Key key)
{
  Display *display = QX11Info::display();
  KeySym keySym = XStringToKeysym(qPrintable(QKeySequence(key).toString()));
  if (XKeysymToString(keySym) == nullptr) {
    keySym = QChar(key).unicode();
  }
  return XKeysymToKeycode(display, keySym);
}

quint32 GlobalAction::nativeModifiers(Qt::KeyboardModifiers modifiers)
{
  quint32 native = 0;
  if (modifiers & Qt::ShiftModifier)
    native |= ShiftMask;
  if (modifiers & Qt::ControlModifier)
    native |= ControlMask;
  if (modifiers & Qt::AltModifier)
    native |= Mod1Mask;
  if (modifiers & Qt::MetaModifier)
    native |= Mod4Mask;
  return native;
}

#endif  // ifdef Q_OS_LINUX

#ifdef Q_OS_WIN
#include <qt_windows.h>

namespace service
{
bool GlobalAction::registerHotKey(quint32 nativeKey, quint32 nativeMods)
{
  return RegisterHotKey(0, nativeMods ^ nativeKey, nativeMods, nativeKey);
}

bool GlobalAction::unregisterHotKey(quint32 nativeKey, quint32 nativeMods)
{
  return UnregisterHotKey(0, nativeMods ^ nativeKey);
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
bool GlobalAction::nativeEventFilter(const QByteArray &eventType, void *message,
                                     qintptr *result)
#else
bool GlobalAction::nativeEventFilter(const QByteArray &eventType, void *message,
                                     long *result)
#endif
{
  Q_UNUSED(eventType);
  Q_UNUSED(result);
  MSG *msg = static_cast<MSG *>(message);
  if (msg->message == WM_HOTKEY) {
    const quint32 keycode = HIWORD(msg->lParam);
    const quint32 modifiers = LOWORD(msg->lParam);
    triggerHotKey(keycode, modifiers);
  }
  return false;
}

quint32 GlobalAction::nativeKeycode(Qt::Key key)
{
  if ((key >= Qt::Key_0 && key <= Qt::Key_9) ||
      (key >= Qt::Key_A && key <= Qt::Key_Z)) {
    return key;
  }

  static constexpr std::pair<Qt::Key, quint32> keyMap[] = {
      {Qt::Key_Escape, VK_ESCAPE},
      {Qt::Key_Tab, VK_TAB},
      {Qt::Key_Backtab, VK_TAB},
      {Qt::Key_Backspace, VK_BACK},
      {Qt::Key_Return, VK_RETURN},
      {Qt::Key_Enter, VK_RETURN},
      {Qt::Key_Insert, VK_INSERT},
      {Qt::Key_Delete, VK_DELETE},
      {Qt::Key_Pause, VK_PAUSE},
      {Qt::Key_Print, VK_PRINT},
      {Qt::Key_Clear, VK_CLEAR},
      {Qt::Key_Home, VK_HOME},
      {Qt::Key_End, VK_END},
      {Qt::Key_Left, VK_LEFT},
      {Qt::Key_Up, VK_UP},
      {Qt::Key_Right, VK_RIGHT},
      {Qt::Key_Down, VK_DOWN},
      {Qt::Key_PageUp, VK_PRIOR},
      {Qt::Key_PageDown, VK_NEXT},
      {Qt::Key_CapsLock, VK_CAPITAL},
      {Qt::Key_NumLock, VK_NUMLOCK},
      {Qt::Key_ScrollLock, VK_SCROLL},
      {Qt::Key_F1, VK_F1},
      {Qt::Key_F2, VK_F2},
      {Qt::Key_F3, VK_F3},
      {Qt::Key_F4, VK_F4},
      {Qt::Key_F5, VK_F5},
      {Qt::Key_F6, VK_F6},
      {Qt::Key_F7, VK_F7},
      {Qt::Key_F8, VK_F8},
      {Qt::Key_F9, VK_F9},
      {Qt::Key_F10, VK_F10},
      {Qt::Key_F11, VK_F11},
      {Qt::Key_F12, VK_F12},
      {Qt::Key_F13, VK_F13},
      {Qt::Key_F14, VK_F14},
      {Qt::Key_F15, VK_F15},
      {Qt::Key_F16, VK_F16},
      {Qt::Key_F17, VK_F17},
      {Qt::Key_F18, VK_F18},
      {Qt::Key_F19, VK_F19},
      {Qt::Key_F20, VK_F20},
      {Qt::Key_F21, VK_F21},
      {Qt::Key_F22, VK_F22},
      {Qt::Key_F23, VK_F23},
      {Qt::Key_F24, VK_F24},
      {Qt::Key_Space, VK_SPACE},
      {Qt::Key_QuoteDbl, VK_OEM_7},
      {Qt::Key_Apostrophe, VK_OEM_7},
      {Qt::Key_Period, VK_DECIMAL},
      {Qt::Key_Colon, VK_OEM_1},
      {Qt::Key_Semicolon, VK_OEM_1},
      {Qt::Key_Less, VK_OEM_COMMA},
      {Qt::Key_Greater, VK_OEM_PERIOD},
      {Qt::Key_Question, VK_OEM_2},
      {Qt::Key_BracketLeft, VK_OEM_4},
      {Qt::Key_Backslash, VK_OEM_5},
      {Qt::Key_BracketRight, VK_OEM_6},
      {Qt::Key_QuoteLeft, VK_OEM_3},
      {Qt::Key_BraceLeft, VK_OEM_4},
      {Qt::Key_Bar, VK_OEM_5},
      {Qt::Key_BraceRight, VK_OEM_6},
      {Qt::Key_Asterisk, VK_MULTIPLY},
      {Qt::Key_Plus, VK_OEM_PLUS},
      {Qt::Key_Comma, VK_OEM_COMMA},
      {Qt::Key_Minus, VK_OEM_MINUS},
      {Qt::Key_Slash, VK_OEM_2},
      {Qt::Key_MediaNext, VK_MEDIA_NEXT_TRACK},
      {Qt::Key_MediaPrevious, VK_MEDIA_PREV_TRACK},
      {Qt::Key_MediaPlay, VK_MEDIA_PLAY_PAUSE},
      {Qt::Key_MediaStop, VK_MEDIA_STOP},
      {Qt::Key_VolumeDown, VK_VOLUME_DOWN},
      {Qt::Key_VolumeUp, VK_VOLUME_UP},
      {Qt::Key_VolumeMute, VK_VOLUME_MUTE},
  };

  for (const auto& pair : keyMap) {
    if (pair.first == key) return pair.second;
  }
  return 0;
}

quint32 GlobalAction::nativeModifiers(Qt::KeyboardModifiers modifiers)
{
  // MOD_ALT, MOD_CONTROL, (MOD_KEYUP), MOD_SHIFT, MOD_WIN
  quint32 native = 0;
  if (modifiers & Qt::ShiftModifier)
    native |= MOD_SHIFT;
  if (modifiers & Qt::ControlModifier)
    native |= MOD_CONTROL;
  if (modifiers & Qt::AltModifier)
    native |= MOD_ALT;
  if (modifiers & Qt::MetaModifier)
    native |= MOD_WIN;
  // if (modifiers & Qt::KeypadModifier)
  // if (modifiers & Qt::GroupSwitchModifier)
  return native;
}

#endif  // ifdef Q_OS_WIN

#ifdef Q_OS_MAC
#include <Carbon/Carbon.h>

namespace service
{
static bool isInited = false;
static QHash<QPair<quint32, quint32>, EventHotKeyRef> hotkeyRefs;

struct ActionAdapter {
  static OSStatus macHandler(EventHandlerCallRef /*nextHandler*/,
                             EventRef event, void * /*userData*/)
  {
    EventHotKeyID id;
    GetEventParameter(event, kEventParamDirectObject, typeEventHotKeyID, NULL,
                      sizeof(id), NULL, &id);
    GlobalAction::triggerHotKey(quint32(id.signature), quint32(id.id));
    return noErr;
  }
};

bool GlobalAction::registerHotKey(quint32 nativeKey, quint32 nativeMods)
{
  if (!isInited) {
    EventTypeSpec spec;
    spec.eventClass = kEventClassKeyboard;
    spec.eventKind = kEventHotKeyPressed;
    InstallApplicationEventHandler(&ActionAdapter::macHandler, 1, &spec, NULL,
                                   NULL);
    isInited = true;
  }

  EventHotKeyID id;
  id.signature = nativeKey;
  id.id = nativeMods;

  EventHotKeyRef ref = NULL;
  OSStatus status = RegisterEventHotKey(nativeKey, nativeMods, id,
                                        GetApplicationEventTarget(), 0, &ref);
  if (status != noErr) {
    LERROR() << "RegisterEventHotKey error:" << LARG(status);
    return false;
  } else {
    hotkeyRefs.insert(qMakePair(nativeKey, nativeMods), ref);
    return true;
  }
}

bool GlobalAction::unregisterHotKey(quint32 nativeKey, quint32 nativeMods)
{
  EventHotKeyRef ref = hotkeyRefs.value(qMakePair(nativeKey, nativeMods));
  ASSERT(ref);
  OSStatus status = UnregisterEventHotKey(ref);
  if (status != noErr) {
    LERROR() << "UnregisterEventHotKey error:" << LARG(status);
    return false;
  } else {
    hotkeyRefs.remove(qMakePair(nativeKey, nativeMods));
    return true;
  }
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
bool GlobalAction::nativeEventFilter(const QByteArray & /*eventType*/,
                                     void * /*message*/, qintptr * /*result*/)
#else
bool GlobalAction::nativeEventFilter(const QByteArray & /*eventType*/,
                                     void * /*message*/, long * /*result*/)
#endif
{
  return false;
}

quint32 GlobalAction::nativeKeycode(Qt::Key key)
{
  static constexpr std::pair<Qt::Key, quint32> keyMap[] = {
      {Qt::Key_A, kVK_ANSI_A},
      {Qt::Key_B, kVK_ANSI_B},
      {Qt::Key_C, kVK_ANSI_C},
      {Qt::Key_D, kVK_ANSI_D},
      {Qt::Key_E, kVK_ANSI_E},
      {Qt::Key_F, kVK_ANSI_F},
      {Qt::Key_G, kVK_ANSI_G},
      {Qt::Key_H, kVK_ANSI_H},
      {Qt::Key_I, kVK_ANSI_I},
      {Qt::Key_J, kVK_ANSI_J},
      {Qt::Key_K, kVK_ANSI_K},
      {Qt::Key_L, kVK_ANSI_L},
      {Qt::Key_M, kVK_ANSI_M},
      {Qt::Key_N, kVK_ANSI_N},
      {Qt::Key_O, kVK_ANSI_O},
      {Qt::Key_P, kVK_ANSI_P},
      {Qt::Key_Q, kVK_ANSI_Q},
      {Qt::Key_R, kVK_ANSI_R},
      {Qt::Key_S, kVK_ANSI_S},
      {Qt::Key_T, kVK_ANSI_T},
      {Qt::Key_U, kVK_ANSI_U},
      {Qt::Key_V, kVK_ANSI_V},
      {Qt::Key_W, kVK_ANSI_W},
      {Qt::Key_X, kVK_ANSI_X},
      {Qt::Key_Y, kVK_ANSI_Y},
      {Qt::Key_Z, kVK_ANSI_Z},
      {Qt::Key_0, kVK_ANSI_0},
      {Qt::Key_1, kVK_ANSI_1},
      {Qt::Key_2, kVK_ANSI_2},
      {Qt::Key_3, kVK_ANSI_3},
      {Qt::Key_4, kVK_ANSI_4},
      {Qt::Key_5, kVK_ANSI_5},
      {Qt::Key_6, kVK_ANSI_6},
      {Qt::Key_7, kVK_ANSI_7},
      {Qt::Key_8, kVK_ANSI_8},
      {Qt::Key_9, kVK_ANSI_9},
      {Qt::Key_F1, kVK_F1},
      {Qt::Key_F2, kVK_F2},
      {Qt::Key_F3, kVK_F3},
      {Qt::Key_F4, kVK_F4},
      {Qt::Key_F5, kVK_F5},
      {Qt::Key_F6, kVK_F6},
      {Qt::Key_F7, kVK_F7},
      {Qt::Key_F8, kVK_F8},
      {Qt::Key_F9, kVK_F9},
      {Qt::Key_F10, kVK_F10},
      {Qt::Key_F11, kVK_F11},
      {Qt::Key_F12, kVK_F12},
      {Qt::Key_F13, kVK_F13},
      {Qt::Key_F14, kVK_F14},
      {Qt::Key_F15, kVK_F15},
      {Qt::Key_F16, kVK_F16},
      {Qt::Key_F17, kVK_F17},
      {Qt::Key_F18, kVK_F18},
      {Qt::Key_F19, kVK_F19},
      {Qt::Key_F20, kVK_F10},
      {Qt::Key_Return, kVK_Return},
      {Qt::Key_Enter, kVK_ANSI_KeypadEnter},
      {Qt::Key_Tab, kVK_Tab},
      {Qt::Key_Space, kVK_Space},
      {Qt::Key_Backspace, kVK_Delete},
      {Qt::Key_Escape, kVK_Escape},
      {Qt::Key_CapsLock, kVK_CapsLock},
      {Qt::Key_Option, kVK_Option},
      {Qt::Key_VolumeUp, kVK_VolumeUp},
      {Qt::Key_VolumeDown, kVK_VolumeDown},
      {Qt::Key_Help, kVK_Help},
      {Qt::Key_Home, kVK_Home},
      {Qt::Key_PageUp, kVK_PageUp},
      {Qt::Key_Delete, kVK_ForwardDelete},
      {Qt::Key_End, kVK_End},
      {Qt::Key_PageDown, kVK_PageDown},
      {Qt::Key_Left, kVK_LeftArrow},
      {Qt::Key_Right, kVK_RightArrow},
      {Qt::Key_Down, kVK_DownArrow},
      {Qt::Key_Up, kVK_UpArrow},
  };

  for (const auto& pair : keyMap) {
    if (pair.first == key) return pair.second;
  }
  return 0;
}

quint32 GlobalAction::nativeModifiers(Qt::KeyboardModifiers modifiers)
{
  quint32 native = 0;
  if (modifiers & Qt::ShiftModifier)
    native |= shiftKey;
  if (modifiers & Qt::ControlModifier)
    native |= cmdKey;
  if (modifiers & Qt::AltModifier)
    native |= optionKey;
  if (modifiers & Qt::MetaModifier)
    native |= controlKey;
  return native;
}

#endif  // ifdef Q_OS_MAC

}  // namespace service
