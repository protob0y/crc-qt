#ifndef CRC_MAIN_H
#define CRC_MAIN_H

#include <QMainWindow>
#include <QMessageBox>
#include <QRegularExpression>
#include <QRandomGenerator>
#include <QLineEdit>
#include <QPlainTextEdit>
#include "calc_crc.h"
#include "resultwindow.h"

#define TAB_INDEX_CAN 0
#define TAB_INDEX_CANFD 1
#define TAB_INDEX_CANXL 2
#define TAB_INDEX_LIN 3
#define TAB_INDEX_FLXRY 4

QT_BEGIN_NAMESPACE
namespace Ui {
class crc_main;
}
QT_END_NAMESPACE

class crc_main : public QMainWindow
{
    Q_OBJECT

public:
    crc_main(QWidget *parent = nullptr);
    ~crc_main();

private slots:
    //LIN
    void lin_updateClickableRadioBtn();
    void lin_update_LineEditID();

    //general
    void preliminary_parse();
    QString stringRemoveDelimiters(QString original);
    QString stringAddDelimiters(QString original, char type);
    //Hilfsfunktionen für preliminary parse
    QString getWidgetText(QLineEdit *senderLineEdit, QPlainTextEdit *senderTextEdit);
    QString getWidgetName(QLineEdit *senderLineEdit, QPlainTextEdit *senderTextEdit);
    void setWidgetText(QLineEdit *senderLineEdit, QPlainTextEdit *senderTextEdit, QString text);
    void setWidgetStylesheet(QLineEdit *senderLineEdit, QPlainTextEdit *senderTextEdit, QString style);

    void clicked_buttonCalcCRC();
    void autofill_fields();



private:
    //Klassen
    Ui::crc_main *ui;
    calc_crc *Calculator;
    QRandomGenerator m_qrand;
    ResultWindow * myResultWindow;

    //RegEx for preliminary check of input fields
    QRegularExpression binaryRegEx;
    QRegularExpression hexRegEx;

    //Liste für textfelder, die den preliminary check bestanden haben
    QList<QString> inputOkList;

    //Methoden:
    void showErrorBox(QString text);
    void invokeLinCRC(bool enhanced);
    void invokeCanCRC(bool fd, bool xl);
    void getInputRules(QString sender, int* acceptBits, bool* acceptMult);
    QString generateFillString(int fillytpe, int numberOfBits);

    void UpdateStatusLabel(QString sender, int detected_bytes, QString message = "");
    //...

	void invokeFrCRC();
	uint8_t getFRpayloadlaenge(QString* query);
};



#endif // CRC_MAIN_H

