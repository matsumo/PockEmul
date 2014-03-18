import QtQuick 1.1


Rectangle {
    width: 800; height: 480

    VisualItemModel {
        id: visualSettingsModel

//        SettingsDelegate { visible: false; id: testcombo; name: "combobox"; labelString: "Combobox"; type: "combo"; saveInput: false }
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

                root.user_register(nameFld.inputText,
                                   emailFld.inputText,
                                   usernameFld.inputText,
                                   passwordFld.inputText);
            }
        }

        SettingsDelegate { name: "apikey"; labelString: "Login"; type: "action"; saveInput:false;
            onButtonClicked: {
//                buttonElementEnabled = false;
                root.user_login(usernameFld.inputText,passwordFld.inputText);
/*
                var key = cloud.generateKey(usernameFld.inputText,passwordFld.inputText);
                serverURL = cloud.getValueFor("serverURL","");
                var url = serverURL+'login?username='+encodeURIComponent(usernameFld.inputText)+'&key='+encodeURIComponent(key);
//                console.log('url:'+url);
//                console.log('Test:'+cloud.generateKey("ABC","DEF"));
                requestGet(url, function (o) {

                    if (o.readyState == 4 ) {
                        if (o.status==200) {
                            buttonElementEnabled = true;
                            cloud.saveValueFor(name,o.responseText);
                            currentApiKey = o.responseText;
                            apikey.inputText = o.responseText;
                            privateCloud.refresh();
                            publicCloud.refresh();
                        }
                        else {
                            apikey.inputText = "Authentification failed !";
                        }
                    }
                });
*/
            }
        }
/*
        SettingsDelegate { name: "register"; labelString: "Register new user"; type: "action"; saveInput:false;
            onButtonClicked: {
//                buttonElementEnabled = false;
                if (passwordFld.inputText != passwordFld2.inputText) {
                    apikey.inputText = "Passwords mismatch !.";
                    return;
                }

                var key = cloud.generateKey(usernameFld.inputText,passwordFld.inputText);
                serverURL = cloud.getValueFor("serverURL","");
                var url = serverURL+'register?username='+encodeURIComponent(usernameFld.inputText)+'&key='+encodeURIComponent(key);
//                console.log('url:'+url);
                requestGet(url, function (o) {

                    if (o.readyState == 4 ) {
                        if (o.status==200) {
                            buttonElementEnabled = true;
                            cloud.saveValueFor("apikey",o.responseText);
                            currentApiKey = o.responseText;
                            apikey.inputText = o.responseText;
                            privateCloud.refresh();
                            publicCloud.refresh();
                        }
                        else {
                            apikey.inputText = "Registration failed. Try an other username.";
                        }
                    }
                });
            }
        }
        SettingsDelegate { id: apikey; name: "out"; labelString: "Retrieved Key"; type: "input"; saveInput:false; }
*/
    }

    ListView {
        focus: true
        id: categories
        anchors.fill: parent

        model: visualSettingsModel
        //            footer: quitButtonDelegate
        //        highlight: Rectangle { color: "steelblue" }
        highlightMoveSpeed: 9999999
    }
    ScrollBar {
        scrollArea: categories; height: categories.height; width: 8
        anchors.right: categories.right
    }
    //        LineInput {
    //            id: username
    //            objectName: "UserName"
    //            height: 30
    //            label: "User Name";
    //            font.pixelSize: 16
    //            onInputAccepted: cloud.saveValueFor(key,value)
    //        }

    //        LineInput {
    //            id: password
    //            objectName: "Password"
    //            height: 30
    //            label: "Password";
    //            font.pointSize: 16
    //            onInputAccepted: cloud.saveValueFor(key,value)
    //        }
}


