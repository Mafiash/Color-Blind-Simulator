/********************************************************************************
** Form generated from reading UI file 'ColorBlindApp.ui'
**
** Created by: Qt User Interface Compiler version 6.8.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_COLORBLINDAPP_H
#define UI_COLORBLINDAPP_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_ColorBlindAppClass
{
public:
    QWidget *centralWidget;
    QLabel *viewOriginal;
    QLabel *viewProcessed;
    QPushButton *btnLoad;
    QPushButton *btnSave;
    QPushButton *btnProcess;
    QGroupBox *groupType;
    QRadioButton *radioDeuteranopia;
    QRadioButton *radioProtanopia;
    QRadioButton *radioTritanopia;
    QGroupBox *groupImplementation;
    QRadioButton *radioCPP;
    QRadioButton *radioASM;
    QSpinBox *spinThreads;
    QLabel *label;
    QMenuBar *menuBar;
    QToolBar *mainToolBar;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *ColorBlindAppClass)
    {
        if (ColorBlindAppClass->objectName().isEmpty())
            ColorBlindAppClass->setObjectName("ColorBlindAppClass");
        ColorBlindAppClass->resize(573, 400);
        centralWidget = new QWidget(ColorBlindAppClass);
        centralWidget->setObjectName("centralWidget");
        viewOriginal = new QLabel(centralWidget);
        viewOriginal->setObjectName("viewOriginal");
        viewOriginal->setGeometry(QRect(140, 20, 141, 131));
        viewOriginal->setScaledContents(true);
        viewProcessed = new QLabel(centralWidget);
        viewProcessed->setObjectName("viewProcessed");
        viewProcessed->setGeometry(QRect(350, 20, 141, 131));
        viewProcessed->setScaledContents(true);
        btnLoad = new QPushButton(centralWidget);
        btnLoad->setObjectName("btnLoad");
        btnLoad->setGeometry(QRect(59, 173, 91, 31));
        btnSave = new QPushButton(centralWidget);
        btnSave->setObjectName("btnSave");
        btnSave->setGeometry(QRect(59, 223, 91, 31));
        btnProcess = new QPushButton(centralWidget);
        btnProcess->setObjectName("btnProcess");
        btnProcess->setGeometry(QRect(59, 273, 91, 31));
        groupType = new QGroupBox(centralWidget);
        groupType->setObjectName("groupType");
        groupType->setGeometry(QRect(190, 180, 161, 91));
        radioDeuteranopia = new QRadioButton(groupType);
        radioDeuteranopia->setObjectName("radioDeuteranopia");
        radioDeuteranopia->setGeometry(QRect(10, 20, 111, 22));
        radioProtanopia = new QRadioButton(groupType);
        radioProtanopia->setObjectName("radioProtanopia");
        radioProtanopia->setGeometry(QRect(10, 40, 131, 22));
        radioTritanopia = new QRadioButton(groupType);
        radioTritanopia->setObjectName("radioTritanopia");
        radioTritanopia->setGeometry(QRect(10, 60, 131, 22));
        groupImplementation = new QGroupBox(centralWidget);
        groupImplementation->setObjectName("groupImplementation");
        groupImplementation->setGeometry(QRect(380, 180, 120, 71));
        radioCPP = new QRadioButton(groupImplementation);
        radioCPP->setObjectName("radioCPP");
        radioCPP->setGeometry(QRect(10, 20, 91, 22));
        radioASM = new QRadioButton(groupImplementation);
        radioASM->setObjectName("radioASM");
        radioASM->setGeometry(QRect(10, 40, 91, 22));
        spinThreads = new QSpinBox(centralWidget);
        spinThreads->setObjectName("spinThreads");
        spinThreads->setGeometry(QRect(370, 270, 51, 27));
        QSizePolicy sizePolicy(QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(spinThreads->sizePolicy().hasHeightForWidth());
        spinThreads->setSizePolicy(sizePolicy);
        spinThreads->setMinimum(1);
        spinThreads->setMaximum(64);
        label = new QLabel(centralWidget);
        label->setObjectName("label");
        label->setGeometry(QRect(230, 280, 121, 16));
        ColorBlindAppClass->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(ColorBlindAppClass);
        menuBar->setObjectName("menuBar");
        menuBar->setGeometry(QRect(0, 0, 573, 21));
        ColorBlindAppClass->setMenuBar(menuBar);
        mainToolBar = new QToolBar(ColorBlindAppClass);
        mainToolBar->setObjectName("mainToolBar");
        ColorBlindAppClass->addToolBar(Qt::ToolBarArea::TopToolBarArea, mainToolBar);
        statusBar = new QStatusBar(ColorBlindAppClass);
        statusBar->setObjectName("statusBar");
        ColorBlindAppClass->setStatusBar(statusBar);

        retranslateUi(ColorBlindAppClass);

        QMetaObject::connectSlotsByName(ColorBlindAppClass);
    } // setupUi

    void retranslateUi(QMainWindow *ColorBlindAppClass)
    {
        ColorBlindAppClass->setWindowTitle(QCoreApplication::translate("ColorBlindAppClass", "ColorBlindApp", nullptr));
        viewOriginal->setText(QCoreApplication::translate("ColorBlindAppClass", "TextLabel", nullptr));
        viewProcessed->setText(QCoreApplication::translate("ColorBlindAppClass", "TextLabel", nullptr));
        btnLoad->setText(QCoreApplication::translate("ColorBlindAppClass", "Wczytaj obraz", nullptr));
        btnSave->setText(QCoreApplication::translate("ColorBlindAppClass", "Zapisz obraz", nullptr));
        btnProcess->setText(QCoreApplication::translate("ColorBlindAppClass", "Symuluj", nullptr));
        groupType->setTitle(QCoreApplication::translate("ColorBlindAppClass", "Wybierz rodzaj daltonizmu", nullptr));
        radioDeuteranopia->setText(QCoreApplication::translate("ColorBlindAppClass", "Deuteranopia", nullptr));
        radioProtanopia->setText(QCoreApplication::translate("ColorBlindAppClass", "Protanopia", nullptr));
        radioTritanopia->setText(QCoreApplication::translate("ColorBlindAppClass", "Tritanopia", nullptr));
        groupImplementation->setTitle(QCoreApplication::translate("ColorBlindAppClass", "Wybierz Algorytm", nullptr));
        radioCPP->setText(QCoreApplication::translate("ColorBlindAppClass", "C++", nullptr));
        radioASM->setText(QCoreApplication::translate("ColorBlindAppClass", "Asembler", nullptr));
        label->setText(QCoreApplication::translate("ColorBlindAppClass", "Wybierz liczb\304\231 w\304\205tk\303\263w:", nullptr));
    } // retranslateUi

};

namespace Ui {
    class ColorBlindAppClass: public Ui_ColorBlindAppClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_COLORBLINDAPP_H
