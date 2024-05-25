///////////////////////////////////////////////////////////////////////////////
/// \file
/// \brief Main form module.
///
/// -
///
///////////////////////////////////////////////////////////////////////////////


#ifndef CONV1_H
#define CONV1_H

//#include <QtGui/QMainWindow>
#include <QMainWindow>



/// Main form class.
/**
 *  -
 */

QT_BEGIN_NAMESPACE
namespace Ui { class conv1Class; } // это ЧЦ
QT_END_NAMESPACE

class conv1 : public QMainWindow
{
    Q_OBJECT

public:
    //conv1(QWidget *parent = 0, Qt::WFlags flags = 0);
     conv1(QWidget *parent = nullptr);
    ~conv1();

    int var;                    ///< Описание переменной var.

    void dbTest1();

private slots:
    void pushbutton1_click();       ///< A slot for a button click signal.
    void btnCRSLPtest1_click();     ///< A slot for a CRS LP test 1 button click signal.
    void btnTest1_click();          ///< General purpose test 1 button onclick slot

private:
    Ui::conv1Class *ui;
};

#endif // CONV1_H
