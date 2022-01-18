#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QCloseEvent>

QString article_sidebar_button_style = \
            "height: 23px; margin: 2px; border-radius: 3px; background-color: #2e2f30; "\
            "outline: none; color: white;";

map<int, QPushButton*> sidebar_buttons;
map<int, QPushButton*> search_results;

Config config (getProgramPath() + "config.json");
HttpsClient client(config.getServerUrl(), !config.ignoreCertificate());
string token = config.getToken();


void show_error_message(QString str)
{
    QMessageBox messageBox;
    messageBox.critical(0, "Error", str);
}

int currentNoteId = -1;

json send_request(string request_content)
{
    shared_ptr<HttpsClient::Response> response;
    string response_content;

    try {
        response = client.request("POST", "/request", request_content);
        stringstream ss;
        ss << response->content.rdbuf();
        response_content = ss.str();
    }  catch (const SimpleWeb::system_error &e) {
        stringstream ss;
        ss << "Client request error: " << e.what() << endl;
        show_error_message(QString::fromUtf8(ss.str().c_str()));
        throw ss.str();
    }

    return json::parse(response_content);
}

map <int, string, greater<int>> MainWindow::get_notes()
{
    json request;
    request["action"] = string("get");
    request["content-type"] = string("notes");
    request["number"] = string("all");

    json json_response = send_request(request.dump());
    map <int, string, greater<int>> notes;
    json_response.at("notes").get_to(notes);

    return notes;
}

string get_note(int id)
{
    json request;
    request["action"] = "get-by-id";
    request["content-type"] = "note";
    request["id"] = id;

    json json_response = send_request(request.dump());
    if(json_response["message"] == "ok")
    {
        return string(json_response["note"]);
    } else {
        stringstream ss;
        ss << "Response error: " << json_response["message"] << endl;
        show_error_message(QString::fromUtf8(ss.str().c_str()));
    }
    return string();
}

void editNote(int id, string newPost)
{
    json request;
    request["action"] = "edit";
    request["content-type"] = "note";
    request["id"] = id;
    request["new_note"] = newPost;
    request["token"] = token;

    json json_response = send_request(request.dump());
    if(json_response["message"] == "not_found")
    {
        show_error_message("Edit error: note not found.");;
    } else if (json_response["message"] == "access denied") {
        show_error_message("Edit error: access denied.");
    }
}

int MainWindow::addNote(string text)
{
    json request;
    request["action"] = "add";
    request["content-type"] = "note";
    request["note_text"] = text;
    request["token"] = token;

    json json_response = send_request(request.dump());
    int id = -1;
    if(json_response["message"] == "ok")
    {
        id = json_response["id"];
    }
    else
    {
        show_error_message("Error: access denied");
    }

    create_sidebar_button("Новая запись...", id, 0);
    return id;
}

void MainWindow::deleteNote(int id)
{
    json request;
    request["action"] = "delete";
    request["content-type"] = "note";
    request["id"] = id;
    request["token"] = token;

    json response = send_request(request.dump());
    if(response["message"] == "access denied")
    {
        show_error_message("Error: access denied.");
    }

    for (auto it = sidebar_buttons.begin(); it != sidebar_buttons.end(); ++it)
    {
        if (it->first == id)
        {
            it->second->hide();
            delete it->second;
            sidebar_buttons.erase(id);
            return;
        }
    }
}

string getNoteTitle(string content)
{
    size_t npos = content.find("\n");
    if(int(npos) == -1)
    {
        npos = content.find(" ");
        if (int(npos) == -1)
        {
            npos = 9;
        }
    }
    return content.substr(0, npos);
}

void MainWindow::create_sidebar_buttons(map <int, string, greater<int>> notes)
{
    for (auto it = notes.begin(); it != notes.end(); ++it)
    {
        create_sidebar_button(QString::fromUtf8(getNoteTitle(it->second).c_str()),
                              it->first);
    }
}

void MainWindow::create_sidebar_button(QString label, int id, int index)
{
    QPushButton *btn = new QPushButton(this);
    btn->setText(label);
    btn->setStyleSheet(article_sidebar_button_style);
    if (index == -1){
        ui->sidebar_articles->addWidget(btn);
    } else {
        ui->sidebar_articles->insertWidget(0, btn);
    }
    connect(btn, &QPushButton::clicked, this,
            [this, id](){ showNote(id); });
    sidebar_buttons.insert(pair<int, QPushButton*>(id, btn));
}


void MainWindow::showNote(int id)
{
    if (currentNoteId == -1 and ui->textEdit->toPlainText() == "")
    {
        ui->textEdit->setText(QString::fromUtf8(
                                  get_note(id).c_str())
                              );
        currentNoteId = id;
    }
    else if (currentNoteId == -1 and ui->textEdit->toPlainText() != "")
    {
        int ret = QMessageBox::warning(this, tr("Предупреждение"), "Сохранить запись?",
                                       QMessageBox::Save | QMessageBox::No | QMessageBox::Cancel,
                                       QMessageBox::Save);
        switch (ret)
        {
            case QMessageBox::Save:
                addNote(ui->textEdit->toPlainText().toStdString());
                break;
            case QMessageBox::Cancel:
                return;
        }
        currentNoteId = id;
        ui->textEdit->setText(QString::fromUtf8(
                              get_note(id).c_str()));

    }
    else if (id == currentNoteId and \
             get_note(id) != ui->textEdit->toPlainText().toStdString()){

        int ret = QMessageBox::warning(this, tr("Предупреждение"), "Сохранить изменения в записи?",
                                       QMessageBox::Save | QMessageBox::No | QMessageBox::Cancel,
                                       QMessageBox::Save);
        switch (ret)
        {
            case QMessageBox::Save:
                editNote(id, ui->textEdit->toPlainText().toStdString());
                break;
            case QMessageBox::Cancel:
                return;
        }
        currentNoteId = id;
        ui->textEdit->setText(QString::fromUtf8(
                                  get_note(id).c_str()));
    }
    else if (id != currentNoteId and get_note(currentNoteId)
             != ui->textEdit->toPlainText().toStdString())
    {
        int ret = QMessageBox::warning(this, tr("Предупреждение"), "Сохранить изменения в записи?",
                                       QMessageBox::Save | QMessageBox::No | QMessageBox::Cancel,
                                       QMessageBox::Save);
        switch (ret)
        {
            case QMessageBox::Save:
                editNote(currentNoteId, ui->textEdit->toPlainText().toStdString());
                break;
            case QMessageBox::Cancel:
                return;
        }
        currentNoteId = id;
        ui->textEdit->setText(QString::fromUtf8(
                                  get_note(id).c_str()));
    }
    else if (id != currentNoteId and
             get_note(currentNoteId) == ui->textEdit->toPlainText().toStdString())
    {
        currentNoteId = id;
        ui->textEdit->setText(QString::fromUtf8(
                                  get_note(id).c_str()));
    }

}

void set_sidebar_buttons_visible(bool visible)
{
    for (auto it = sidebar_buttons.begin(); it != sidebar_buttons.end(); ++it)
    {
        if (visible)
        {
            it->second->show();
        } else {
            it->second->hide();
        }
    }
}

map <int, string, greater<int>> find_notes(string key)
{
    json request;
    request["action"] = "find";
    request["content-type"] = "notes";
    request["key"] = key;

    json json_response = send_request(request.dump());
    map <int, string, greater<int>> notes;
    json_response.at("notes").get_to(notes);

    return notes;
}

void showFoundNotes(map<int, string, greater<int>> foundNotes)
{
    for(auto i = sidebar_buttons.begin(); i != sidebar_buttons.end(); i++)
    {
        for(auto j = foundNotes.begin(); j != foundNotes.end(); j++)
        {
            if(j->first == i->first)
            {
                i->second->show();
            }
        }
    }
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setWindowTitle(" ");
    ui->clearSearchBar->hide();
    create_sidebar_buttons(get_notes());
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_upload_clicked()
{
    string newNote = ui->textEdit->toPlainText().toStdString();
    if(currentNoteId < 0)
    {
        addNote(newNote);
    } else {
        editNote(currentNoteId, newNote);
    }
    statusBar()->showMessage("Выполнено", 2500);
}


void MainWindow::on_add_button_clicked()
{
    int id = addNote("Новая запись...\n");
    showNote(id);
    statusBar()->showMessage("Создана запись", 1000);
}


void MainWindow::on_delete_btn_clicked()
{
    if(currentNoteId == -1) return;
    int ret = QMessageBox::warning(this, tr("Предупреждение"), "Удалить запись?",
                                   QMessageBox::Yes | QMessageBox::No,
                                   QMessageBox::No);
    switch(ret)
    {
        case QMessageBox::Yes:
            deleteNote(currentNoteId);
            currentNoteId = -1;
            ui->textEdit->setText("");
            statusBar()->showMessage("Запись удалена", 1000);
        case QMessageBox::No:
            break;
    }
}

void MainWindow::on_textEdit_textChanged()
{
    if (currentNoteId != -1)
    {
        sidebar_buttons[currentNoteId]->setText(
                    QString::fromUtf8(
                        getNoteTitle(ui->textEdit->toPlainText().toStdString()).c_str()
                    )
        );
    }
}

void MainWindow::on_search_textChanged(const QString &content)
{
    if(content != "")
    {
        set_sidebar_buttons_visible(false);
        ui->clearSearchBar->show();
        showFoundNotes(find_notes(content.toStdString()));
    } else {
        set_sidebar_buttons_visible(true);
        ui->clearSearchBar->hide();
    }
}

void MainWindow::on_clearSearchBar_released()
{
    ui->search->clear();
}

void MainWindow::on_bold_clicked()
{
    QTextCharFormat format;
    QTextCursor cursor = ui->textEdit->textCursor();

    if (!cursor.hasSelection())
    {
        cursor.select(QTextCursor::WordUnderCursor);
    }
    if(cursor.charFormat().fontWeight() == QFont::Bold)
    {
        format.setFontWeight(QFont::Normal);
    } else {
        format.setFontWeight(QFont::Bold);
    }

    cursor.mergeCharFormat(format);
    ui->textEdit->mergeCurrentCharFormat(format);
}

void MainWindow::on_underline_clicked()
{
    QTextCharFormat format;
    QTextCursor cursor = ui->textEdit->textCursor();
    if(!cursor.hasSelection())
    {
        cursor.select(QTextCursor::WordUnderCursor);
    }
    format.setFontUnderline(!cursor.charFormat().fontUnderline());
    cursor.mergeCharFormat(format);
    ui->textEdit->mergeCurrentCharFormat(format);
}

void MainWindow::on_strikethrough_clicked()
{
    QTextCharFormat format;
    QTextCursor cursor = ui->textEdit->textCursor();
    if(!cursor.hasSelection())
    {
        cursor.select(QTextCursor::WordUnderCursor);
    }
    format.setFontStrikeOut(!cursor.charFormat().fontStrikeOut());
    cursor.mergeCharFormat(format);
    ui->textEdit->mergeCurrentCharFormat(format);
}


void MainWindow::closeEvent (QCloseEvent *event)
{
    if(currentNoteId == -1 and ui->textEdit->toPlainText() == "") event->accept();
    else if(currentNoteId == -1 and ui->textEdit->toPlainText() != "")
    {
        int ret = QMessageBox::warning(this, "Предупреждение", "Сохранить запись?",
                                       QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel,
                                       QMessageBox::Cancel);
        switch(ret)
        {
            case QMessageBox::Yes:
                addNote(ui->textEdit->toPlainText().toStdString());
                event->accept();
                break;

            case QMessageBox::No:
                event->accept();
                break;

            case QMessageBox::Cancel:
                event->ignore();
                break;
        }
    }
    else if(get_note(currentNoteId) != ui->textEdit->toPlainText().toStdString())
    {
        int ret = QMessageBox::warning(this, "Предупреждение", "Сохранить запись?",
                                       QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel,
                                       QMessageBox::Cancel);
        switch(ret)
        {
            case QMessageBox::Yes:
                editNote(currentNoteId, ui->textEdit->toPlainText().toStdString());
                event->accept();
                break;

            case QMessageBox::No:
                event->accept();
                break;

            case QMessageBox::Cancel:
                event->ignore();
                break;
        }
    }
}
