import QtQuick 2.0
import QtQuick.Controls 1.2

Rectangle {
    width: 800; height: 480

    VisualItemModel {
        id: visualSettingsModel

//        SettingsDelegate { id: testcombo; name: "combobox"; labelString: "Combobox"; type: "combo"; saveInput: false }
        SettingsDelegate { id: labelCloud; name: "label1"; labelString: "Cloud settings"; type: "text"; saveInput: false }
        SettingsDelegate { id: servername; name: "serverURL"; labelString: "Cloud Server"; type: "input"; defaultText: "http://pockemul.dscloud.me/elgg/"}
        SettingsDelegate { id: nameFld; name: "name"; labelString: "Name"; type: "input"; }
        SettingsDelegate { id: emailFld; name: "email"; labelString: "Email"; type: "input"; }
        SettingsDelegate { id: usernameFld; name: "username"; labelString: "User Name"; type: "input"; }
        SettingsDelegate { id: passwordFld; name: "password"; labelString: "Password"; type: "input"; echoMode: TextInput.Password; }
        SettingsDelegate { id: passwordFld2; name: "password2"; labelString: "Confirm Password"; type: "input"; echoMode: TextInput.Password; saveInput:false;}
        SettingsDelegate { name: "registercloud"; labelString: "Register elgg"; type: "action"; saveInput:false;
            onButtonClicked: {
                if (passwordFld.inputText != passwordFld2.inputText) {
                    apikey.inputText = "Passwords mismatch !.";
                    return;
                }

                rootCloud.user_register(nameFld.inputText,
                                   emailFld.inputText,
                                   usernameFld.inputText,
                                   passwordFld.inputText);
            }
        }
        SettingsDelegate { name: "apikey"; labelString: "Login"; type: "action"; saveInput:false;
            onButtonClicked: {
                rootCloud.user_login(usernameFld.inputText,passwordFld.inputText);
            }
        }
        SettingsDelegate { id: syncEnabled; name: "syncEnabled"; labelString: "Enable file synchronization"; type: "checkbox"; defaultText: "on"}

        SettingsDelegate { id: labelAppSettings; name: "labelAppSettings"; labelString: "Application Settings"; type: "text"; saveInput: false }
        SettingsDelegate { id: soundEnabled; name: "soundEnabled"; labelString: "Enable sound"; type: "checkbox"; defaultText: "on"}
        SettingsDelegate { id: hiRes; name: "hiRes"; labelString: "Enable High Resolution"; type: "checkbox"; defaultText: "on"}

        SettingsDelegate { id: labelServer; name: "labelServer"; labelString: "Telnet Server settings"; type: "text"; saveInput: false }
        SettingsDelegate { id: telnetPort; name: "telnetPort"; labelString: "Telnet Server port"; type: "input"; defaultText: "4000"}
        SettingsDelegate { id: telnetEnabled; name: "telnetEnabled"; labelString: "Enable server"; type: "checkbox"; defaultText: "on"}
        SettingsDelegate { id: telnetLocalHost; name: "telnetLocalHost"; labelString: "Local Host only"; type: "checkbox"; defaultText: "on"}

    }

    ListView {
        focus: true
        id: categories
        anchors.fill: parent
        interactive: true;
        model: visualSettingsModel
        //            footer: quitButtonDelegate
        //        highlight: Rectangle { color: "steelblue" }
        highlightMoveVelocity: 9999999
    }
    ScrollBar {
        scrollArea: categories; height: categories.height; width: 8
        anchors.right: categories.right
    }

}


