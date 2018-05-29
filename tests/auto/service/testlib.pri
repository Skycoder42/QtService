TEMPLATE = app

QT = core service testlib

CONFIG   += console
CONFIG   -= app_bundle

DEFINES += SRCDIR=\\\"$$_PRO_FILE_PWD_/\\\"

mac: QMAKE_LFLAGS += '-Wl,-rpath,\'$$OUT_PWD/../../../../lib\''

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../TestBaseLib/release/ -ltestbase
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../TestBaseLib/debug/ -ltestbase
else:unix: LIBS += -L$$OUT_PWD/../TestBaseLib/ -ltestbase

INCLUDEPATH += $$PWD/TestBaseLib
DEPENDPATH += $$PWD/TestBaseLib

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../TestBaseLib/release/libtestbase.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../TestBaseLib/debug/libtestbase.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../TestBaseLib/release/testbase.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../TestBaseLib/debug/testbase.lib
else:unix: PRE_TARGETDEPS += $$OUT_PWD/../TestBaseLib/libtestbase.a
