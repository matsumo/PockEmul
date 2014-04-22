import QtQuick 1.0
import "TabbedQuickApp"
import "content"
import "."
//import QtSensors 5.0 as Sensors

Rectangle {
    width: 360
    height: 640

    id: root
    signal sendWarning(string test)

    property string serverURL: cloud.getValueFor("serverURL","http://pockemul.dscloud.me/elgg/")
    property string currentUserid: "pock emul"
    property string currentApiKey: cloud.getValueFor("apikey","0")
    property bool   isPortrait: false
//    property alias bigposter: bigposter
//    property alias publicCloudTabName: publicCloudTab.name

    property color backGroundColor: "white"
    property color textColor : "black"
    property color inputBorderColor: "black"
    property color buttonBorderColor: "orange"
    property color textButtonColor: "black"

    property string auth_token: ""

    Component.onCompleted: {
        console.log("start");
        user_login(cloud.getValueFor("username"),cloud.getValueFor("password"));
        console.log("logged:"+auth_token);
    }

    onWidthChanged: {
        isPortrait = cloud.isPortraitOrientation();
        //        console.log(isPortrait)
    }

    VisualItemModel {
        id: tabsModel
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

        Tab {
            name: ""
            icon: "pics/back-white.png"
            MouseArea {
                anchors.fill: parent
                onClicked: Qt.quit()
            }

        }

    }

    TabbedUI {
        id: tabbedui
        tabsHeight: 72
        tabIndex: 0
        tabsModel: tabsModel
        quitIndex: 3

    }

/*
    XmlListModel {
         id: tmpXmlListModel
         query: "/item"

         XmlRole { name: "insert"; query: "insert/string()"; }
         XmlRole { name: "pmlid"; query: "pmlid/string()"; }
         XmlRole { name: "username"; query: "username/string()" }
         XmlRole { name: "objects"; query: "objects/string()" }
         XmlRole { name: "listobjects"; query: "listobjects/string()" }
         XmlRole { name: "ispublic"; query: "ispublic/number()" }
         XmlRole { name: "isdeleted"; query: "deleted/number()" }
         XmlRole { name: "title"; query: "title/string()" }
         XmlRole { name: "description"; query: "description/string()" }
         onStatusChanged: {

             if (status == XmlListModel.Ready) {

                 var ind = privateCloud.refpmlModel.count;
                 for (var i=0; i<count; i++) {
                     var item = get(i)
//                     console.log(item.listobjects)
                     if (item.insert == 1) {
//                         console.log("INSERT MODE");
                         privateCloud.refpmlModel.append({rowid : ind+i,
                                                             pmlid: item.pmlid,
                                                             username: item.username,
                                                             objects: item.objects,
                                                             listobjects: item.listobjects,
                                                             ispublic: item.ispublic,
                                                             isdeleted: item.isdeleted,
                                                             title: item.title,
                                                             description: item.description})
                     }
                     if (item.insert == 0) {
//                         console.log("UPDATE MODE");
                         // fetch refpmlModel and find pmlid
                         for (var j=0; j<privateCloud.refpmlModel.count; j++) {
                             var existingitem = privateCloud.refpmlModel.get(j);
                             if (existingitem.pmlid == item.pmlid) {
                                 // update record
                                 privateCloud.refpmlModel.set(j,
                                                              {objects: item.objects,listobjects: item.listobjects})

                             }
                         }


                     }
                     tabbedui.tabClicked(0);
                     cloud.saveCache(privateCloud.cacheFileName,privateCloud.serializerefpmlModel());
                     privateCloud.populateCategoryModel("");
                     privateCloud.objid = 0;
                     privateCloud.populatePMLModel("");
                     privateCloud.focusPml(item.pmlid);
                 }

             }
         }
     }
*/
    // this function is included locally, but you can also include separately via a header definition
    function requestGet(url, callback) {
        var xhr = new XMLHttpRequest();
        xhr.onreadystatechange = function() { callback(xhr);}

        xhr.open('GET', url, true);
        xhr.send('');
    }

    function requestPost(url, data, callback) {

        var xhr = new XMLHttpRequest();

        xhr.onreadystatechange = function() { callback(xhr);}

        xhr.open('POST', url);
        xhr.send(data);

    }

    function user_register(name,email,username,password) {
        var serverURL = cloud.getValueFor("serverURL","")+'services/api/rest/json/';  //cloud.getValueFor("serverURL","");
        var url = serverURL+'?method=user.register&'+
                '&name='+encodeURIComponent(name)+
                '&email='+encodeURIComponent(email)+
                '&username='+encodeURIComponent(username)+
                '&password='+encodeURIComponent(password)+
                '&api_key=7118206e08fed2c5ec8c0f2db61bbbdc09ab2dfa';
        console.log('url:'+url);
        requestGet(url, function (o) {

            if (o.readyState == 4 ) {
                if (o.status==200) {
                    var obj = JSON.parse(o.responseText);
                    console.log(o.responseText);
                    if (obj.status == 0) {
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

    function user_login(username,password) {
        var serverURL = cloud.getValueFor("serverURL","")+'services/api/rest/json/';  //cloud.getValueFor("serverURL","");
        var url = serverURL+'?method=auth.gettoken&'+
                '&username='+encodeURIComponent(username)+
                '&password='+encodeURIComponent(password)+
                '&api_key=7118206e08fed2c5ec8c0f2db61bbbdc09ab2dfa';
        console.log('url:'+url);
        requestPost(url, "" , function (o) {

            if (o.readyState == 4 ) {
                if (o.status==200) {
                    var obj = JSON.parse(o.responseText);
                    console.log(o.responseText);
                    if (obj.status == 0) {
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
        var serverURL = cloud.getValueFor("serverURL","")+'services/api/rest/json/';
        var url = serverURL+ '?method=file.save_pml&'+
                '&title='+encodeURIComponent(title)+
                '&description='+encodeURIComponent(description)+
                '&api_key=7118206e08fed2c5ec8c0f2db61bbbdc09ab2dfa'+
                '&auth_token='+auth_token;
        var xml = cloud.save();
        console.log('url:'+url);
        requestPost(url, xml , function (o) {

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

    function set_access(pmlid,access,on_success,on_failure) {
        var serverURL = cloud.getValueFor("serverURL","")+'services/api/rest/json/';
        var url = serverURL+ '?method=file.set_access'+
                '&file_guid='+pmlid+
                '&access='+access+
                '&api_key=7118206e08fed2c5ec8c0f2db61bbbdc09ab2dfa'+
                '&auth_token='+auth_token;
        console.log('url:'+url);
        requestPost(url, "" , function (o) {

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
        requestPost(url, "" , function (o) {

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
        requestPost(url, "" , function (o) {

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
