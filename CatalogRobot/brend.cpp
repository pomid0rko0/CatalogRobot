#include "brend.h"
#include "addform.h"

const int         brend::max_tryings = 10;
const int         brend::timer_tick  = 60 * 60;
const QTextCodec *brend::cp1251      = QTextCodec::codecForName("Windows-1251");

const QVector<QString> brend::HTML_entities = { "&lt;"  , "&gt;"  , "&amp;"  , "&quot;",
                                                "&apos;", "&euro;", "&copy;", "&reg;" };
const QVector<QString> brend::symbols = { "<", ">", "&", "\"", "'", "€", "©", "®" };

int     brend::pause_time = 1000;
QString brend::login;
QString brend::password;
QString brend::parser;

QNetworkReply *brend::SendRequest(const QString &err_msg, const QString &err_file, QUrl url, REQUEST t, void * const postData)
{
    QNetworkReply *reply = nullptr;
    int status, tryings = 0;

    do {
        if (isRunning == STOP) {
            return nullptr;
        }
        QNetworkRequest request(url);
        emit log("<font size=\"4\" color=\"black\">Пауза " +
                 QString::number(pause_time + tryings) + " миллисекунд</font>");
        QThread::msleep(static_cast<unsigned long>(pause_time + tryings * 1000));
        tryings++;
        switch (t) {
        case GET:
            emit log("<font size=\"4\" color=\"black\">Выполняется get-запрос к " +
                     url.url() + "\nПопытка номер " + QString::number(tryings) + "</font>");
            reply = network->get(request);
            break;
        case POST:
            emit log("<font size=\"4\" color=\"black\">Выполняется простой post-запрос к " +
                     url.url() + "\nПопытка номер " + QString::number(tryings) + "</font>");
            request.setHeader(QNetworkRequest::KnownHeaders::ContentTypeHeader,
                              "application/x-www-form-urlencoded");
            reply = network->post(request, *static_cast<QByteArray*>(postData));
            break;
        case MULTIPART:
            emit log("<font size=\"4\" color=\"black\">Выполняется multipart-formdata post-запрос к " +
                     url.url() + "\nПопытка номер " + QString::number(tryings) + "</font>");
            reply = network->post(request, static_cast<QHttpMultiPart*>(postData));
            break;
        }
        barrier->exec();
        if (t == MULTIPART) {
            static_cast<QHttpMultiPart*>(postData)->setParent(reply);
        }

        status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        emit log("<font size=\"4\" color=\"black\">Запрос выполнен. Статус: " +
                 QString::number(status) + "</font>");
        // обработка перенаправления
        if (300 <= status && status <= 399) {
            tryings = 0;
            QUrl newUrl = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
            if (newUrl.isEmpty()) {
                emit log("<font color=\"red\">Перенаправление на пустой адрес</font>");
                reply->deleteLater();
                return nullptr;
            }
            if (newUrl == url) {
                emit log("<font color=\"red\">Бесконечное перенаправление на адрес " + url.url() + "</font>");
                reply->deleteLater();
                return nullptr;
            }
            url = newUrl;
            t = GET;
        }
        QThread::msleep(static_cast<unsigned long>(pause_time + tryings * 1000));
    } while ((status == 302 || status == 403 || status == 502 || status == 503) && tryings <= max_tryings);
    if (tryings > max_tryings) {
        emit log("<font color=\"red\">Превышено число попыток (" + QString::number(max_tryings) + ")</font>");
    }
    status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (status != 200) {
        emit log(err_msg);
        PrintPage(new QFile("./logs/" + id + "_" +
                            QDateTime::currentDateTime().toString().replace(' ', '_').replace(':', '.') +
                            "_" + id + "_" + err_file), reply->readAll());
        reply->deleteLater();
        return nullptr;
    }
    return reply;
}

QNetworkReply *brend::GoToParsers()
{
    auto reply = SendRequest("<font size=\"6\" color=\"red\">Не удалось перейти в магазин парсеров. "
                             "Остановка процесса обновления</font>",
                             "parser_shop.html", parser_shop_page(), GET);
    if (reply == nullptr) {
        return nullptr;
    }
    reply->deleteLater();
    return SendRequest("<font size=\"6\" color=\"red\">Не удалось перейти в ваши парсеров. "
                       "Остановка процесса обновления</font>",
                       "your_parsers.html", parser_page(), GET);
}

QString brend::login_page() const
{
    return "https://kuz-sp.ru/site/login/";
}

QString brend::brend_page(const int page) const
{
    return "https://kuz-sp.ru/org/catalog/index/brendId/" + id + "/?page=" + QString::number(page);
}

QString brend::brend_imageless_page(const int page) const
{
    return "https://kuz-sp.ru/org/catalog/index/brendId/" + id + "/img/" + QString::number(page);
}

QString brend::brend_hidden_page(const int page) const
{
    return "https://kuz-sp.ru/org/catalog/index/brendId/" + id + "/hide/" + QString::number(page);
}

QString brend::hide_item_page(const QString &item_id) const
{
    return "https://kuz-sp.ru/org/catalog/hide/id/" + item_id + "/active/CatalogActive";
}

QString brend::delete_hidden_page() const
{
    return "https://kuz-sp.ru/org/catalog/delAllHided/brendId/" + id;
}

QString brend::restore_item_page(const QString &item_id) const
{
    return "https://kuz-sp.ru/org/catalog/show/id/" + item_id + "/active/CatalogActive";
}

QString brend::parser_shop_page() const
{
    return "https://kuz-sp.ru/site/oauth/?url=%2Forg%2Fparser%2Fshop%2F";
}

QString brend::parser_page() const
{
    return "https://rf-sp.ru/parsers/";
}

QString brend::download_page(const QString &hash) const
{
    return "https://rf-sp.ru/org/parser/addQueue/id/" + id_site + "/hash/" + hash;
}

QString brend::upload_page(const QString &another_part_of_url) const
{
    return "https://rf-sp.ru/org/offers/transfer" + another_part_of_url;
}

QString brend::edit_page(const int column) const
{
    switch (column) {
    case 1:
        return "https://kuz-sp.ru/org/catalog/edit/active/CatalogActive/column/art/";
    case 2:
        return "https://kuz-sp.ru/org/catalog/edit/active/CatalogActive/column/name/";
    case 3:
        return "https://kuz-sp.ru/org/catalogAjax/edit/active/CatalogActive/column/price/";
    default:
        return "https://kuz-sp.ru/org/catalog/edit/active/CatalogActive/column/dop" + QString::number(column - 3) + "/";
    }
}

QString brend::add_page() const
{
    return "https://kuz-sp.ru/org/catalog/importCheck/brendId/" + id + "/newCatalog/1/";
}

QString brend::finish_page() const
{
    return "https://kuz-sp.ru/org/catalog/importFinish/";
}

QString brend::upload_photo_page(const QString &id) const
{
    return "https://kuz-sp.ru/org/catalog/saveImageForFile/id/" + id + "/active/CatalogActive/";
}

void brend::PrintPage(QFile *f, const QByteArray &page)
{
    f->remove();
    f->open(QFile::WriteOnly);
    f->write(page);
    emit log("Страница сохранена в файле " + f->fileName());
    f->close();
    delete f;
}

QVector<QVector<QString> > brend::ParseHTMLTable(QString page) const
{
    int i = page.indexOf("table_body", Qt::CaseInsensitive);
    page = page.mid(i, page.indexOf("</tbody>", Qt::CaseInsensitive) - i);
    QVector<QVector<QString>> res;

    QRegularExpression re_rows("<tr>(.*)</tr>",
                               QRegularExpression::InvertedGreedinessOption |
                               QRegularExpression::CaseInsensitiveOption |
                               QRegularExpression::DotMatchesEverythingOption);
    QRegularExpression re_cols("<td>(.*)</td>",
                               QRegularExpression::InvertedGreedinessOption |
                               QRegularExpression::CaseInsensitiveOption |
                               QRegularExpression::DotMatchesEverythingOption);
    for (int i = 0; re_rows.match(page, i).hasMatch(); i = re_rows.match(page, i).capturedEnd()) {
        res.push_back(QVector<QString>());
        QString row = re_rows.match(page, i).captured(1);
        for (int j = 0; re_cols.match(row, j).hasMatch(); j = re_cols.match(row, j).capturedEnd()) {
            res.back().push_back(re_cols.match(row, j).captured(1));
            for (int k = 0; k < HTML_entities.size(); k++) {
                res.back().back().replace(HTML_entities[k], symbols[k], Qt::CaseInsensitive);
            }
        }
    }
    if (!res.isEmpty() && res.back().isEmpty()) {
        res.clear();
    }
    return res;
}

brend::brend(QWidget *parent) :
    QObject(parent)
{
    w = new QWidget;
    w->setFixedHeight(22);

    w->setLayout(new QHBoxLayout);
    w->layout()->setSpacing(5);
    w->layout()->setContentsMargins(5, 0, 5, 0);

    download_checkBox = new QCheckBox;
    download_checkBox->setFixedSize(20, 20);
    download_checkBox->setText("");
    w->layout()->addWidget(download_checkBox);

    upload_checkBox = new QCheckBox;
    upload_checkBox->setFixedSize(20, 20);
    upload_checkBox->setText("");
    w->layout()->addWidget(upload_checkBox);

    id_label = new QLabel;
    id_label->setFixedWidth(42);
    w->layout()->addWidget(id_label);

    name_label = new QLabel;
    w->layout()->addWidget(name_label);

    delete_button = new QPushButton;
    delete_button->setFixedSize(22, 22);
    delete_button->setIcon(QIcon(":/res/delete.png"));
    connect(delete_button, &QPushButton::clicked, [&]() {
        emit del();
    });
    w->layout()->addWidget(delete_button);

    edit_button = new QPushButton;
    edit_button->setFixedSize(22, 22);
    edit_button->setIcon(QIcon(":/res/edit.png"));
    connect(edit_button, &QPushButton::clicked, [&]() {
        emit edit();
    });
    w->layout()->addWidget(edit_button);

    stop_button = new QPushButton;
    stop_button->setFixedSize(22, 22);
    stop_button->setIcon(QIcon(":/res/stop.png"));
    stop_button->setDisabled(true);
    connect(stop_button, &QPushButton::clicked, [&]() {
        emit stop();
    });
    w->layout()->addWidget(stop_button);

    start_button = new QPushButton;
    start_button->setFixedSize(22, 22);
    start_button->setIcon(QIcon(":/res/start.png"));
    connect(start_button, &QPushButton::clicked, [&]() {
        emit start();
    });
    w->layout()->addWidget(start_button);

    network = new QNetworkAccessManager(this);
    barrier = new QEventLoop(this);

    connect(network, SIGNAL(finished(QNetworkReply*)), barrier, SLOT(quit()));
}

QByteArray brend::Serialize()
{
    return (id             + "\n" + \
            name           + "\n" + \
            disc           + "\n" + \
            min_price      + "\n" + \
            max_price      + "\n" + \
            min_r          + "\n" + \
            max_r          + "\n" + \
            sort           + "\n" + \
            _r             + "\n" + \
            d1             + "\n" + \
            d2             + "\n" + \
            d3             + "\n" + \
            d4             + "\n" + \
            d5             + "\n" + \
            price          + "\n" + \
            name_up        + "\n" + \
            rub            + "\n" + \
            isImg          + "\n" + \
            hideOther      + "\n" + \
            trWithoutIm    + "\n" + \
            onlyArt        + "\n" + \
            filtr          + "\n" + \
            id_site        + "\n" + \
            parser_project + "\n" + \
            proc           + "\n" + \
            kol_dop        + "\n" + \
            ident          + "\n" + \
            (download_checkBox->isChecked() ? "1" : "0") + "\n" + \
            (upload_checkBox->isChecked()   ? "1" : "0")).toUtf8();
}

void brend::Deserialize(QFile &f)
{
    id               = f.readLine(INT16_MAX); id            .remove('\n');
    name             = f.readLine(INT16_MAX); name          .remove('\n');
    disc             = f.readLine(INT16_MAX); disc          .remove('\n');
    min_price        = f.readLine(INT16_MAX); min_price     .remove('\n');
    max_price        = f.readLine(INT16_MAX); max_price     .remove('\n');
    min_r            = f.readLine(INT16_MAX); min_r         .remove('\n');
    max_r            = f.readLine(INT16_MAX); max_r         .remove('\n');
    sort             = f.readLine(INT16_MAX); sort          .remove('\n');
    _r               = f.readLine(INT16_MAX); _r            .remove('\n');
    d1               = f.readLine(INT16_MAX); d1            .remove('\n');
    d2               = f.readLine(INT16_MAX); d2            .remove('\n');
    d3               = f.readLine(INT16_MAX); d3            .remove('\n');
    d4               = f.readLine(INT16_MAX); d4            .remove('\n');
    d5               = f.readLine(INT16_MAX); d5            .remove('\n');
    price            = f.readLine(INT16_MAX); price         .remove('\n');
    name_up          = f.readLine(INT16_MAX); name_up       .remove('\n');
    rub              = f.readLine(INT16_MAX); rub           .remove('\n');
    isImg            = f.readLine(INT16_MAX); isImg         .remove('\n');
    hideOther        = f.readLine(INT16_MAX); hideOther     .remove('\n');
    trWithoutIm      = f.readLine(INT16_MAX); trWithoutIm   .remove('\n');
    onlyArt          = f.readLine(INT16_MAX); onlyArt       .remove('\n');
    filtr            = f.readLine(INT16_MAX); filtr         .remove('\n');
    id_site          = f.readLine(INT16_MAX); id_site       .remove('\n');
    parser_project   = f.readLine(INT16_MAX); parser_project.remove('\n');
    proc             = f.readLine(INT16_MAX); proc          .remove('\n');
    kol_dop          = f.readLine(INT16_MAX); kol_dop       .remove('\n');
    ident            = f.readLine(INT16_MAX); ident         .remove('\n');
    QString download = f.readLine(INT16_MAX); download      .remove('\n');
    QString upload   = f.readLine(INT16_MAX); upload        .remove('\n');

    id_label->setText(id);
    name_label->setText(name);
    download_checkBox->setChecked(download == "1");
    upload_checkBox->setChecked(upload == "1");
}

bool brend::Login(const QString login, const QString password)
{
    QUrlQuery data;
    data.addQueryItem("LoginForm[username]", login);
    data.addQueryItem("LoginForm[password]", password);
    data.addQueryItem("LoginForm[rememberMe]", "1");
    QByteArray postData(data.toString(QUrl::FullyEncoded).toUtf8());

    auto reply = SendRequest("<font size=\"6\" color=\"red\">Не удалось выполнить вход. "
                             "Остановка процесса обновления</font>",
                             "login.html", login_page(), POST, static_cast<void*>(&postData));
    if (reply != nullptr) {
        QByteArray page = reply->readAll();
        reply->deleteLater();
        if (page.contains("logout")) {
            emit log("<font size=\"6\" color=\"green\">Вход выполнен успешно</font>");
            return true;
        }
        else {
            emit log("<font size=\"6\" color=\"red\">Вход не выполнен. Остановка процесса обновления</font>");
            PrintPage(new QFile("./logs/login.html"), page);
        }
    }
    return false;

}

bool brend::Download()
{
    if (parser_project.isEmpty()) {
        auto reply = GoToParsers();
        if (reply == nullptr) {
            return false;
        }
        site_parsers = reply->readAll();
        reply->deleteLater();
        int i = site_parsers.indexOf(id_site);
        i = site_parsers.indexOf("Обновить", i);
        i = site_parsers.lastIndexOf("/hash/", i) + QString("/hash/").length();
        QString url = download_page(site_parsers.mid(i, site_parsers.indexOf('"', i) - i));
        reply = SendRequest("<font size=\"6\" color=\"red\">Не удалось запустить встройку. "
                            "Остановка процесса обновления</font>",
                            "download.html", url, GET);
        if  (reply == nullptr) {
            return false;
        }
        reply->deleteLater();
        do {
            if (isRunning == STOP) {
                return false;
            }
            emit log("<font size=\"5\" color=\"green\">Ожидание загрузки обновления</font>");
            QThread::sleep(timer_tick);
        } while (!isDownloaded());
    }
    else {
        GetProjectSettings();

        QDir(catalog).removeRecursively();

        parser_process = new QProcess(this);
        parser_process->start(parser, { parser_project, "scan", "minimize", "close" });
        emit log("<font size=\"4\" color=\"black\">Парсер запущен; PID процесса:" +
                 QString::number(parser_process->processId()) + "</font>");
        emit log("<font size=\"5\" color=\"green\">Ожидание загрузки обновления</font>");
        while (!parser_process->waitForFinished(-1)) {
            if (isRunning == STOP) {
                return false;
            }
        }
    }
    emit log("<font size=\"6\" color=\"green\">Обновление загружено</font>");
    return true;
}

bool brend::Upload()
{
    QByteArray page;
    if (parser_project.isEmpty()) {
        if (site_parsers.isEmpty()) {
            auto reply = GoToParsers();
            if (reply == nullptr || isRunning == STOP) {
                return false;
            }
            site_parsers = reply->readAll();
            reply->deleteLater();
        }

        int i = site_parsers.indexOf(id_site);
        i = site_parsers.indexOf("Скопировать", i);
        i = site_parsers.lastIndexOf("/cityTarget/", i);
        QString url = upload_page(site_parsers.mid(i, site_parsers.indexOf('"', i) - i));

        QHttpMultiPart *data = new QHttpMultiPart(QHttpMultiPart::ContentType::FormDataType);

        QHttpPart _disc;
        _disc.setHeader(QNetworkRequest::KnownHeaders::ContentDispositionHeader,
                        "form-data; name=\"disc\"");
        _disc.setBody(disc.toUtf8());
        data->append(_disc);

        QHttpPart _min_price;
        _min_price.setHeader(QNetworkRequest::KnownHeaders::ContentDispositionHeader,
                             "form-data; name=\"min_price\"");
        _min_price.setBody(min_price.toUtf8());
        data->append(_min_price);

        QHttpPart _max_price;
        _max_price.setHeader(QNetworkRequest::KnownHeaders::ContentDispositionHeader,
                             "form-data; name=\"max_price\"");
        _max_price.setBody(max_price.toUtf8());
        data->append(_max_price);

        QHttpPart _min_r;
        _min_r.setHeader(QNetworkRequest::KnownHeaders::ContentDispositionHeader,
                         "form-data; name=\"min_r\"");
        _min_r.setBody(min_r.toUtf8());
        data->append(_min_r);

        QHttpPart _max_r;
        _max_r.setHeader(QNetworkRequest::KnownHeaders::ContentDispositionHeader,
                         "form-data; name=\"max_r\"");
        _max_r.setBody(max_r.toUtf8());
        data->append(_max_r);

        QHttpPart _sort;
        _sort.setHeader(QNetworkRequest::KnownHeaders::ContentDispositionHeader,
                        "form-data; name=\"sort\"");
        _sort.setBody(sort.toUtf8());
        data->append(_sort);

        QHttpPart __r;
        __r.setHeader(QNetworkRequest::KnownHeaders::ContentDispositionHeader,
                      "form-data; name=\"_r\"");
        __r.setBody(_r.toUtf8());
        data->append(__r);

        QHttpPart _d1;
        _d1.setHeader(QNetworkRequest::KnownHeaders::ContentDispositionHeader,
                      "form-data; name=\"d1\"");
        _d1.setBody(d1.toUtf8());
        data->append(_d1);

        QHttpPart _d2;
        _d2.setHeader(QNetworkRequest::KnownHeaders::ContentDispositionHeader,
                      "form-data; name=\"d2\"");
        _d2.setBody(d2.toUtf8());
        data->append(_d2);

        QHttpPart _d3;
        _d3.setHeader(QNetworkRequest::KnownHeaders::ContentDispositionHeader,
                      "form-data; name=\"d3\"");
        _d3.setBody(d3.toUtf8());
        data->append(_d3);

        QHttpPart _d4;
        _d4.setHeader(QNetworkRequest::KnownHeaders::ContentDispositionHeader,
                      "form-data; name=\"d4\"");
        _d4.setBody(d4.toUtf8());
        data->append(_d4);

        QHttpPart _d5;
        _d5.setHeader(QNetworkRequest::KnownHeaders::ContentDispositionHeader,
                      "form-data; name=\"d5\"");
        _d5.setBody(d5.toUtf8());
        data->append(_d5);

        QHttpPart _price;
        _price.setHeader(QNetworkRequest::KnownHeaders::ContentDispositionHeader,
                         "form-data; name=\"price\"");
        _price.setBody(price.toUtf8());
        data->append(_price);

        QHttpPart _name_up;
        _name_up.setHeader(QNetworkRequest::KnownHeaders::ContentDispositionHeader,
                           "form-data; name=\"name_up\"");
        _name_up.setBody(name_up.toUtf8());
        data->append(_name_up);

        QHttpPart _rub;
        _rub.setHeader(QNetworkRequest::KnownHeaders::ContentDispositionHeader,
                       "form-data; name=\"rub\"");
        _rub.setBody(rub.toUtf8());
        data->append(_rub);

        QHttpPart _isImg;
        _isImg.setHeader(QNetworkRequest::KnownHeaders::ContentDispositionHeader,
                         "form-data; name=\"isImg\"");
        _isImg.setBody(isImg.toUtf8());
        data->append(_isImg);

        QHttpPart _hideOther;
        _hideOther.setHeader(QNetworkRequest::KnownHeaders::ContentDispositionHeader,
                             "form-data; name=\"hideOther\"");
        _hideOther.setBody(hideOther.toUtf8());
        data->append(_hideOther);

        QHttpPart _trWithoutIm;
        _trWithoutIm.setHeader(QNetworkRequest::KnownHeaders::ContentDispositionHeader,
                               "form-data; name=\"trWithoutIm\"");
        _trWithoutIm.setBody(trWithoutIm.toUtf8());
        data->append(_trWithoutIm);

        QHttpPart _onlyArt;
        _onlyArt.setHeader(QNetworkRequest::KnownHeaders::ContentDispositionHeader,
                           "form-data; name=\"onlyArt\"");
        _onlyArt.setBody(onlyArt.toUtf8());
        data->append(_onlyArt);

        QHttpPart _filtr;
        _filtr.setHeader(QNetworkRequest::KnownHeaders::ContentDispositionHeader,
                         "form-data; name=\"filtr\"");
        _filtr.setBody(filtr.toUtf8());
        data->append(_filtr);

        auto reply = SendRequest("<font size=\"6\" color=\"red\">Не удалось скопировать встройку. "
                                 "Остановка процесса обновления</font>",
                                 "upload.html", url, MULTIPART, data);
        if (reply == nullptr || isRunning == STOP) {
            return false;
        }
        emit set_format_upload_progress("Товаров обновлено: %v00");
        do {
            page = reply->readAll();
            reply->deleteLater();
            i = page.indexOf("/cityTarget/");
            url = upload_page(page.mid(i, page.indexOf('"', i) - i));
            reply = SendRequest("<font size=\"6\" color=\"red\">Не удалось скопировать встройку. "
                                "Остановка процесса обновления</font>",
                                "upload.html", url, GET);
            if (reply == nullptr || isRunning == STOP) {
                return false;
            }
            reply->deleteLater();
            emit set_maximum_upload_progress(upload_progress->value() + 1);
            emit increase_upload_progress();
        } while(i >= 0 && !reply->url().url().contains("kuz-sp"));
    }
    else {
        if (!download_checkBox->isChecked()) {
            GetProjectSettings();
        }

        QHash<QString, QVector<QString>> new_catalog;
        emit log("<font size=\"4\" color=\"green\">Получение нового каталога</font>");
        auto files = QDir(catalog).entryInfoList({ "*.csv"},
                                                 QDir::Files |
                                                 QDir::NoSymLinks |
                                                 QDir::NoDotAndDotDot |
                                                 QDir::Readable);
        QRegularExpression re_cols(container + "(.+)" + container + "(" + separator + "|\r\n)",
                                   QRegularExpression::InvertedGreedinessOption |
                                   QRegularExpression::CaseInsensitiveOption |
                                   QRegularExpression::DotMatchesEverythingOption);
        for (auto &el : files) {
            if (isRunning == STOP) {
                return false;
            }
            QFile f(el.filePath());
            f.open(QFile::ReadOnly);
            QString file = cp1251->toUnicode(f.readAll());
            f.close();

            QVector<QString> item;
            for (int i = 0; re_cols.match(file, i).hasMatch(); i = re_cols.match(file, i).capturedEnd()) {
                item.push_back(re_cols.match(file, i).captured(1));
                for (int k = 0; k < HTML_entities.size(); k++) {
                    item.back().replace(HTML_entities[k], symbols[k], Qt::CaseInsensitive);
                }
                item.back().replace("+", " ");
                if (item.size() == num_of_fields) {
                    new_catalog.insert(item[ident == "art" ? 0 : 1], item);
                    item.clear();
                }
            }
        }
        emit log("<font size=\"5\" color=\"green\">Новый каталог получен. Количество товаров: " +
                 QString::number(new_catalog.size()) + "</font>");

        if (new_catalog.isEmpty()) {
            emit log("<font size=\"6\" color=\"red\">Новый каталог пуст. "
                     "Проверьте правильность работы парсера. Остановка обновления</font>");
            return false;
        }

        emit log("<font size=\"4\" color=\"green\">Ищем все изменения</font>");
        QVector<QString> to_hide;
        QVector<QPair<QString, QPair<QString, int>>> to_edit;
        emit log("<font size=\"4\" color=\"green\">Получение текущего каталога</font>");
        for (int i = 1; i <= 1000; i++) {
            auto reply = SendRequest("<font color=\"red\">Не удалось загрузить страницу " +
                                     QString::number(i) + " каталога</font>",
                                     "brend_page_" + QString::number(i) + ".html",
                                     brend_page(i), GET);
            if (reply == nullptr || isRunning == STOP) {
                emit log("<font size=\"6\" color=\"red\">Не удалось получить текущий каталог. "
                         "Остановка обновления</font>");
                return false;
            }
            auto t = ParseHTMLTable(reply->readAll());
            reply->deleteLater();
            if (t.isEmpty()) {
                break;
            }
            for (auto &e : t) {
                if (isRunning == STOP) {
                    return false;
                }
                // если сортировка == 2, то это закладка и с ней ничего не делаем
                if (e[11] == "2") {
                    continue;
                }
                // если нет картинки, то помечаем товар как товар для удаления
                if (e[1].contains("/images/nofoto.png") || e[1].contains("0.0.0.0")) {
                    to_hide.push_back(e[0]);
                }
                else {
                    e.remove(1); // удалить картинку
                    e.remove(4); // удалить цену орга
                    // e выглядит вот так:
                    // id_товара \ артикул \ название \ цена прайс \ дополнительные столбцы

                    // удалить все лишние столбцы
                    e.resize(4 + kol_dop.toInt());
                    // если такой товар есть, тогда проверить его поля на необходимость обновления
                    // иначе скрыть его
                    auto key = e[ident == "art" ? 1 : 2];
                    if (new_catalog.contains(key)) {
                        auto &new_r = new_catalog[key];
                        for (int j = 1; j < e.size(); j++) {
                            if (isRunning == STOP) {
                                return false;
                            }
                            if (e[j] != new_r[j - 1]) {
                                qDebug() << "id : " << e[0];
                                qDebug() << "col: " << j;
                                qDebug() << "old: " << e[j];
                                qDebug() << "new: " << new_r[j - 1];
                                qDebug() << "------------------------";
                                to_edit.push_back({ e[0], { new_r[j - 1], j }});
                            }
                        }
                        new_catalog.remove(key);
                    }
                    else {
                        to_hide.push_back(e[0]);
                    }
                }
            }
        }

        int k = 0;
        QString write_data;
        for (auto &item : new_catalog) {
            if (isRunning == STOP) {
                return false;
            }
            if (k % 2000 == 0 && !write_data.isNull()) {
                QFile f(catalog + QString::number(k / 2000) + ".csv");
                f.open(QFile::WriteOnly);
                f.write(cp1251->fromUnicode(write_data));
                f.close();
                write_data.clear();
            }
            for (int j = 0; j < item.size(); j++) {
                write_data += container + item[j] + container + separator;
            }
            write_data += "\r\n";
            k++;
            // удалим все ненужные поля, оставив только названия фотографий
            item[0] = item[3 + kol_dop.toInt()];
            item.resize(1);
        }
        QFile f(catalog + QString::number((k + 1999) / 2000) + ".csv");
        f.open(QFile::WriteOnly);
        f.write(cp1251->fromUnicode(write_data));
        f.close();

        emit log("<font size=\"5\" color=\"green\">Товаров для скрытия: " + QString::number(to_hide.size()) + "</font>");
        emit log("<font size=\"5\" color=\"green\">Полей для обновления: " + QString::number(to_edit.size()) + "</font>");
        emit log("<font size=\"5\" color=\"green\">Товаров для добавления: " + QString::number(new_catalog.size()) + "</font>");
        if (to_edit.isEmpty()) {
            emit set_format_edit_progress("Нечего редактировать");
            emit set_maximum_edit_progress(1);
        }
        else {
            emit set_format_edit_progress("Редактировано полей: %v/%m");
            emit set_maximum_edit_progress(to_edit.size());
        }
        if (to_hide.isEmpty()) {
            emit set_format_hide_progress("Нечего скрывать");
            emit set_maximum_hide_progress(1);
        }
        else {
            emit set_format_hide_progress("Скрыто товаров: %v/%m");
            emit set_maximum_hide_progress(to_hide.size());
        }
        if (new_catalog.isEmpty()) {
            emit set_format_upload_progress("Нет новых товаров для загрузки");
            emit set_maximum_upload_progress(1);
            emit set_format_photo_progress("Нет товаров без фото");
            emit set_maximum_photo_progress(1);
        }
        else {
            emit set_format_upload_progress("Загружено файлов csv: %v/%m");
            emit set_maximum_upload_progress((new_catalog.size() + 1999) / 2000);
            emit set_format_photo_progress("Загружено фотографий: %v/%m");
            emit set_maximum_photo_progress(new_catalog.size());
        }

        emit log("<font size=\"4\" color=\"green\">Редактирование полей</font>");
        for (auto &e : to_edit) {
            if (isRunning == STOP) {
                return false;
            }
            auto &item_id = e.first;
            auto &value = e.second.first;
            auto &n = e.second.second;
            emit log("<font size=\"4\" color=\"green\">Редактирование товара " + item_id + "</font>");
            emit log("<font size=\"4\" color=\"green\">Столбец номер " + QString::number(n) + "</font>");
            QUrlQuery data;
            data.addQueryItem("id", item_id);
            data.addQueryItem("value", value);
            QByteArray postData(data.toString(QUrl::FullyEncoded).toUtf8());

            auto reply = SendRequest("<font color=\"orange\">Не удалось обновить столбец номер " +
                                     QString::number(n) + " товара " + item_id + "</font>",
                                     "edit_column_" + QString::number(n) + "_item_id_" + item_id + ".html",
                                     edit_page(n), POST, static_cast<void*>(&postData));
            if (reply != nullptr) {
                if (reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() == 200) {
                    emit increase_edit_progress();
                }
                reply->deleteLater();
            }
        }

        emit log("<font size=\"4\" color=\"green\">Скрытие товаров</font>");
        for (auto &e : to_hide) {
            if (isRunning == STOP) {
                return false;
            }
            emit log("<font size=\"4\" color=\"green\">Скрытие товара " + e + "</font>");
            auto reply = SendRequest("<font color=\"orange\">Не удалось скрыть товар с id " + e + "</font>",
                                     "hide_item_id_" + e + ".html",
                                     hide_item_page(e), GET);
            if (reply != nullptr) {
                reply->deleteLater();
                emit increase_hide_progress();
            }
        }

        emit log("<font size=\"5\" color=\"green\">Загрузка новых товаров</font>");
        for (int i = 1; i <= (k + 1999) / 2000; i++) {
            if (isRunning == STOP) {
                return false;
            }
            f.setFileName(catalog + QString::number(i) + ".csv");
            QHttpMultiPart *upload_catalog = new QHttpMultiPart(QHttpMultiPart::ContentType::FormDataType);
            emit log("<font size=\"4\" color=\"green\">Загрузка файла " + f.fileName() + "</font>");

            QHttpPart csv;
            csv.setHeader(QNetworkRequest::KnownHeaders::ContentDispositionHeader,
                          "form-data; name=\"filename\"; filename=\"" + f.fileName() + "\"");
            f.open(QFile::ReadOnly);
            csv.setBodyDevice(&f);
            upload_catalog->append(csv);

            QHttpPart proc_part;
            proc_part.setHeader(QNetworkRequest::KnownHeaders::ContentDispositionHeader,
                                "form-data; name=\"proc\"");
            proc_part.setBody(proc.toUtf8());
            upload_catalog->append(proc_part);

            QHttpPart kol_dop_part;
            kol_dop_part.setHeader(QNetworkRequest::KnownHeaders::ContentDispositionHeader,
                                   "form-data; name=\"kol_dop\"");
            kol_dop_part.setBody(kol_dop.toUtf8());
            upload_catalog->append(kol_dop_part);

            auto reply = SendRequest("<font size=\"4\" color=\"orange\">Не удалось загрузить файл " +
                                     f.fileName() + "</font>",
                                     "upload_ " + f.fileName() + ".html",
                                     add_page(), MULTIPART, static_cast<void*>(upload_catalog));
            f.close();

            if (reply != nullptr) {
                upload_catalog->setParent(reply);
                emit log("<font size=\"4\" color=\"green\">Подтверждение загрузки файла " +
                         f.fileName() + "</font>");
                QByteArray page = reply->readAll();
                reply->deleteLater();
                int pos = page.indexOf("Finish/");
                pos = page.indexOf("value=\"", pos) + int(strlen("value=\""));
                QByteArray data = "data=" + page.mid(pos, page.indexOf('"', pos) - pos).toPercentEncoding();

                auto reply = SendRequest("<font size=\"4\" color=\"orange\">Не удалось подтвердить загрузку файла " +
                                         f.fileName() + "</font>",
                                         "finish_" + f.fileName() + ".html",
                                         finish_page(), POST, static_cast<void*>(&data));
                if (reply != nullptr) {
                    if (reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() == 200) {
                        emit increase_upload_progress();
                    }
                    reply->deleteLater();
                }
            }
        }

        emit log("<font size=\"5\" color=\"green\">Загрузка фотографий</font>");
        for (int i = 1; i <= (k + 99) / 100; i++) {
            if (isRunning == STOP) {
                return false;
            }
            auto reply = SendRequest("<font size=\"4\" color=\"orange\">Не удалось загрузить страницу " +
                                     QString::number(i) + " каталога</font>",
                                     "brend_page_" + QString::number(i) + ".html",
                                     brend_imageless_page(i), GET);
            if (reply == nullptr) {
                continue;
            }
            auto t = ParseHTMLTable(reply->readAll());
            reply->deleteLater();
            if (t.isEmpty()) {
                break;
            }
            for (auto &e : t) {
                if (isRunning == STOP) {
                    return false;
                }
                auto key = e[ident == "art" ? 2 : 3];
                emit log("<font size=\"4\" color=\"green\">Загрузка картинки в товар " + key + "</font>");
                if (!(new_catalog.contains(key) && QFile::exists(catalog + images + new_catalog[key][0]))) {
                    emit log("<font size=\"5\" color=\"orange\">Не найдена картинка для товара " + e[0] + "</font>");
                    continue;
                }
                QString imageName = new_catalog[key][0];
                QHttpMultiPart *image = new QHttpMultiPart(QHttpMultiPart::ContentType::FormDataType);
                QFile img(catalog + images + imageName, image);
                QHttpPart part;
                part.setHeader(QNetworkRequest::KnownHeaders::ContentDispositionHeader,
                               QVariant("Content-Disposition: form-data; name=\"file\"; filename=\"" + imageName + "\""));
                part.setHeader(QNetworkRequest::KnownHeaders::ContentTypeHeader,
                               QVariant("Content-Type: image/" + imageName.mid(imageName.lastIndexOf('.') + 1,
                                                                               imageName.size() - imageName.lastIndexOf('.'))));
                img.open(QIODevice::ReadOnly);
                part.setBodyDevice(&img);
                image->append(part);
                reply = SendRequest("<font color=\"orange\">Не удалось загрузить фотографию в товар " +
                                    e[0] + "</font>",
                        "upload_photo_id_" + e[0] + ".html",
                        upload_photo_page(e[0]), MULTIPART, static_cast<void*>(image));
                img.close();
                if (reply != nullptr) {
                    if (reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() == 200) {
                        emit increase_photo_progress();
                    }
                    reply->deleteLater();
                }
            }
        }
    }

    if (page.contains("Скрытых")) {
        emit log("<font size=\"6\" color=\"yellow\">Скрытых товаров больше, чем активных. "
                 "Проверьте правильность работы парсера или вручную удалите скрытые товары</font>");
    }
    emit log("<font size=\"4\" color=\"black\">Обновление загружено!</font>");
    return true;
}

bool brend::RestoreBookmark()
{
    QString bookmark_id;
    for (int i = 1; true; i++) {
        if (isRunning == STOP) {
            return false;
        }
        auto reply = SendRequest("<font size=\"6\" color=\"orange\">Не удалось восстановить закладку. "
                                 "Удаления скрытых товаров не будет</font>",
                                 "bookmark.html", brend_hidden_page(i), GET);
        if (reply == nullptr) {
            return false;
        }
        auto t = ParseHTMLTable(reply->readAll());
        reply->deleteLater();
        if (t.isEmpty()) {
            break;
        }
        for (auto &e : t) {
            if (isRunning == STOP) {
                return false;
            }
            if (e[e.size() - 3] == "2") {
                bookmark_id = e[0];
                break;
            }
        }
        if (!bookmark_id.isEmpty()) {
            break;
        }
    }
    if (bookmark_id.isEmpty()) {
        emit log("<font size=\"5\" color=\"green\">Закладка не найдена в скрытых товарах</font>");
    }
    else {
        auto reply = SendRequest("<font size=\"6\" color=\"orange\">Не удалось восстановить закладку. "
                                 "Удаления скрытых товаров не будет</font>",
                                 "bookmark.html", restore_item_page(bookmark_id), GET);
        if (reply == nullptr) {
            return false;
        }
        reply->deleteLater();
    }
    emit log("<font size=\"6\" color=\"green\">Закладка восстановлена</font>");
    return true;
}

void brend::DeleteHidden()
{
    auto reply = SendRequest("<font size=\"5\" color=\"yellow\">Не удалось загрузить страницу закупки. "
                             "Скрытые товары не были удалены</font>",
                             "hidden.html", brend_page(), GET);
    if (reply == nullptr) {
        return;
    }
    QByteArray page = reply->readAll();
    reply->deleteLater();
    if (page.contains("Скрытых")) {
        emit log("<font size=\"6\" color=\"yellow\">Скрытых товаров больше, чем активных. "
                 "Проверьте правильность работы парсера или вручную удалите скрытые товары</font>");
        return;
    }
    reply = SendRequest("<font size=\"6\" color=\"yellow\">Не удалось удалить скрытые товары</font>",
                        "hidden.html", delete_hidden_page(), GET);
    if (reply != nullptr) {
        reply->deleteLater();
    }
}

void brend::process()
{
    if (download_checkBox->isChecked() && upload_checkBox->isChecked()) {
        emit set_maximum_total_progress(6);
    }
    if (download_checkBox->isChecked() ^ upload_checkBox->isChecked()) {
        emit set_maximum_total_progress(5);
    }
    if (!(download_checkBox->isChecked() || upload_checkBox->isChecked())) {
        emit set_maximum_total_progress(4);
    }

    emit log("<font size=\"6\" color=\"green\">Обновление закупки запущено</font>");
    emit log("<font size=\"6\" color=\"green\">Выполняется вход</font>");
    if (!Login(login, password)) {
        emit stop();
        return;
    }
    emit increase_total_progress();

    emit log("<font size=\"6\" color=\"green\">Восстановление закладки</font>");
    if (RestoreBookmark()) {
        auto reply = SendRequest("<font size=\"6\" color=\"red\">Не удалось проверить количество скрытых товаров. "
                                 "Удаление скрытых товаров не будет</font>",
                                 "check_hidden.html", brend_page(), GET);
        if (reply != nullptr) {
            emit increase_total_progress();
            if (reply->readAll().contains("Скрытых")) {
                emit log("<font size=\"6\" color=\"yellow\">Скрытых товаров больше, чем активных. "
                         "Проверьте правильность работы парсера или вручную удалите скрытые товары</font>");
            }
            else {
                emit log("<font size=\"6\" color=\"green\">Удаление скрытых товаров</font>");
                DeleteHidden();
                emit increase_total_progress();
            }
            reply->deleteLater();
        }
    }
    if (download_checkBox->isChecked()) {
        emit log("<font size=\"6\" color=\"green\">Загрузка обновлений</font>");
        if (!Download()) {
            emit stop();
            return;
        }
        emit increase_total_progress();
    }
    if (upload_checkBox->isChecked()) {
        emit log("<font size=\"6\" color=\"green\">Обновление каталога</font>");
        if (Upload()) {
            emit increase_total_progress();
        }
    }
    emit log("<font size=\"6\" color=\"green\">Восстановление закладки</font>");
    if (RestoreBookmark()) {
        emit increase_total_progress();
    }
    emit log("<font size=\"6\" color=\"green\">Обновление завершено!</font>");
    emit stop();
}

bool brend::isDownloaded()
{
    emit log("Проверка на завершение загрузки обновлений");
    auto reply = GoToParsers();
    if (reply == nullptr) {
        return false;
    }
    QString page = reply->readAll();
    // найти строку, соответствующую закупку
    int i = page.indexOf(id_site);
    // выделить только строку
    page = page.mid(i, page.indexOf("</tr", i) - i);
    // ищем дату обновления черещ регулярное выражение
    i = page.indexOf(QRegularExpression("\\d\\d\\d\\d-\\d\\d-\\d\\d"));
    // выделяем дату
    QString date = page.mid(i, 10);
    // ищем время обновления черещ регулярное выражение
    i = page.indexOf(QRegularExpression("\\d\\d:\\d\\d:\\d\\d"), i);
    // выделяем время
    QString time = page.mid(i, 8);
    QDateTime download;
    download.setDate(QDate(date.mid(0, 4).toInt(), // год
                           date.mid(5, 2).toInt(), // месяц
                           date.mid(8, 2).toInt()  // день
                           )
                     );
    download.setTime(QTime(time.mid(0, 2).toInt(), // часы
                           time.mid(3, 2).toInt(), // минуты
                           time.mid(6, 2).toInt()  // секунды
                           )
                     );
    // перевод с Московского времени на Новосибирское
    download = download.addSecs(4 * 3600);
    //QDateTime::currentDateTime().timeZone()
    // download.setTimeZone()
    return QDateTime::currentSecsSinceEpoch() - download.toSecsSinceEpoch() < 60 * 60 * 24;
}

void brend::GetProjectSettings()
{
    QFile f(parser_project);
    f.open(QFile::ReadOnly);
    QByteArray d = f.readAll();
    f.close();
    // в файле проекта ищем папку, в которой хранится скачанный каталог
    int i = d.indexOf("<CD_PARSING_EDIT_7>");
    catalog = d.mid(i, d.indexOf("</CD_PARSING_EDIT_7>", i) - i);
    catalog = catalog.remove("<CD_PARSING_EDIT_7>").replace("\\\\", "/").replace("\\", "/");
    if (catalog.back() != '/') {
        catalog.push_back('/');
    }
    // в файле проекта ищем папку, в которой хранятся скачанные изображения
    i = d.indexOf("<CD_PARSING_EDIT_12>");
    images = d.mid(i, d.indexOf("</CD_PARSING_EDIT_12>", i) - i);
    images = images.remove("<CD_PARSING_EDIT_12>").replace("\\\\", "/").replace("\\", "/");
    if (images.back() != '/') {
        images.push_back('/');
    }
    // в файле проекта ищем разделитель ячее csv
    i = d.indexOf("<F82_E_1>");
    separator = d.mid(i, d.indexOf("</F82_E_1>", i) - i);
    separator = separator.remove("<F82_E_1>");
    // в файле проекта ищем контейнер ячеек csv
    i = d.indexOf("<F82_E_2>");
    container = d.mid(i, d.indexOf("</F82_E_2>", i) - i);
    container = container.remove("<F82_E_2>");

    // в файле проекта ищем количество полей в одной строке
    i = d.indexOf("<CD_PARSING_SYN_F46_1>");
    num_of_fields = d.mid(i, d.indexOf("</CD_PARSING_SYN_F46_1>") - i).count("[CSVCS]") + 1;
}
