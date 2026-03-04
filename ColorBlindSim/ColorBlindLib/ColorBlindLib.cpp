/*
 * ================================================================================================
 * Temat projektu:   Symulacja Daltonizmu
 * Opis algorytmu:   Implementacja w jêzyku C++. Algorytm przetwarza obraz
 * w przestrzeni barw RGB przy u¿yciu modelu macierzowego Machado (2009).
 * Zastosowano arytmetykê sta³oprzecinkow¹ (Fixed Point Q10) w celu
 * unikniêcia operacji zmiennoprzecinkowych. Obraz jest dzielony na
 * poziome pasy i przetwarzany równolegle przez zadan¹ liczbê w¹tków.
 *
 * Data wykonania:   Semestr Zimowy 2025/2026
 * Autor:            Mateusz Smuda
 *
 * * Wersja programu:  1.0
 * Historia zmian:
 * v1.0 - Implementacja programu w C++.
 * ================================================================================================
 */

#include <vector>
#include <thread>
#include <algorithm>
#include <cmath>

 // Makro definuj¹ce eksport funkcji z biblioteki DLL (wymagane dla Windows/Visual Studio)
// Umo¿liwia widocznoœæ funkcji `ProcessImage` dla aplikacji Qt.
#define DLLEXPORT extern "C" __declspec(dllexport)

/*
 * ================================================================================================
 * Sta³e: Tablice wspó³czynników macierzy symulacji (Algorytm Machado)
 * Opis:  Wspó³czynniki zosta³y przeskalowane przez 1024 (przesuniêcie bitowe << 10).
 * Dziêki temu operacje float zosta³y zast¹pione przez szybkie int.
 * Wartoœæ 1.0 jest reprezentowana jako 1024.
 * Uk³ad tablicy: [R_r, R_g, R_b, G_r, G_g, G_b, B_r, B_g, B_b]
 * ================================================================================================
 */

 // Macierz dla Deuteranopii (Niewra¿liwoœæ na zieleñ)
const int coeffs_deuter[9] = { 376, 882, -233,  287, 689, 48,  -12, 44, 992 };

// Macierz dla Protanopii (Niewra¿liwoœæ na czerwieñ)
const int coeffs_protan[9] = { 156, 1077, -210, 118, 805, -101, -3, -49, 1077 };

// Macierz dla Tritanopii (Niewra¿liwoœæ na niebieski)
const int coeffs_tritan[9] = { 1286, -79, -183, -80, 953, 152,  5, 708, -311 };

/*
 * ================================================================================================
 * Nazwa procedury:  clamp
 * Opis:             Funkcja pomocnicza dokonuj¹ca saturacji (przyciêcia) wartoœci ca³kowitej
 * do zakresu 8-bitowego (0-255). Zapobiega przek³amaniom kolorów przy
 * przepe³nieniu (overflow) lub niedomiarze (underflow).
 *
 * Parametry wejœciowe:
 * value - (int) Wartoœæ obliczona przez macierz, mo¿e byæ ujemna lub > 255.
 * Zakres: pe³ny zakres typu int.
 *
 * Parametry wyjœciowe:
 * (return) - (unsigned char) Wartoœæ przyciêta do zakresu [0, 255].
 *
 * Rejestry/Zasoby:  Operuje na stosie/rejestrach ogólnego przeznaczenia (zale¿nie od kompilatora).
 * ================================================================================================
 */
unsigned char clamp(int value) {
    if (value < 0) return 0;     // Jeœli wartoœæ ujemna, zwróæ 0
    if (value > 255) return 255; // Jeœli wartoœæ > 255, zwróæ 255
    return (unsigned char)value; // Rzutowanie na bajt
}

/*
 * ================================================================================================
 * Nazwa procedury:  RunCppWorker
 * Opis:             Funkcja robocza wykonywana przez pojedynczy w¹tek. Przetwarza przydzielony
 * fragment bufora obrazu, aplikuj¹c przekszta³cenie macierzowe na ka¿dym pikselu.
 *
 * Parametry wejœciowe:
 * startPtr  - (unsigned char*) WskaŸnik do pocz¹tku przydzielonego fragmentu pamiêci obrazu.
 * numPixels - (int) Liczba pikseli do przetworzenia przez ten w¹tek. Zakres: > 0.
 * type      - (int) Typ symulacji daltonizmu.
 * Wartoœci: 0 - Deuteranopia, 1 - Protanopia, 2 - Tritanopia.
 *
 * Parametry wyjœciowe:
 * Brak (Funkcja void). Modyfikuje bezpoœrednio pamiêæ wskazywan¹ przez startPtr.
 *
 * Efekty:
 * Modyfikuje wartoœci sk³adowych R, G, B w buforze pamiêci.
 * ================================================================================================
 */
void RunCppWorker(unsigned char* startPtr, int numPixels, int type) {
    const int* c = nullptr; // WskaŸnik na wybran¹ tablicê wspó³czynników

    // Wybór odpowiedniej macierzy na podstawie typu symulacji
    if (type == 0) c = coeffs_deuter;
    else if (type == 1) c = coeffs_protan;
    else c = coeffs_tritan;

    // Pêtla iteruj¹ca po wszystkich przydzielonych pikselach
    for (int i = 0; i < numPixels; ++i) {
        // Obliczenie wskaŸnika na bie¿¹cy piksel (ka¿dy piksel to 4 bajty: B, G, R, A)
        unsigned char* px = startPtr + (i * 4);

        // Odczyt sk³adowych koloru (Architektura Little Endian: kolejnoœæ w pamiêci to B, G, R)
        int oldB = px[0]; // Sk³adowa Blue
        int oldG = px[1]; // Sk³adowa Green
        int oldR = px[2]; // Sk³adowa Red

        // Obliczenia macierzowe w arytmetyce sta³oprzecinkowej (Q10)
        // Wzór: NowyKolor = (R*c1 + G*c2 + B*c3) / 1024
        // Przesuniêcie bitowe >> 10 jest odpowiednikiem dzielenia ca³kowitego przez 1024.

        // Obliczenie nowego kana³u RED
        int newR = (oldR * c[0] + oldG * c[1] + oldB * c[2]) >> 10;

        // Obliczenie nowego kana³u GREEN
        int newG = (oldR * c[3] + oldG * c[4] + oldB * c[5]) >> 10;

        // Obliczenie nowego kana³u BLUE
        int newB = (oldR * c[6] + oldG * c[7] + oldB * c[8]) >> 10;

        // Zapis przetworzonych wartoœci do pamiêci z u¿yciem saturacji (clamp)
        px[2] = clamp(newR); // Zapis Red
        px[1] = clamp(newG); // Zapis Green
        px[0] = clamp(newB); // Zapis Blue
        // px[3] (Alpha) pozostaje bez zmian
    }
}

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

    // Walidacja i korekta liczby w¹tków
    if (threads < 1) threads = 1;              // Zabezpieczenie przed zerow¹/ujemn¹ liczb¹ w¹tków
    if (threads > height) threads = height;    // // Liczba w¹tków nie mo¿e byæ wiêksza ni¿ liczba wierszy obrazu

    // Wektor przechowuj¹cy obiekty w¹tków
    std::vector<std::thread> threadPool;

    // Obliczenie podzia³u pracy (Dekompozycja pozioma)
    // Dzielimy wysokoœæ obrazu przez liczbê w¹tków.
    int rowsPerThread = height / threads;      // Liczba wierszy dla jednego w¹tku
    int remainingRows = height % threads;      // Reszta wierszy (dla ostatniego w¹tku)
    int currentStartRow = 0;                   // Licznik bie¿¹cego wiersza startowego

    // Pêtla tworz¹ca i uruchamiaj¹ca w¹tki
    for (int i = 0; i < threads; ++i) {
        int rowsToDo = rowsPerThread;          // Domyœlna liczba wierszy

        // Jeœli to ostatni w¹tek, dodajemy mu resztê wierszy (¿eby pokryæ ca³y obraz)
        if (i == threads - 1) rowsToDo += remainingRows;

        // Obliczenie przesuniêcia (offsetu) w pamiêci do pocz¹tku fragmentu dla danego w¹tku
        // Offset = NumerWiersza * Szerokoœæ * 4 bajty/piksel
        unsigned long long offset = (unsigned long long)currentStartRow * width * 4;

        // Ustalenie wskaŸnika startowego dla w¹tku
        unsigned char* threadStartPtr = imgData + offset;

        // Obliczenie liczby pikseli do przetworzenia przez w¹tek
        int pixelsToDo = rowsToDo * width;

        // Uruchomienie w¹tku z funkcj¹ robocz¹ RunCppWorker
        // Przekazywane s¹ wskaŸnik startowy, liczba pikseli i typ symulacji.
        threadPool.push_back(std::thread(RunCppWorker, threadStartPtr, pixelsToDo, type));

        // Aktualizacja licznika wierszy dla nastêpnej iteracji
        currentStartRow += rowsToDo;
    }

    // 4. Synchronizacja w¹tków (Faza Join)
    // G³ówny w¹tek czeka, a¿ wszystkie w¹tki robocze zakoñcz¹ swoje zadania.
    for (auto& t : threadPool) {
        if (t.joinable()) t.join(); // Blokuje wykonanie do momentu zakoñczenia w¹tku t
    }
}