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
#    opengl \
    xml \
#    multimedia \
    script declarative sensors \

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

PROJECT_TYPE = \
    STANDARD \
#    EMSCRIPTEN \
#    ANDROID \
#   IOS \


android {
ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android-src
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

}

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
    src/machine/sharp/ce120p.h \
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


FORMS += ui/about.ui \
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
    ui/cregssc61860widget.ui \
    ui/cregslh5801widget.ui \
    ui/cregsz80widget.ui \
    ui/uartconsole.ui \
    ui/cregshd61700widget.ui \
    ui/dialogvkeyboard.ui \


HEADERS += src/core/Connect.h \
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
    src/core/dialogconsole.h \
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
    src/cpu/cpu.h \
    src/cpu/lh5801.h \
    src/cpu/lh5810.h \
    src/cpu/pd1990ac.h \
    src/cpu/sc61860.h \
    src/cpu/sc62015.h \
    src/machine/ccable.h \
    src/machine/sharp/ce125.h \
    src/machine/sharp/Ce126.h \
    src/machine/sharp/ce150.h \
    src/machine/sharp/ce152.h \
    src/machine/sharp/pc1245.h \
    src/machine/sharp/pc1250.h \
    src/machine/sharp/pc1251.h \
    src/machine/sharp/pc1255.h \
    src/machine/sharp/pc1260.h \
    src/machine/sharp/pc1350.h \
    src/machine/sharp/pc1360.h \
    src/machine/sharp/pc1401.h \
    src/machine/sharp/pc1402.h \
    src/machine/sharp/pc1403.h \
    src/machine/sharp/pc1450.h \
    src/machine/sharp/pc1475.h \
    src/machine/sharp/pc1500.h \
    src/machine/sharp/pc1600.h \
    src/machine/sharp/ce1600p.h \
    src/machine/sharp/pc2500.h \
    src/machine/sharp/pc1421.h \
    src/machine/sharp/pc1280.h \
    src/machine/sharp/e500.h \
    src/machine/sharp/g850v.h \
    src/machine/sharp/ce140p.h \
    src/machine/sharp/pc1425.h \
    src/machine/sharp/ce140f.h \
    src/machine/pcxxxx.h \
    src/machine/sio.h \
    src/cpu/z80.h \
    src/cpu/z80memory.h \
    src/cpu/lh5803.h \
    src/cpu/hd61102.h \
    src/cpu/z80-2.h \
    src/cpu/lu57813p.h \
    src/core/Keyb1600.h \
    src/machine/potar.h \
    src/core/dialogpotar.h \
    src/machine/cmotor.h \
    src/cpu/tc8576p.h \
    src/core/cextension.h \
    src/core/Keyb2500.h \
    src/machine/ce515p.h \
    src/core/Keyb1280.h \
    src/core/weblinksparser.h \
#    src/core/wavfile.h \
    qcodemodel2/qcodenode.h \
    ui/dialogdasm.h \
    ui/cregssc61860widget.h \
    ui/cregcpu.h \
    ui/cregslh5801widget.h \
    src/machine/cx07.h \
    src/cpu/ct6834.h \
    src/machine/cx07char.h \
    ui/cregsz80widget.h \
    src/cpu/hd61700.h \
    src/machine/cx710.h \
    src/cpu/uart.h \
    ui/uartconsole.h \
    src/cpu/hd44352.h \
    ui/cregshd61700widget.h \
    src/lcd/Lcdc_x07.h \
    src/lcd/Lcdc_symb.h \
    src/lcd/Lcdc_pc1600.h \
    src/lcd/Lcdc_pb1000.h \
    src/lcd/Lcdc_e500.h \
    src/lcd/Lcdc_fp200.h \
    src/lcd/Lcdc_pc1211.h \
    src/lcd/Lcdc.h \
    pictureflow/pictureflow.h \
    pictureflow/materialflow.h \
    pictureflow/fluidlauncher.h \
    pictureflow/launcher.h \
    pictureflow/slideshow.h \
    src/core/launchbuttonwidget.h \
    src/core/tapandholdgesture.h \
    src/machine/casio/pb1000.h \
    src/machine/casio/pb2000.h \
    src/machine/casio/md100.h \
    src/machine/casio/casiodisk.h \
    src/machine/casio/fp100.h \
    src/machine/casio/fp200.h \
    src/machine/casio/z1.h \
    src/cpu/sed1560.h \
    src/lcd/Lcdc_g850.h \
    src/cpu/i80x86.h \
    src/cpu/hd66108.h \
    src/lcd/Lcdc_z1.h \
    src/cpu/cf79107pj.h \
    src/cpu/i8085.h \
    src/cpu/i8085cpu.h \
    src/cpu/i8085daa.h \
    src/cpu/rp5c01.h \
    src/cpu/ctronics.h \
    ui/dialogvkeyboard.h \
    src/core/qcommandline.h \
    src/core/version.h \
    tinybasic/tinybasic.h \
    src/machine/sharp/pc1211.h \
    src/machine/sharp/ce122.h \
    src/core/downloadmanager.h \
    src/machine/pc2001.h \
    src/cpu/upd16434.h \
    src/lcd/Lcdc_pc2001.h \
    src/cpu/upd7907/upd7907.h \
    src/machine/pc2021.h \
    src/machine/printerctronics.h \
    src/machine/casio/fp40.h \
    src/cpu/i80L188EB.h \
    src/cpu/pit8253.h \
    src/core/servertcp.h \
    src/machine/general/lbc1100.h \
    src/machine/general/cl1000.h \
    src/machine/cesimu.h \
    src/core/dialogsimulator.h \
    src/core/bineditor/bineditor.h \
    src/core/bineditor/colorscheme.h \
    src/lcd/Lcdc_lbc1100.h \
    src/machine/tpc8300.h \
    src/lcd/Lcdc_tpc8300.h \
    src/machine/tp83.h \
    src/machine/panasonic/rlh1000.h \
    src/machine/panasonic/rlp6001.h \
    src/machine/panasonic/rlp9001.h \
    src/machine/panasonic/rlp1004a.h \
    src/lcd/Lcdc_rlh1000.h \
    src/cpu/m6502/m6502.h \
    src/cpu/m6502/opsc02.h \
    src/cpu/m6502/ops02.h \
    src/machine/postit.h \
    src/core/sizegrip.h \
    src/machine/hp/hp41.h \
    src/cpu/ti57cpu.h \
    src/machine/ti/ti57.h \
    src/lcd/Lcdc_ti57.h \
    src/machine/hp/hp41Cpu.h \
    src/machine/hp/hp41mod.h \
    src/lcd/Lcdc_hp41.h \
    src/machine/hp/hp82143A.h \
    src/core/viewobject.h \
    src/machine/bus.h \
    src/core/allobjects.h \
    src/machine/panasonic/rlp4002.h \
    src/machine/panasonic/rlp1002.h \
    src/machine/panasonic/rlp2001.h \
    src/machine/sharp/ce1560.h \
    src/cpu/s6b0108.h \
    src/lcd/Lcdc_ce1560.h \
    src/machine/panasonic/buspanasonic.h \
    src/machine/sharp/buspc1500.h \
    src/machine/sharp/ce162e.h \
    src/machine/sharp/ce153.h \
    src/core/breakpoint.h \
    resources/lcc/model/__graph.h \
    src/cpu/mc6847.h \
    src/machine/panasonic/rlp3001.h \
    src/machine/panasonic/rlext.h \
    src/cpu/tms7000/tms7000.h \
    src/machine/ti/cc40.h \
    src/cpu/hd44780.h \
    src/lcd/Lcdc_cc40.h \
    src/machine/ti/ti74.h \
    src/lcd/Lcdc_ti74.h

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
    resources/ext.qrc \
    resources/pc1245.qrc \
    resources/pc1250.qrc \
    resources/pc1251.qrc \
    resources/pc1255.qrc \
    resources/pc1260.qrc \
    resources/pc1261.qrc \
    resources/pc1262.qrc \
    resources/pc1350.qrc \
    resources/pc1360.qrc \
    resources/pc1401.qrc \
    resources/pc1402.qrc \
    resources/pc1403.qrc \
    resources/pc1450.qrc \
    resources/pc1475.qrc \
    resources/pc1500.qrc \
    resources/pockemul.qrc \
    resources/pc1600.qrc \
    resources/pc2500.qrc \
    resources/pc1421.qrc \
    resources/pc1425.qrc \
    resources/pc1460.qrc \
    resources/pc1280.qrc \
    resources/x07.qrc \
    resources/pb1000.qrc \
    resources/e500.qrc \
    resources/pb2000.qrc \
    resources/g850v.qrc \
    resources/ext2.qrc \
    resources/z1.qrc \
    resources/fp200.qrc \
    resources/pc1211.qrc \
    resources/pc2001.qrc \
    resources/lbc1100.qrc \
    resources/tpc8300.qrc \
    resources/rlh1000.qrc \
    resources/ti57.qrc \
    resources/hp41.qrc
}
else {
DEFINES += LOCRES
}

RESOURCES +=  \
    resources/keymap.qrc \
    resources/stdlibs.qrc \
    qcodeedit/qxs/qxs.qrc \
    resources/asmlibs.qrc \
    resources/core.qrc \
    resources/cc40.qrc \
    resources/ti74.qrc

SOURCES +=  \
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
    src/core/cprinter.cpp \
    src/core/dialogabout.cpp \
    src/core/dialoganalog.cpp \
    src/core/dialogconsole.cpp \
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
    src/machine/sharp/Ce126.cpp \
    src/machine/ccable.cpp \
    src/machine/sharp/ce125.cpp \
    src/machine/sharp/ce150.cpp \
    src/machine/sharp/ce152.cpp \
    src/machine/sharp/pc1245.cpp \
    src/machine/sharp/pc1250.cpp \
    src/machine/sharp/pc1251.cpp \
    src/machine/sharp/pc1255.cpp \
    src/machine/sharp/pc1260.cpp \
    src/machine/sharp/pc1350.cpp \
    src/machine/sharp/pc1360.cpp \
    src/machine/sharp/pc1401.cpp \
    src/machine/sharp/pc1402.cpp \
    src/machine/sharp/pc1403.cpp \
    src/machine/sharp/pc1450.cpp \
    src/machine/sharp/pc1475.cpp \
    src/machine/sharp/pc1500.cpp \
    src/machine/sharp/pc1421.cpp \
    src/machine/sharp/pc1280.cpp \
    src/machine/sharp/ce140p.cpp \
    src/machine/sharp/pc2500.cpp \
    src/machine/sharp/g850v.cpp \
    src/machine/sharp/pc1600.cpp \
    src/machine/sharp/ce1600p.cpp \
    src/machine/sharp/ce120p.cpp \
    src/machine/sharp/e500.cpp \
    src/machine/sharp/pc1211.cpp \
    src/machine/sharp/pc1425.cpp \
    src/machine/pcxxxx.cpp \
    src/machine/sio.cpp \
    src/machine/cx07.cpp \
    src/machine/cx710.cpp \
    src/machine/casio/pb1000.cpp \
    src/machine/casio/pb2000.cpp \
    src/machine/casio/md100.cpp \
    src/machine/casio/casiodisk.cpp \
    src/machine/casio/z1.cpp \
    src/machine/casio/fp40.cpp \
    src/machine/casio/fp100.cpp \
    src/machine/casio/fp200.cpp \
    src/machine/pc2001.cpp \
    src/machine/pc2021.cpp \
    src/machine/printerctronics.cpp \
    src/machine/general/lbc1100.cpp \
    src/machine/general/cl1000.cpp \
    src/machine/potar.cpp \
    src/machine/cmotor.cpp \
    src/machine/ce515p.cpp \
    src/core/weblinksparser.cpp \
#    src/core/wavfile.cpp \
    src/machine/sharp/ce140f.cpp \
    src/machine/cesimu.cpp \
    src/core/dialogsimulator.cpp \
    src/core/bineditor/bineditor.cpp \
    src/core/bineditor/colorscheme.cpp \
    src/core/dialogpotar.cpp \
    src/lcd/Lcdc_pc1211.cpp \
    src/machine/sharp/ce122.cpp \
    qcodemodel2/qcodenode.cpp \
    ui/dialogdasm.cpp \
    ui/cregssc61860widget.cpp \
    ui/cregcpu.cpp \
    ui/cregslh5801widget.cpp \
    src/cpu/cpu.cpp \
    src/cpu/lh5801.cpp \
    src/cpu/lh5810.cpp \
    src/cpu/pd1990ac.cpp \
    src/cpu/sc61860.cpp \
    src/cpu/sc62015.cpp \
    src/cpu/ct6834.cpp \
    src/cpu/z80.cpp \
    src/cpu/lh5803.cpp \
    src/cpu/z80_dasm.cpp \
    src/cpu/hd61102.cpp \
    src/cpu/lu57813p.cpp \
    ui/cregsz80widget.cpp \
    src/cpu/tc8576p.cpp \
    src/cpu/hd61700d.cpp \
    src/cpu/hd61700.cpp \
    src/cpu/uart.cpp \
    src/cpu/hd44352.cpp \
    src/cpu/i80x86.cpp \
    src/cpu/hd66108.cpp \
    src/cpu/upd16434.cpp \
    src/cpu/i8085_dasm.cpp \
    src/cpu/i80x86_dasm.cpp \
    src/cpu/cf79107pj.cpp \
    src/cpu/i8085.cpp \
    src/cpu/rp5c01.cpp \
    src/cpu/ctronics.cpp \
    src/core/cextension.cpp \
    ui/cregshd61700widget.cpp \
    src/lcd/Lcdc_x07.cpp \
    src/lcd/Lcdc_pc1600.cpp \
    src/lcd/Lcdc_pb1000.cpp \
    src/lcd/Lcdc_e500.cpp \
    src/lcd/Lcdc.cpp \
    ui/uartconsole.cpp \
    pictureflow/pictureflow.cpp \
    pictureflow/materialflow.cpp \
    pictureflow/fluidlauncher.cpp \
    pictureflow/launcher.cpp \
    pictureflow/slideshow.cpp \
    src/core/launchbuttonwidget.cpp \
    src/core/tapandholdgesture.cpp \
    src/cpu/sed1560.cpp \
    src/lcd/Lcdc_g850.cpp \
    src/lcd/Lcdc_fp200.cpp \
    src/lcd/Lcdc_z1.cpp \
    ui/dialogvkeyboard.cpp \
    src/core/qcommandline.cpp \
    tinybasic/tinybasic.cpp \
    src/core/downloadmanager.cpp \
    src/lcd/Lcdc_pc2001.cpp \
    src/cpu/upd7907/upd7907d.cpp \
    src/cpu/upd7907/upd7907.cpp \
    src/cpu/i80L188EB.cpp \
    src/cpu/pit8253.cpp \
    src/core/servertcp.cpp \
    src/lcd/Lcdc_lbc1100.cpp \
    src/machine/tpc8300.cpp \
    src/lcd/Lcdc_tpc8300.cpp \
    src/machine/tp83.cpp \
    src/machine/panasonic/rlh1000.cpp \
    src/machine/panasonic/rlp6001.cpp \
    src/machine/panasonic/rlp9001.cpp \
    src/machine/panasonic/rlp1004a.cpp \
    src/lcd/Lcdc_rlh1000.cpp \
    src/cpu/m6502/m6502.cpp \
    src/cpu/m6502/m6502_dasm.cpp \
    src/machine/postit.cpp \
    src/core/sizegrip.cpp \
    src/machine/hp/hp41.cpp \
    src/cpu/ti57cpu.cpp \
    src/machine/ti/ti57.cpp \
    src/cpu/ti57cpu_dasm.cpp \
    src/lcd/Lcdc_ti57.cpp \
    src/machine/hp/hp41trace.cpp \
    src/machine/hp/hp41display.cpp \
    src/machine/hp/hp41Timer.cpp \
    src/machine/hp/hp41File.cpp \
    src/machine/hp/hp41Cpu.cpp \
    src/machine/hp/hp41mod.cpp \
    src/lcd/Lcdc_hp41.cpp \
    src/machine/hp/hp82143A.cpp \
    src/machine/bus.cpp \
    src/machine/panasonic/rlp4002.cpp \
    src/machine/panasonic/rlp1002.cpp \
    src/machine/panasonic/rlp2001.cpp \
    src/machine/sharp/ce1560.cpp \
    src/cpu/s6b0108.cpp \
    src/lcd/Lcdc_ce1560.cpp \
    src/machine/panasonic/buspanasonic.cpp \
    src/machine/sharp/buspc1500.cpp \
    src/machine/sharp/ce162e.cpp \
    src/machine/sharp/ce153.cpp \
    src/core/breakpoint.cpp \
    src/cpu/mc6847.cpp \
    src/machine/panasonic/rlp3001.cpp \
    src/machine/panasonic/rlext.cpp \
    src/cpu/tms7000/tms7000.cpp \
    src/cpu/tms7000/tms7000d.cpp \
    src/machine/ti/cc40.cpp \
    src/cpu/hd44780.cpp \
    src/lcd/Lcdc_cc40.cpp \
    src/machine/ti/ti74.cpp \
    src/lcd/Lcdc_ti74.cpp

greaterThan(QT_MAJOR_VERSION, 4): SOURCES += \


OTHER_FILES += \
    resources/keymap/trspc2.map \
    resources/keymap/pc2500.map \
    resources/keymap/pc1600.map \
    resources/keymap/pc1500.map \
    resources/keymap/pc1450.map \
    resources/keymap/pc1403.map \
    resources/keymap/pc1401.map \
    resources/keymap/pc1360.map \
    resources/keymap/pc1350.map \
    resources/keymap/pc1280.map \
    resources/keymap/pc1250.map \
    resources/keymap/ce1600p.map \
    resources/keymap/ce150.map \
    resources/pockemul/release_notes.html \
    resources/keymap/ce125tape.map \
    resources/keymap/ce126.map \
    resources/keymap/ce125.map \
    resources/keymap/ce129.map \
    resources/keymap/ce123.map \
    resources/keymap/ce120.map \
    resources/keymap/263591.map \
    resources/keymap/ce140p.map \
    resources/weblinks.xml \
    resources/notes.txt \
    resources/pockemul/script_help.html \
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
    resources/keymap/x07.map \
    resources/keymap/e500.map \
    resources/keymap/pb1000.map \
    resources/keymap/x710.map \
    resources/pockemul/config.xml \
    resources/pockemul/configExt.xml \
    resources/keymap/pb2000.map \
    resources/keymap/g850v.map \
    resources/keymap/z1.map \
    resources/keymap/fp200.map \
    resources/keymap/pc1260.map \
    resources/keymap/pc1211.map \
    resources/keymap/ce122.map \
    resources/keymap/pc2001.map \
    resources/keymap/pc2021.map \
    resources/keymap/ce127r.map \
    resources/keymap/ce152.map \
    resources/keymap/pc2081.map \
    resources/keymap/fp40.map \
    resources/keymap/lbc1100.map \
    resources/keymap/cl1000.map \
    resources/keymap/pc1425.map \
    resources/pockemul/weblinks.xml \
    resources/keymap/tpc8300.map \
    resources/keymap/tp83.map \
    resources/keymap/rlh1000.map \
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
    cloud/content/ScrollBar.qml \
    cloud/content/NewsDelegate.qml \
    cloud/content/CategoryDelegate.qml \
    cloud/content/BusyIndicator.qml \
    cloud/main.qml \
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
    cloud/PmlView.qml \
    cloud/content/SettingsDelegate.qml \
    cloud/content/Edit.qml \
    cloud/content/ComboBox.qml \
    cloud/content/Switch.qml \
    cloud/content/knob.svg \
    cloud/content/counter.js \
    cloud/content/SortListModel.qml \
    cloud/content/Test.qml \
    cloud/content/Grid.qml \
    resources/keymap/ti57.map \
    resources/keymap/hp41.map \
    resources/keymap/hp82143a.map \
    cloud/content/Message.qml \
    cloud/PmlView2.qml \
    cloud/content/NewsDelegate2.qml \
    cloud/content/Actions.qml \
    resources/keymap/rlp9006.map \
    resources/keymap/rlp1004a.map \
    resources/keymap/rlp4002.map \
    resources/keymap/ce1560.map \
    resources/keymap/ce140f.map \
    android/AndroidManifest.xml \
    android/src/org/qtproject/pockemul/PockemulActivity.java \
    resources/keymap/cc40.map

ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android













