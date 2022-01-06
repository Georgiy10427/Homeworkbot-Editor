#include "config.h"

Config::Config(QString path)
{
    QFile config_file(path);
    if(!config_file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug() << "Invalid path: " << path << endl;
        throw -1;
    }

    QString config_content = QTextStream(&config_file).readAll();
    this->config = json::parse(config_content.toStdString());
}

string Config::getServerUrl()
{
    return this->config["server_url"];
}

string Config::getToken()
{
    return this->config["token"];
}

bool Config::ignoreCertificate()
{
    return this->config["ignore_ssl_certificate"];
}

std::string getExecutablePath() {
    return boost::dll::program_location().parent_path().string();
}

QString getProgramPath()
{
    return QString::fromUtf8(getExecutablePath().c_str()) + "/";
}
