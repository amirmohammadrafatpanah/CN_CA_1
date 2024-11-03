QT       += core gui multimedia qml quick websockets widgets
CONFIG   += c++17

# Define paths for required libraries
PATH_TO_LIBDATACHANNEL = "C:\Users\amir\Desktop\libdatachannel"
PATH_TO_OPUS = "C:\Users\amir\Desktop\opus"
PATH_TO_OPENSSL = "C:\Qt\Tools\OpenSSLv3\Win_x64"


# Source and header files
SOURCES += \
    AudioApp.cpp \
    AudioInput.cpp \
    AudioOutput.cpp \
    main.cpp \
    mainwindow.cpp \
    webRTC.cpp

HEADERS += \
    AudioApp.h \
    AudioInput.h \
    AudioOutput.h \
    WebRTCClient.h \
    mainwindow.h \
    webRTC.h

FORMS += \
    mainwindow.ui

# Library paths and header files
INCLUDEPATH += $$PATH_TO_LIBDATACHANNEL/include
LIBS       += -L$$PATH_TO_LIBDATACHANNEL/Windows/Mingw64 -ldatachannel

INCLUDEPATH += $$PATH_TO_OPENSSL/include
LIBS       += -L$$PATH_TO_OPENSSL/lib/VC/x64/MT -lssl -lcrypto

INCLUDEPATH += $$PATH_TO_OPUS/include
LIBS       += -L$$PATH_TO_OPUS/build -lopus

LIBS       += -lws2_32 -lssp

INCLUDEPATH += $$PATH_TO_BOOST
LIBS       += -L$$PATH_TO_BOOST/stage/lib

INCLUDEPATH += $$PATH_TO_ASIO/include

# Other settings
DISTFILES += main.qml
RESOURCES += resources.qrc

DEFINES += _WEBSOCKETPP_CPP11_STL_
DEFINES += _WEBSOCKETPP_CPP11_FUNCTIONAL_
DEFINES += SIO_TLS
DEFINES += ASIO_STANDALONE
DEFINES += BOOST_ASIO_HAS_STD_ADDRESSOF
DEFINES += BOOST_ASIO_USE_BOOST_REGEX
DEFINES += BOOST_ASIO_SEPARATE_COMPILATION
DEFINES += BOOST_ASIO_HAS_STD_CHRONO
DEFINES += BOOST_ASIO_ENABLE_HANDLER_TRACKING

QMAKE_CXXFLAGS += -Wno-deprecated-declarations -Wno-unused-parameter -Wno-deprecated-copy -Wno-class-memaccess
