#include <windows.h>
#include <vector>
#include <chrono>
#include <thread>
#include <string>

#pragma comment(lib, "user32.lib")

struct MacroEvent {
    int type; // 0: Hareket, 1: Tık, 2: Klavye
    int x, y, keyCode;
    bool isDown;
    long long timeOffset;
};

// --- VARSAYILAN TUŞLAR ---
int KEY_REC_START = VK_F1;
int KEY_REC_STOP  = VK_F2;
int KEY_PLAY_ONCE = VK_F3;
int KEY_PLAY_LOOP = VK_F4;
int KEY_PANIC     = VK_F5;

// Tuş atama değişkenleri
bool waitingForKey = false;
int* keyToChange = nullptr;

std::vector<MacroEvent> macroData;
bool isRecording = false;
bool isPlaying = false;
int loopMode = 1; 

// Oynatma Motoru
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
    static HWND hInfo;
    if (msg == WM_CREATE) {
        hInfo = CreateWindow("STATIC", "", WS_CHILD | WS_VISIBLE, 220, 20, 150, 200, hwnd, NULL, NULL, NULL);
    }
    if (msg == WM_COMMAND) {
        waitingForKey = true;
        switch (LOWORD(wParam)) {
            case 1: keyToChange = &KEY_REC_START; break;
            case 2: keyToChange = &KEY_REC_STOP;  break;
            case 3: keyToChange = &KEY_PLAY_ONCE; break;
            case 4: keyToChange = &KEY_PLAY_LOOP; break;
            case 5: keyToChange = &KEY_PANIC;     break;
        }
        SetWindowText(hwnd, "BIR TUSA BASIN...");
    }
    if (msg == WM_PAINT) {
        std::string info = "Guncel Tuslar:\n\nBaslat: " + std::to_string(KEY_REC_START) + 
                           "\nDurdur: " + std::to_string(KEY_REC_STOP) + 
                           "\nOynat: " + std::to_string(KEY_PLAY_ONCE) + 
                           "\nDongu: " + std::to_string(KEY_PLAY_LOOP) + 
                           "\nPANIK: " + std::to_string(KEY_PANIC);
        SetWindowText(hInfo, info.c_str());
    }
    if (msg == WM_DESTROY) PostQuitMessage(0);
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hI, HINSTANCE hP, LPSTR lpC, int nS) {
    WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_HREDRAW|CS_VREDRAW, WndProc, 0, 0, hI, NULL, 
                      LoadCursor(NULL, IDC_ARROW), (HBRUSH)(COLOR_WINDOW+1), NULL, "YahyaUltimate", NULL };
    RegisterClassEx(&wc);
    HWND hwnd = CreateWindowEx(0, "YahyaUltimate", "Yahya G300s Pro - Full Custom", WS_OVERLAPPEDWINDOW, 
                               CW_USEDEFAULT, CW_USEDEFAULT, 400, 350, NULL, NULL, hI, NULL);

    // Butonlar
    CreateWindow("BUTTON", "Kayit Baslat Tusunu Sec", WS_VISIBLE | WS_CHILD, 10, 10, 180, 30, hwnd, (HMENU)1, hI, NULL);
    CreateWindow("BUTTON", "Kayit Durdur Tusunu Sec", WS_VISIBLE | WS_CHILD, 10, 50, 180, 30, hwnd, (HMENU)2, hI, NULL);
    CreateWindow("BUTTON", "Oynat (1) Tusunu Sec", WS_VISIBLE | WS_CHILD, 10, 90, 180, 30, hwnd, (HMENU)3, hI, NULL);
    CreateWindow("BUTTON", "Sonsuz Dongu Tusunu Sec", WS_VISIBLE | WS_CHILD, 10, 130, 180, 30, hwnd, (HMENU)4, hI, NULL);
    CreateWindow("BUTTON", "Panik Durdur Tusunu Sec", WS_VISIBLE | WS_CHILD, 10, 170, 180, 30, hwnd, (HMENU)5, hI, NULL);

    ShowWindow(hwnd, nS);
    MSG msg;
    auto recStart = std::chrono::steady_clock::now();
    bool keys[256] = {0};

    while (true) {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) break;
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        if (waitingForKey) {
            for (int i = 1; i < 255; i++) {
                if (GetAsyncKeyState(i) & 0x8000) {
                    *keyToChange = i;
                    waitingForKey = false;
                    SetWindowText(hwnd, "Tus Atandi!");
                    InvalidateRect(hwnd, NULL, TRUE);
                    std::this_thread::sleep_for(std::chrono::milliseconds(300));
                    break;
                }
            }
            continue;
        }

        // KONTROLLER
        if (GetAsyncKeyState(KEY_REC_START) & 0x8000 && !isRecording) {
            macroData.clear(); recStart = std::chrono::steady_clock::now();
            isRecording = true; SetWindowText(hwnd, "KAYITTA...");
        }
        if (GetAsyncKeyState(KEY_REC_STOP) & 0x8000 && isRecording) {
            isRecording = false; SetWindowText(hwnd, "Kayit Bitti.");
        }
        if (GetAsyncKeyState(KEY_PLAY_ONCE) & 0x8000 && !isRecording && !isPlaying) {
            loopMode = 1; std::thread(PlayEngine).detach();
        }
        if (GetAsyncKeyState(KEY_PLAY_LOOP) & 0x8000 && !isRecording && !isPlaying) {
            loopMode = 0; std::thread(PlayEngine).detach();
        }
        if (GetAsyncKeyState(KEY_PANIC) & 0x8000) isPlaying = false;

        if (isRecording) {
            long long off = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now()-recStart).count();
            POINT p; GetCursorPos(&p);
            macroData.push_back({0, p.x, p.y, 0, false, off});
            for (int i = 0; i < 256; i++) {
                if (i == KEY_REC_START || i == KEY_REC_STOP) continue;
                bool down = (GetAsyncKeyState(i) & 0x8000);
                if (down != keys[i]) {
                    macroData.push_back({(i<3?1:2), 0, 0, i, down, off});
                    keys[i] = down;
                }
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }
    return 0;
}
