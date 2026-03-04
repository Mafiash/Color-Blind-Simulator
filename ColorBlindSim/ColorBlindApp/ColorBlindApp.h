/*
 * ================================================================================================
 * Temat projektu:   Symulator Daltonizmu - Algorytm Machado
 * Opis pliku:       Plik nag³ówkowy g³ównej klasy aplikacji (Qt GUI).
 * Definiuje strukturê g³ównego okna, sloty obs³uguj¹ce zdarzenia
 * u¿ytkownika (przyciski) oraz prywatne zmienne przechowuj¹ce
 * stan aplikacji (obrazy w pamiêci).
 *
 * Data wykonania:   Semestr Zimowy 2025/2026
 * Autor:            Mateusz Smuda
 *
 * Wersja:           2.0
 * Historia zmian:
 * v1.0 - Utworzenie podstawowej klasy okna w Qt Designer.
 * v2.0 - Dodanie obs³ugi slotów przetwarzania i zmiennych QImage.
 * ================================================================================================
 */

#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_ColorBlindApp.h" 
#include <QImage>           
#include <QPixmap>           
#include <QFileDialog>       
#include <QMessageBox>       
#include <QElapsedTimer>     

 /*
  * ================================================================================================
  * Nazwa klasy:      ColorBlindApp
  * Opis:             G³ówna klasa okna aplikacji dziedzicz¹ca po QMainWindow.
  * Odpowiada za warstwê prezentacji, obs³ugê zdarzeñ (Sygna³y i Sloty)
  * oraz zarz¹dzanie cyklem ¿ycia przetwarzania obrazu.
  * ================================================================================================
  */
class ColorBlindApp : public QMainWindow
{
    Q_OBJECT

public:
    
    ColorBlindApp(QWidget* parent = nullptr);

    ~ColorBlindApp();

private slots:
    void onLoadClicked();

    void onSaveClicked();

    void onProcessClicked();

private:
    Ui::ColorBlindAppClass ui;
    QImage originalImage;
    QImage processedImage;
    void updateImageViews();
};