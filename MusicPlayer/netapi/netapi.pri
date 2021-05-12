QT       += network
LIBS += -lpthread libwsock32 libws2_32
LIBS += -lpthread libMswsock libMswsock

HEADERS += \
    $$PWD/qmytcpclient.h \
    $$PWD/TCPNet.h \
    $$PWD/udpnet.h \
    $$PWD/tcpSelectServer.h \
    $$PWD/qmytcpselectserver.h

SOURCES += \
    $$PWD/qmytcpclient.cpp \
    $$PWD/TCPNet.cpp \
    $$PWD/udpnet.cpp \
    $$PWD/tcpSelectServer.cpp \
    $$PWD/qmytcpselectserver.cpp
