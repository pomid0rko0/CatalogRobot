#pragma once

#include <QWidget>
#include <QHBoxLayout>
#include <QCheckBox>
#include <QLabel>
#include <QPushButton>
#include <QString>
#include <QtNetwork>
#include <QTextEdit>
#include <QSpinBox>
#include <QProgressBar>

class brend : public QObject
{
    Q_OBJECT

private:
    static const int max_tryings;
    static const int timer_tick;
    static const QTextCodec *cp1251;
    static const QVector<QString> HTML_entities;
    static const QVector<QString> symbols;

    enum REQUEST {
        GET, POST, MULTIPART
    };

    QString login_page() const;
    QString brend_page(const int page = 1) const;
    QString brend_imageless_page(const int page = 1) const;
    QString brend_hidden_page(const int page = 1) const;
    QString hide_item_page(const QString &item_id) const;
    QString delete_hidden_page() const;
    QString restore_item_page(const QString &item_id) const;
    QString parser_shop_page() const;
    QString parser_page() const;
    QString download_page(const QString &hash) const;
    QString upload_page(const QString &hash) const;
    QString edit_page(const int column) const;
    QString add_page() const;
    QString finish_page() const;
    QString upload_photo_page(const QString &id) const;

    QNetworkAccessManager *network        = nullptr;
    QEventLoop            *barrier        = nullptr;
    QProcess              *parser_process = nullptr;
    QByteArray             site_parsers;

    void PrintPage(QFile *f, const QByteArray &page);
    QVector<QVector<QString>> ParseHTMLTable(QString page) const;
    QNetworkReply *SendRequest(const QString &err_msg, const QString &err_file, QUrl url, REQUEST t, void * const postData = nullptr);
    QNetworkReply *GoToParsers();
    bool Login(const QString login, const QString password);
    bool Download();
    bool Upload();
    bool RestoreBookmark();
    void DeleteHidden();
    bool isDownloaded();
    void GetProjectSettings();

public:
    explicit brend(QWidget *parent = nullptr);
    QByteArray Serialize();
    void Deserialize(QFile &f);

    enum STATE {
        STOP, PAUSE, RUN
    };

    static QString login, password, parser;
    static int pause_time;

    QString id = "", name = "";
    QString ident = "art", proc = "15", kol_dop = "0";

    QString disc = "1", min_price = "0", max_price = "999999", min_r = "0", max_r = "100";
    QString sort = "1", _r = "0", d1 = "1", d2 = "1", d3 = "1", d4 = "1", d5 = "1";
    QString price = "1", name_up = "1", rub = "0", isImg = "1", hideOther = "1";
    QString trWithoutIm = "1", onlyArt = "0", filtr = "1", id_site = "";

    QString parser_project = "";
    QString catalog = "", images = "", separator = "", container = "";
    int num_of_fields = 0;

    QWidget *w;
    QCheckBox *download_checkBox, *upload_checkBox;
    QLabel *id_label, *name_label;
    QPushButton *edit_button, *delete_button, *stop_button, *start_button;

    QProgressBar *edit_progress, *hide_progress, *upload_progress, *photo_progress, *total_progress;

    STATE isRunning = STOP;

public slots:
    void process();

signals:
    void edit();
    void del();
    void start();
    void pause();
    void stop();

    void log(const QString& text);
    void downloaded();

    void increase_edit_progress();
    void increase_hide_progress();
    void increase_upload_progress();
    void increase_photo_progress();
    void increase_total_progress();

    void set_maximum_edit_progress(int);
    void set_maximum_hide_progress(int);
    void set_maximum_upload_progress(int);
    void set_maximum_photo_progress(int);
    void set_maximum_total_progress(int);

    void set_format_edit_progress(const QString&);
    void set_format_hide_progress(const QString&);
    void set_format_upload_progress(const QString&);
    void set_format_photo_progress(const QString&);
    void set_format_total_progress(const QString&);
};
