TEMPLATE = app

QMAKE_CXXFLAGS += -fsigned-char


CONFIG += qt \
    resources \
    thread \
    warn_on \


CONFIG += rtti

QT += \
    printsupport \
    core \
    gui \
    network \
    qml quick \
#    opengl \
    xml \
#    multimedia \
    script declarative sensors \

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets quickwidgets

PROJECT_TYPE = \
    STANDARD \
#    EMSCRIPTEN \
#    ANDROID \
#   IOS \


android {
ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android
PROJECT_TYPE = ANDROID
}

ios {
PROJECT_TYPE = IOS
}

contains(PROJECT_TYPE,STANDARD) {

QT += multimedia

PROJECT_MODULE += \
    M_EMB_QRC \
    M_CLOUD \
}


contains(PROJECT_TYPE,EMSCRIPTEN) {
DEFINES += NO_SOUND EMSCRIPTEN

PROJECT_MODULE += \
    M_CLOUD \
}

contains(PROJECT_TYPE,ANDROID) {
QT +=  androidextras multimedia

PROJECT_MODULE += \
    M_EMB_QRC \
    M_CLOUD \
}

contains(PROJECT_TYPE,IOS) {
DEFINES += NO_SOUND

BUNDLE_DATA.files = $$PWD/iOS_BundleData/Icon-60.png \
                        $$PWD/iOS_BundleData/Icon-60@2x.png \
                        $$PWD/iOS_BundleData/Icon-72.png \
                        $$PWD/iOS_BundleData/Icon-72@2x.png \
                        $$PWD/iOS_BundleData/Icon-76.png \
                        $$PWD/iOS_BundleData/Icon-76@2x.png \
                        $$PWD/iOS_BundleData/Icon-Small-40.png \
                        $$PWD/iOS_BundleData/Icon-Small-40@2x.png \
                        $$PWD/iOS_BundleData/Icon-Small-50.png \
                        $$PWD/iOS_BundleData/Icon-Small-50@2x.png \
                        $$PWD/iOS_BundleData/Icon-Small.png \
                        $$PWD/iOS_BundleData/Icon-Small@2x.png \
                        $$PWD/iOS_BundleData/Icon.png \
                        $$PWD/iOS_BundleData/Icon@2x.png
QMAKE_BUNDLE_DATA += BUNDLE_DATA

PROJECT_MODULE += \
    M_EMB_QRC \
    M_CLOUD \
}

POCKET_LIST += \
    PC1253 \
    PC1251 \

DEFINES += \
    P_PC1253 \


TARGET = Pockemul
TEMPLATE = app
TRANSLATIONS += pockemul_fr.ts
UI_DIR += build/ui


mac {
#DEFINES += NO_SOUND
ICON=$$PWD/MacOs-BundleData/Icon.icns
    #debug:OBJECTS_DIR += build/o/wd
    release:OBJECTS_DIR += build/o/wr
}
unix {
#DEFINES += NO_SOUND
    Debug:OBJECTS_DIR += build/o/wd
    Release:OBJECTS_DIR += build/o/wr
}
win32 {
    Release:OBJECTS_DIR += build/o/wr
    Debug:OBJECTS_DIR += build/o/wd
    QMAKE_LFLAGS += -static-libgcc -static-libstdc++
}


contains(PROJECT_MODULE,M_CLOUD) {
DEFINES += P_CLOUD

RESOURCES +=  \
    cloud/cloud.qrc \


HEADERS+= \
    cloud/cloudwindow.h \
    cloud/cloudimageprovider.h


SOURCES+= \
    cloud/cloudwindow.cpp \
    cloud/cloudimageprovider.cpp

}




FORMS += ui/about.ui \
    ui/dialoganalog.ui \
    ui/dialogconnectorlink.ui \
    ui/dialogdump.ui \
    ui/dialogkeylist.ui \
    ui/dialoglog.ui \
    ui/pockemul.ui \
    ui/serialconsole.ui \
    ui/startup.ui \
#    ui/dialogpotar.ui \
    ui/simulatorconsole.ui \
    ui/dialogdasm.ui \
    ui/cregssc61860widget.ui \
    ui/cregslh5801widget.ui \
    ui/cregsz80widget.ui \
#    ui/uartconsole.ui \
    ui/cregshd61700widget.ui \
    ui/dialogvkeyboard.ui \



HEADERS += \
    src/core/renderView.h \
    src/core/Connect.h \
    src/core/Dasm.h \
    src/core/Debug.h \
    src/core/Inter.h \
    src/core/Keyb.h \
    src/core/Keyb1250.h \
    src/core/Keyb1251.h \
    src/core/Keyb1350.h \
    src/core/Keyb1360.h \
    src/core/Keyb1401.h \
    src/core/Keyb1403.h \
    src/core/Keyb1450.h \
    src/core/Keyb1500.h \
    src/core/Log.h \
    src/core/analog.h \
    src/core/autoupdater.h \
    src/core/clink.h \
    src/core/common.h \
    src/core/cprinter.h \
    src/core/dialogabout.h \
    src/core/dialoganalog.h \
#    src/core/dialogconsole.h \
    src/core/dialogdump.h \
    src/core/dialogkeylist.h \
    src/core/dialoglog.h \
    src/core/dialogstartup.h \
    src/core/global.h \
    src/core/init.h \
    src/core/keybce150.h \
    src/core/keybce152.h \
    src/core/mainwindowpockemul.h \
    src/core/paperwidget.h \
    src/core/pobject.h \
    src/core/slot.h \
    src/core/xmlwriter.h \
    src/core/lcc/parser/parser.h \
    src/core/Keyb1600.h \
#    src/core/dialogpotar.h \
    src/core/cextension.h \
    src/core/Keyb2500.h \
    src/core/Keyb1280.h \
    src/core/weblinksparser.h \
#    src/core/wavfile.h \
    qcodemodel2/qcodenode.h \
    ui/dialogdasm.h \
    ui/cregssc61860widget.h \
    ui/cregcpu.h \
    ui/cregslh5801widget.h \
    ui/cregsz80widget.h \
#    ui/uartconsole.h \
    ui/cregshd61700widget.h \
    pictureflow/pictureflow.h \
    pictureflow/materialflow.h \
    pictureflow/fluidlauncher.h \
    pictureflow/launcher.h \
    pictureflow/slideshow.h \
    src/core/launchbuttonwidget.h \
    ui/dialogvkeyboard.h \
    src/core/qcommandline.h \
    src/core/version.h \
    src/core/downloadmanager.h \
    src/core/servertcp.h \
    src/core/dialogsimulator.h \
    src/core/bineditor/bineditor.h \
    src/core/bineditor/colorscheme.h \
    src/core/sizegrip.h \
    src/core/viewobject.h \
    src/core/allobjects.h \
    src/core/breakpoint.h \
    src/core/modelids.h \
    src/core/watchpoint.h \


HEADERS += \
    src/cpu/cpu.h \
    src/cpu/sc61860.h \
    src/cpu/sc62015.h \
#    tinybasic/tinybasic.h \
#    src/cpu/ctronics.h \
#    src/cpu/ct6834.h \
#    src/cpu/tc8576p.h \
#    src/cpu/sed1560.h \
#    src/cpu/i80x86.h \
#    src/cpu/hd66108.h \
#    src/cpu/hd61700.h \
#    src/cpu/uart.h \
#    src/cpu/hd44352.h \
#    src/cpu/lh5801.h \
#    src/cpu/lh5810.h \
#    src/cpu/pd1990ac.h \
#    src/cpu/z80.h \
#    src/cpu/z80memory.h \
#    src/cpu/lh5803.h \
#    src/cpu/hd61102.h \
#    src/cpu/z80-2.h \
#    src/cpu/lu57813p.h \
#    src/cpu/cf79107pj.h \
#    src/cpu/i8085.h \
#    src/cpu/i8085cpu.h \
#    src/cpu/i8085daa.h \
#    src/cpu/rp5c01.h \
#    src/cpu/upd16434.h \
#    src/cpu/upd7907/upd7907.h \
#    src/cpu/i80L188EB.h \
#    src/cpu/pit8253.h \
#    src/cpu/m6502/m6502.h \
#    src/cpu/m6502/opsc02.h \
#    src/cpu/m6502/ops02.h \\
#    src/cpu/ti57cpu.h \
#    src/cpu/s6b0108.h \
#    src/cpu/mc6847.h \
#    src/cpu/tms7000/tms7000.h \
#    src/cpu/hd44780.h \
#    src/cpu/upd1007.h \
#    src/cpu/upd1007d.h \
#    src/cpu/i8085_dasm.h \
#    src/cpu/i80x86_dasm.h \
#    src/cpu/hd61710.h \
#    src/cpu/hpnut.h \
#    src/cpu/hpnutd.h \


HEADERS += \
    src/lcd/Lcdc.h \
#    src/lcd/Lcdc_z1.h \
#    src/lcd/Lcdc_x07.h \
#    src/lcd/Lcdc_symb.h \
#    src/lcd/Lcdc_pc1600.h \
#    src/lcd/Lcdc_pb1000.h \
#    src/lcd/Lcdc_e500.h \
#    src/lcd/Lcdc_fp200.h \
#    src/lcd/Lcdc_pc1211.h \
#    src/lcd/Lcdc_g850.h \
#    src/lcd/Lcdc_pc2001.h \
#    src/lcd/Lcdc_lbc1100.h \
#    src/lcd/Lcdc_tpc8300.h \
#    src/lcd/Lcdc_rlh1000.h \
#    src/lcd/Lcdc_ti57.h \
#    src/lcd/Lcdc_hp41.h \
#    src/lcd/Lcdc_ce1560.h \
#    src/lcd/Lcdc_cc40.h \
#    src/lcd/Lcdc_ti74.h \
#    src/lcd/Lcdc_fx8000g.h \
#    src/lcd/Lcdc_hp15c.h \
#    src/lcd/Lcdc_pc1350.h \
    src/lcd/Lcdc_pc1250.h \
#    src/lcd/Lcdc_pc1500.h \
#    src/lcd/Lcdc_pc1475.h \
#    src/lcd/Lcdc_pc1403.h \
#    src/lcd/Lcdc_pc1450.h \
#    src/lcd/Lcdc_pc1401.h \
#    src/lcd/Lcdc_symb2x.h \


HEADERS += \
    src/machine/pcxxxx.h \
    src/machine/bus.h \
#    src/machine/cx710.h \
#    src/machine/cx07.h \
#    src/machine/cx07char.h \
#    src/machine/potar.h \
#    src/machine/cmotor.h \
#    src/machine/ce515p.h \
#    src/machine/casio/pb1000.h \
#    src/machine/casio/pb2000.h \
#    src/machine/casio/md100.h \
#    src/machine/casio/casiodisk.h \
#    src/machine/casio/fp100.h \
#    src/machine/casio/fp200.h \
#    src/machine/casio/z1.h \
#    src/machine/sharp/pc1211.h \
#    src/machine/sharp/ce122.h \
#    src/machine/pc2001.h \
#    src/machine/pc2021.h \
#    src/machine/printerctronics.h \
#    src/machine/casio/fp40.h \
#    src/machine/general/lbc1100.h \
#    src/machine/general/cl1000.h \
#    src/machine/cesimu.h \
#    src/machine/hp/hp41.h \
#    src/machine/ti/ti57.h \
#    src/machine/hp/hp41Cpu.h \
#    src/machine/hp/hp41mod.h \
#    src/machine/sharp/ce1560.h \
#    src/machine/hp/hp82143A.h \
#    src/machine/postit.h \
#    src/machine/sharp/buspc1500.h \
#    src/machine/sharp/ce162e.h \
#    src/machine/sharp/ce153.h \
#    src/machine/ti/cc40.h \
#    src/machine/ti/ti74.h \
#    src/machine/casio/fx8000g.h \
#    src/machine/sharp/ce1600f.h \
#    src/machine/sharp/ce2xxx.h \
#    src/machine/ccable.h \
#    src/machine/casio/fa80.h \
#    src/machine/hp/hp15c.h \
#    src/machine/tpc8300.h \
#    src/machine/tp83.h \
#    src/machine/panasonic/buspanasonic.h \
#    src/machine/panasonic/rlh1000.h \
#    src/machine/panasonic/rlp3001.h \
#    src/machine/panasonic/rlext.h \
#    src/machine/panasonic/rlp4002.h \
#    src/machine/panasonic/rlp1002.h \
#    src/machine/panasonic/rlp2001.h \
#    src/machine/panasonic/rlp6001.h \
#    src/machine/panasonic/rlp9001.h \
#    src/machine/panasonic/rlp1004a.h \
#    src/machine/panasonic/rlp1005.h \
#    src/machine/sharp/ce125.h \
#    src/machine/sharp/Ce126.h \
#    src/machine/sharp/ce150.h \
#    src/machine/sharp/ce152.h \
#    src/machine/sharp/pc1245.h \
#    src/machine/sharp/pc1250.h \
#    src/machine/sharp/pc1251.h \
#    src/machine/sharp/pc1255.h \
#    src/machine/sharp/pc1260.h \
#    src/machine/sharp/pc1350.h \
#    src/machine/sharp/pc1360.h \
#    src/machine/sharp/pc1401.h \
#    src/machine/sharp/pc1402.h \
#    src/machine/sharp/pc1403.h \
#    src/machine/sharp/pc1450.h \
#    src/machine/sharp/pc1475.h \
#    src/machine/sharp/pc1500.h \
#    src/machine/sharp/pc1600.h \
#    src/machine/sharp/ce1600p.h \
#    src/machine/sharp/pc2500.h \
#    src/machine/sharp/pc1421.h \
#    src/machine/sharp/pc1280.h \
#    src/machine/sharp/e500.h \
#    src/machine/sharp/g850v.h \
#    src/machine/sharp/ce140p.h \
#    src/machine/sharp/pc1425.h \
#    src/machine/sharp/ce140f.h \
#    src/machine/sio.h \



greaterThan(QT_MAJOR_VERSION, 4): HEADERS += \

INCLUDEPATH += . \
    src/core \
    src/cpu \
    src/machine \
    src/lcd \
    src/qmdilib \
    pictureflow

INCLUDEPATH += qcodeedit qcodeedit/document qcodeedit/language qcodeedit/widgets qcodeedit/snippets qcodeedit/qnfa \
                libavoid
LANGUAGE += C++
LICENSE += GPL
MOC_DIR += build/moc
OPENEDFILES += 

RCC_DIR += build/rcc

contains(PROJECT_MODULE, M_EMB_QRC) {
RESOURCES +=  \
    resources/keymap.qrc \
#    resources/stdlibs.qrc \
#    qcodeedit/qxs/qxs.qrc \
#    resources/asmlibs.qrc \
    resources/core.qrc \
#    resources/cc40.qrc \
#    resources/ti74.qrc \
#    resources/fx8000g.qrc \
#    resources/hp15c.qrc \
#    resources/hp11c.qrc \
#    resources/hp12c.qrc \
#    resources/hp16c.qrc \
#    resources/ce2xxx.qrc
#    resources/ext.qrc \
#    resources/pc1245.qrc \
#    resources/pc1250.qrc \
    resources/pc1251.qrc \
#    resources/pc1255.qrc \
#    resources/pc1260.qrc \
#    resources/pc1261.qrc \
#    resources/pc1262.qrc \
#    resources/pc1350.qrc \
#    resources/pc1360.qrc \
#    resources/pc1401.qrc \
#    resources/pc1402.qrc \
#    resources/pc1403.qrc \
#    resources/pc1450.qrc \
#    resources/pc1475.qrc \
#    resources/pc1500.qrc \
    resources/pockemul.qrc \
#    resources/pc1600.qrc \
#    resources/pc2500.qrc \
#    resources/pc1421.qrc \
#    resources/pc1425.qrc \
#    resources/pc1460.qrc \
#    resources/pc1280.qrc \
#    resources/x07.qrc \
#    resources/pb1000.qrc \
#    resources/e500.qrc \
#    resources/pb2000.qrc \
#    resources/g850v.qrc \
#    resources/ext2.qrc \
#    resources/z1.qrc \
#    resources/fp200.qrc \
#    resources/pc1211.qrc \
#    resources/pc2001.qrc \
#    resources/lbc1100.qrc \
#    resources/tpc8300.qrc \
#    resources/rlh1000.qrc \
#    resources/ti57.qrc \
#    resources/hp41.qrc \


}
else {
DEFINES += LOCRES
}

RESOURCES +=  \

SOURCES +=  \
    src/core/pockemul.cpp \
    src/core/viewobject.cpp \
    src/core/pobject.cpp \
    src/core/Connect.cpp \
    src/core/Debug.cpp \
    src/core/Inter.cpp \
    src/core/Keyb.cpp \
    src/core/Log.cpp \
    src/core/analog.cpp \
    src/core/autoupdater.cpp \
    src/core/clink.cpp \
    src/core/dialogabout.cpp \
    src/core/dialoganalog.cpp \
#    src/core/dialogconsole.cpp \
    src/core/dialogdump.cpp \
    src/core/dialogkeylist.cpp \
    src/core/dialoglog.cpp \
    src/core/dialogstartup.cpp \
    src/core/init.cpp \
    src/core/mainwindowpockemul.cpp \
    src/core/paperwidget.cpp \
    src/core/slot.cpp \
    src/core/xmlwriter.cpp \
    src/core/lcc/parser/parser.cpp \
    src/core/weblinksparser.cpp \
#    src/core/wavfile.cpp \
    src/core/dialogsimulator.cpp \
    src/core/bineditor/bineditor.cpp \
    src/core/bineditor/colorscheme.cpp \
#    src/core/dialogpotar.cpp \
    qcodemodel2/qcodenode.cpp \
    src/core/launchbuttonwidget.cpp \
    src/core/qcommandline.cpp \
    src/core/cextension.cpp \
    src/core/downloadmanager.cpp \
    src/core/breakpoint.cpp \
    src/core/renderView.cpp \
    src/core/sizegrip.cpp \
    src/core/servertcp.cpp \
    pictureflow/pictureflow.cpp \
    pictureflow/materialflow.cpp \
    pictureflow/fluidlauncher.cpp \
    pictureflow/launcher.cpp \
    pictureflow/slideshow.cpp \
    ui/dialogdasm.cpp \
    ui/cregssc61860widget.cpp \
    ui/cregcpu.cpp \
    ui/cregslh5801widget.cpp \
    ui/cregshd61700widget.cpp \
#    ui/uartconsole.cpp \
    ui/dialogvkeyboard.cpp \


SOURCES += \
    src/cpu/cpu.cpp \
#    src/cpu/sed1560.cpp \
#    tinybasic/tinybasic.cpp \
#    src/cpu/lh5801.cpp \
#    src/cpu/lh5810.cpp \
#    src/cpu/pd1990ac.cpp \
     src/cpu/sc61860.cpp \
     src/cpu/sc62015.cpp \
#    src/cpu/ct6834.cpp \
#    src/cpu/z80.cpp \
#    src/cpu/lh5803.cpp \
    src/cpu/z80_dasm.cpp \
#    src/cpu/hd61102.cpp \
#    src/cpu/lu57813p.cpp \
    ui/cregsz80widget.cpp \
#    src/cpu/tc8576p.cpp \
    src/cpu/hd61700d.cpp \
#    src/cpu/hd61700.cpp \
#    src/cpu/uart.cpp \
#    src/cpu/hd44352.cpp \
#    src/cpu/i80x86.cpp \
#    src/cpu/hd66108.cpp \
#    src/cpu/upd16434.cpp \
#    src/cpu/i8085_dasm.cpp \
#    src/cpu/i80x86_dasm.cpp \
#    src/cpu/cf79107pj.cpp \
#    src/cpu/i8085.cpp \
#    src/cpu/rp5c01.cpp \
#    src/cpu/ctronics.cpp \
    src/cpu/upd7907/upd7907d.cpp \
#    src/cpu/upd7907/upd7907.cpp \
#    src/cpu/i80L188EB.cpp \
#    src/cpu/pit8253.cpp \
#    src/cpu/m6502/m6502.cpp \
    src/cpu/m6502/m6502_dasm.cpp \
#    src/cpu/ti57cpu.cpp \
#    src/cpu/s6b0108.cpp \
#    src/cpu/mc6847.cpp \
    src/cpu/ti57cpu_dasm.cpp \
#    src/cpu/tms7000/tms7000.cpp \
    src/cpu/tms7000/tms7000d.cpp \
#    src/cpu/hd44780.cpp \
#    src/cpu/upd1007d.cpp \
#    src/cpu/hd61710.cpp \
#    src/cpu/hpnut.cpp \
#    src/cpu/upd1007.cpp \
#    src/cpu/hpnutd.cpp \


SOURCES += \
    src/lcd/Lcdc.cpp \
#    src/lcd/Lcdc_pc1211.cpp \
#    src/lcd/Lcdc_pc2001.cpp \
#    src/lcd/Lcdc_x07.cpp \
#    src/lcd/Lcdc_pc1600.cpp \
#    src/lcd/Lcdc_pb1000.cpp \
#    src/lcd/Lcdc_e500.cpp \
#    src/lcd/Lcdc_g850.cpp \
#    src/lcd/Lcdc_fp200.cpp \
#    src/lcd/Lcdc_z1.cpp \
#    src/lcd/Lcdc_lbc1100.cpp \
#    src/lcd/Lcdc_tpc8300.cpp \
#    src/lcd/Lcdc_rlh1000.cpp \
#    src/lcd/Lcdc_ti57.cpp \
#    src/lcd/Lcdc_hp41.cpp \
#    src/lcd/Lcdc_ce1560.cpp \
#    src/lcd/Lcdc_cc40.cpp \
#    src/lcd/Lcdc_ti74.cpp \
#    src/lcd/Lcdc_fx8000g.cpp \
#    src/lcd/Lcdc_hp15c.cpp \
#    src/lcd/Lcdc_pc1350.cpp \
     src/lcd/Lcdc_pc1250.cpp \
#    src/lcd/Lcdc_pc1500.cpp \
#    src/lcd/Lcdc_pc1475.cpp \
#    src/lcd/Lcdc_pc1403.cpp \
#    src/lcd/Lcdc_pc1450.cpp \
#    src/lcd/Lcdc_pc1401.cpp \


SOURCES += \
    src/machine/pcxxxx.cpp \
    src/machine/bus.cpp \
    src/core/cprinter.cpp \
#    src/machine/sharp/ce140f.cpp \
#    src/machine/cesimu.cpp \
#    src/machine/sharp/ce122.cpp \
#    src/machine/hp/hp41.cpp \
#    src/machine/postit.cpp \
#    src/machine/tpc8300.cpp \
#    src/machine/tp83.cpp \
#    src/machine/panasonic/rlh1000.cpp \
#    src/machine/panasonic/rlp6001.cpp \
#    src/machine/panasonic/rlp9001.cpp \
#    src/machine/panasonic/rlp1004a.cpp \
#    src/machine/ti/ti57.cpp \
#    src/machine/hp/hp41trace.cpp \
#    src/machine/hp/hp41display.cpp \
#    src/machine/hp/hp41Timer.cpp \
#    src/machine/hp/hp41File.cpp \
#    src/machine/hp/hp41Cpu.cpp \
#    src/machine/hp/hp41mod.cpp \
#    src/machine/sharp/buspc1500.cpp \
#    src/machine/sharp/ce162e.cpp \
#    src/machine/sharp/ce153.cpp \
#    src/machine/hp/hp82143A.cpp \
#    src/machine/panasonic/rlp4002.cpp \
#    src/machine/panasonic/rlp1002.cpp \
#    src/machine/panasonic/rlp2001.cpp \
#    src/machine/sharp/ce1560.cpp \
#    src/machine/panasonic/buspanasonic.cpp \
#    src/machine/panasonic/rlp3001.cpp \
#    src/machine/panasonic/rlext.cpp \
#    src/machine/ti/cc40.cpp \
#    src/machine/ti/ti74.cpp \
#    src/machine/hp/hp15c.cpp \
#    src/machine/panasonic/rlp1005.cpp \
#    src/machine/sharp/ce1600f.cpp \
#    src/machine/casio/fx8000g.cpp \
#    src/machine/casio/fa80.cpp \
#    src/machine/sharp/ce2xxx.cpp \
#    src/machine/sharp/Ce126.cpp \
#    src/machine/ccable.cpp \
#    src/machine/sharp/ce125.cpp \
#    src/machine/sharp/ce150.cpp \
#    src/machine/sharp/ce152.cpp \
#    src/machine/sharp/pc1245.cpp \
     src/machine/sharp/pc1250.cpp \
     src/machine/sharp/pc1251.cpp \
#    src/machine/sharp/pc1255.cpp \
#    src/machine/sharp/pc1260.cpp \
#    src/machine/sharp/pc1350.cpp \
#    src/machine/sharp/pc1360.cpp \
#    src/machine/sharp/pc1401.cpp \
#    src/machine/sharp/pc1402.cpp \
#    src/machine/sharp/pc1403.cpp \
#    src/machine/sharp/pc1450.cpp \
#    src/machine/sharp/pc1475.cpp \
#    src/machine/sharp/pc1500.cpp \
#    src/machine/sharp/pc1421.cpp \
#    src/machine/sharp/pc1280.cpp \
#    src/machine/sharp/ce140p.cpp \
#    src/machine/sharp/pc2500.cpp \
#    src/machine/sharp/g850v.cpp \
#    src/machine/sharp/pc1600.cpp \
#    src/machine/sharp/ce1600p.cpp \
#    src/machine/sharp/ce120p.cpp \
#    src/machine/sharp/e500.cpp \
#    src/machine/sharp/pc1211.cpp \
#    src/machine/sharp/pc1425.cpp \
#    src/machine/sio.cpp \
#    src/machine/cx07.cpp \
#    src/machine/cx710.cpp \
#    src/machine/casio/pb1000.cpp \
#    src/machine/casio/pb2000.cpp \
#    src/machine/casio/md100.cpp \
#    src/machine/casio/casiodisk.cpp \
#    src/machine/casio/z1.cpp \
#    src/machine/casio/fp40.cpp \
#    src/machine/casio/fp100.cpp \
#    src/machine/casio/fp200.cpp \
#    src/machine/pc2001.cpp \
#    src/machine/pc2021.cpp \
#    src/machine/printerctronics.cpp \
#    src/machine/general/lbc1100.cpp \
#    src/machine/general/cl1000.cpp \
#    src/machine/potar.cpp \
#    src/machine/cmotor.cpp \
#    src/machine/ce515p.cpp \


greaterThan(QT_MAJOR_VERSION, 4): SOURCES += \


OTHER_FILES += \
#    resources/keymap/trspc2.map \
#    resources/keymap/pc2500.map \
#    resources/keymap/pc1600.map \
#    resources/keymap/pc1500.map \
#    resources/keymap/pc1450.map \
#    resources/keymap/pc1403.map \
#    resources/keymap/pc1401.map \
#    resources/keymap/pc1360.map \
#    resources/keymap/pc1350.map \
#    resources/keymap/pc1280.map \
#    resources/keymap/pc1250.map \
#    resources/keymap/ce1600p.map \
#    resources/keymap/ce150.map \
#    resources/pockemul/release_notes.html \
#    resources/keymap/ce125tape.map \
#    resources/keymap/ce126.map \
#    resources/keymap/ce125.map \
#    resources/keymap/ce129.map \
#    resources/keymap/ce123.map \
#    resources/keymap/ce120.map \
#    resources/keymap/263591.map \
#    resources/keymap/ce140p.map \
#    resources/weblinks.xml \
#    resources/notes.txt \
#    resources/pockemul/script_help.html \
#    logitheque/serial1251.qs \
#    qcodeedit/qxs/marks.qxm \
#    qcodeedit/qxs/xml.qxf \
#    qcodeedit/qxs/tex.qxf \
#    qcodeedit/qxs/qtscript.qxf \
#    qcodeedit/qxs/python.qxf \
#    qcodeedit/qxs/pro.qxf \
#    qcodeedit/qxs/php.qxf \
#    qcodeedit/qxs/formats.qxf \
#    qcodeedit/qxs/dox.qxf \
#    qcodeedit/qxs/css.qxf \
#    qcodeedit/qxs/csharp.qxf \
#    qcodeedit/qxs/cpp.qxf \
#    qcodeedit/qxs/bibtex.qxf \
#    qcodeedit/qxs/xml.qnfa \
#    qcodeedit/qxs/tex.qnfa \
#    qcodeedit/qxs/snippet.qnfa \
#    qcodeedit/qxs/qtscript.qnfa \
#    qcodeedit/qxs/python.qnfa \
#    qcodeedit/qxs/pro.qnfa \
#    qcodeedit/qxs/php.qnfa \
#    qcodeedit/qxs/dox.qnfa \
#    qcodeedit/qxs/css.qnfa \
#    qcodeedit/qxs/csharp.qnfa \
#    qcodeedit/qxs/cpp.qnfa \
#    qcodeedit/qxs/coldfusion.qnfa \
#    qcodeedit/qxs/bibtex.qnfa \
#    qcodeedit/qxs/asm.qnfa \
#    qcodeedit/qxs/asm.qxf \
#    resources/asmlibs/sc61860/xor16.lib \
#    resources/asmlibs/sc61860/xor8.lib \
#    resources/asmlibs/sc61860/sr16.lib \
#    resources/asmlibs/sc61860/sr8.lib \
#    resources/asmlibs/sc61860/sl16.lib \
#    resources/asmlibs/sc61860/sl8.lib \
#    resources/asmlibs/sc61860/mul16.lib \
#    resources/asmlibs/sc61860/mul8.lib \
#    resources/asmlibs/sc61860/divmod8.lib \
#    resources/asmlibs/sc61860/cpse16.lib \
#    resources/asmlibs/sc61860/cps16.lib \
#    resources/asmlibs/sc61860/cpne16.lib \
#    resources/asmlibs/sc61860/cpge16.lib \
#    resources/asmlibs/sc61860/cpg16.lib \
#    resources/asmlibs/sc61860/cpeq16.lib \
#    resources/asmlibs/sc61860/cpe16.lib \
#    resources/asmlibs/sc61860/divmod16.lib \
#    resources/keymap/x07.map \
#    resources/keymap/e500.map \
#    resources/keymap/pb1000.map \
#    resources/keymap/x710.map \
#    resources/pockemul/config.xml \
#    resources/pockemul/configExt.xml \
#    resources/keymap/pb2000.map \
#    resources/keymap/g850v.map \
#    resources/keymap/z1.map \
#    resources/keymap/fp200.map \
    resources/keymap/pc1253.map \
#    resources/keymap/pc1260.map \
#    resources/keymap/pc1211.map \
#    resources/keymap/ce122.map \
#    resources/keymap/pc2001.map \
#    resources/keymap/pc2021.map \
#    resources/keymap/ce127r.map \
#    resources/keymap/ce152.map \
#    resources/keymap/pc2081.map \
#    resources/keymap/fp40.map \
#    resources/keymap/lbc1100.map \
#    resources/keymap/cl1000.map \
#    resources/keymap/pc1425.map \
#    resources/pockemul/weblinks.xml \
#    resources/keymap/tpc8300.map \
#    resources/keymap/tp83.map \
#    resources/keymap/rlh1000.map \
#    resources/keymap/ti57.map \
#    resources/keymap/hp41.map \
#    resources/keymap/hp82143a.map \
#    resources/keymap/rlp9006.map \
#    resources/keymap/rlp1004a.map \
#    resources/keymap/rlp4002.map \
#    resources/keymap/ce1560.map \
#    resources/keymap/ce140f.map \
#    resources/keymap/cc40.map \
#    resources/keymap/ti74.map \
#    resources/keymap/ti95.map \
#    resources/keymap/fx8000g.map \
#    resources/keymap/hp15c.map \
#    resources/keymap/rlp1002.map \
#    resources/keymap/rlp1005.map \
    android/src/org/kde/necessitas/ministro/IMinistro.aidl \
    android/src/org/kde/necessitas/ministro/IMinistroCallback.aidl \
    android/src/org/qtproject/qt5/android/bindings/QtActivity.java \
    android/src/org/qtproject/qt5/android/bindings/QtApplication.java \
    android/AndroidManifest.xml \
    android/res/layout/splash.xml \
    android/res/values/libs.xml \
    android/res/values/strings.xml \
    android/res/values-de/strings.xml \
    android/res/values-el/strings.xml \
    android/res/values-es/strings.xml \
    android/res/values-et/strings.xml \
    android/res/values-fa/strings.xml \
    android/res/values-fr/strings.xml \
    android/res/values-id/strings.xml \
    android/res/values-it/strings.xml \
    android/res/values-ja/strings.xml \
    android/res/values-ms/strings.xml \
    android/res/values-nb/strings.xml \
    android/res/values-nl/strings.xml \
    android/res/values-pl/strings.xml \
    android/res/values-pt-rBR/strings.xml \
    android/res/values-ro/strings.xml \
    android/res/values-rs/strings.xml \
    android/res/values-ru/strings.xml \
    android/res/values-zh-rCN/strings.xml \
    android/res/values-zh-rTW/strings.xml \
    android/src/org/kde/necessitas/ministro/IMinistro.aidl \
    android/src/org/kde/necessitas/ministro/IMinistroCallback.aidl \
    android/src/org/qtproject/qt5/android/bindings/QtActivity.java \
    android/src/org/qtproject/qt5/android/bindings/QtApplication.java \
    android/version.xml \
    android/src/org/kde/necessitas/ministro/IMinistro.aidl \
    android/src/org/kde/necessitas/ministro/IMinistroCallback.aidl \
    android/src/org/qtproject/qt5/android/bindings/QtActivity.java \
    android/src/org/qtproject/qt5/android/bindings/QtApplication.java \
    android/res/layout/splash.xml \
    android/res/values/strings.xml \
    android/res/values-de/strings.xml \
    android/res/values-el/strings.xml \
    android/res/values-es/strings.xml \
    android/res/values-et/strings.xml \
    android/res/values-fa/strings.xml \
    android/res/values-fr/strings.xml \
    android/res/values-id/strings.xml \
    android/res/values-it/strings.xml \
    android/res/values-ja/strings.xml \
    android/res/values-ms/strings.xml \
    android/res/values-nb/strings.xml \
    android/res/values-nl/strings.xml \
    android/res/values-pl/strings.xml \
    android/res/values-pt-rBR/strings.xml \
    android/res/values-ro/strings.xml \
    android/res/values-rs/strings.xml \
    android/res/values-ru/strings.xml \
    android/res/values-zh-rCN/strings.xml \
    android/res/values-zh-rTW/strings.xml \
    android/src/org/kde/necessitas/ministro/IMinistro.aidl \
    android/src/org/kde/necessitas/ministro/IMinistroCallback.aidl \
    android/src/org/qtproject/qt5/android/bindings/QtActivity.java \
    android/src/org/qtproject/qt5/android/bindings/QtApplication.java \
    android/version.xml \
    android/AndroidManifest.xml \
    android/res/values/libs.xml \
#    cloud/content/ScrollBar.qml \
#    cloud/content/NewsDelegate.qml \
#    cloud/content/CategoryDelegate.qml \
#    cloud/content/BusyIndicator.qml \
#    cloud/content/TabWidget.qml \
#    cloud/qmltemplates/CoverFlow/example/main.qml \
#    cloud/qmltemplates/CoverFlow/component/CoverFlowDelegate.qml \
#    cloud/qmltemplates/CoverFlow/component/CoverFlow.qml \
#    cloud/qmltemplates/CoverFlow/component/AlphaGradient.qml \
#    cloud/qmltemplates/CoverFlow/component/gfx/splash_screen.png \
#    cloud/qmltemplates/CoverFlow/component/gfx/splash.png \
#    cloud/qmltemplates/CoverFlow/component/gfx/rotation_icon.png \
#    cloud/qmltemplates/CoverFlow/component/gfx/photo_back.png \
#    cloud/qmltemplates/CoverFlow/component/gfx/media_browser_visuals.png \
#    cloud/qmltemplates/CoverFlow/component/gfx/image_placeholder.png \
#    cloud/qmltemplates/CoverFlow/component/gfx/background.png \
#    cloud/qmltemplates/CoverFlow/component/gfx/accordionlist.png \
#    cloud/qmltemplates/AccordionList/example/AccordionList.qml \
#    cloud/qmltemplates/AccordionList/component/ListItem.qml \
#    cloud/qmltemplates/AccordionList/component/AccordionListModel.qml \
#    cloud/qmltemplates/AccordionList/component/AccordionList.qml \
#    cloud/TabbedQuickApp/TabbedUI.qml \
#    cloud/TabbedQuickApp/Tab.qml \
#    cloud/RatingIndicatorQML/MyRatingIndicator.qml \
#    cloud/RatingIndicatorQML/star.png \
#    cloud/content/TextButton.qml \
#    cloud/content/Settings.qml \
#    cloud/content/SearchBox.qml \
#    cloud/content/LineInput.qml \
#    cloud/PmlView.qml \
#    cloud/content/SettingsDelegate.qml \
#    cloud/content/Edit.qml \
#    cloud/content/ComboBox.qml \
#    cloud/content/Switch.qml \
#    cloud/content/knob.svg \
#    cloud/content/counter.js \
#    cloud/content/SortListModel.qml \
#    cloud/content/Test2.qml \
#    cloud/content/Grid.qml \
#    cloud/content/Message.qml \
#    cloud/PmlView2.qml \
#    cloud/Test.qml \
#    cloud/content/NewsDelegate2.qml \
#    cloud/content/Actions.qml \
    android/src/org/qtproject/pockemul/PockemulActivity.java \
    android/AndroidManifest.xml \
#    cloud/Main.qml \



ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android













