#include "resultwindow.h"
#include "ui_resultwindow.h"

ResultWindow::ResultWindow(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ResultWindow)
{
    ui->setupUi(this);

    ui->tableWidget->setHorizontalHeaderLabels(QStringList() << "Attribut" << "Wert");
}

ResultWindow::~ResultWindow()
{
    delete ui;
}


void ResultWindow::push_result(QList<QString> *resultList){
    if(resultList == NULL){
        //received pointer is null
        return;
    }

    if(resultList->length() == 0 || resultList->length() % 2 != 0){
        //show label, received list is malformed or cointains no data
        return;
    }

    for(int i = 0; i < resultList->length(); i+=2){
        QString attr = resultList->at(i);
        QString value = resultList->at(i+1);

        QTableWidgetItem *att = new QTableWidgetItem(attr);
        QTableWidgetItem *val = new QTableWidgetItem(value);

        ui->tableWidget->setItem(i / 2, 0, att);
        ui->tableWidget->setItem(i / 2, 1, val);
    }

    //cosmetics
    ui->tableWidget->resizeColumnToContents(0);
    ui->tableWidget->horizontalHeader()->setStretchLastSection(true);
}
