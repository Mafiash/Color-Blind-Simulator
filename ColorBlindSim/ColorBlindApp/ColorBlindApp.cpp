/*
 * ================================================================================================
 * Temat projektu:   Symulator Daltonizmu
 * Opis pliku:       G³ówna logika aplikacji okienkowej (GUI) opartej na bibliotece Qt.
 * Odpowiada za:
 * 1. Interakcjê z u¿ytkownikiem (wczytywanie/zapisywanie obrazów).
 * 2. Dynamiczne ³adowanie bibliotek DLL (C++ lub ASM) w czasie rzeczywistym.
 * 3. Pomiar czasu wykonywania i zu¿ycia pamiêci RAM.
 * 4. Zapisywanie wyników pomiarów do pliku CSV.
 *
 * Data wykonania:   Semestr Zimowy 2025/2026
 * Autor:            Mateusz Smuda
 *
 * Wersja:           2.0
 * Historia zmian:
 * v1.0 - Podstawowe GUI w Qt, statyczne linkowanie bibliotek.
 * v2.0 - Dodanie obs³ugi dynamicznego ³adowania DLL (LoadLibrary),
 * pomiaru czasu (std::chrono) i zu¿ycia pamiêci (Windows API).
 * ================================================================================================
 */

#include "ColorBlindApp.h"

#include <chrono>    
#include <fstream>  
#include <windows.h> 
#include <psapi.h>   
#pragma comment(lib, "psapi.lib") 
#include <QFileDialog> 
#include <QMessageBox>
#include <QDebug>      

typedef void (*ProcessImageFunc)(unsigned char*, int, int, int, int);

/*
 * ================================================================================================
 * Nazwa procedury:  GetMemoryUsageMB
 * Opis:             Funkcja pomocnicza pobieraj¹ca aktualne zu¿ycie pamiêci RAM przez proces.
 *
 * Parametry wejœciowe:
 * Brak.
 *
 * Parametry wyjœciowe:
 * (double) - Rozmiar zestawu roboczego (Working Set) procesu w megabajtach (MB).
 *
 * Efekty uboczne:
 * Odwo³uje siê do struktur systemowych procesu.
 * ================================================================================================
 */
double GetMemoryUsageMB() {
    PROCESS_MEMORY_COUNTERS pmc;
    // Pobranie informacji o pamiêci dla bie¿¹cego procesu
    if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc))) {
        // Konwersja z bajtów na megabajty
        return (double)pmc.WorkingSetSize / (1024.0 * 1024.0);
    }
    return 0.0;
}

/*
 * ================================================================================================
 * Nazwa procedury:  ColorBlindApp (Konstruktor)
 * Opis:             Inicjalizuje g³ówne okno aplikacji i ³¹czy sygna³y UI ze slotami (funkcjami).
 *
 * Parametry wejœciowe:
 * parent - (QWidget*) WskaŸnik do rodzica widgetu (zazwyczaj nullptr dla g³ównego okna).
 * ================================================================================================
 */
ColorBlindApp::ColorBlindApp(QWidget* parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);

    connect(ui.btnLoad, &QPushButton::clicked, this, &ColorBlindApp::onLoadClicked);
    connect(ui.btnSave, &QPushButton::clicked, this, &ColorBlindApp::onSaveClicked);
    connect(ui.btnProcess, &QPushButton::clicked, this, &ColorBlindApp::onProcessClicked);
}

ColorBlindApp::~ColorBlindApp()
{
}

/*
 * ================================================================================================
 * Nazwa procedury:  onLoadClicked
 * Opis:             Obs³uga zdarzenia klikniêcia przycisku "Wczytaj".
 * Otwiera dialog wyboru pliku i ³aduje obraz do pamiêci.
 *
 * Zmiany stanu:
 * Modyfikuje zmienne: originalImage, processedImage.
 * Aktualizuje widok interfejsu.
 * ================================================================================================
 */
void ColorBlindApp::onLoadClicked()
{
    QString fileName = QFileDialog::getOpenFileName(this,
        tr("Wybierz obraz"), "", tr("Obrazy JPG (*.jpg *.jpeg)"));

    if (fileName.isEmpty()) return; // Anulowano wybór

    bool loaded = originalImage.load(fileName);
    if (!loaded) {
        QMessageBox::warning(this, "Blad", "Nie udalo sie wczytac pliku.");
        return;
    }

    // KONWERSJA FORMATU: Kluczowy krok dla zgodnoœci z algorytmem ASM.
    // Format_ARGB32 gwarantuje wyrównanie pikseli do 4 bajtów (B, G, R, A).
    originalImage = originalImage.convertToFormat(QImage::Format_ARGB32);
    processedImage = originalImage.copy();
    updateImageViews();
}

/*
 * ================================================================================================
 * Nazwa procedury:  updateImageViews
 * Opis:             Odœwie¿a etykiety (QLabel) wyœwietlaj¹ce obrazy.
 * Skaluje obrazy do rozmiaru kontrolek z zachowaniem proporcji.
 * ================================================================================================
 */
void ColorBlindApp::updateImageViews()
{
    if (originalImage.isNull()) return;
    QPixmap pixOrig = QPixmap::fromImage(originalImage);
    QPixmap pixProc = QPixmap::fromImage(processedImage);
    ui.viewOriginal->setPixmap(pixOrig.scaled(ui.viewOriginal->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    ui.viewProcessed->setPixmap(pixProc.scaled(ui.viewProcessed->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

/*
 * ================================================================================================
 * Nazwa procedury:  onSaveClicked
 * Opis:             Obs³uga zdarzenia klikniêcia przycisku "Zapisz".
 * Zapisuje przetworzony obraz na dysku.
 * ================================================================================================
 */
void ColorBlindApp::onSaveClicked()
{
    if (processedImage.isNull()) {
        QMessageBox::information(this, "Info", "Najpierw wczytaj obraz!");
        return;
    }

    QString fileName = QFileDialog::getSaveFileName(this,
        tr("Zapisz obraz"), "", tr("Obrazy JPG (*.jpg)"));

    if (!fileName.isEmpty()) {
        if (processedImage.save(fileName)) {
            QMessageBox::information(this, "Sukces", "Obraz zostal zapisany.");
        }
        else {
            QMessageBox::warning(this, "Blad", "Nie udalo sie zapisac obrazu.");
        }
    }
}

/*
 * ================================================================================================
 * Nazwa procedury:  onProcessClicked
 * Opis:             G³ówna procedura steruj¹ca procesem symulacji.
 * Odpowiada za dynamiczne za³adowanie wybranej biblioteki DLL,
 * uruchomienie algorytmu, pomiar czasu oraz logowanie wyników.
 *
 * Parametry wejœciowe:
 * Pobiera stan kontrolek UI (liczba w¹tków, wybór biblioteki, typ symulacji).
 *
 * Zasoby:
 * Wykorzystuje funkcje systemowe LoadLibrary/GetProcAddress/FreeLibrary.
 * Operuje na plikach (zapis do CSV).
 * ================================================================================================
 */
void ColorBlindApp::onProcessClicked()
{
    if (originalImage.isNull()) {
        QMessageBox::warning(this, "B³¹d", "Najpierw wczytaj obraz!");
        return;
    }

   
    processedImage = originalImage.copy();

    int threads = ui.spinThreads->value();    
    bool useAsm = ui.radioASM->isChecked();   

   
    LPCWSTR dllName = useAsm ? L"ColorBlindAsmLib.dll" : L"ColorBlindLib.dll";

    int type = 0;
    if (ui.radioProtanopia->isChecked()) type = 1;
    if (ui.radioTritanopia->isChecked()) type = 2;

    unsigned char* data = processedImage.bits();
    int w = processedImage.width();
    int h = processedImage.height();

    auto startTime = std::chrono::high_resolution_clock::now();
    HMODULE hDLL = LoadLibrary(dllName);

    if (hDLL != NULL) {
        ProcessImageFunc procFunc = (ProcessImageFunc)GetProcAddress(hDLL, "ProcessImage");

        if (procFunc != NULL) {
            procFunc(data, w, h, threads, type);
        }
        else {
            QMessageBox::critical(this, "B³¹d DLL", "Nie znaleziono funkcji ProcessImage w bibliotece!");
        }

        FreeLibrary(hDLL);
    }
    else {
        QString missingDll = QString::fromWCharArray(dllName);
        QMessageBox::critical(this, "B³¹d DLL", "Nie znaleziono pliku biblioteki:\n" + missingDll + "\nUpewnij siê, ¿e plik .dll jest w folderze z plikiem .exe!");
        return;
    }

    // 6. Zakoñczenie pomiaru czasu i pamiêci
    auto endTime = std::chrono::high_resolution_clock::now();
    double memEnd = GetMemoryUsageMB();

    // Obliczenie czasu trwania w milisekundach
    std::chrono::duration<double, std::milli> duration = endTime - startTime;
    double timeMs = duration.count();

    // Zapis wyników do pliku CSV (Analiza wydajnoœci)
    std::ofstream file("wyniki.csv", std::ios::app); 
    if (file.is_open()) {
        file << (useAsm ? "ASM" : "C++") << ";"
            << threads << ";"
            << w << "x" << h << ";"
            << timeMs << ";"
            << memEnd << "\n";
        file.close();
    }

    qDebug() << "Czas:" << timeMs << "ms | RAM:" << memEnd << "MB";
    updateImageViews();
}