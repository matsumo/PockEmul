
SHARP_PACKAGE *= \
    PC1211 \
    PC1245 \
    PC1250 \
    PC1251 \
    PC1253 \
    PC1255 \
    PC1260 \
    PC1280 \
    PC1350 \
    PC1360 \
    PC1401 \
    PC1402 \
    PC1403 \
    PC1421 \
    PC1425 \
    PC1450 \
    PC1475 \
    PC1500 \
    PC1600 \
    PC2500 \
    G850 \
    E500 \
    CE126P

CASIO_PACKAGE *= \
    CASIO_ALL

TI_PACKAGE *= \
    TI_ALL \

HP_PACKAGE *= \
    HP_ALL \

OTHER_PACKAGE *= \
    JR800 \
    HX20 \
    PANASONIC \


PROJECT_PACKAGE *= \
    $$SHARP_PACKAGE \
    $$CASIO_PACKAGE \
    $$TI_PACKAGE \
    $$HP_PACKAGE \
    $$OTHER_PACKAGE \


TEMPLATE = app

QMAKE_CXXFLAGS += -fsigned-char

#DEFINES += GL

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
    M_IDE \
    M_EMB_QRC \
    M_LIBAVOID \
    M_CLOUD \
}



contains(PROJECT_TYPE,EMSCRIPTEN) {
DEFINES += NO_SOUND EMSCRIPTEN

PROJECT_MODULE += \
    M_CLOUD \
}

contains(PROJECT_TYPE,ANDROID) {
QT +=  androidextras multimedia
DEFINES += NO_SOUND
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



# PC1475
contains(PROJECT_PACKAGE,PC1475) {
PROJECT_PACKAGE *= SC61860
PROJECT_PACKAGE *= PC1360
}
# PC1450
contains(PROJECT_PACKAGE,PC1450) {
PROJECT_PACKAGE *= SC61860
PROJECT_PACKAGE *= PC1350
}
# PC1260 PC1261 PC1262
contains(PROJECT_PACKAGE,PC1260) {
PROJECT_PACKAGE *= SC61860
PROJECT_PACKAGE *= PC1250
}
# PC1255
contains(PROJECT_PACKAGE,PC1255) {
PROJECT_PACKAGE *= SC61860
PROJECT_PACKAGE *= PC1250
}
# PC1253
contains(PROJECT_PACKAGE,PC1253) {
PROJECT_PACKAGE *= SC61860
PROJECT_PACKAGE *= PC1251
}
# PC1251
contains(PROJECT_PACKAGE,PC1251) {
PROJECT_PACKAGE *= SC61860
PROJECT_PACKAGE *= PC1250
}
# PC1245
contains(PROJECT_PACKAGE,PC1245) {
PROJECT_PACKAGE *= SC61860
PROJECT_PACKAGE *= PC1250
}
# PC1250
contains(PROJECT_PACKAGE,PC1250) {
PROJECT_PACKAGE *= SC61860
}
# PC1280
contains(PROJECT_PACKAGE,PC1280) {
PROJECT_PACKAGE *= SC61860
PROJECT_PACKAGE *= PC1360
}
# PC2500
contains(PROJECT_PACKAGE,PC2500) {
PROJECT_PACKAGE *= PC1350
PROJECT_PACKAGE *= SC61860
}
# PC1360
contains(PROJECT_PACKAGE,PC1360) {
PROJECT_PACKAGE *= PC1350
PROJECT_PACKAGE *= SC61860
}
# PC1350
contains(PROJECT_PACKAGE,PC1350) {
PROJECT_PACKAGE *= SC61860
}
# PC1425
contains(PROJECT_PACKAGE,PC1425) {
PROJECT_PACKAGE *= SC61860
PROJECT_PACKAGE *= PC1403
}
# PC1421
contains(PROJECT_PACKAGE,PC1421) {
PROJECT_PACKAGE *= SC61860
PROJECT_PACKAGE *= PC1401
}
# PC1403
contains(PROJECT_PACKAGE,PC1403) {
PROJECT_PACKAGE *= SC61860
PROJECT_PACKAGE *= PC1401
}
# PC1402
contains(PROJECT_PACKAGE,PC1402) {
PROJECT_PACKAGE *= SC61860
PROJECT_PACKAGE *= PC1401
}
# PC1401
contains(PROJECT_PACKAGE,PC1401) {
PROJECT_PACKAGE *= SC61860
}

# PC1500
contains(PROJECT_PACKAGE,PC1500) {
PROJECT_PACKAGE *= LH5801
}

# E500 E550 E500S
contains(PROJECT_PACKAGE,E500) {
PROJECT_PACKAGE *= SC62015
}
# JR800
contains(PROJECT_PACKAGE,JR800) {
PROJECT_PACKAGE *= HD44102
PROJECT_PACKAGE *= MC6800
}
# HX20
contains(PROJECT_PACKAGE,HX20) {
#PROJECT_PACKAGE *= UPD16434
PROJECT_PACKAGE *= MC6800
}

contains(PROJECT_PACKAGE,PANASONIC) {
#PROJECT_PACKAGE *= UPD16434
PROJECT_PACKAGE *= M6502
}

contains(PROJECT_PACKAGE,TI_ALL) {
#PROJECT_PACKAGE *= UPD16434
PROJECT_PACKAGE *= TMS7000
}

# E500
contains(PROJECT_PACKAGE,E500) {
DEFINES *= P_E500
HEADERS *= src/machine/sharp/e500.h \
           src/lcd/Lcdc_e500.h
SOURCES *= src/machine/sharp/e500.cpp \
           src/lcd/Lcdc_e500.cpp
RESOURCES *= resources/e500.qrc
}
# PC1211
contains(PROJECT_PACKAGE,PC1211) {
DEFINES *= P_PC1211
HEADERS *= src/machine/sharp/pc1211.h \
           src/lcd/Lcdc_pc1211.h \
           tinybasic/tinybasic.h
SOURCES *= src/machine/sharp/pc1211.cpp \
           src/lcd/Lcdc_pc1211.cpp \
           tinybasic/tinybasic.cpp
RESOURCES *= resources/pc1211.qrc
}
# PC1245
contains(PROJECT_PACKAGE,PC1245) {
DEFINES *= P_PC1245
HEADERS *= src/machine/sharp/pc1245.h
SOURCES *= src/machine/sharp/pc1245.cpp
RESOURCES *= resources/pc1245.qrc
}
# PC1250
contains(PROJECT_PACKAGE,PC1250) {
DEFINES *= P_PC1250
HEADERS *= src/machine/sharp/pc1250.h \
           src/core/Keyb1250.h \
           src/lcd/Lcdc_pc1250.h
SOURCES *= src/machine/sharp/pc1250.cpp \
           src/lcd/Lcdc_pc1250.cpp
RESOURCES *= resources/pc1250.qrc
OTHER_FILES *=  resources/keymap/pc1250.map \
}

# PC1251
contains(PROJECT_PACKAGE,PC1251) {
DEFINES *= P_PC1251
HEADERS *= src/machine/sharp/pc1251.h \
           src/core/Keyb1251.h
SOURCES *= src/machine/sharp/pc1251.cpp
RESOURCES *= resources/pc1251.qrc
}
# PC1253
contains(PROJECT_PACKAGE,PC1251) {
DEFINES *= P_PC1253
HEADERS *= src/machine/sharp/pc1253.h
SOURCES *= src/machine/sharp/pc1253.cpp
RESOURCES *=
}
# PC1255
contains(PROJECT_PACKAGE,PC1255) {
DEFINES *= P_PC1255
HEADERS *= src/machine/sharp/pc1255.h
SOURCES *= src/machine/sharp/pc1255.cpp
RESOURCES *= resources/pc1255.qrc
}
# PC1260 61 62
contains(PROJECT_PACKAGE,PC1260) {
DEFINES *= P_PC1260
HEADERS *= src/machine/sharp/pc1260.h
SOURCES *= src/machine/sharp/pc1260.cpp
RESOURCES *= resources/pc1260.qrc \
             resources/pc1261.qrc \
             resources/pc1262.qrc
}
# PC1280
contains(PROJECT_PACKAGE,PC1280) {
DEFINES *= P_PC1280
HEADERS *= src/machine/sharp/pc1280.h \
           src/lcd/Lcdc_pc1475.h  \
           src/core/Keyb1280.h
SOURCES *= src/machine/sharp/pc1280.cpp \
           src/lcd/Lcdc_pc1475.cpp
RESOURCES *= resources/pc1280.qrc
OTHER_FILES *=  resources/keymap/pc1280.map \
}
# PC1350
contains(PROJECT_PACKAGE,PC1350) {
DEFINES *= P_PC1350
HEADERS *= src/machine/sharp/pc1350.h \
           src/lcd/Lcdc_pc1350.h \
           src/core/Keyb1350.h
SOURCES *= src/machine/sharp/pc1350.cpp \
           src/lcd/Lcdc_pc1350.cpp
RESOURCES *= resources/pc1350.qrc
OTHER_FILES *=  resources/keymap/pc1350.map \
}
# PC1360
contains(PROJECT_PACKAGE,PC1360) {
DEFINES *= P_PC1360
HEADERS *= src/machine/sharp/pc1360.h \
           src/core/Keyb1360.h
SOURCES *= src/machine/sharp/pc1360.cpp
RESOURCES *= resources/pc1360.qrc
OTHER_FILES *=  resources/keymap/pc1360.map \
}

# PC2500
contains(PROJECT_PACKAGE,PC2500) {
DEFINES *= P_PC2500
HEADERS *= \
    src/machine/sharp/pc2500.h \
    src/core/Keyb2500.h
SOURCES *= src/machine/sharp/pc2500.cpp
RESOURCES *= resources/pc2500.qrc
OTHER_FILES *= resources/keymap/pc2500.map
}

# PC1401
contains(PROJECT_PACKAGE,PC1401) {
DEFINES *= P_PC1401
HEADERS *= src/machine/sharp/pc1401.h \
           src/lcd/Lcdc_pc1401.h \
           src/core/Keyb1401.h
SOURCES *= src/machine/sharp/pc1401.cpp \
           src/lcd/Lcdc_pc1401.cpp
RESOURCES *= resources/pc1401.qrc
OTHER_FILES *=  resources/keymap/pc1401.map \
}
# PC1402
contains(PROJECT_PACKAGE,PC1402) {
DEFINES *= P_PC1402
HEADERS *= src/machine/sharp/pc1402.h
SOURCES *= src/machine/sharp/pc1402.cpp
RESOURCES *= resources/pc1402.qrc
}
# PC1403
contains(PROJECT_PACKAGE,PC1403) {
DEFINES *= P_PC1403
HEADERS *= src/machine/sharp/pc1403.h \
           src/lcd/Lcdc_pc1403.h \
           src/core/Keyb1403.h
SOURCES *= src/machine/sharp/pc1403.cpp \
           src/lcd/Lcdc_pc1403.cpp
RESOURCES *= resources/pc1403.qrc
OTHER_FILES *=  resources/keymap/pc1403.map \
}
# PC1421
contains(PROJECT_PACKAGE,PC1421) {
DEFINES *= P_PC1421
HEADERS *= src/machine/sharp/pc1421.h
SOURCES *= src/machine/sharp/pc1421.cpp
RESOURCES *= resources/pc1421.qrc
}
# PC1425
contains(PROJECT_PACKAGE,PC1425) {
DEFINES *= P_PC1425
HEADERS *= src/machine/sharp/pc1425.h
SOURCES *= src/machine/sharp/pc1425.cpp
RESOURCES *= resources/pc1425.qrc
OTHER_FILES *=  resources/keymap/pc1425.map \
}

# PC1450
contains(PROJECT_PACKAGE,PC1450) {
DEFINES *= P_PC1450
HEADERS *= src/machine/sharp/pc1450.h \
           src/lcd/Lcdc_pc1450.h
SOURCES *= src/machine/sharp/pc1450.cpp \
           src/lcd/Lcdc_pc1450.cpp
RESOURCES *= resources/pc1450.qrc
OTHER_FILES *=  resources/keymap/pc1450.map \
}

# PC1475
contains(PROJECT_PACKAGE,PC1475) {
DEFINES *= P_PC1475
HEADERS *= src/machine/sharp/pc1475.h \
           src/lcd/Lcdc_pc1475.h
SOURCES *= src/machine/sharp/pc1475.cpp \
           src/lcd/Lcdc_pc1475.cpp
RESOURCES *= resources/pc1475.qrc
OTHER_FILES *=  resources/keymap/pc1475.map \
}

# PC1500
contains(PROJECT_PACKAGE,PC1500) {
DEFINES *= P_PC1500
HEADERS *= \
    src/machine/sharp/buspc1500.h \
    src/machine/sharp/pc1500.h \
    src/lcd/Lcdc_pc1500.h \
    src/core/Keyb1500.h
SOURCES *= \
    src/machine/sharp/buspc1500.cpp \
    src/machine/sharp/pc1500.cpp \
    src/lcd/Lcdc_pc1500.cpp
RESOURCES *= resources/pc1500.qrc
OTHER_FILES *=  resources/keymap/pc1500.map
}

# PC1600
contains(PROJECT_PACKAGE,PC1600) {
DEFINES *= P_PC1600
HEADERS *= \
    src/core/Keyb1600.h \
    src/machine/sharp/pc1600.h \
    src/machine/sharp/ce1600p.h \
    src/machine/sharp/ce1600f.h \
    src/lcd/Lcdc_pc1600.h
SOURCES *= \
    src/machine/sharp/pc1600.cpp \
    src/machine/sharp/ce1600p.cpp \
    src/machine/sharp/ce1600f.cpp \
    src/lcd/Lcdc_pc1600.cpp
RESOURCES *= \
    resources/pc1600.qrc
OTHER_FILES *= \
    resources/keymap/pc1600.map \
    resources/keymap/ce1600p.map
}

# G850
contains(PROJECT_PACKAGE,G850) {
DEFINES *= P_G850
HEADERS *= \
    src/machine/sharp/g850v.h \
    src/lcd/Lcdc_g850.h
SOURCES *= \
    src/machine/sharp/g850v.cpp \
    src/lcd/Lcdc_g850.cpp
RESOURCES *= \
    resources/g850v.qrc
OTHER_FILES *= \
    resources/keymap/g850v.map
}

# CE126P and others
contains(PROJECT_PACKAGE,CE126P) {
DEFINES *= P_CE126P
HEADERS *= \
    src/machine/sharp/ce125.h \
    src/machine/sharp/Ce126.h \
    src/machine/sharp/ce120p.h
SOURCES *= \
    src/machine/sharp/Ce126.cpp \
    src/machine/sharp/ce125.cpp \
    src/machine/sharp/ce120p.cpp
RESOURCES *= resources/ce126p.qrc
OTHER_FILES *= \
    resources/keymap/ce125tape.map \
    resources/keymap/ce126.map \
    resources/keymap/ce125.map \
    resources/keymap/ce129.map \
    resources/keymap/ce123.map \
    resources/keymap/ce120.map \
    resources/keymap/263591.map
}


# SC61860
contains(PROJECT_PACKAGE,SC61860) {
DEFINES *= P_SC61860
FORMS *= ui/cregssc61860widget.ui
HEADERS *= src/cpu/sc61860.h \
           src/cpu/sc61860d.h
SOURCES *= src/cpu/sc61860.cpp \
           src/cpu/sc61860d.cpp \
           ui/cregssc61860widget.cpp
}
# SC62015
contains(PROJECT_PACKAGE,SC62015) {
DEFINES *= P_SC62015
FORMS *= ui/cregsz80widget.ui
HEADERS *= src/cpu/sc62015.h \
           src/cpu/sc62015d.h
SOURCES *= src/cpu/sc62015.cpp \
           src/cpu/sc62015d.cpp
}

# LH5801
contains(PROJECT_PACKAGE,LH5801) {
DEFINES *= P_LH5801
FORMS   *=   ui/cregslh5801widget.ui
HEADERS *= \
    ui/cregslh5801widget.h \
    src/cpu/lh5801.h \
    src/cpu/lh5801d.h \
    src/cpu/lh5803.h
SOURCES *= \
    ui/cregslh5801widget.cpp \
    src/cpu/lh5801.cpp \
    src/cpu/lh5801d.cpp \
    src/cpu/lh5803.cpp
}

# TMS7000
contains(PROJECT_PACKAGE,TMS7000) {
DEFINES *= P_TMS7000
FORMS *= ui/cregsz80widget.ui
HEADERS *= \
    src/cpu/tms7000/tms7000.h \
    src/cpu/tms7000/tms7000d.h
SOURCES *= \
    src/cpu/tms7000/tms7000.cpp \
    src/cpu/tms7000/tms7000d.cpp \
}

# HD44102
contains(PROJECT_PACKAGE,HD44102) {
DEFINES *= P_HD44102
HEADERS *= src/cpu/hd44102.h
SOURCES *= src/cpu/hd44102.cpp \
}

# MC6800
contains(PROJECT_PACKAGE,MC6800) {
DEFINES *= P_MC6800
FORMS *= ui/cregsz80widget.ui
HEADERS *= src/cpu/mc6800/mc6800.h \
           src/cpu/mc6800/mc6800d.h
SOURCES *= src/cpu/mc6800/mc6800.cpp \
           src/cpu/mc6800/mc6800d.cpp
}

# M6502
contains(PROJECT_PACKAGE,M6502) {
DEFINES *= P_M6502
FORMS *= ui/cregsz80widget.ui
HEADERS *= \
    src/cpu/m6502/m6502_dasm.h \
    src/cpu/m6502/m6502.h \
    src/cpu/m6502/opsc02.h \
    src/cpu/m6502/ops02.h
SOURCES *= \
    src/cpu/m6502/m6502.cpp \
    src/cpu/m6502/m6502_dasm.cpp
}

# JR800
contains(PROJECT_PACKAGE,JR800) {
DEFINES *= P_JR800
HEADERS *= src/machine/jr800.h \
           src/lcd/lcdc_jr800.h
SOURCES *= src/machine/jr800.cpp \
           src/lcd/lcdc_jr800.cpp
RESOURCES *= resources/jr800.qrc
OTHER_FILES *=
    resources/keymap/jr800.map
}
# HX20
contains(PROJECT_PACKAGE,HX20) {
DEFINES *= P_HX20
HEADERS *= src/machine/epson/hx20.h \
           src/machine/epson/hx20rc.h \
           src/machine/epson/m160.h \
           src/lcd/lcdc_hx20.h
SOURCES *= src/machine/epson/hx20.cpp \
           src/machine/epson/hx20rc.cpp \
           src/machine/epson/m160.cpp \
           src/lcd/lcdc_hx20.cpp
RESOURCES *= resources/hx20.qrc
OTHER_FILES *= cloud/hx20rc.qml  \
             resources/keymap/hx20.map \
             resources/hx20/hx20rcslots.xml
}

# PANASONIC
contains(PROJECT_PACKAGE,PANASONIC) {
DEFINES *= P_PANASONIC
HEADERS *= \
    src/machine/panasonic/buspanasonic.h \
    src/machine/panasonic/rlp3001.h \
    src/machine/panasonic/rlext.h \
    src/machine/panasonic/rlh1000.h \
    src/machine/panasonic/rlp4002.h \
    src/machine/panasonic/rlp1002.h \
    src/machine/panasonic/rlp2001.h \
    src/machine/panasonic/rlp1005.h \
    src/machine/panasonic/rlp6001.h \
    src/machine/panasonic/rlp9001.h \
    src/machine/panasonic/rlp1004a.h \
    src/lcd/Lcdc_rlh1000.h
SOURCES *= \
    src/machine/panasonic/buspanasonic.cpp \
    src/machine/panasonic/rlh1000.cpp \
    src/machine/panasonic/rlp6001.cpp \
    src/machine/panasonic/rlp9001.cpp \
    src/machine/panasonic/rlp1004a.cpp \
    src/machine/panasonic/rlp4002.cpp \
    src/machine/panasonic/rlp1002.cpp \
    src/machine/panasonic/rlp2001.cpp \
    src/machine/panasonic/rlp3001.cpp \
    src/machine/panasonic/rlp1005.cpp \
    src/machine/panasonic/rlext.cpp \
    src/lcd/Lcdc_rlh1000.cpp
RESOURCES *= resources/rlh1000.qrc
OTHER_FILES *= \
    resources/keymap/rlh1000.map \
    resources/keymap/rlp9006.map \
    resources/keymap/rlp1004a.map \
    resources/keymap/rlp4002.map \
    resources/keymap/rlp1002.map \
    resources/keymap/rlp1005.map
}

# HP_ALL
contains(PROJECT_PACKAGE,HP_ALL) {
DEFINES *= P_HP_ALL
HEADERS *=  src/cpu/hpnut.h \
            src/cpu/hpnutd.h \
            src/machine/hp/hp15c.h \
            src/machine/hp/hp41.h \
            src/machine/hp/hp41Cpu.h \
            src/machine/hp/hp41mod.h \
            src/machine/hp/hp82143A.h \
            src/lcd/Lcdc_hp41.h \
            src/lcd/Lcdc_hp15c.h
SOURCES *=  src/cpu/hpnut.cpp \
            src/cpu/hpnutd.cpp \
            src/machine/hp/hp15c.cpp \
            src/machine/hp/hp41.cpp \
            src/machine/hp/hp41Cpu.cpp \
            src/machine/hp/hp41mod.cpp \
            src/machine/hp/hp41trace.cpp \
            src/machine/hp/hp41display.cpp \
            src/machine/hp/hp41Timer.cpp \
            src/machine/hp/hp41File.cpp \
            src/machine/hp/hp82143A.cpp \
            src/lcd/Lcdc_hp41.cpp \
            src/lcd/Lcdc_hp15c.cpp
RESOURCES *= resources/hp41.qrc \
            resources/hp15c.qrc \
            resources/hp11c.qrc \
            resources/hp12c.qrc \
            resources/hp16c.qrc
OTHER_FILES *= resources/keymap/hp41.map \
            resources/keymap/hp82143a.map \
            resources/keymap/hp15c.map
}

# TI_ALL
contains(PROJECT_PACKAGE,TI_ALL) {
DEFINES *= P_TI_ALL
HEADERS *= src/cpu/ti57cpu.h \
           src/cpu/ti57cpu_dasm.h \
           src/cpu/tmc0501.h \
           src/cpu/tmc0501_dasm.h \
           src/machine/ti/ti57.h \
           src/machine/ti/ti59.h \
           src/machine/ti/pc100.h \
           src/machine/ti/cc40.h \
           src/machine/ti/ti74.h \
           src/lcd/Lcdc_ti57.h \
           src/lcd/lcdc_ti59.h \
           src/lcd/Lcdc_ti74.h \
           src/lcd/Lcdc_cc40.h
SOURCES *= src/cpu/ti57cpu.cpp \
           src/cpu/ti57cpu_dasm.cpp \
           src/cpu/tmc0501.cpp \
           src/cpu/tmc0501_dasm.cpp \
           src/machine/ti/ti57.cpp \
           src/machine/ti/ti59.cpp \
           src/machine/ti/pc100.cpp \
           src/machine/ti/cc40.cpp \
           src/machine/ti/ti74.cpp \
           src/lcd/Lcdc_ti57.cpp \
           src/lcd/lcdc_ti59.cpp \
           src/lcd/Lcdc_ti74.cpp \
           src/lcd/Lcdc_cc40.cpp
RESOURCES *= resources/cc40.qrc \
            resources/ti74.qrc \
            resources/ti57.qrc \
            resources/ti59.qrc
OTHER_FILES *= resources/keymap/ti57.map \
               resources/keymap/cc40.map \
               resources/keymap/ti74.map \
               resources/keymap/ti95.map \
               resources/keymap/ti59.map \
               resources/keymap/pc100.map
}

# CASIO_ALL
contains(PROJECT_PACKAGE,CASIO_ALL) {
DEFINES *= P_CASIO_ALL
HEADERS *= \
    ui/cregshd61700widget.h \
    src/cpu/hd61700.h \
    src/cpu/hd61700d.h \
    src/machine/casio/casiodisk.h \
    src/machine/casio/fa80.h \
    src/machine/casio/fp40.h \
    src/machine/casio/fp100.h \
    src/machine/casio/fp200.h \
    src/machine/casio/fx8000g.h \
    src/machine/casio/md100.h \
    src/machine/casio/pb1000.h \
    src/machine/casio/pb2000.h \
    src/machine/casio/z1.h \
    src/lcd/Lcdc_fx8000g.h \
    src/lcd/Lcdc_pb1000.h \
    src/lcd/Lcdc_fp200.h \
    src/lcd/Lcdc_z1.h
SOURCES *= \
    ui/cregshd61700widget.cpp \
    src/cpu/hd61700.cpp \
    src/cpu/hd61700d.cpp \
    src/machine/casio/casiodisk.cpp \
    src/machine/casio/fa80.cpp \
    src/machine/casio/fp40.cpp \
    src/machine/casio/fp100.cpp \
    src/machine/casio/fp200.cpp \
    src/machine/casio/fx8000g.cpp \
    src/machine/casio/md100.cpp \
    src/machine/casio/pb1000.cpp \
    src/machine/casio/pb2000.cpp \
    src/machine/casio/z1.cpp \
    src/lcd/Lcdc_fx8000g.cpp \
    src/lcd/Lcdc_pb1000.cpp \
    src/lcd/Lcdc_fp200.cpp \
    src/lcd/Lcdc_z1.cpp
FORMS *=    ui/cregshd61700widget.ui
RESOURCES *= \
    resources/fx8000g.qrc \
    resources/z1.qrc \
    resources/pb1000.qrc \
    resources/pb2000.qrc \
    resources/fp200.qrc
OTHER_FILES *= \
    resources/keymap/fx8000g.map \
    resources/keymap/fp200.map \
    resources/keymap/fp40.map \
    resources/keymap/z1.map \
    resources/keymap/pb1000.map \
    resources/keymap/pb2000.map
}

FORMS *= ui/about.ui \
    ui/dialoganalog.ui \
    ui/dialogconnectorlink.ui \
    ui/dialogdump.ui \
    ui/dialogkeylist.ui \
    ui/dialoglog.ui \
    ui/pockemul.ui \
    ui/serialconsole.ui \
    ui/startup.ui \
    ui/dialogpotar.ui \
    ui/simulatorconsole.ui \
    ui/dialogdasm.ui \
    ui/cregsz80widget.ui \
    ui/uartconsole.ui \
    ui/dialogvkeyboard.ui


HEADERS *= \
    src/core/Connect.h \
    src/core/Dasm.h \
    src/core/Debug.h \
    src/core/Inter.h \
    src/core/Keyb.h \
    src/core/Keyb1450.h \
    src/core/keybce150.h \
    src/core/keybce152.h \
    src/core/Log.h \
    src/core/analog.h \
    src/core/autoupdater.h \
    src/core/clink.h \
    src/core/common.h \
    src/core/dialogabout.h \
    src/core/dialoganalog.h \
    src/core/dialogconsole.h \
    src/core/dialogdump.h \
    src/core/dialogkeylist.h \
    src/core/dialoglog.h \
    src/core/dialogstartup.h \
    src/core/global.h \
    src/core/init.h \
    src/core/mainwindowpockemul.h \
    src/core/pobject.h \
    src/core/slot.h \
    src/core/xmlwriter.h \
    src/core/lcc/parser/parser.h \
    src/core/dialogpotar.h \
    src/core/cextension.h \
    src/core/weblinksparser.h \
#    src/core/wavfile.h \
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
    src/core/renderView.h \
    src/core/breakpoint.h \
    src/core/watchpoint.h \
    src/core/launchbuttonwidget.h \
    src/cpu/upd7907/upd7907d.h \
    src/cpu/z80_dasm.h \
    src/core/pobjectInterface.h \
    src/machine/sharp/cemem.h \
    src/machine/extslot.h \
    cloud/pocketimageprovider.h \



HEADERS *= \
    src/cpu/cpu.h \
    src/cpu/lh5810.h \
    src/cpu/pd1990ac.h \
    src/cpu/z80.h \
    src/cpu/z80memory.h \
    src/cpu/hd61102.h \
    src/cpu/z80-2.h \
    src/cpu/lu57813p.h \
    src/cpu/ct6834.h \
    src/cpu/tc8576p.h \
    src/cpu/uart.h \
    src/cpu/hd44352.h \
    src/cpu/cf79107pj.h \
    src/cpu/i8085.h \
    src/cpu/i8085cpu.h \
    src/cpu/i8085daa.h \
    src/cpu/rp5c01.h \
    src/cpu/ctronics.h \
    src/cpu/sed1560.h \
    src/cpu/i80x86.h \
    src/cpu/hd66108.h \
    src/cpu/upd16434.h \
    src/cpu/upd7907/upd7907.h \
    src/cpu/i80L188EB.h \
    src/cpu/pit8253.h \
    src/cpu/s6b0108.h \
    src/cpu/mc6847.h \
    src/cpu/hd44780.h \
    src/cpu/upd1007.h \
    src/cpu/upd1007d.h \
    src/cpu/i8085_dasm.h \
    src/cpu/i80x86_dasm.h \
    src/core/modelids.h \
    src/cpu/hd61710.h \

HEADERS *= \
    src/machine/pcxxxx.h \
    src/machine/bus.h \
    src/machine/sharp/ce122.h \
    src/machine/sharp/ce150.h \
    src/machine/sharp/ce152.h \
    src/machine/sharp/ce140p.h \
    src/machine/sharp/ce140f.h \
    src/machine/sharp/ce1560.h \
    src/machine/sharp/ce162e.h \
    src/machine/sharp/ce153.h \
    src/machine/sharp/ce2xxx.h \
    src/machine/cx07char.h \
    src/machine/pc2001.h \
    src/machine/pc2021.h \
    src/machine/printerctronics.h \
    src/machine/general/lbc1100.h \
    src/machine/general/cl1000.h \
    src/machine/cesimu.h \
    src/machine/tpc8300.h \
    src/machine/tp83.h \
    src/machine/postit.h \
    src/machine/paperwidget.h \
    src/machine/cprinter.h \
    src/machine/sio.h \
    src/machine/potar.h \
    src/machine/cmotor.h \
    src/machine/ce515p.h \
    src/machine/cx07.h \
    src/machine/cx710.h \
    src/machine/ccable.h \

HEADERS *= \
    src/lcd/Lcdc.h \
    src/lcd/Lcdc_x07.h \
    src/lcd/Lcdc_symb.h \
    src/lcd/Lcdc_lbc1100.h \
    src/lcd/Lcdc_tpc8300.h \
    src/lcd/Lcdc_pc2001.h \
    src/lcd/Lcdc_ce1560.h \
    src/lcd/Lcdc_symb2x.h \


HEADERS *= \
    qcodemodel2/qcodenode.h \
    ui/dialogdasm.h \
    ui/cregssc61860widget.h \
    ui/cregcpu.h \
    ui/cregsz80widget.h \
    ui/uartconsole.h \
    ui/dialogvkeyboard.h \
    pictureflow/pictureflow.h \
    pictureflow/materialflow.h \
    pictureflow/fluidlauncher.h \
    pictureflow/launcher.h \
    pictureflow/slideshow.h \

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
    resources/stdlibs.qrc \
    qcodeedit/qxs/qxs.qrc \
    resources/asmlibs.qrc \
    resources/core.qrc \
    resources/pockemul.qrc \
    resources/ce2xxx.qrc \
    resources/ext.qrc \
    resources/pc1460.qrc \
    resources/ext2.qrc \
    resources/pc2001.qrc \
    resources/lbc1100.qrc \
    resources/tpc8300.qrc \
    resources/x07.qrc \
    resources/cemem.qrc \


}
else {
RESOURCES =
DEFINES += LOCRES
}




SOURCES *=  \
    src/core/viewobject.cpp \
    src/core/pobject.cpp \
    src/core/pockemul.cpp \
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
    src/core/dialogconsole.cpp \
    src/core/dialogdump.cpp \
    src/core/dialogkeylist.cpp \
    src/core/dialoglog.cpp \
    src/core/dialogstartup.cpp \
    src/core/init.cpp \
    src/core/mainwindowpockemul.cpp \
    src/core/slot.cpp \
    src/core/xmlwriter.cpp \
    src/core/lcc/parser/parser.cpp \
    src/core/dialogsimulator.cpp \
    src/core/bineditor/bineditor.cpp \
    src/core/bineditor/colorscheme.cpp \
    src/core/dialogpotar.cpp \
    src/core/weblinksparser.cpp \
#    src/core/wavfile.cpp \
    src/core/launchbuttonwidget.cpp \
    src/core/qcommandline.cpp \
    src/core/renderView.cpp \
    src/core/breakpoint.cpp \
    src/core/sizegrip.cpp \
    src/core/servertcp.cpp \
    src/core/downloadmanager.cpp \
    src/core/cextension.cpp \
    ui/uartconsole.cpp \
    ui/dialogvkeyboard.cpp \
    pictureflow/pictureflow.cpp \
    pictureflow/materialflow.cpp \
    pictureflow/fluidlauncher.cpp \
    pictureflow/launcher.cpp \
    pictureflow/slideshow.cpp \
    src/machine/sharp/cemem.cpp \
    src/machine/extslot.cpp \
    cloud/pocketimageprovider.cpp \




SOURCES *=  \
    src/machine/pcxxxx.cpp \
    src/machine/bus.cpp \
    src/machine/cprinter.cpp \
    src/machine/paperwidget.cpp \
    src/machine/sharp/ce140p.cpp \
    src/machine/sharp/ce150.cpp \
    src/machine/sharp/ce152.cpp \
    src/machine/sharp/ce1560.cpp \
    src/machine/sharp/ce162e.cpp \
    src/machine/sharp/ce153.cpp \
    src/machine/sharp/ce122.cpp \
    src/machine/sharp/ce2xxx.cpp \
    src/machine/sharp/ce140f.cpp \
    src/machine/printerctronics.cpp \
    src/machine/general/lbc1100.cpp \
    src/machine/general/cl1000.cpp \
    src/machine/sio.cpp \
    src/machine/cx07.cpp \
    src/machine/cx710.cpp \
    src/machine/pc2001.cpp \
    src/machine/pc2021.cpp \
    src/machine/cesimu.cpp \
    src/machine/postit.cpp \
    src/machine/potar.cpp \
    src/machine/cmotor.cpp \
    src/machine/ce515p.cpp \
    src/machine/tpc8300.cpp \
    src/machine/tp83.cpp \
    src/machine/ccable.cpp \
    qcodemodel2/qcodenode.cpp \


SOURCES *=  \
    ui/dialogdasm.cpp \
    ui/cregcpu.cpp \
    ui/cregsz80widget.cpp \
    src/cpu/cpu.cpp \
    src/cpu/z80.cpp \
    src/cpu/z80_dasm.cpp \
    src/cpu/i8085.cpp \
    src/cpu/i8085_dasm.cpp \
    src/cpu/i80x86.cpp \
    src/cpu/i80x86_dasm.cpp \
    src/cpu/i80L188EB.cpp \
    src/cpu/upd7907/upd7907d.cpp \
    src/cpu/upd7907/upd7907.cpp \
    src/cpu/pd1990ac.cpp \
    src/cpu/lh5810.cpp \
    src/cpu/ct6834.cpp \
    src/cpu/hd61102.cpp \
    src/cpu/lu57813p.cpp \
    src/cpu/tc8576p.cpp \
    src/cpu/uart.cpp \
    src/cpu/hd44352.cpp \
    src/cpu/hd66108.cpp \
    src/cpu/upd16434.cpp \
    src/cpu/cf79107pj.cpp \
    src/cpu/rp5c01.cpp \
    src/cpu/ctronics.cpp \
    src/cpu/pit8253.cpp \
    src/cpu/s6b0108.cpp \
    src/cpu/mc6847.cpp \
    src/cpu/hd44780.cpp \
    src/cpu/upd1007.cpp \
    src/cpu/upd1007d.cpp \
    src/cpu/hd61710.cpp \
    src/cpu/sed1560.cpp \


SOURCES *=  \
    src/lcd/Lcdc.cpp \
    src/lcd/Lcdc_x07.cpp \
    src/lcd/Lcdc_pc2001.cpp \
    src/lcd/Lcdc_lbc1100.cpp \
    src/lcd/Lcdc_tpc8300.cpp \
    src/lcd/Lcdc_ce1560.cpp \


greaterThan(QT_MAJOR_VERSION, 4): SOURCES += \


OTHER_FILES += \
    resources/weblinks.xml \
    resources/notes.txt \
    resources/pockemul/release_notes.html \
    resources/pockemul/script_help.html \
    resources/pockemul/config.xml \
    resources/pockemul/configExt.xml \
    resources/keymap/trspc2.map \
    resources/keymap/ce150.map \
    resources/keymap/ce140p.map \
    resources/keymap/x07.map \
    resources/keymap/e500.map \
    resources/keymap/x710.map \
    resources/keymap/pc1260.map \
    resources/keymap/pc1211.map \
    resources/keymap/ce122.map \
    resources/keymap/pc2001.map \
    resources/keymap/pc2021.map \
    resources/keymap/ce127r.map \
    resources/keymap/ce152.map \
    resources/keymap/pc2081.map \
    resources/keymap/lbc1100.map \
    resources/keymap/cl1000.map \
    resources/pockemul/weblinks.xml \
    resources/keymap/tpc8300.map \
    resources/keymap/tp83.map \
    resources/keymap/ce1560.map \
    resources/keymap/ce140f.map \
    resources/keymap/pc1253.map \
    logitheque/serial1251.qs \
    qcodeedit/qxs/marks.qxm \
    qcodeedit/qxs/xml.qxf \
    qcodeedit/qxs/tex.qxf \
    qcodeedit/qxs/qtscript.qxf \
    qcodeedit/qxs/python.qxf \
    qcodeedit/qxs/pro.qxf \
    qcodeedit/qxs/php.qxf \
    qcodeedit/qxs/formats.qxf \
    qcodeedit/qxs/dox.qxf \
    qcodeedit/qxs/css.qxf \
    qcodeedit/qxs/csharp.qxf \
    qcodeedit/qxs/cpp.qxf \
    qcodeedit/qxs/bibtex.qxf \
    qcodeedit/qxs/xml.qnfa \
    qcodeedit/qxs/tex.qnfa \
    qcodeedit/qxs/snippet.qnfa \
    qcodeedit/qxs/qtscript.qnfa \
    qcodeedit/qxs/python.qnfa \
    qcodeedit/qxs/pro.qnfa \
    qcodeedit/qxs/php.qnfa \
    qcodeedit/qxs/dox.qnfa \
    qcodeedit/qxs/css.qnfa \
    qcodeedit/qxs/csharp.qnfa \
    qcodeedit/qxs/cpp.qnfa \
    qcodeedit/qxs/coldfusion.qnfa \
    qcodeedit/qxs/bibtex.qnfa \
    qcodeedit/qxs/asm.qnfa \
    qcodeedit/qxs/asm.qxf \
    resources/asmlibs/sc61860/xor16.lib \
    resources/asmlibs/sc61860/xor8.lib \
    resources/asmlibs/sc61860/sr16.lib \
    resources/asmlibs/sc61860/sr8.lib \
    resources/asmlibs/sc61860/sl16.lib \
    resources/asmlibs/sc61860/sl8.lib \
    resources/asmlibs/sc61860/mul16.lib \
    resources/asmlibs/sc61860/mul8.lib \
    resources/asmlibs/sc61860/divmod8.lib \
    resources/asmlibs/sc61860/cpse16.lib \
    resources/asmlibs/sc61860/cps16.lib \
    resources/asmlibs/sc61860/cpne16.lib \
    resources/asmlibs/sc61860/cpge16.lib \
    resources/asmlibs/sc61860/cpg16.lib \
    resources/asmlibs/sc61860/cpeq16.lib \
    resources/asmlibs/sc61860/cpe16.lib \
    resources/asmlibs/sc61860/divmod16.lib \
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
    android/AndroidManifest.xml \
    android/src/org/qtproject/pockemul/PockemulActivity.java \



contains(PROJECT_MODULE,M_IDE) {
DEFINES += P_IDE

HEADERS += \
    src/core/lcc/lcc.h \
    src/core/ide/highlighter.h \
    src/core/lcc/lcpp.h \
    src/core/lcc/codegen.h \
    src/core/lcc/pasm.h \
    resources/lcc/model/PC-1350/stdio.h \
    resources/lcc/model/PC-1350/stdio.h \
    src/core/lcc/cstdlib.h \
    resources/lcc/model/PC-1350/graph.h \
    qcodeedit/qreliablefilewatch.h \
    qcodeedit/qpanellayout.h \
    qcodeedit/qlinemarksinfocenter.h \
    qcodeedit/qlanguagefactory.h \
    qcodeedit/qlanguagedefinition.h \
    qcodeedit/qformatscheme.h \
    qcodeedit/qformatfactory.h \
    qcodeedit/qformat.h \
    qcodeedit/qeditsession.h \
    qcodeedit/qeditorinputbindinginterface.h \
    qcodeedit/qeditorinputbinding.h \
    qcodeedit/qeditorfactory.h \
    qcodeedit/qeditor.h \
    qcodeedit/qcodeedit.h \
    qcodeedit/qcodecompletionengine.h \
    qcodeedit/qce-config.h \
    qcodeedit/document/qdocumentsearch.h \
    qcodeedit/document/qdocumentline_p.h \
    qcodeedit/document/qdocumentline.h \
    qcodeedit/document/qdocumentcursor_p.h \
    qcodeedit/document/qdocumentcursor.h \
    qcodeedit/document/qdocumentcommand.h \
    qcodeedit/document/qdocumentbuffer.h \
    qcodeedit/document/qdocument_p.h \
    qcodeedit/document/qdocument.h \
    qcodeedit/qnfa/qnfadefinition.h \
    qcodeedit/qnfa/qnfa.h \
    qcodeedit/qnfa/light_vector.h \
    qcodeedit/snippets/qsnippetpatternloader.h \
    qcodeedit/snippets/qsnippetmanager.h \
    qcodeedit/snippets/qsnippetedit.h \
    qcodeedit/snippets/qsnippetbinding.h \
    qcodeedit/snippets/qsnippet_p.h \
    qcodeedit/snippets/qsnippet.h \
    qcodeedit/widgets/qstatuspanel.h \
    qcodeedit/widgets/qsimplecolorpicker.h \
    qcodeedit/widgets/qsearchreplacepanel.h \
    qcodeedit/widgets/qpanel.h \
    qcodeedit/widgets/qlinenumberpanel.h \
    qcodeedit/widgets/qlinemarkpanel.h \
    qcodeedit/widgets/qlinechangepanel.h \
    qcodeedit/widgets/qgotolinepanel.h \
    qcodeedit/widgets/qgotolinedialog.h \
    qcodeedit/widgets/qformatconfig.h \
    qcodeedit/widgets/qfoldpanel.h \
    qcodeedit/widgets/qeditconfig.h \
    qcodeedit/widgets/qcalltip.h \
    src/core/ide/editorwidget.h \
    ui/windowide.h \
    qcodeedit/widgets/qhexpanel.h \
    qcodeedit/widgets/qoutpanel.h \
    resources/lcc/model/PC-1350/sound.h \
    resources/lcc/model/__sound.h \
    resources/lcc/model/PC-1350/internal.h \
    resources/lcc/model/PC-1251/internal.h \
    resources/lcc/model/PC-1251/sound.h \
    resources/lcc/model/__stdio.h \
    resources/lcc/model/PC-1251/stdio.h \
    resources/lcc/model/__internal.h \
    resources/lcc/model/PC-1262/stdio.h \
    resources/lcc/model/PC-1262/sound.h \
    resources/lcc/model/PC-1262/internal.h \
    resources/lcc/model/PC-1360/stdio.h \
    resources/lcc/model/PC-1360/sound.h \
    resources/lcc/model/PC-1360/internal.h \
    resources/lcc/model/PC-1360/graph.h \
    qcodeedit/ccompletion.h \
    qcodeedit/qcodecompletionwidget_p.h \
    qcodeedit/qcodecompletionwidget.h \
    resources/lcc/model/PC-1261/stdio.h \
    resources/lcc/model/PC-1261/sound.h \
    resources/lcc/model/PC-1261/internal.h \
    resources/lcc/model/PC-1250/stdio.h \
    resources/lcc/model/PC-1250/sound.h \
    resources/lcc/model/PC-1250/internal.h \
    resources/lcc/model/PC-1255/stdio.h \
    resources/lcc/model/PC-1255/sound.h \
    resources/lcc/model/PC-1255/internal.h \
    resources/lcc/model/PC-1260/stdio.h \
    resources/lcc/model/PC-1260/sound.h \
    resources/lcc/model/PC-1260/internal.h \
    resources/lcc/model/PC-1475/stdio.h \
    resources/lcc/model/PC-1475/sound.h \
    resources/lcc/model/PC-1475/internal.h \

SOURCES += \
    src/core/lcc/lcc.cpp \
    src/core/lcc/calcunit.cpp \
    src/core/ide/highlighter.cpp \
    src/core/lcc/lcpp.cpp \
    src/core/lcc/codegen.cpp \
    src/core/lcc/pasm.cpp \
    src/core/lcc/cstdlib.cpp \
    qcodeedit/qreliablefilewatch.cpp \
    qcodeedit/qpanellayout.cpp \
    qcodeedit/qlinemarksinfocenter.cpp \
    qcodeedit/qlanguagefactory.cpp \
    qcodeedit/qlanguagedefinition.cpp \
    qcodeedit/qformatscheme.cpp \
    qcodeedit/qeditsession.cpp \
    qcodeedit/qeditorinputbinding.cpp \
    qcodeedit/qeditorfactory.cpp \
    qcodeedit/qeditor.cpp \
    qcodeedit/qcodeedit.cpp \
    qcodeedit/qcodecompletionengine.cpp \
    qcodeedit/document/qdocumentsearch.cpp \
    qcodeedit/document/qdocumentline.cpp \
    qcodeedit/document/qdocumentcursor.cpp \
    qcodeedit/document/qdocumentcommand.cpp \
    qcodeedit/document/qdocumentbuffer.cpp \
    qcodeedit/document/qdocument.cpp \
    qcodeedit/qnfa/xml2qnfa.cpp \
    qcodeedit/qnfa/qnfadefinition.cpp \
    qcodeedit/qnfa/qnfa.cpp \
    qcodeedit/snippets/qsnippetmanager.cpp \
    qcodeedit/snippets/qsnippetedit.cpp \
    qcodeedit/snippets/qsnippetbinding.cpp \
    qcodeedit/snippets/qsnippet.cpp \
    qcodeedit/widgets/qstatuspanel.cpp \
    qcodeedit/widgets/qsimplecolorpicker.cpp \
    qcodeedit/widgets/qsearchreplacepanel.cpp \
    qcodeedit/widgets/qpanel.cpp \
    qcodeedit/widgets/qlinenumberpanel.cpp \
    qcodeedit/widgets/qlinemarkpanel.cpp \
    qcodeedit/widgets/qlinechangepanel.cpp \
    qcodeedit/widgets/qgotolinepanel.cpp \
    qcodeedit/widgets/qgotolinedialog.cpp \
    qcodeedit/widgets/qformatconfig.cpp \
    qcodeedit/widgets/qfoldpanel.cpp \
    qcodeedit/widgets/qeditconfig.cpp \
    qcodeedit/widgets/qcalltip.cpp \
    src/core/ide/editorwidget.cpp \
    ui/windowide.cpp \
    qcodeedit/widgets/qhexpanel.cpp \
    qcodeedit/widgets/qoutpanel.cpp \
    qcodeedit/ccompletion.cpp \
    qcodeedit/qcodecompletionwidget.cpp \


FORMS += \
    qcodeedit/widgets/searchreplace.ui \
    qcodeedit/widgets/gotolinedialog.ui \
    qcodeedit/widgets/gotoline.ui \
    qcodeedit/widgets/formatconfig.ui \
    qcodeedit/widgets/editconfig.ui \
    qcodeedit/snippets/snippetedit.ui \
    ui/windowide.ui \


}


contains(PROJECT_MODULE,M_LIBAVOID) {
DEFINES += P_AVOID

HEADERS += \
    libavoid/vpsc.h \
    libavoid/visibility.h \
    libavoid/viscluster.h \
    libavoid/vertices.h \
    libavoid/timer.h \
    libavoid/shape.h \
    libavoid/router.h \
    libavoid/orthogonal.h \
    libavoid/obstacle.h \
    libavoid/makepath.h \
    libavoid/libavoid.h \
    libavoid/junction.h \
    libavoid/graph.h \
    libavoid/geomtypes.h \
    libavoid/geometry.h \
    libavoid/debug.h \
    libavoid/connend.h \
    libavoid/connector.h \
    libavoid/connectionpin.h \
    libavoid/assertions.h \

SOURCES += \
    libavoid/vpsc.cpp \
    libavoid/visibility.cpp \
    libavoid/viscluster.cpp \
    libavoid/vertices.cpp \
    libavoid/timer.cpp \
    libavoid/shape.cpp \
    libavoid/router.cpp \
    libavoid/orthogonal.cpp \
    libavoid/obstacle.cpp \
    libavoid/makepath.cpp \
    libavoid/junction.cpp \
    libavoid/graph.cpp \
    libavoid/geomtypes.cpp \
    libavoid/geometry.cpp \
    libavoid/connend.cpp \
    libavoid/connector.cpp \
    libavoid/connectionpin.cpp \


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

OTHER_FILES *= \
    cloud/content/ScrollBar.qml \
    cloud/content/NewsDelegate.qml \
    cloud/content/CategoryDelegate.qml \
    cloud/content/BusyIndicator.qml \
    cloud/content/TabWidget.qml \
    cloud/qmltemplates/CoverFlow/example/main.qml \
    cloud/qmltemplates/CoverFlow/component/CoverFlowDelegate.qml \
    cloud/qmltemplates/CoverFlow/component/CoverFlow.qml \
    cloud/qmltemplates/CoverFlow/component/AlphaGradient.qml \
    cloud/qmltemplates/CoverFlow/component/gfx/splash_screen.png \
    cloud/qmltemplates/CoverFlow/component/gfx/splash.png \
    cloud/qmltemplates/CoverFlow/component/gfx/rotation_icon.png \
    cloud/qmltemplates/CoverFlow/component/gfx/photo_back.png \
    cloud/qmltemplates/CoverFlow/component/gfx/media_browser_visuals.png \
    cloud/qmltemplates/CoverFlow/component/gfx/image_placeholder.png \
    cloud/qmltemplates/CoverFlow/component/gfx/background.png \
    cloud/qmltemplates/CoverFlow/component/gfx/accordionlist.png \
    cloud/qmltemplates/AccordionList/example/AccordionList.qml \
    cloud/qmltemplates/AccordionList/component/ListItem.qml \
    cloud/qmltemplates/AccordionList/component/AccordionListModel.qml \
    cloud/qmltemplates/AccordionList/component/AccordionList.qml \
    cloud/TabbedQuickApp/TabbedUI.qml \
    cloud/TabbedQuickApp/Tab.qml \
    cloud/RatingIndicatorQML/MyRatingIndicator.qml \
    cloud/RatingIndicatorQML/star.png \
    cloud/content/TextButton.qml \
    cloud/content/Settings.qml \
    cloud/content/SearchBox.qml \
    cloud/content/LineInput.qml \
    cloud/content/SettingsDelegate.qml \
    cloud/content/Edit.qml \
    cloud/content/ComboBox.qml \
    cloud/content/Switch.qml \
    cloud/content/knob.svg \
    cloud/content/counter.js \
    cloud/content/SortListModel.qml \
    cloud/content/Test2.qml \
    cloud/content/Grid.qml \
    cloud/content/Message.qml \
    cloud/content/NewsDelegate2.qml \
    cloud/content/Actions.qml \
    cloud/PmlView.qml \
    cloud/PmlView2.qml \
    cloud/Test.qml \
    cloud/Main.qml \
    cloud/Launchmenu.qml

}


ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android

RESOURCES += \















