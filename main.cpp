#include <windows.h>   // Hatanın ana sebebi buydu, VK_ tuşları için şart.
#include <vector>      // Liste işlemleri için.
#include <chrono>      // Zamanlama (ms) için.
#include <thread>      // Arka planda oynatma (PlayMacro) için.
#include <string>      // Yazılar için.

#pragma comment(lib, "user32.lib")

// Makro Veri Yapısı
struct MacroEvent {
    int type; // 0: Fare, 1: Tık, 2: Klavye
    int x, y, keyCode;
    bool isDown;
    long long timeOffset;
};

// --- TUŞ ATAMALARI ---
int KEY_RECORD_START = VK_F1;
int KEY_RECORD_STOP  = VK_F2;
int KEY_PLAY_ONCE    = VK_F3;
int KEY_PLAY_LOOP    = VK_F4;
int KEY_PANIC_STOP   = VK_F5;

std::vector<MacroEvent> macroData;
bool isRecording = false;
bool isPlaying = false;
int loopMode = 1; // 1: Tek, 0: Sonsuz

void PlayEngine() {
    if (macroData.empty()) return;
    isPlaying = true;

    do {
        auto start = std::chrono::steady_clock::now();
        for (const auto& ev : macroData) {
            if (!isPlaying) break;

            while (std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now() - start).count() < ev.timeOffset) {
                std::this_thread::sleep_for(std::chrono::microseconds(100));
            }

            if (ev.type == 0) SetCursorPos(ev.x, ev.y);
            else if (ev.type == 1) {
                INPUT in = {0}; in.type = INPUT_MOUSE;
                in.mi.dwFlags = ev.isDown ? MOUSEEVENTF_LEFTDOWN : MOUSEEVENTF_LEFTUP;
                SendInput(1, &in, sizeof(INPUT));
            }
            else if (ev.type == 2) {
                INPUT in = {0}; in.type = INPUT_KEYBOARD;
                in.ki.wVk = (WORD)ev.keyCode;
                in.ki.dwFlags = ev.isDown ? 0 : KEYEVENTF_KEYUP;
                SendInput(1, &in, sizeof(INPUT));
            }
        }
    } while (loopMode == 0 && isPlaying);
    isPlaying = false;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == WM_DESTROY) PostQuitMessage(0);
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hI, HINSTANCE hP, LPSTR lpC, int nS) {
    WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_HREDRAW|CS_VREDRAW, WndProc, 0, 0, hI, NULL, 
                      LoadCursor(NULL, IDC_ARROW), (HBRUSH)(COLOR_WINDOW+1), NULL, "YahyaMacroV2", NULL };
    RegisterClassEx(&wc);
    HWND hwnd = CreateWindowEx(0, "YahyaMacroV2", "Yahya G300s Style Pro", WS_OVERLAPPEDWINDOW, 
                               CW_USEDEFAULT, CW_USEDEFAULT, 400, 300, NULL, NULL, hI, NULL);
    ShowWindow(hwnd, nS);

    std::string info = "F1: Kayit Baslat\nF2: Kayit Durdur\nF3: Oynat (1 Kez)\nF4: Sonsuz Dongu\nF5: DURDUR";
    CreateWindow("STATIC", info.c_str(), WS_CHILD | WS_VISIBLE, 20, 20, 350, 200, hwnd, NULL, hI, NULL);

    MSG msg;
    auto recStart = std::chrono::steady_clock::now();
    bool keys[256] = {0};

    while (true) {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) break;
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        if (GetAsyncKeyState(KEY_RECORD_START) & 0x8000 && !isRecording) {
            macroData.clear();
            recStart = std::chrono::steady_clock::now();
            isRecording = true;
        }
        if (GetAsyncKeyState(KEY_RECORD_STOP) & 0x8000 && isRecording) {
            isRecording = false;
        }
        if (GetAsyncKeyState(KEY_PLAY_ONCE) & 0x8000 && !isRecording && !isPlaying) {
            loopMode = 1;
            std::thread(PlayEngine).detach();
        }
        if (GetAsyncKeyState(KEY_PLAY_LOOP) & 0x8000 && !isRecording && !isPlaying) {
            loopMode = 0;
            std::thread(PlayEngine).detach();
        }
        if (GetAsyncKeyState(KEY_PANIC_STOP) & 0x8000) isPlaying = false;

        if (isRecording) {
            long long offset = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now() - recStart).count();
            POINT p; GetCursorPos(&p);
            macroData.push_back({0, p.x, p.y, 0, false, offset});

            for (int i = 0; i < 256; i++) {
                if (i == KEY_RECORD_START || i == KEY_RECORD_STOP) continue;
                bool down = (GetAsyncKeyState(i) & 0x8000);
                if (down != keys[i]) {
                    int type = (i == VK_LBUTTON || i == VK_RBUTTON) ? 1 : 2;
                    macroData.push_back({type, 0, 0, i, down, offset});
                    keys[i] = down;
                }
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }
    return 0;
}
