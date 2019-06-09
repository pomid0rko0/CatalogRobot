#include "form.h"
#include "ui_form.h"

Form::Form(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Form)
{
    ui->setupUi(this);
}

Form::~Form()
{
    delete ui;
}

void Form::on_parser_search_button_clicked()
{
    ui->parser_lineEdit->setText(
                QFileDialog::getOpenFileName(this, "Указать путь до Content Downloader.exe"));
}

void Form::on_start_button_clicked()
{
    if (ui->start_button->text() == "Старт") {
        ui->start_button->setText("Стоп");
        ui->pause_button->setEnabled(true);
    }
    else {
        ui->start_button->setText("Старт");
        ui->pause_button->setEnabled(false);
    }
}

void Form::on_pause_button_clicked()
{
    if (ui->pause_button->text() == "Пауза") {
        ui->pause_button->setText("Продолжить");
    }
    else {
        ui->pause_button->setText("Пауза");
    }
}
