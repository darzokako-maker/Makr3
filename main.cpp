#include <windows.h>
#include <iostream>
#include <thread>
#include <chrono>
#include <random>

// Global Ayarlar
bool aktif = false;
const int TUS_F = 0x46; // F tuşunun kodu
const int SOL_TIK = VK_LBUTTON;

void SanalTikla() {
    INPUT inputs[2] = {0};
    inputs[0].type = INPUT_MOUSE;
    inputs[0].mi.dwFlags = MOUSEEVENTF_LEFTDOWN; // Sanal basış
    inputs[1].type = INPUT_MOUSE;
    inputs[1].mi.dwFlags = MOUSEEVENTF_LEFTUP;   // Sanal çekiş
    SendInput(2, inputs, sizeof(INPUT));
}

int main() {
    // Konsol Ayarları
    SetConsoleTitleA("Yahya Stealth v18.2 [Stable]");
    std::cout << "====================================" << std::endl;
    std::cout << "     YAHYA STEALTH MOUSE MODULE     " << std::endl;
    std::cout << "====================================" << std::endl;
    std::cout << "Durum: KAPALI (F tusuna basarak ac)" << std::endl;

    std::default_random_engine gen;
    std::uniform_int_distribution<int> dist(45, 62); // 16-20 CPS Arası

    while (true) {
        // Tuş Hatası Olmaması İçin Gelişmiş Toggle Kontrolü
        if (GetAsyncKeyState(TUS_F) & 0x8000) { 
            aktif = !aktif; // Durumu değiştir
            
            // Konsolu güncelle
            system("cls");
            std::cout << "====================================" << std::endl;
            std::cout << "     YAHYA STEALTH MOUSE MODULE     " << std::endl;
            std::cout << "====================================" << std::endl;
            if (aktif) {
                std::cout << "Durum: >>> AKTIF <<< (Ikinci Mouse Devrede)" << std::endl;
            } else {
                std::cout << "Durum: KAPALI" << std::endl;
            }
            
            // Tuşa basılı tutulduğunda sürekli aç/kapat yapmasın diye kısa bekleme
            std::this_thread::sleep_for(std::chrono::milliseconds(300));
        }

        // Eğer makro aktifse VE gerçek mouse sol tıkına basılıyorsa
        if (aktif && (GetAsyncKeyState(SOL_TIK) & 0x8000)) {
            SanalTikla(); // Sanal mouse devreye girer
            
            // Rastgele gecikme (Anti-cheat ve lag koruması)
            std::this_thread::sleep_for(std::chrono::milliseconds(dist(gen)));
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(1)); // İşlemciyi yormaz
    }
    return 0;
}
