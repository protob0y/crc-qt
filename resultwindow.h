#ifndef RESULTWINDOW_H
#define RESULTWINDOW_H

#include <QDialog>

namespace Ui {
class ResultWindow;
}

class ResultWindow : public QDialog
{
    Q_OBJECT

public:
    explicit ResultWindow(QWidget *parent = nullptr);
    void push_result(QList<QString> * resultList);
    ~ResultWindow();

private:
    Ui::ResultWindow *ui;
};

#endif // RESULTWINDOW_H
