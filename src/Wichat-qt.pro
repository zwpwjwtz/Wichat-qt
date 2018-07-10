#-------------------------------------------------
#
# Project created by QtCreator 2018-02-23T21:02:25
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

!win32{
CONFIG += link_pkgconfig
PKGCONFIG += openssl
}

win32{
# Please change the value of "COMPONENT_DIR" to the directory
# that contains headers and static libraries of openssl
COMPONENT_DIR = D:\Qt\Tools\mingw492_32\opt
OPENSSL_INCDIR = $$COMPONENT_DIR\include
OPENSSL_LIBDIR = $$COMPONENT_DIR\lib

QMAKE_INCDIR += $$OPENSSL_INCDIR
QMAKE_LIBDIR += $$OPENSSL_LIBDIR
QMAKE_LIBS += -lssl -lcrypto
}

macx{
# Please change the value of "COMPONENT_DIR" to the directory
# that contains headers and static libraries of openssl
COMPONENT_DIR = /usr/local/Cellar/openssl/1.0.2o_2
OPENSSL_INCDIR = $$COMPONENT_DIR/include
OPENSSL_LIBDIR = $$COMPONENT_DIR/lib

QMAKE_INCDIR += $$OPENSSL_INCDIR
QMAKE_LIBS += $$OPENSSL_LIBDIR/libssl.a $$OPENSSL_LIBDIR/libcrypto.a
}

QMAKE_CXXFLAGS += -std=c++0x

TARGET = Wichat-qt
TEMPLATE = app

VER_MAJ = 2
VER_MIN = 0
VER_PAT = 0
VERSION = 2.0.0
VERSION_PE_HEADER = 2.0.0

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS APP_VERSION=\\\"$$VERSION\\\"

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += main.cpp\
        mainwindow.cpp \
    loginwindow.cpp \
    Modules/serverconnection.cpp \
    Modules/encryptor.cpp \
    Modules/wichatconfig.cpp \
    Modules/usersession.cpp \
    Modules/peersession.cpp \
    Modules/abstractsession.cpp \
    Modules/notification.cpp \
    Modules/account.cpp \
    serverconfigdialog.cpp \
    global_objects.cpp \
    accountinfodialog.cpp \
    Modules/requestmanager.cpp \
    Modules/conversation.cpp \
    systraynotification.cpp \
    emoticonchooser.cpp \
    Modules/emojispecparser.cpp \
    editablelabel.cpp \
    preferencedialog.cpp \
    Modules/sessionmessagelist.cpp \
    aboutwindow.cpp

HEADERS  += mainwindow.h \
    loginwindow.h \
    Modules/serverconnection.h \
    common.h \
    Modules/encryptor.h \
    Modules/wichatconfig.h \
    Modules/usersession.h \
    Modules/peersession.h \
    Modules/abstractsession.h \
    Modules/notification.h \
    Modules/account.h \
    Modules/Private/account_p.h \
    Modules/Private/abstractsession_p.h \
    Modules/Private/encryptor_p.h \
    Modules/Private/notification_p.h \
    Modules/Private/peersession_p.h \
    Modules/Private/serverconnection_p.h \
    Modules/Private/usersession_p.h \
    Modules/Private/wichatconfig_p.h \
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
    Modules/requestmanager.h \
    Modules/Private/requestmanager_p.h \
    Modules/conversation.h \
    Modules/Private/conversation_p.h \
    systraynotification.h \
    emoticonchooser.h \
    Modules/emojispecparser.h \
    Modules/Private/emojispecparserprivate.h \
    editablelabel.h \
    preferencedialog.h \
    Modules/sessionmessagelist.h \
    Modules/Private/sessionemessagelist_p.h \
    aboutwindow.h

FORMS    += mainwindow.ui \
    loginwindow.ui \
    serverconfigdialog.ui \
    accountinfodialog.ui \
    systraynotification.ui \
    emoticonchooser.ui \
    editablelabel.ui \
    preferencedialog.ui \
    aboutwindow.ui

RESOURCES += \
    icon.qrc \
    emoticons.qrc

DISTFILES +=

isEmpty(PREFIX){
    PREFIX = .
}
!win32{
target.path = $$PREFIX/bin

emoticons.files = Emoticon/Twemoji/2/72x72/*
emoticons.path = $$PREFIX/share/emoticons/Twemoji/72x72
}
win32{
target.path = $$PREFIX

emoticons.files = Emoticon/Twemoji/2/72x72/*
emoticons.path = $$PREFIX/Resource/emoticons/Twemoji/72x72
}

INSTALLS += target emoticons
