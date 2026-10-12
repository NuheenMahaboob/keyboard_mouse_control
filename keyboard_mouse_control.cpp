#include <windows.h>
#include <iostream>
#include <thread>
#include <atomic>

std::atomic<bool> keyboardMouseEnabled = false;
std::atomic<bool> leftMouseHeld = false; // Track if left mouse is held
const int moveSpeed = 15;          // Normal cursor speed
const int slowMoveSpeed = 2;       // Slow speed when Shift is held

void MoveCursorLoop() {
    while (true) {
        if (keyboardMouseEnabled) {
            int dx = 0, dy = 0;
            int currentSpeed = moveSpeed;

            // Check if Shift key is held
            if (GetAsyncKeyState(VK_SHIFT) & 0x8000) {
                currentSpeed = slowMoveSpeed;
            }

            // Check key states in real-time
            if (GetAsyncKeyState(VK_LEFT) & 0x8000) dx -= currentSpeed;
            if (GetAsyncKeyState(VK_RIGHT) & 0x8000) dx += currentSpeed;
            if (GetAsyncKeyState(VK_UP) & 0x8000) dy -= currentSpeed;
            if (GetAsyncKeyState(VK_DOWN) & 0x8000) dy += currentSpeed;

            if (dx != 0 || dy != 0) {
                POINT cursorPos;
                GetCursorPos(&cursorPos);
                SetCursorPos(cursorPos.x + dx, cursorPos.y + dy);
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10)); // Smooth movement
    }
}

void PerformClick(int button) {
    INPUT input = { 0 };
    input.type = INPUT_MOUSE;
    input.mi.dwFlags = button;
    SendInput(1, &input, sizeof(INPUT));
}

LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode >= 0) {
        KBDLLHOOKSTRUCT* kbdStruct = (KBDLLHOOKSTRUCT*)lParam;
        bool keyDown = (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN);
        bool keyUp = (wParam == WM_KEYUP || wParam == WM_SYSKEYUP);

        if (keyDown) {
            switch (kbdStruct->vkCode) {
                case VK_F2: // Toggle keyboard-mouse mode
                    keyboardMouseEnabled = !keyboardMouseEnabled;
                    std::cout << "\nKeyboard Mouse Mode: " << (keyboardMouseEnabled ? "ON" : "OFF") << "\n";
                    break;
                case VK_RETURN:
                    if (keyboardMouseEnabled) {
                        if (!leftMouseHeld) {
                            PerformClick(MOUSEEVENTF_LEFTDOWN); // Hold left mouse button
                            leftMouseHeld = true;
                        }
                    }
                    break;
                case VK_RCONTROL:
                    if (keyboardMouseEnabled) PerformClick(MOUSEEVENTF_RIGHTDOWN | MOUSEEVENTF_RIGHTUP);
                    break;
            }
        } else if (keyUp) {
            switch (kbdStruct->vkCode) {
                case VK_RETURN:
                    if (keyboardMouseEnabled && leftMouseHeld) {
                        PerformClick(MOUSEEVENTF_LEFTUP); // Release left mouse button
                        leftMouseHeld = false;
                    }
                    break;
            }
        }
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

int main() {
    std::cout << "Hello Nuheen Senpai \n";
    std::cout << "Press F2 to toggle keyboard-mouse mode.\n";

    // Start a separate thread for continuous movement
    std::thread mouseMover(MoveCursorLoop);
    mouseMover.detach();

    // Set up keyboard hook
    HHOOK keyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, GetModuleHandle(NULL), 0);
    if (!keyboardHook) {
        std::cerr << "Failed to install keyboard hook!\n";
        return 1;
    }

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    UnhookWindowsHookEx(keyboardHook);
    return 0;
}