QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    config.cpp \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    asio_compatibility.hpp \
    client_http.hpp \
    client_https.hpp \
    config.h \
    crypto.hpp \
    json.hpp \
    mainwindow.h \
    mutex.hpp \
    status_code.hpp \
    utility.hpp

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    root.qrc

LIBS += -L/usr/lib -lssl -lcrypto -lboost_filesystem -lboost_thread -lboost_system -ldl
