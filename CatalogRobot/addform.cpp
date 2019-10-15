#include "addform.h"
#include "ui_addform.h"

addform::addform(brend *_b, bool parser, QWidget *parent) :
    QMainWindow(parent),
    b(_b),
    ui(new Ui::addform)
{
    ui->setupUi(this);
    QString s;
    for (int i = 100; i > 0; i--) {
        ui->disc_comboBox->addItem("-" + QString::number(i) + "%");
        ui->proc_comboBox->addItem(QString::number(100 - i));
    }
    ui->disc_comboBox->addItem("0%");
    ui->disc_comboBox->setCurrentIndex(100);

    ui->name_lineEdit->setText(b->name);
    ui->id_lineEdit->setText(b->id);

    if (parser) {
        ui->project_lineEdit->setText(b->parser_project);
        ui->proc_comboBox   ->setCurrentIndex(b->proc.toInt());
        ui->kol_dop_comboBox->setCurrentIndex(b->kol_dop.toInt());
        ui->ident_comboBox  ->setCurrentIndex(b->ident == "art" ? 0 : 1);
        ui->stackedWidget->setCurrentIndex(0);
    }
    else {
        ui->id_site_lineEdit    ->setText(b->id_site);
        ui->disc_comboBox       ->setCurrentIndex(int(b->disc.toDouble() * 100));
        ui->min_price_lineEdit  ->setText(b->min_price);
        ui->max_price_lineEdit  ->setText(b->max_price);
        ui->min_r_lineEdit      ->setText(b->min_r);
        ui->max_r_lineEdit      ->setText(b->max_r);
        ui->sort_comboBox       ->setCurrentIndex(b->sort        == "1" ? 0 : 1);
        ui->_r_comboBox         ->setCurrentIndex(b->_r          == "1" ? 0 : 1);
        ui->d1_comboBox         ->setCurrentIndex(b->d1          == "1" ? 0 : 1);
        ui->d2_comboBox         ->setCurrentIndex(b->d2          == "1" ? 0 : 1);
        ui->d3_comboBox         ->setCurrentIndex(b->d3          == "1" ? 0 : 1);
        ui->d4_comboBox         ->setCurrentIndex(b->d4          == "1" ? 0 : 1);
        ui->d5_comboBox         ->setCurrentIndex(b->d5          == "1" ? 0 : 1);
        ui->price_comboBox      ->setCurrentIndex(b->price       == "1" ? 0 : 1);
        ui->name_up_comboBox    ->setCurrentIndex(b->name_up     == "1" ? 0 : 1);
        ui->rub_comboBox        ->setCurrentIndex(b->rub         == "1" ? 0 : 1);
        ui->isImg_comboBox      ->setCurrentIndex(b->isImg       == "1" ? 0 : 1);
        ui->hideOther_comboBox  ->setCurrentIndex(b->hideOther   == "1" ? 0 : 1);
        ui->trWithoutIm_comboBox->setCurrentIndex(b->trWithoutIm == "1" ? 0 : 1);
        ui->onlyArt_comboBox    ->setCurrentIndex(b->onlyArt     == "1" ? 0 : 1);
        ui->filtr_comboBox      ->setCurrentIndex(b->filtr       == "1" ? 0 : 1);
        ui->stackedWidget->setCurrentIndex(1);
    }

    if (!b->id.isEmpty()) {
        setWindowTitle("Редактировать каталог");
    }
    setAttribute(Qt::WA_DeleteOnClose);
}

addform::~addform()
{
    delete ui;
}

void addform::on_save_pushButton_clicked()
{
    if (ui->id_lineEdit->text().isEmpty()) {
        QMessageBox(QMessageBox::Icon::Critical, "Ошибка!", "Укажите id каталога!", QMessageBox::Ok).exec();
        return;
    }
    b->name = ui->name_lineEdit->text();
    b->name_label->setText(b->name);
    b->id = ui->id_lineEdit->text();
    b->id_label->setText(b->id);


    if (ui->stackedWidget->currentIndex() == 1) {
        if (ui->id_site_lineEdit->text().isEmpty()) {
            QMessageBox(QMessageBox::Icon::Critical, "Ошибка!", "Укажите id встройки!", QMessageBox::Ok).exec();
            return;
        }
        if (ui->min_price_lineEdit->text().isEmpty()) {
            QMessageBox(QMessageBox::Icon::Critical, "Ошибка!", "Укажите минимальную цену!", QMessageBox::Ok).exec();
            return;
        }
        if (ui->max_price_lineEdit->text().isEmpty()) {
            QMessageBox(QMessageBox::Icon::Critical, "Ошибка!", "Укажите максимальную цену!", QMessageBox::Ok).exec();
            return;
        }
        if (ui->min_r_lineEdit->text().isEmpty()) {
            QMessageBox(QMessageBox::Icon::Critical, "Ошибка!", "Укажите минимальное количество товаров в ряду!", QMessageBox::Ok).exec();
            return;
        }
        if (ui->max_r_lineEdit->text().isEmpty()) {
            QMessageBox(QMessageBox::Icon::Critical, "Ошибка!", "Укажите максимальное количество товаров в ряду!", QMessageBox::Ok).exec();
            return;
        }
        b->id_site = ui->id_site_lineEdit->text();
        b->disc = QString::number(double(ui->disc_comboBox->currentIndex()) / 100.);
        while ((b->disc.back() == '0' || b->disc.back() == '.') && b->disc.size() > 1) {
            b->disc.remove(b->disc.size() - 1, 1);
        }
        b->min_price      = ui->min_price_lineEdit->text();
        b->max_price      = ui->max_price_lineEdit->text();
        b->min_r          = ui->min_r_lineEdit->text();
        b->max_r          = ui->max_r_lineEdit->text();
        b->sort           = QString::number((ui->sort_comboBox       ->currentIndex() + 1) % 2);
        b->_r             = QString::number((ui->_r_comboBox         ->currentIndex() + 1) % 2);
        b->d1             = QString::number((ui->d1_comboBox         ->currentIndex() + 1) % 2);
        b->d2             = QString::number((ui->d2_comboBox         ->currentIndex() + 1) % 2);
        b->d3             = QString::number((ui->d3_comboBox         ->currentIndex() + 1) % 2);
        b->d4             = QString::number((ui->d4_comboBox         ->currentIndex() + 1) % 2);
        b->d5             = QString::number((ui->d5_comboBox         ->currentIndex() + 1) % 2);
        b->price          = QString::number((ui->price_comboBox      ->currentIndex() + 1) % 2);
        b->name_up        = QString::number((ui->name_up_comboBox    ->currentIndex() + 1) % 2);
        b->rub            = QString::number((ui->rub_comboBox        ->currentIndex() + 1) % 2);
        b->isImg          = QString::number((ui->isImg_comboBox      ->currentIndex() + 1) % 2);
        b->hideOther      = QString::number((ui->hideOther_comboBox  ->currentIndex() + 1) % 2);
        b->trWithoutIm    = QString::number((ui->trWithoutIm_comboBox->currentIndex() + 1) % 2);
        b->onlyArt        = QString::number((ui->onlyArt_comboBox    ->currentIndex() + 1) % 2);
        b->filtr          = QString::number((ui->filtr_comboBox      ->currentIndex() + 1) % 2);
    }
    else {
        if (ui->project_lineEdit->text().isEmpty()) {
            QMessageBox(QMessageBox::Icon::Critical, "Ошибка!", "Укажите путь до проекта парсера!", QMessageBox::Ok).exec();
            return;
        }
        b->parser_project = ui->project_lineEdit->text();
        b->proc           = ui->proc_comboBox->currentText();
        b->kol_dop        = ui->kol_dop_comboBox->currentText();
        b->ident          = ui->ident_comboBox->currentIndex() == 0 ? "art" : "name";
    }

    close();
}

void addform::on_decline_pushButton_clicked()
{
    close();
}

void addform::on_project_pushButton_clicked()
{
    ui->project_lineEdit->setText(QFileDialog::getOpenFileName(this, "Указать путь до проекта Content Downloader"));
}
