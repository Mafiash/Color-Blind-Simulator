/*
 * ================================================================================================
 * Temat projektu:   Symulator Daltonizmu
 * Opis algorytmu:   Modu³ zarz¹dzaj¹cy wielow¹tkowoœci¹ (Wrapper) dla biblioteki Asemblerowej.
 * Odpowiada za podzia³ obrazu na poziome pasy (dekompozycja domeny)
 * i równoleg³e uruchomienie procedury `RunAsm` w osobnych w¹tkach.
 * Realizuje model przetwarzania wspó³bie¿nego typu Fork-Join.
 *
 * Data wykonania:   Semestr Zimowy 2025/2026
 * Autor:            Mateusz Smuda
 *
 * ================================================================================================
 */

#include <vector>
#include <thread>
#include <algorithm> 

 // Makro definuj¹ce eksport funkcji z biblioteki DLL (wymagane dla kompilatora MSVC/Windows)
 // Umo¿liwia widocznoœæ funkcji `ProcessImage` dla Qt.
#define DLLEXPORT extern "C" __declspec(dllexport)

// --- DEKLARACJA FUNKCJI ZEWNÊTRZNEJ (ASEMBLER) ---
// Deklaracja funkcji zaimplementowanej w pliku .asm.
// extern "C" zapobiega "name mangling" (zmianie nazwy funkcji przez kompilator C++).
extern "C" void RunAsm(unsigned char* imgData, int numPixels, int type);

/*
 * ================================================================================================
 * Nazwa procedury:  ProcessImage
 * Opis:             G³ówna funkcja eksportowana z biblioteki DLL (Manager W¹tków).
 * Odpowiada za dekompozycjê domeny (podzia³ obrazu na poziome pasy)
 * i uruchomienie przetwarzania równoleg³ego w w¹tkach.
 *
 * Parametry wejœciowe:
 * imgData - (unsigned char*) WskaŸnik do danych obrazu (format BGRA 32-bit).
 * width   - (int) Szerokoœæ obrazu w pikselach. Zakres: > 0.
 * height  - (int) Wysokoœæ obrazu w pikselach. Zakres: > 0.
 * threads - (int) ¯¹dana liczba w¹tków. Zakres: 1 - 64 (lub wysokoœæ obrazu).
 * type    - (int) Typ symulacji (0, 1, 2).
 *
 * Parametry wyjœciowe:
 * Brak. Funkcja zarz¹dza w¹tkami, które modyfikuj¹ bufor imgData.
 * ================================================================================================
 */
DLLEXPORT void ProcessImage(unsigned char* imgData, int width, int height, int threads, int type) {

    // 1. Walidacja i korekta liczvy w¹tków
    if (threads < 1) threads = 1;           // Zabezpieczenie przed zerow¹/ujemn¹ liczb¹ w¹tków
    if (threads > height) threads = height; // Liczba w¹tków nie mo¿e byæ wiêksza ni¿ liczba wierszy obrazu

    // Wektor przechowuj¹cy obiekty w¹tków
    std::vector<std::thread> threadPool;

    // Obliczenie podzia³u pracy (Dekompozycja pozioma)
    // Dzielimy wysokoœæ obrazu przez liczbê w¹tków.
    int rowsPerThread = height / threads;      // Liczba wierszy dla jednego w¹tku
    int remainingRows = height % threads;      // Reszta wierszy (dla ostatniego w¹tku)
    int currentStartRow = 0;                   // Licznik bie¿¹cego wiersza startowego

    // 3. Pêtla tworz¹ca i uruchamiaj¹ca w¹tki
    for (int i = 0; i < threads; ++i) {
        int rowsToDo = rowsPerThread;          // Domyœlna liczba wierszy

        // Jeœli to ostatni w¹tek, dodajemy mu resztê wierszy (¿eby pokryæ ca³y obraz)
        if (i == threads - 1) rowsToDo += remainingRows;

        // Obliczenie przesuniêcia (offsetu) w pamiêci do pocz¹tku fragmentu dla danego w¹tku
        // Offset = NumerWiersza * Szerokoœæ * 4 bajty/piksel
        unsigned long long offset = (unsigned long long)currentStartRow * width * 4;

        // Ustalenie wskaŸnika startowego dla w¹tku
        unsigned char* threadStartPtr = imgData + offset;

        // Obliczenie liczby pikseli do przetworzenia przez ten w¹tek
        int pixelsToDo = rowsToDo * width;

        // Uruchomienie nowego w¹tku wykonuj¹cego procedurê RunAsm
        // Przekazywane s¹ wskaŸnik startowy, liczba pikseli i typ symulacji.
        threadPool.push_back(std::thread(RunAsm, threadStartPtr, pixelsToDo, type));

        // Aktualizacja licznika wierszy dla nastêpnej iteracji
        currentStartRow += rowsToDo;
    }

    // 4. Synchronizacja w¹tków (Faza Join)
    // G³ówny w¹tek czeka, a¿ wszystkie w¹tki robocze zakoñcz¹ swoje zadania.
    for (auto& t : threadPool) {
        if (t.joinable()) t.join(); // Blokuje wykonanie do momentu zakoñczenia w¹tku t
    }
}