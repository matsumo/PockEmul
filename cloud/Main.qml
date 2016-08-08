import QtQuick 2.3
import QtQuick.Controls 1.2
import QtQuick.Dialogs 1.2
import "TabbedQuickApp"
import "content"
import "."
//import QtSensors 5.3 as Sensors


Rectangle {
    width: 360
    height: 640

    id: rootCloud
    signal sendWarning(string test)
    signal close

    property string api: cloud.getValueFor("api","elgg");

    property string serverURL: cloud.getValueFor("serverURL","http://pockemul.dscloud.me/elgg/")
    property string currentUserid: "pock emul"
    property string currentApiKey: cloud.getValueFor("apikey","0")
    property bool   isPortrait: false
//    property alias bigposter: bigposter
//    property alias publicCloudTabName: publicCloudTab.name

    property color backGroundColor: "white"
    property color textColor : "black"
    property color inputBorderColor: "black"

    property string auth_token: ""

    Component.onCompleted: {
        console.log("Main.qml: Completed",new Date());

        if (cloud.getValueFor("username","") != "") {
            user_login(cloud.getValueFor("username"),cloud.getValueFor("password"));
            console.log("logged:"+auth_token);
        }

//        showroomPocket.launched.connect(cloudHide);
//        showroomExt.launched.connect(cloudHide);
    }

    onWidthChanged: {
        isPortrait = cloud.isPortraitOrientation();
        //        console.log(isPortrait)
    }

    VisualItemModel {
        id: tabsModel
//        Tab {name: "New"
//            ShowRoom {
//                id: showroomPocket
////                z: 9999
//                visible: true
//                exitOnBack: false
//                source: "qrc:/pockemul/config.xml"
//                anchors.fill: parent
//            }
//        }
//        Tab {name: "New Ext."
//            ShowRoom {
//                id: showroomExt
////                z: 9999
//                visible: true
//                exitOnBack: false
//                source: "qrc:/pockemul/configExt.xml"
//                anchors.fill: parent
//            }
//        }
        Tab {name: "Cloud"
            icon: "pics/public-cloud-white.png"

            color: "yellow"

            PmlView2   {
                id: myCloud
                anchors.top: newprivateSearchItem.bottom
                anchors.bottom: parent.bottom
                width: parent.width
                ispublicCloud: false
                searchText: newprivateSearchItem.text
                cacheFileName: "newprivateCloud.xml"
//                xml: cloud.loadCache(cacheFileName)
            }

            SearchBox {
                id: newprivateSearchItem
                width: parent.width
                height: 50
                objectName: "searchFld"
                onTextChanged: {
                    myCloud.populate(text)

                }

            }
        }

        Tab {name: "Action"
            icon: "pics/action-white.png"
            Actions {}
        }

        Tab {name: "Settings"
            icon: "pics/settings-white.png"

            color: "red"

            Settings {
                anchors.fill: parent
            }
        }

        Tab {name: ""
            icon: "pics/back-white.png"
            MouseArea {
                anchors.fill: parent
                onClicked: close();
            }
        }
        //        Tab {
        //            name: "Test"
        //            icon: "pics/white-about-256.png"
        //            PdfViewer {
        //                anchors.fill: parent
        //                url: "qrc:/pdfjs/web/viewer.html"
        //            }
        //        }

    }

    TabbedUI {
        id: tabbedui
        tabsHeight: 72 * cloud.getValueFor("hiResRatio","1")
        tabIndex: 0
        tabsModel: tabsModel
        quitIndex: 3
        onClose: rootCloud.close();

    }


    // this function is included locally, but you can also include separately via a header definition
    function requestGet(url, callback) {
        var xhr = new XMLHttpRequest();
        xhr.onreadystatechange = function() { callback(xhr);}

        xhr.open('GET', url, true);
        xhr.send('');
    }

    function requestPost(url, data, callback) {

        var xhr = new XMLHttpRequest();

        if (api==='wp') {
            xhr.setRequestHeader("authorization", "Basic cG9ja2VtdWw6dTNZbCBwc0RzIGhYVVIgQnpEVSA3VU9sIGVER2Y=");
        }

        xhr.onreadystatechange = function() { callback(xhr);}

        console.log('before POST:');
        xhr.open('POST', url,true);
        console.log('before SEND*:'+data+'*');
        xhr.send(data);
        console.log('after SEND:');

    }


    function user_register(name,email,username,password) {
        if (api==='elgg') {
            var serverURL = cloud.getValueFor("serverURL","")+'services/api/rest/json/';  //cloud.getValueFor("serverURL","");
            var url = serverURL+'?method=user.register&'+
                    '&name='+encodeURIComponent(name)+
                    '&email='+encodeURIComponent(email)+
                    '&username='+encodeURIComponent(username)+
                    '&password='+encodeURIComponent(password)+
                    '&api_key=7118206e08fed2c5ec8c0f2db61bbbdc09ab2dfa';

            console.log('url:'+url);
            renderArea.showWorkingScreen();
            requestGet(url, function (o) {
                renderArea.hideWorkingScreen();

                if (o.readyState === 4 ) {
                    if (o.status===200) {
                        var obj = JSON.parse(o.responseText);
                        console.log(o.responseText);
                        if (obj.status=== 0) {
                            if (obj.result.success) {
                                // sucess so login
                                message.showMessage("User Created. Please Login.",5000);
                            }
                            else {
                                message.showErrorMessage(obj.result.message,5000);
                            }
                        }
                        else {
                            message.showErrorMessage(obj.message,5000);
                        }
                    }
                }
            });
        }

        if(api==='wp') {
//            serverURL = cloud.getValueFor("serverURL","")+'wordpress/wp-json/wp/v2/users/'
            url = 'http://pockemul.ddns.net/wordpress/wp-json/wp/v2/users/'

            var data= 'name='+encodeURIComponent(name)+
                      '&email='+encodeURIComponent(email)+
                      '&username='+encodeURIComponent(username)+
                      '&password='+encodeURIComponent(password);

            renderArea.showWorkingScreen();
            requestPost(url, data , function (o) {
                console.log('ERREUR:'+o.readyState);

                if (o.readyState === 4 ) {
                    console.log('STATUS:'+o.status);
                    if (o.status===200) {
                        var obj = JSON.parse(o.responseText);
                        console.log(o.responseText);
                        if (obj.status === 0) {
                            message.showMessage("User logged.<p>",5000);
                            auth_token = obj.result;
                            cloud.saveValueFor("auth_token",auth_token);
                        }
                        else {
                            message.showErrorMessage(obj.message,5000);
                        }
                    }
                }
            });
        }
    }

    function user_login(username,password) {
        var data = '';
        var url = '';
        var serverURL='';

        if (api==='elgg') {
            serverURL = cloud.getValueFor("serverURL","")+'services/api/rest/json/';  //cloud.getValueFor("serverURL","");
            url = serverURL+'?method=auth.gettoken&'+
                    '&username='+encodeURIComponent(username)+
                    '&password='+encodeURIComponent(password)+
                    '&api_key=7118206e08fed2c5ec8c0f2db61bbbdc09ab2dfa';
        }

        if(api==='wp') {
//            serverURL = cloud.getValueFor("serverURL","")+'wordpress/wp-json/wp/v2/users/'
            url = 'http://pockemul.ddns.net/wordpress/wp-json/wp/v2/users/'

        }

        console.log('url:'+url);
        requestPost(url, data , function (o) {
            console.log('ERREUR:'+o.readyState);

            if (o.readyState === 4 ) {
                console.log('STATUS:'+o.status);
                if (o.status===200) {
                    var obj = JSON.parse(o.responseText);
                    console.log(o.responseText);
                    if (obj.status === 0) {
                        message.showMessage("User logged.<p>",5000);
                        auth_token = obj.result;
                        cloud.saveValueFor("auth_token",auth_token);
                    }
                    else {
                        message.showErrorMessage(obj.message,5000);
                    }
                }
            }
        });
    }

    function save_pml(title,description) {
        var serverURL = '';
        var url = '';
        var xml = '';

        if(api==='elgg') {
            serverURL = cloud.getValueFor("serverURL","")+'services/api/rest/json/';
            url = serverURL+ '?method=file.save_pml&'+
                    '&title='+encodeURIComponent(title)+
                    '&description='+encodeURIComponent(description)+
                    '&api_key=7118206e08fed2c5ec8c0f2db61bbbdc09ab2dfa'+
                    '&auth_token='+auth_token;
            xml = cloud.save();
            console.log('url:'+url);

            requestPost(url, xml , function (o) {
                renderArea.hideWorkingScreen();

                if (o.readyState == 4 ) {
                    if (o.status==200) {
                        var obj = JSON.parse(o.responseText);
                        console.log(o.responseText);
                        if (obj.status == 0) {
                            message.showMessage("Session saved",5000);
                        }
                        else {
                            message.showErrorMessage(obj.message,5000);
                        }
                    }
                }
            });
        }
        if(api==='wp') {
            serverURL = cloud.getValueFor("serverURL","")+'services/api/rest/json/';
            url = serverURL+ '?method=file.save_pml&'+
                    '&title='+encodeURIComponent(title)+
                    '&description='+encodeURIComponent(description)+
                    '&api_key=7118206e08fed2c5ec8c0f2db61bbbdc09ab2dfa'+
                    '&auth_token='+auth_token;
            xml = cloud.save();
            console.log('url:'+url);

            requestPost(url, xml , function (o) {
                renderArea.hideWorkingScreen();

                if (o.readyState == 4 ) {
                    if (o.status==200) {
                        var obj = JSON.parse(o.responseText);
                        console.log(o.responseText);
                        if (obj.status == 0) {
                            message.showMessage("Session saved",5000);
                        }
                        else {
                            message.showErrorMessage(obj.message,5000);
                        }
                    }
                }
            });
        }
    }

    function set_access(pmlid,access,on_success,on_failure) {
        var serverURL = cloud.getValueFor("serverURL","")+'services/api/rest/json/';
        var url = serverURL+ '?method=file.set_access'+
                '&file_guid='+pmlid+
                '&access='+access+
                '&api_key=7118206e08fed2c5ec8c0f2db61bbbdc09ab2dfa'+
                '&auth_token='+auth_token;
        console.log('url:'+url);
        renderArea.showWorkingScreen();

        requestPost(url, "" , function (o) {
            renderArea.hideWorkingScreen();

            if (o.readyState == 4 ) {
                if (o.status==200) {
                    var obj = JSON.parse(o.responseText);
                    console.log(o.responseText);
                    if (obj.status == 0) {
                        message.showMessage("Access rights changed",2000);
                        on_success();
                    }
                    else {
                        message.showErrorMessage(obj.message,5000);
                        on_failure();
                    }
                }
            }
        });
    }

    function clone_pml(pmlid,on_success,on_failure) {
        var serverURL = cloud.getValueFor("serverURL","")+'services/api/rest/json/';
        var url = serverURL+ '?method=file.clone_pml'+
                '&file_guid='+pmlid+
                '&api_key=7118206e08fed2c5ec8c0f2db61bbbdc09ab2dfa'+
                '&auth_token='+auth_token;
        console.log('url:'+url);
        renderArea.showWorkingScreen();

        requestPost(url, "" , function (o) {
            renderArea.hideWorkingScreen();

            if (o.readyState == 4 ) {
                if (o.status==200) {
                    var obj = JSON.parse(o.responseText);
                    console.log(o.responseText);
                    if (obj.status == 0) {
                        message.showMessage("Session cloned",2000);
                        on_success();
                    }
                    else {
                        message.showErrorMessage(obj.message,5000);
                        on_failure();
                    }
                }
            }
        });
    }

    function delete_pml(pmlid,on_success,on_failure) {
        var serverURL = cloud.getValueFor("serverURL","")+'services/api/rest/json/';
        var url = serverURL+ '?method=file.delete_pml'+
                '&file_guid='+pmlid+
                '&api_key=7118206e08fed2c5ec8c0f2db61bbbdc09ab2dfa'+
                '&auth_token='+auth_token;
        console.log('url:'+url);

        requestPost(url, "" , function (o) {
            renderArea.hideWorkingScreen();

            if (o.readyState == 4 ) {
                if (o.status==200) {
                    var obj = JSON.parse(o.responseText);
                    console.log(o.responseText);
                    if (obj.status == 0) {
                        message.showMessage("File deleted",2000);
                        on_success();
                    }
                    else {
                        message.showErrorMessage(obj.message,5000);
                        on_failure();
                    }
                }
            }
        });
    }

    function update_pml(title,description) {
        var serverURL = cloud.getValueFor("serverURL","")+'services/api/rest/json/';
        var url = serverURL+ '?method=file.update&'+
                '&title='+encodeURIComponent(title)+
                '&description='+encodeURIComponent(description)+
                '&api_key=7118206e08fed2c5ec8c0f2db61bbbdc09ab2dfa'+
                '&auth_token='+auth_token;
        console.log('url:'+url);
        renderArea.showWorkingScreen();

        requestPost(url, "" , function (o) {
            renderArea.hideWorkingScreen();

            if (o.readyState == 4 ) {
                if (o.status==200) {
                    var obj = JSON.parse(o.responseText);
                    console.log(o.responseText);
                    if (obj.status == 0) {
                        message.showMessage("Session updated",5000);
                    }
                    else {
                        message.showErrorMessage(obj.message,5000);
                    }
                }
            }
        });
    }


    function addRefPmlModel(_pmlid,
                            _username,
                            _objects,
                            _listobjects,
                            _ispublic,
                            _isdeleted,
                            _title,
                            _description) {
//        console.log("count before:"+refpmlModel.count());
        refpmlModel.append({rowid: refpmlModel.count(),
                               pmlid: _pmlid,
                               username: _username,
                               objects: _objects,
                               listobjects: _listobjects,
                               ispublic: _ispublic,
                               isdeleted: _isdeleted,
                               title: _title,
                               description: _description});
//        console.log("count after:"+refpmlModel.count());

    }

    function encodeXml(s) {
        return s.replace(/([\&"<>])/g, function(str, item) {
            var xml_special_to_escaped_one_map = {
                '&': '&amp;',
                '"': '&quot;',
                '<': '&lt;',
                '>': '&gt;'
            };
            return xml_special_to_escaped_one_map[item];
        });
    }

    function decodeXml(s) {
        return s.replace(/(&quot;|&lt;|&gt;|&amp;)/g,
            function(str, item) {
                var escaped_one_to_xml_special_map = {
                    '&amp;': '&',
                    '&quot;': '"',
                    '&lt;': '<',
                    '&gt;': '>'
                };
                return escaped_one_to_xml_special_map[item];
        });
    }

    Message {
        id:message
        anchors.top: parent.top
        anchors.left: parent.left
        width : parent.width
        height: 30
    }
}
