INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD
INCLUDEPATH += $$PWD/../vitamtp/src

!DISABLE_FFMPEG {
    PKGCONFIG += libavformat libavcodec libavutil libswscale
}

unix|win32: LIBS += -L ../common/ -lqcma_common
LIBS += ../libvitamtp/build/src/.libs/libvitamtp.a -lpthread
PKGCONFIG += libusb-1.0 libxml-2.0
PRE_TARGETDEPS += ../libvitamtp/build/src/.libs/libvitamtp.a

win32:!win32-g++: PRE_TARGETDEPS += ../common/qcma_common.lib
else:unix|win32-g++: PRE_TARGETDEPS += ../common/libqcma_common.a
