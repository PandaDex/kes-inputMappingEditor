#include "KeyCatalog.h"

#include <QKeyEvent>

#if defined(Q_OS_WIN)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

namespace imap
{
    namespace KeyCatalog
    {

        const QVector<KeyInfo>& all()
        {
            static const QVector<KeyInfo> keys = {
                // Letters
                { "Keyboard_A", "A", KeyCategory::Keyboard, false, Qt::Key_A },
                { "Keyboard_B", "B", KeyCategory::Keyboard, false, Qt::Key_B },
                { "Keyboard_C", "C", KeyCategory::Keyboard, false, Qt::Key_C },
                { "Keyboard_D", "D", KeyCategory::Keyboard, false, Qt::Key_D },
                { "Keyboard_E", "E", KeyCategory::Keyboard, false, Qt::Key_E },
                { "Keyboard_F", "F", KeyCategory::Keyboard, false, Qt::Key_F },
                { "Keyboard_G", "G", KeyCategory::Keyboard, false, Qt::Key_G },
                { "Keyboard_H", "H", KeyCategory::Keyboard, false, Qt::Key_H },
                { "Keyboard_I", "I", KeyCategory::Keyboard, false, Qt::Key_I },
                { "Keyboard_J", "J", KeyCategory::Keyboard, false, Qt::Key_J },
                { "Keyboard_K", "K", KeyCategory::Keyboard, false, Qt::Key_K },
                { "Keyboard_L", "L", KeyCategory::Keyboard, false, Qt::Key_L },
                { "Keyboard_M", "M", KeyCategory::Keyboard, false, Qt::Key_M },
                { "Keyboard_N", "N", KeyCategory::Keyboard, false, Qt::Key_N },
                { "Keyboard_O", "O", KeyCategory::Keyboard, false, Qt::Key_O },
                { "Keyboard_P", "P", KeyCategory::Keyboard, false, Qt::Key_P },
                { "Keyboard_Q", "Q", KeyCategory::Keyboard, false, Qt::Key_Q },
                { "Keyboard_R", "R", KeyCategory::Keyboard, false, Qt::Key_R },
                { "Keyboard_S", "S", KeyCategory::Keyboard, false, Qt::Key_S },
                { "Keyboard_T", "T", KeyCategory::Keyboard, false, Qt::Key_T },
                { "Keyboard_U", "U", KeyCategory::Keyboard, false, Qt::Key_U },
                { "Keyboard_V", "V", KeyCategory::Keyboard, false, Qt::Key_V },
                { "Keyboard_W", "W", KeyCategory::Keyboard, false, Qt::Key_W },
                { "Keyboard_X", "X", KeyCategory::Keyboard, false, Qt::Key_X },
                { "Keyboard_Y", "Y", KeyCategory::Keyboard, false, Qt::Key_Y },
                { "Keyboard_Z", "Z", KeyCategory::Keyboard, false, Qt::Key_Z },

                // Number row
                { "Keyboard_0", "0", KeyCategory::Keyboard, false, Qt::Key_0 },
                { "Keyboard_1", "1", KeyCategory::Keyboard, false, Qt::Key_1 },
                { "Keyboard_2", "2", KeyCategory::Keyboard, false, Qt::Key_2 },
                { "Keyboard_3", "3", KeyCategory::Keyboard, false, Qt::Key_3 },
                { "Keyboard_4", "4", KeyCategory::Keyboard, false, Qt::Key_4 },
                { "Keyboard_5", "5", KeyCategory::Keyboard, false, Qt::Key_5 },
                { "Keyboard_6", "6", KeyCategory::Keyboard, false, Qt::Key_6 },
                { "Keyboard_7", "7", KeyCategory::Keyboard, false, Qt::Key_7 },
                { "Keyboard_8", "8", KeyCategory::Keyboard, false, Qt::Key_8 },
                { "Keyboard_9", "9", KeyCategory::Keyboard, false, Qt::Key_9 },

                // Function keys
                { "Keyboard_F1", "F1", KeyCategory::Keyboard, false, Qt::Key_F1 },
                { "Keyboard_F2", "F2", KeyCategory::Keyboard, false, Qt::Key_F2 },
                { "Keyboard_F3", "F3", KeyCategory::Keyboard, false, Qt::Key_F3 },
                { "Keyboard_F4", "F4", KeyCategory::Keyboard, false, Qt::Key_F4 },
                { "Keyboard_F5", "F5", KeyCategory::Keyboard, false, Qt::Key_F5 },
                { "Keyboard_F6", "F6", KeyCategory::Keyboard, false, Qt::Key_F6 },
                { "Keyboard_F7", "F7", KeyCategory::Keyboard, false, Qt::Key_F7 },
                { "Keyboard_F8", "F8", KeyCategory::Keyboard, false, Qt::Key_F8 },
                { "Keyboard_F9", "F9", KeyCategory::Keyboard, false, Qt::Key_F9 },
                { "Keyboard_F10", "F10", KeyCategory::Keyboard, false, Qt::Key_F10 },
                { "Keyboard_F11", "F11", KeyCategory::Keyboard, false, Qt::Key_F11 },
                { "Keyboard_F12", "F12", KeyCategory::Keyboard, false, Qt::Key_F12 },
                { "Keyboard_F13", "F13", KeyCategory::Keyboard, false, Qt::Key_F13 },
                { "Keyboard_F14", "F14", KeyCategory::Keyboard, false, Qt::Key_F14 },
                { "Keyboard_F15", "F15", KeyCategory::Keyboard, false, Qt::Key_F15 },
                { "Keyboard_F16", "F16", KeyCategory::Keyboard, false, Qt::Key_F16 },
                { "Keyboard_F17", "F17", KeyCategory::Keyboard, false, Qt::Key_F17 },
                { "Keyboard_F18", "F18", KeyCategory::Keyboard, false, Qt::Key_F18 },
                { "Keyboard_F19", "F19", KeyCategory::Keyboard, false, Qt::Key_F19 },
                { "Keyboard_F20", "F20", KeyCategory::Keyboard, false, Qt::Key_F20 },
                { "Keyboard_F21", "F21", KeyCategory::Keyboard, false, Qt::Key_F21 },
                { "Keyboard_F22", "F22", KeyCategory::Keyboard, false, Qt::Key_F22 },
                { "Keyboard_F23", "F23", KeyCategory::Keyboard, false, Qt::Key_F23 },
                { "Keyboard_F24", "F24", KeyCategory::Keyboard, false, Qt::Key_F24 },

                // Modifiers
                { "Keyboard_LeftShift", "Left Shift", KeyCategory::Keyboard, false, Qt::Key_Shift },
                { "Keyboard_RightShift", "Right Shift", KeyCategory::Keyboard, false, Qt::Key_Shift },
                { "Keyboard_LeftControl", "Left Ctrl", KeyCategory::Keyboard, false, Qt::Key_Control },
                { "Keyboard_RightControl", "Right Ctrl", KeyCategory::Keyboard, false, Qt::Key_Control },
                { "Keyboard_LeftAlt", "Left Alt", KeyCategory::Keyboard, false, Qt::Key_Alt },
                { "Keyboard_RightAlt", "Right Alt", KeyCategory::Keyboard, false, Qt::Key_Alt },
                { "Keyboard_LeftMeta", "Left Meta", KeyCategory::Keyboard, false, Qt::Key_Meta },
                { "Keyboard_RightMeta", "Right Meta", KeyCategory::Keyboard, false, Qt::Key_Meta },

                // Main keyboard
                { "Keyboard_Escape", "Escape", KeyCategory::Keyboard, false, Qt::Key_Escape },
                { "Keyboard_Tab", "Tab", KeyCategory::Keyboard, false, Qt::Key_Tab },
                { "Keyboard_Backtab", "Backtab", KeyCategory::Keyboard, false, Qt::Key_Backtab },
                { "Keyboard_CapsLock", "Caps Lock", KeyCategory::Keyboard, false, Qt::Key_CapsLock },
                { "Keyboard_Backspace", "Backspace", KeyCategory::Keyboard, false, Qt::Key_Backspace },
                { "Keyboard_Return", "Return", KeyCategory::Keyboard, false, Qt::Key_Return },
                { "Keyboard_Enter", "Enter", KeyCategory::Keyboard, false, Qt::Key_Enter },
                { "Keyboard_Space", "Space Bar", KeyCategory::Keyboard, false, Qt::Key_Space },
                { "Keyboard_Insert", "Insert", KeyCategory::Keyboard, false, Qt::Key_Insert },
                { "Keyboard_Delete", "Delete", KeyCategory::Keyboard, false, Qt::Key_Delete },
                { "Keyboard_Home", "Home", KeyCategory::Keyboard, false, Qt::Key_Home },
                { "Keyboard_End", "End", KeyCategory::Keyboard, false, Qt::Key_End },
                { "Keyboard_PageUp", "Page Up", KeyCategory::Keyboard, false, Qt::Key_PageUp },
                { "Keyboard_PageDown", "Page Down", KeyCategory::Keyboard, false, Qt::Key_PageDown },

                // Arrow keys
                { "Keyboard_Left", "Left Arrow", KeyCategory::Keyboard, false, Qt::Key_Left },
                { "Keyboard_Right", "Right Arrow", KeyCategory::Keyboard, false, Qt::Key_Right },
                { "Keyboard_Up", "Up Arrow", KeyCategory::Keyboard, false, Qt::Key_Up },
                { "Keyboard_Down", "Down Arrow", KeyCategory::Keyboard, false, Qt::Key_Down },

                // Punctuation / symbols
                { "Keyboard_Exclam", "!", KeyCategory::Keyboard, false, Qt::Key_Exclam },
                { "Keyboard_QuoteDbl", "\"", KeyCategory::Keyboard, false, Qt::Key_QuoteDbl },
                { "Keyboard_NumberSign", "#", KeyCategory::Keyboard, false, Qt::Key_NumberSign },
                { "Keyboard_Dollar", "$", KeyCategory::Keyboard, false, Qt::Key_Dollar },
                { "Keyboard_Percent", "%", KeyCategory::Keyboard, false, Qt::Key_Percent },
                { "Keyboard_Ampersand", "&", KeyCategory::Keyboard, false, Qt::Key_Ampersand },
                { "Keyboard_Apostrophe", "'", KeyCategory::Keyboard, false, Qt::Key_Apostrophe },
                { "Keyboard_ParenLeft", "(", KeyCategory::Keyboard, false, Qt::Key_ParenLeft },
                { "Keyboard_ParenRight", ")", KeyCategory::Keyboard, false, Qt::Key_ParenRight },
                { "Keyboard_Asterisk", "*", KeyCategory::Keyboard, false, Qt::Key_Asterisk },
                { "Keyboard_Plus", "+", KeyCategory::Keyboard, false, Qt::Key_Plus },
                { "Keyboard_Comma", ",", KeyCategory::Keyboard, false, Qt::Key_Comma },
                { "Keyboard_Minus", "-", KeyCategory::Keyboard, false, Qt::Key_Minus },
                { "Keyboard_Period", ".", KeyCategory::Keyboard, false, Qt::Key_Period },
                { "Keyboard_Slash", "/", KeyCategory::Keyboard, false, Qt::Key_Slash },
                { "Keyboard_Colon", ":", KeyCategory::Keyboard, false, Qt::Key_Colon },
                { "Keyboard_Semicolon", ";", KeyCategory::Keyboard, false, Qt::Key_Semicolon },
                { "Keyboard_Less", "<", KeyCategory::Keyboard, false, Qt::Key_Less },
                { "Keyboard_Equal", "=", KeyCategory::Keyboard, false, Qt::Key_Equal },
                { "Keyboard_Greater", ">", KeyCategory::Keyboard, false, Qt::Key_Greater },
                { "Keyboard_Question", "?", KeyCategory::Keyboard, false, Qt::Key_Question },
                { "Keyboard_At", "@", KeyCategory::Keyboard, false, Qt::Key_At },
                { "Keyboard_BracketLeft", "[", KeyCategory::Keyboard, false, Qt::Key_BracketLeft },
                { "Keyboard_Backslash", "\\", KeyCategory::Keyboard, false, Qt::Key_Backslash },
                { "Keyboard_BracketRight", "]", KeyCategory::Keyboard, false, Qt::Key_BracketRight },
                { "Keyboard_AsciiCircum", "^", KeyCategory::Keyboard, false, Qt::Key_AsciiCircum },
                { "Keyboard_Underscore", "_", KeyCategory::Keyboard, false, Qt::Key_Underscore },
                { "Keyboard_QuoteLeft", "`", KeyCategory::Keyboard, false, Qt::Key_QuoteLeft },
                { "Keyboard_BraceLeft", "{", KeyCategory::Keyboard, false, Qt::Key_BraceLeft },
                { "Keyboard_Bar", "|", KeyCategory::Keyboard, false, Qt::Key_Bar },
                { "Keyboard_BraceRight", "}", KeyCategory::Keyboard, false, Qt::Key_BraceRight },
                { "Keyboard_AsciiTilde", "~", KeyCategory::Keyboard, false, Qt::Key_AsciiTilde },

                // Lock / system keys
                { "Keyboard_NumLock", "Num Lock", KeyCategory::Keyboard, false, Qt::Key_NumLock },
                { "Keyboard_ScrollLock", "Scroll Lock", KeyCategory::Keyboard, false, Qt::Key_ScrollLock },
                { "Keyboard_Print", "Print Screen", KeyCategory::Keyboard, false, Qt::Key_Print },
                { "Keyboard_SysReq", "SysReq", KeyCategory::Keyboard, false, Qt::Key_SysReq },
                { "Keyboard_Pause", "Pause", KeyCategory::Keyboard, false, Qt::Key_Pause },
                { "Keyboard_Menu", "Menu", KeyCategory::Keyboard, false, Qt::Key_Menu },
                { "Keyboard_Help", "Help", KeyCategory::Keyboard, false, Qt::Key_Help },

                // Keypad
                { "Keyboard_NumPad_0", "Numpad 0", KeyCategory::Keyboard, false, Qt::Key_0 },
                { "Keyboard_NumPad_1", "Numpad 1", KeyCategory::Keyboard, false, Qt::Key_1 },
                { "Keyboard_NumPad_2", "Numpad 2", KeyCategory::Keyboard, false, Qt::Key_2 },
                { "Keyboard_NumPad_3", "Numpad 3", KeyCategory::Keyboard, false, Qt::Key_3 },
                { "Keyboard_NumPad_4", "Numpad 4", KeyCategory::Keyboard, false, Qt::Key_4 },
                { "Keyboard_NumPad_5", "Numpad 5", KeyCategory::Keyboard, false, Qt::Key_5 },
                { "Keyboard_NumPad_6", "Numpad 6", KeyCategory::Keyboard, false, Qt::Key_6 },
                { "Keyboard_NumPad_7", "Numpad 7", KeyCategory::Keyboard, false, Qt::Key_7 },
                { "Keyboard_NumPad_8", "Numpad 8", KeyCategory::Keyboard, false, Qt::Key_8 },
                { "Keyboard_NumPad_9", "Numpad 9", KeyCategory::Keyboard, false, Qt::Key_9 },
                { "Keyboard_NumPad_Decimal", "Numpad Decimal", KeyCategory::Keyboard, false, Qt::Key_Period },
                { "Keyboard_NumPad_Divide", "Numpad Divide", KeyCategory::Keyboard, false, Qt::Key_Slash },
                { "Keyboard_NumPad_Multiply", "Numpad Multiply", KeyCategory::Keyboard, false, Qt::Key_Asterisk },
                { "Keyboard_NumPad_Subtract", "Numpad Subtract", KeyCategory::Keyboard, false, Qt::Key_Minus },
                { "Keyboard_NumPad_Add", "Numpad Add", KeyCategory::Keyboard, false, Qt::Key_Plus },
                { "Keyboard_NumPad_Enter", "Numpad Enter", KeyCategory::Keyboard, false, Qt::Key_Enter },

                { "Mouse_LeftButton", "Mouse Left Button", KeyCategory::Mouse, false, 0 },
                { "Mouse_RightButton", "Mouse Right Button", KeyCategory::Mouse, false, 0 },
                { "Mouse_MiddleButton", "Mouse Middle Button", KeyCategory::Mouse, false, 0 },
                { "Mouse_Button4", "Mouse Button 4", KeyCategory::Mouse, false, 0 },
                { "Mouse_Button5", "Mouse Button 5", KeyCategory::Mouse, false, 0 },

                { "Mouse_2D_Axis", "Mouse XY 2D-Axis", KeyCategory::Mouse, true, 0 },
                { "Mouse_X_Axis", "Mouse X-Axis", KeyCategory::Mouse, true, 0 },
                { "Mouse_Y_Axis", "Mouse Y-Axis", KeyCategory::Mouse, true, 0 },
                { "Mouse_Wheel", "Mouse Wheel Click", KeyCategory::Mouse, false, 0 },
                { "Mouse_Wheel_Y", "Mouse Wheel Y-Axis", KeyCategory::Mouse, true, 0 },

                // Thumbsticks
                { "Gamepad_LeftAxis_2D", "Gamepad Left Thumbstick 2D-Axis", KeyCategory::Gamepad, true, 0 },
                { "Gamepad_LeftAxis_X", "Gamepad Left Thumbstick X-Axis", KeyCategory::Gamepad, true, 0 },
                { "Gamepad_LeftAxis_Y", "Gamepad Left Thumbstick Y-Axis", KeyCategory::Gamepad, true, 0 },
                { "Gamepad_RightAxis_2D", "Gamepad Right Thumbstick 2D-Axis", KeyCategory::Gamepad, true, 0 },
                { "Gamepad_RightAxis_X", "Gamepad Right Thumbstick X-Axis", KeyCategory::Gamepad, true, 0 },
                { "Gamepad_RightAxis_Y", "Gamepad Right Thumbstick Y-Axis", KeyCategory::Gamepad, true, 0 },

                // Face buttons
                { "Gamepad_Face_Top", "Gamepad Face Button Top", KeyCategory::Gamepad, false, 0 },
                { "Gamepad_Face_Left", "Gamepad Face Button Left", KeyCategory::Gamepad, false, 0 },
                { "Gamepad_Face_Bottom", "Gamepad Face Button Bottom", KeyCategory::Gamepad, false, 0 },
                { "Gamepad_Face_Right", "Gamepad Face Button Right", KeyCategory::Gamepad, false, 0 },

                // Special buttons
                { "Gamepad_Special_Left", "Gamepad Special Left (Back/View)", KeyCategory::Gamepad, false, 0 },
                { "Gamepad_Special_Right", "Gamepad Special Right (Start/Menu)", KeyCategory::Gamepad, false, 0 },

                // Stick buttons
                { "Gamepad_LeftStick", "Gamepad Left Thumbstick Click", KeyCategory::Gamepad, false, 0 },
                { "Gamepad_RightStick", "Gamepad Right Thumbstick Click", KeyCategory::Gamepad, false, 0 },

                // Bumpers
                { "Gamepad_LeftBumper", "Gamepad Left Bumper", KeyCategory::Gamepad, false, 0 },
                { "Gamepad_RightBumper", "Gamepad Right Bumper", KeyCategory::Gamepad, false, 0 },

                // D-Pad
                { "Gamepad_DPad_Up", "Gamepad D-Pad Up", KeyCategory::Gamepad, false, 0 },
                { "Gamepad_DPad_Down", "Gamepad D-Pad Down", KeyCategory::Gamepad, false, 0 },
                { "Gamepad_DPad_Left", "Gamepad D-Pad Left", KeyCategory::Gamepad, false, 0 },
                { "Gamepad_DPad_Right", "Gamepad D-Pad Right", KeyCategory::Gamepad, false, 0 },

                // Triggers
                { "Gamepad_LeftTrigger", "Gamepad Left Trigger", KeyCategory::Gamepad, true, 0 },
                { "Gamepad_RightTrigger", "Gamepad Right Trigger", KeyCategory::Gamepad, true, 0 },
            };
            return keys;
        }

        const KeyInfo* find(const QString& engineName)
        {
            for (const KeyInfo& k : all())
                if (k.name == engineName) return &k;
            return nullptr;
        }

        QString categoryName(KeyCategory c)
        {
            switch (c)
            {
            case KeyCategory::Keyboard: return QStringLiteral("Keyboard");
            case KeyCategory::Mouse:    return QStringLiteral("Mouse");
            case KeyCategory::Gamepad:  return QStringLiteral("Gamepad");
            }
            return {};
        }

        // Qt gives the same key for left/right shift and ctrl, ask the OS which one it was
        static bool isRightSide(const QKeyEvent* e, bool shift)
        {
#if defined(Q_OS_WIN)
            Q_UNUSED(e)
            return (GetKeyState(shift ? VK_RSHIFT : VK_RCONTROL) & 0x8000) != 0;
#elif defined(Q_OS_MACOS)
            return shift ? e->nativeVirtualKey() == 60 : e->nativeVirtualKey() == 62;
#elif defined(Q_OS_LINUX)
            return shift ? e->nativeScanCode() == 62 : e->nativeScanCode() == 105;
#else
            Q_UNUSED(e)
            Q_UNUSED(shift)
            return false;
#endif
        }

        QString nameFromKeyEvent(const QKeyEvent* e)
        {
            const int k = e->key();
            if (k == Qt::Key_Shift)
                return isRightSide(e, true) ? QStringLiteral("Keyboard_RightShift") : QStringLiteral("Keyboard_LeftShift");
            if (k == Qt::Key_Control)
                return isRightSide(e, false) ? QStringLiteral("Keyboard_RightControl") : QStringLiteral("Keyboard_LeftControl");

            for (const KeyInfo& info : all())
                if (info.qtKey != 0 && info.qtKey == k && info.qtKey != Qt::Key_Shift && info.qtKey != Qt::Key_Control)
                    return info.name;
            return {};
        }

    } // namespace KeyCatalog
} // namespace imap
