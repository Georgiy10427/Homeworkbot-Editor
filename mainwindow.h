#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QMessageBox>
#include "client_https.hpp"
#include <json.hpp>
#include <vector>
#include <functional>
#include "config.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

using namespace std;
using HttpsClient = SimpleWeb::Client<SimpleWeb::HTTPS>;
using namespace std;
using json = nlohmann::json;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);

    map <int, string, greater<int>> get_notes();

    int addNote(string body);

    void deleteNote(int id);

    void create_sidebar_buttons(map <int, string, greater<int>> notes);

    void create_sidebar_button(QString label, int id, int index=-1);

    void showNote(int id);

    ~MainWindow();

private slots:
    void on_upload_clicked();

    void on_add_button_clicked();

    void on_delete_btn_clicked();

    void on_textEdit_textChanged();

    void on_search_textChanged(const QString &content);

    void on_clearSearchBar_released();

    void on_bold_clicked();

    void on_underline_clicked();

    void on_strikethrough_clicked();

    void closeEvent (QCloseEvent *event);

private:
    Ui::MainWindow *ui;

};
#endif // MAINWINDOW_H
