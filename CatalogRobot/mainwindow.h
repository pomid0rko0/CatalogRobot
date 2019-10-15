#pragma once

#include <QMainWindow>
#include <QTextEdit>
#include <QFileDialog>
#include <QMessageBox>
#include <QDir>
#include "addform.h"
#include "brend.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    Ui::MainWindow *ui;

public slots:
    void edit_brend();
    void delete_brend();
    void thread_started();
    void thread_finished();

private slots:
    void log(QString text);
    void on_add_button_clicked();
    void on_start_button_clicked();
    void on_tabWidget_tabCloseRequested(int index);
    void on_parser_search_button_clicked();
    void on_action_3_triggered();
    void on_pause_lineEdit_textChanged(const QString &arg1);

    void increase_edit_progress();
    void increase_hide_progress();
    void increase_upload_progress();
    void increase_photo_progress();
    void increase_total_progress();

private:
    static const QString config;
    QVector<brend*> brends;
    QVector<QThread*> threads;
    int running_threads = 0;

    void Save();
    void Load();
    void AddBrend(brend *b);
    QWidget *MakeLog(brend *b);
};
