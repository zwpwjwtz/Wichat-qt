#-------------------------------------------------
#
# Project created by QtCreator 2018-02-23T21:02:25
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += link_pkgconfig
PKGCONFIG += openssl
QMAKE_CXXFLAGS += -std=c++0x

TARGET = Wichat-qt
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += main.cpp\
        mainwindow.cpp \
    loginwindow.cpp \
    serverconnection.cpp \
    encryptor.cpp \
    wichatconfig.cpp \
    usersession.cpp \
    peersession.cpp \
    abstractsession.cpp \
    notification.cpp \
    account.cpp \
    serverconfigdialog.cpp \
    global_objects.cpp \
    accountinfodialog.cpp \
    requestmanager.cpp \
    conversation.cpp \
    systraynotification.cpp \
    emoticonchooser.cpp \
    emojispecparser.cpp \
    editablelabel.cpp \
    preferencedialog.cpp \
    sessionmessagelist.cpp

HEADERS  += mainwindow.h \
    loginwindow.h \
    serverconnection.h \
    common.h \
    encryptor.h \
    wichatconfig.h \
    usersession.h \
    peersession.h \
    abstractsession.h \
    notification.h \
    account.h \
    Private/account_p.h \
    Private/abstractsession_p.h \
    Private/encryptor_p.h \
    Private/notification_p.h \
    Private/peersession_p.h \
    Private/serverconnection_p.h \
    Private/usersession_p.h \
    Private/wichatconfig_p.h \
    opensslpp/include/opensslpp/details/common.h \
    opensslpp/include/opensslpp/aes_cbc.h \
    opensslpp/include/opensslpp/aes_gcm.h \
    opensslpp/include/opensslpp/base64.h \
    opensslpp/include/opensslpp/random.h \
    opensslpp/include/opensslpp/rsa.h \
    opensslpp/include/opensslpp/sha.h \
    serverconfigdialog.h \
    global_objects.h \
    accountinfodialog.h \
    requestmanager.h \
    Private/requestmanager_p.h \
    conversation.h \
    Private/conversation_p.h \
    systraynotification.h \
    emoticonchooser.h \
    emojispecparser.h \
    Private/emojispecparserprivate.h \
    editablelabel.h \
    preferencedialog.h \
    sessionmessagelist.h \
    Private/sessionemessagelist_p.h

FORMS    += mainwindow.ui \
    loginwindow.ui \
    serverconfigdialog.ui \
    accountinfodialog.ui \
    systraynotification.ui \
    emoticonchooser.ui \
    editablelabel.ui \
    preferencedialog.ui

RESOURCES += \
    icon.qrc \
    emoticons.qrc

DISTFILES +=

target.path = $${PREFIX}/bin/

emoticons.files = Emoticon/Twemoji/2/72x72/*
emoticons.path = $${PREFIX}/share/emoticons/Twemoji/72x72/

INSTALLS += target emoticons
