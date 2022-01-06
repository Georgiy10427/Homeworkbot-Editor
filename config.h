#ifndef CONFIG_H
#define CONFIG_H

#include <QString>
#include <json.hpp>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <boost/dll/runtime_symbol_info.hpp>

using json = nlohmann::json;
using namespace std;

QString getProgramPath();

class Config
{
public:
    Config(QString path);
    string getServerUrl();
    string getToken();
    bool ignoreCertificate();
private:
    json config;
};

#endif // CONFIG_H
