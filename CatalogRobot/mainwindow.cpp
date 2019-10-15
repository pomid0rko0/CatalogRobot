#include "mainwindow.h"
#include "ui_mainwindow.h"

const QString MainWindow::config = "config";

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(ui->login_lineEdit, &QLineEdit::textChanged, [&](const QString login) {
        brend::login = login;
    });
    connect(ui->password_lineEdit, &QLineEdit::textChanged, [&](const QString password) {
        brend::password = password;
    });
    connect(ui->parser_lineEdit, &QLineEdit::textChanged, [&](const QString parser) {
        brend::parser = parser;
    });
    QDir::current().mkdir("logs");
    Load();
}

MainWindow::~MainWindow()
{
    Save();
    delete ui;
}

void MainWindow::edit_brend()
{
    auto b = static_cast<brend*>(sender());
    (new addform(b, b->id_site.isEmpty(), this))->show();
}

void MainWindow::delete_brend()
{
    delete static_cast<brend*>(sender())->w;
    int i = brends.indexOf(static_cast<brend*>(sender()));
    brends.removeAt(i);
    threads.removeAt(i);
    delete sender();
}

void MainWindow::thread_started()
{
    auto b = static_cast<brend*>(sender());
    if (b->download_checkBox->isChecked() || b->upload_checkBox->isChecked()) {
        b->isRunning = brend::RUN;
        QFile::remove("./logs/" + b->id + "_" + b->name + ".log");
        QFile f("./logs/" + b->id + "_" + b->name + ".log");
        f.open(QFile::WriteOnly);
        f.write("");
        f.close();

        b->edit_button->setDisabled(true);
        b->delete_button->setDisabled(true);
        b->download_checkBox->setDisabled(true);
        b->upload_checkBox->setDisabled(true);
        b->start_button->setIcon(QIcon(":/res/pause.png"));
        b->stop_button->setEnabled(true);

        ui->tabWidget->addTab(MakeLog(b), b->name);
        ui->start_button->setText("Стоп");
        ui->pause_button->setEnabled(true);

        running_threads++;
        threads[brends.indexOf(b)]->start();
    }
}

void MainWindow::thread_finished()
{
    auto b = static_cast<brend*>(sender());
    b->isRunning = brend::STOP;
    b->edit_button->setEnabled(true);
    b->delete_button->setEnabled(true);
    b->download_checkBox->setEnabled(true);
    b->upload_checkBox->setEnabled(true);
    b->start_button->setIcon(QIcon(":/res/start.png"));
    b->stop_button->setDisabled(true);

    running_threads--;
    if (running_threads == 0) {
        ui->start_button->setText("Старт");
        ui->pause_button->setDisabled(true);
    }
    threads[brends.indexOf(b)]->quit();
}

void MainWindow::log(QString text)
{
    QFile f("./logs/" + static_cast<brend*>(sender())->id + "_" + static_cast<brend*>(sender())->name + ".log");
    if (text.contains("Обновление закупки запущено")) {
        f.remove();
    }
    f.open(QIODevice::WriteOnly | QIODevice::Append);
    // удалить html-тэги
    QXmlStreamReader xml(text);
    QString plain_text;
    while (!xml.atEnd()) {
        if ( xml.readNext() == QXmlStreamReader::Characters ) {
            plain_text += xml.text();
        }
    }
    f.write((QDateTime::currentDateTime().toString() + "|> " +
            plain_text + "\n").toUtf8());
    f.close();
}

void MainWindow::on_add_button_clicked()
{
    int parser = -1;

    QMessageBox *ask = new QMessageBox;
    connect(ask->addButton("Парсер", QMessageBox::ButtonRole::ApplyRole),
            &QPushButton::clicked, [&](){
        parser = 1;
    });
    connect(ask->addButton("Встройка на сайте", QMessageBox::ButtonRole::YesRole),
            &QPushButton::clicked, [&](){
        parser = 0;
    });
    auto b = ask->addButton("", QMessageBox::ButtonRole::NoRole);
    b->hide();
    connect(b, &QPushButton::clicked, [&](){
        parser = -1;
    });

    ask->setAttribute(Qt::WA_DeleteOnClose);
    ask->setText("Каким способом обновлять закупку?");
    ask->setWindowIcon(QIcon(":/res/question.ico"));
    ask->exec();
    if (parser < 0) {
        return;
    }
    brends.push_back(new brend);
    addform *a = new addform(brends.back(), parser, this);
    connect(a, &addform::destroyed, [&](){
        if (brends.back()->id == "") {
            delete brends.back();
            brends.pop_back();
        }
        else {
            AddBrend(brends.back());
        }
    });
    a->show();
}

void MainWindow::on_start_button_clicked()
{
    if (ui->start_button->text() == "Старт") {
        for (int i = 0; i < threads.size(); i++) {
            emit brends[i]->start();
        }
    }
    else {
        for (int i = 0; i < threads.size(); i++) {
            emit brends[i]->stop();
        }
        running_threads = 0;
    }
}

void MainWindow::Save()
{
    QFile f(config);
    f.remove();
    f.open(QFile::WriteOnly);
    f.write(ui->login_lineEdit   ->text().toUtf8() + "\n");
    f.write(ui->password_lineEdit->text().toUtf8() + "\n");
    f.write(ui->parser_lineEdit  ->text().toUtf8() + "\n");
    f.write(ui->pause_lineEdit   ->text().toUtf8() + "\n");
    for (int i = 0; i < brends.size() - 1; i++) {
        f.write(brends[i]->Serialize() + "\n");
    }
    f.write(brends.back()->Serialize());
    f.close();
}

void MainWindow::Load()
{
    QString t;
    QFile f(config);
    f.open(QFile::ReadOnly);

    t = f.readLine(INT16_MAX);
    t.remove('\n');
    ui->login_lineEdit->setText(t);
    t = f.readLine(INT16_MAX);
    t.remove('\n');
    ui->password_lineEdit->setText(t);
    t = f.readLine(INT16_MAX);
    t.remove('\n');
    ui->parser_lineEdit->setText(t);
    t = f.readLine(INT16_MAX);
    t.remove('\n');
    ui->pause_lineEdit->setText(t);

    while (f.canReadLine()) {
        brends.push_back(new brend);
        brends.back()->Deserialize(f);
        AddBrend(brends.back());
    }

    f.close();
}

void MainWindow::AddBrend(brend *b)
{
    ui->scrollArea_layout->addWidget(b->w);
    connect(b, SIGNAL(edit()),  this, SLOT(edit_brend()));
    connect(b, SIGNAL(del()),   this, SLOT(delete_brend()));
    connect(b, SIGNAL(start()), this, SLOT(thread_started()));
    connect(b, SIGNAL(stop()),  this, SLOT(thread_finished()));

    threads.push_back(new QThread());
    b->moveToThread(threads.back());
    connect(threads.back(), SIGNAL(started()), b, SLOT(process()));
}

QWidget *MainWindow::MakeLog(brend *b)
{
    QWidget *log_widget = new QWidget;
    log_widget->setLayout(new QVBoxLayout);
    QTextEdit *log_textEdit = new QTextEdit;
    log_textEdit->setReadOnly(true);
    log_widget->layout()->addWidget(log_textEdit);
    connect(b, SIGNAL(log(const QString&)), log_textEdit, SLOT(append(const QString&)));
    connect(b, SIGNAL(log(const QString&)), this, SLOT(log(QString)));

    QProgressBar *total_progress = new QProgressBar;
    total_progress->setTextVisible(true);
    total_progress->setFormat("Общий прогресс: %p%");
    total_progress->setAlignment(Qt::AlignCenter);
    total_progress->setValue(0);
    b->total_progress = total_progress;

    QProgressBar *upload_progress = new QProgressBar;
    upload_progress->setAlignment(Qt::AlignCenter);
    upload_progress->setValue(0);
    upload_progress->setTextVisible(true);
    upload_progress->setFormat("");
    upload_progress->setMaximum(1);
    b->upload_progress = upload_progress;

    if (!b->parser_project.isEmpty()) {
        QProgressBar *edit_progress = new QProgressBar;
        edit_progress->setAlignment(Qt::AlignCenter);
        edit_progress->setValue(0);
        edit_progress->setTextVisible(true);
        edit_progress->setFormat("");
        edit_progress->setMaximum(1);
        b->edit_progress = edit_progress;

        QProgressBar *hide_progress = new QProgressBar;
        hide_progress->setAlignment(Qt::AlignCenter);
        hide_progress->setValue(0);
        hide_progress->setTextVisible(true);
        hide_progress->setFormat("");
        hide_progress->setMaximum(1);
        b->hide_progress = hide_progress;

        QProgressBar *photo_progress = new QProgressBar;
        photo_progress->setAlignment(Qt::AlignCenter);
        photo_progress->setValue(0);
        photo_progress->setTextVisible(true);
        photo_progress->setFormat("");
        photo_progress->setMaximum(1);
        b->photo_progress = photo_progress;

        log_widget->layout()->addWidget(edit_progress);
        log_widget->layout()->addWidget(hide_progress);
        log_widget->layout()->addWidget(upload_progress);
        log_widget->layout()->addWidget(photo_progress);

        connect(b, SIGNAL(increase_edit_progress()), this, SLOT(increase_edit_progress()));
        connect(b, SIGNAL(set_maximum_edit_progress(int)), edit_progress, SLOT(setMaximum(int)));
        connect(b, &brend::set_format_edit_progress, edit_progress, &QProgressBar::setFormat);

        connect(b, SIGNAL(increase_hide_progress()), this, SLOT(increase_hide_progress()));
        connect(b, SIGNAL(set_maximum_hide_progress(int)), hide_progress, SLOT(setMaximum(int)));
        connect(b, &brend::set_format_hide_progress, hide_progress, &QProgressBar::setFormat);

        connect(b, SIGNAL(increase_upload_progress()), this, SLOT(increase_upload_progress()));
        connect(b, SIGNAL(set_maximum_upload_progress(int)), upload_progress, SLOT(setMaximum(int)));
        connect(b, &brend::set_format_upload_progress, upload_progress, &QProgressBar::setFormat);

        connect(b, SIGNAL(increase_photo_progress()), this, SLOT(increase_photo_progress()));
        connect(b, SIGNAL(set_maximum_photo_progress(int)), photo_progress, SLOT(setMaximum(int)));
        connect(b, &brend::set_format_photo_progress, photo_progress, &QProgressBar::setFormat);
    }
    else {
        log_widget->layout()->addWidget(upload_progress);
    }
    log_widget->layout()->addWidget(total_progress);

    connect(b, SIGNAL(increase_total_progress()), this, SLOT(increase_total_progress()));
    connect(b, SIGNAL(set_maximum_total_progress(int)), total_progress, SLOT(setMaximum(int)));
    connect(b, &brend::set_format_total_progress, total_progress, &QProgressBar::setFormat);

    return log_widget;
}

void MainWindow::on_tabWidget_tabCloseRequested(int index)
{
    if (index > 0) {
        auto w = ui->tabWidget->widget(index);
        ui->tabWidget->removeTab(index);
        delete w;
    }
}

void MainWindow::on_parser_search_button_clicked()
{
    ui->parser_lineEdit->setText(QFileDialog::getOpenFileName(this, "Указать путь до Content Downloader.exe"));
}

void MainWindow::on_action_3_triggered()
{
    QMessageBox *msg = new QMessageBox(this);
    msg->setAttribute(Qt::WA_DeleteOnClose);
    msg->addButton("OK", QMessageBox::ButtonRole::YesRole);
    msg->setWindowTitle("О программн");
    msg->setText("Каталогробот\n"
                 "Версия: 1.0.0\n"
                 "Дата: 08.07.2019\n"
                 "Разработчик: Сычев Егор\n"
                 "E-mail: Lev55i@ya.ru");
    msg->setIcon(QMessageBox::Icon::Information);
    msg->exec();
}

void MainWindow::increase_edit_progress()
{
    auto p = static_cast<brend*>(sender())->edit_progress;
    p->setValue(p->value() + 1);
}

void MainWindow::increase_hide_progress()
{
    auto p = static_cast<brend*>(sender())->hide_progress;
    p->setValue(p->value() + 1);
}

void MainWindow::increase_upload_progress()
{
    auto p = static_cast<brend*>(sender())->upload_progress;
    p->setValue(p->value() + 1);
}

void MainWindow::increase_photo_progress()
{
    auto p = static_cast<brend*>(sender())->photo_progress;
    p->setValue(p->value() + 1);
}

void MainWindow::increase_total_progress()
{
    auto p = static_cast<brend*>(sender())->total_progress;
    p->setValue(p->value() + 1);
}

void MainWindow::on_pause_lineEdit_textChanged(const QString &arg1)
{
    brend::pause_time = arg1.toInt();
}
