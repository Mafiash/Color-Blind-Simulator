#include "ColorBlindApp.h"
#include <QtWidgets/QApplication>

/*
 * ================================================================================================
 * Nazwa procedury:  main
 * Opis:             Standardowa funkcja startowa programu w jêzyku C++.
 * Inicjalizuje obiekt aplikacji Qt, tworzy i wyœwietla g³ówne okno,
 * a nastêpnie przekazuje sterowanie do pêtli zdarzeñ systemu.
 *
 * Parametry wejœciowe:
 * argc - (int) Liczba argumentów wiersza poleceñ przekazanych przez system operacyjny.
 * argv - (char*[]) Tablica wskaŸników na argumenty tekstowe (np. parametry uruchomieniowe).
 *
 * Parametry wyjœciowe:
 * (int) - Kod zakoñczenia procesu (Exit Code).
 * Wartoœæ 0 oznacza poprawne zakoñczenie, inne wartoœci sygnalizuj¹ b³êdy.
 * Wartoœæ ta pochodzi z metody app.exec().
 * ================================================================================================
 */
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    ColorBlindApp window;
    window.show();
    return app.exec();
}
