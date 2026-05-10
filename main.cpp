#include <windows.h>
#include <vector>
#include <chrono>
#include <thread>
#include <string>

#pragma comment(lib, "user32.lib")

// Makro verisi yapısı
struct MacroEvent {
    int type; // 0: Fare Hareket, 1: Tık, 2: Klavye
    int x, y;
    int keyCode;
    bool isDown;
    long long timeOffset;
};

std::vector<MacroEvent> macroData;
bool isRecording = false;
bool isPlaying = false;
int loopCount = 1; // Varsayılan 1 tekrar

// Oynatma Motoru
void PlayMacro() {
    if (macroData.empty()) return;
    isPlaying = true;

    for (int i = 0; i < loopCount || loopCount == 0; i++) {
        if (!isPlaying) break;
        auto startTime = std::chrono::steady_clock::now();
        
        for (const auto& ev : macroData) {
            if (!isPlaying) break;

            // Gerçek zamanlı bekleme (G300s hassasiyeti)
            while (std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now() - startTime).count() < ev.timeOffset) {
                std::this_thread::sleep_for(std::chrono::microseconds(500));
            }

            if (ev.type == 0) {
                SetCursorPos(ev.x, ev.y);
            } else if (ev.type == 1) {
                INPUT in = {0}; in.type = INPUT_MOUSE;
                in.mi.dwFlags = ev.isDown ? MOUSEEVENTF_LEFTDOWN : MOUSEEVENTF_LEFTUP;
                SendInput(1, &in, sizeof(INPUT));
            } else if (ev.type == 2) {
                INPUT in = {0}; in.type = INPUT_KEYBOARD;
                in.ki.wVk = (WORD)ev.keyCode;
                in.ki.dwFlags = ev.isDown ? 0 : KEYEVENTF_KEYUP;
                SendInput(1, &in, sizeof(INPUT));
            }
        }
        if (loopCount != 0 && i == loopCount - 1) break;
    }
    isPlaying = false;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == WM_DESTROY) PostQuitMessage(0);
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR lpCmd, int nShow) {
    WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_HREDRAW|CS_VREDRAW, WndProc, 0, 0, hInst, NULL, 
                      LoadCursor(NULL, IDC_ARROW), (HBRUSH)(COLOR_WINDOW+1), NULL, "YahyaMacro", NULL };
    RegisterClassEx(&wc);
    HWND hwnd = CreateWindowEx(0, "YahyaMacro", "Yahya Stealth Macro v1.0", WS_OVERLAPPEDWINDOW, 
                               CW_USEDEFAULT, CW_USEDEFAULT, 400, 250, NULL, NULL, hInst, NULL);
    ShowWindow(hwnd, nShow);

    CreateWindow("STATIC", "F1: Kaydi Baslat\nF2: Kaydi Durdur\nF3: Oynat (1 Kez)\nF4: Sonsuz Dongu\nF5: Durdur", 
                 WS_CHILD | WS_VISIBLE, 20, 20, 350, 150, hwnd, NULL, hInst, NULL);

    MSG msg;
    auto recStart = std::chrono::steady_clock::now();

    while (true) {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) break;
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        // KONTROLLER
        if (GetAsyncKeyState(VK_F1) & 0x8000 && !isRecording) {
            macroData.clear();
            recStart = std::chrono::steady_clock::now();
            isRecording = true;
            SetWindowText(hwnd, "KAYIT YAPILIYOR...");
        }
        if (GetAsyncKeyState(VK_F2) & 0x8000 && isRecording) {
            isRecording = false;
            SetWindowText(hwnd, "Kayit Tamam.");
        }
        if (GetAsyncKeyState(VK_F3) & 0x8000 && !isRecording && !isPlaying) {
            loopCount = 1;
            std::thread(PlayMacro).detach();
        }
        if (GetAsyncKeyState(VK_F4) & 0x8000 && !isRecording && !isPlaying) {
            loopCount = 0; // 0 = Sonsuz
            std::thread(PlayMacro).detach();
        }
        if (GetAsyncKeyState(VK_F5) & 0x8000) isPlaying = false;

        // KAYIT MANTIĞI
        if (isRecording) {
            long long offset = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now() - recStart).count();
            POINT p; GetCursorPos(&p);
            macroData.push_back({0, p.x, p.y, 0, false, offset});

            static bool keys[256] = {0};
            for (int i = 0; i < 256; i++) {
                if (i == VK_F1 || i == VK_F2) continue;
                bool down = (GetAsyncKeyState(i) & 0x8000);
                if (down != keys[i]) {
                    int type = (i == VK_LBUTTON || i == VK_RBUTTON) ? 1 : 2;
                    macroData.push_back({type, 0, 0, i, down, offset});
                    keys[i] = down;
                }
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
    return 0;
}
