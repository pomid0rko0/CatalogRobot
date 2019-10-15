#pragma once

#include <QMainWindow>
#include <QFileDialog>
#include <QMessageBox>
#include "brend.h"

namespace Ui {
class addform;
}

class addform : public QMainWindow
{
    Q_OBJECT

public:
    explicit addform(brend *_b, bool parser, QWidget *parent = nullptr);
    ~addform();

private slots:
    void on_save_pushButton_clicked();
    void on_decline_pushButton_clicked();
    void on_project_pushButton_clicked();

private:
    brend *b;
    Ui::addform *ui;
};
