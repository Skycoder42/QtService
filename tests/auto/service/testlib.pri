TEMPLATE = app

QT = core service testlib

CONFIG   += console
CONFIG   -= app_bundle

DEFINES += SRCDIR=\\\"$$_PRO_FILE_PWD_/\\\"
DEFINES += OUTDIR=\\\"$$OUT_PWD/\\\"

mac: QMAKE_LFLAGS += '-Wl,-rpath,\'$$OUT_PWD/../../../../lib\''

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../TestBasicService/release/ -lbasicservice
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../TestBasicService/debug/ -lbasicservice
else:unix: LIBS += -L$$OUT_PWD/../TestBasicService/ -lbasicservice

INCLUDEPATH += $$PWD/TestBasicService
DEPENDPATH += $$PWD/TestBasicService

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../TestBasicService/release/libbasicservice.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../TestBasicService/debug/libbasicservice.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../TestBasicService/release/basicservice.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../TestBasicService/debug/basicservice.lib
else:unix: PRE_TARGETDEPS += $$OUT_PWD/../TestBasicService/libbasicservice.a
