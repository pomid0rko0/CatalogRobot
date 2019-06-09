#ifndef FORM_H
#define FORM_H

#include <QWidget>
#include <QFileDialog>

namespace Ui {
class Form;
}

class Form : public QWidget
{
    Q_OBJECT

public:
    explicit Form(QWidget *parent = nullptr);
    ~Form();

private slots:
    void on_parser_search_button_clicked();

    void on_start_button_clicked();

    void on_pause_button_clicked();

private:
    Ui::Form *ui;
};

#endif // FORM_H
