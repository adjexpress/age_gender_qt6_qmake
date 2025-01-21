/*
  this file only includes application settings UI
*/

// pragma Singleton

import QtCore
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQml.Models

Rectangle {
    id: settingsView

    property string colorset: "transparent"


    anchors.fill: parent
    color: colorset
    component  Foldable: Rectangle {
        id: container

        property string headerTXT: "value"
        property string sourceCmp

        property int idx
        property bool selected: false
        property int animationDuration: 300

        onSelectedChanged: {
            if (selected)
                state = "details"
            else
                state = "summery"
        }

        width: parent.width
        // height: childrenRect.height
        color: "white"

        border.color: "lightBlue"
        border.width: 1
        radius: 2
        state : "summery"

        states: [
            State {
                name: "details"
                PropertyChanges { target: setLDR;     sourceComponent: retComp(sourceCmp) ; Layout.fillWidth: true; opacity: 1.0}
                PropertyChanges { target: mainCol;    height: mainCol.visibleChildren.height }
                PropertyChanges { target: container;  height: mainCol.childrenRect.height  +  20 }
            },
            State {
                name: "summery"
                PropertyChanges { target: setLDR;     sourceComponent: null ; opacity: 0 ; }
                PropertyChanges { target: mainCol;    height: mainCol.visibleChildren.height }
                PropertyChanges { target: container;  height:  mainCol.implicitHeight + 5 ; }
            }
        ]

        transitions: [
            Transition {
                to: "summery"
                ParallelAnimation {
                    PropertyAnimation {target: container; property: "height";  duration: animationDuration}
                    PropertyAnimation {target: mainCol;   property: "height";  duration: animationDuration}
                    NumberAnimation   {target: setLDR;    property: "opacity"; from: 1.0 ;   to: 0.0  ;duration: animationDuration }
                }
            },
            Transition {
                to: "details"
                ParallelAnimation {
                    NumberAnimation   {target: setLDR;    property: "opacity"; from: 0.0 ;    to: 1.0  ;duration: animationDuration}
                    PropertyAnimation {target: mainCol;   property: "height"; duration: animationDuration}
                    PropertyAnimation {target: container; property: "height"; duration: animationDuration}
                }
            }
        ]

        MouseArea{

            anchors.fill: parent
            onClicked:
            {
                if (container.selected)
                    container.ListView.view.currentIndex = -1
                else
                    container.ListView.view.currentIndex = container.idx
                console.log("item at ",idx,"selected: ",container.selected );
            }
        }

        ColumnLayout {
            id: mainCol

            // height: childrenRect.height
            // clip:  true
            anchors {
                // left: parent.left
                // right: parent.right
                // top: parent.top
                // bottom: parent.bottom
                // margins: 2
                fill: parent
            }
            // minimumHeight: header.height
            // height: (setLDR.item !== null && typeof(setLDR.item)!== 'undefined')? setLDR.item.height + header.height+ 10: header.height+ 10

            // width: parent.width
            // implicitHeight: setLDR.loaded
            //                 ?
            //                     (setLDR.item.height + header.height + 10)
            //                 :
            //                     (  header.height+ 10 )
            // height: setLDR.height + header.height
            // maximumHeight:
            // height:

            Rectangle {
                id: header

                height: 20
                color:  "transparent"
                Layout.fillWidth: true
                Layout.alignment:  Qt.AlignTop
                Layout.margins: 5
                // width: parent.width
                // anchors.top: parent.top
                // anchors.margins: 5

                property string colorStr: "red"

                RowLayout {

                    anchors.fill: parent
                    Text {
                        id: headrTxt
                        text: container.headerTXT
                        // Layout: {
                        //     top: parent.top
                        //     bottom: parent.bottom
                        //     margins: 5
                        // }

                        Layout.alignment: Qt.AlignLeft
                        // Layout.alignment: Qt.AlignVCenter
                        Layout.margins: 3
                    }

                    Rectangle {
                        id: arrow

                        // Layout: {
                        //     top: parent.top
                        //     bottom: parent.bottom
                        //     right: parent.right
                        //     margins: 5
                        // }
                        Layout.alignment: Qt.AlignRight
                        Layout.margins: 3
                        height: 20
                        width: height

                        Image {
                            id: arrowImg
                            source: "qrc:/media/down-arrow.png"
                            anchors.fill: parent
                            fillMode: Image.PreserveAspectCrop
                        }

                        radius: width/2
                        // color: container.selected ?  header.colorStr : "transparent"
                        // border.width: 1
                        // border.color:  header.colorStr
                        opacity: container.selected ?  1 : 0.5
                        rotation: container.selected ?  180 : 0
                        Behavior on rotation {
                                RotationAnimation {duration: animationDuration}
                        }

                    }
                }

            }

            Loader {
                id:setLDR
                property int ctr: 0

                // visible: true
                visible: status == Loader.Ready
                opacity: 0
                // Layout.fillHeight: true
                Layout.fillWidth:true
                Layout.alignment:  Qt.AlignTop
                Layout.margins: 10

                // width: parent.width
                // anchors.top: header.bottom
                // anchors.margins: 5

                // onStatusChanged: {
                //     if(status == Loader.Loading ){
                //         console.log("status == Loader.Loading ")
                //         getHeight(mainCol.visibleChildren)
                //     }
                //     if(status == Loader.Ready ){
                //         console.log("status == Loader.Ready ")
                //         getHeight(mainCol.visibleChildren)
                //         console.log("setLDR.item.height:",setLDR.item.height)
                //         // container.height = children.height+10;
                //         console.log("container.height:",container.height)
                //     }
                // }
                onLoaded: {
                    container.height = mainCol.childrenRect.height + mainCol.spacing*2
                    // setLDR.item.Layout.fillHeight = true
                    // width = setLDR.item.width
                    // height = setLDR.item.height
                    // getHeight(mainCol.visibleChildren)
                    // binder.target = mainCol;
                    // ctr++;
                    // console.log(ctr," loaded:",typeof(item),  setLDR.item.width,"x",setLDR.item.height , "  itemH:", getHeight(retComp(container.sourceCmp)) )
                    // container.height = setLDR.item.height+ 30;
                }
            }

            // Binding on height {
            //     // id: binder

            //     property: "height"
            //     when: setLDR.loaded
            //     value: (setLDR.item !== null && typeof(setLDR.item)!== 'undefined')? setLDR.item.height + header.height+ 10: header.height+ 10
            // }
        }
    }

    component TexteditBack: Rectangle {
        anchors.fill: parent
        color: "gray"
        border.color: "lightBlue"
        border.width: 1

        // opacity: 0.7
        radius: 5
    }

    Component {
        id: secComp0
        // objectName: "secComp"

        Rectangle {
            id: set
            // anchors.fill: parent
            color: colorset
            width: parent.width
            height: r0.height + r1.implicitHeight + r2.implicitHeight + r3.implicitHeight + (4*c1.spacing)//cLayout.implicitHeight

            ColumnLayout {
                id:c1
                anchors.fill: parent


                RowLayout {
                    id:r0
                    // width:  parent.width
                    Layout.fillWidth: true
                    Text {
                        text: qsTr("RTSP")
                    }

                    Switch {
                        id: cameraSw
                        Layout.fillWidth: true
                        enabled: true
                        checked: videoStreamSetting.videoInput == "camera"
                        onCheckedChanged: !checked ?  videoStreamSetting.videoInput = "rtsp" :  videoStreamSetting.videoInput = "camera"
                    }

                    Text {
                        text: qsTr("Camera")
                    }

                }

                RowLayout {
                    id:r1
                    // width:  parent.width
                    Layout.fillWidth: true
                    Text {

                        // anchors.left: parent.left
                        text: qsTr("stream url: ")
                    }
                    TextField {

                        background: Rectangle {
                            opacity: 0.5
                            radius: 5
                        }
                        Layout.alignment: Qt.AlignLeft
                        Layout.fillWidth: true
                        // anchors.right: parent.right
                        text: videoStreamSetting.streamSource

                         onAccepted: editingFinished; onEditingFinished: videoStreamSetting.streamSource = text

                    }
                }

                RowLayout {
                    id:r2
                    // width:  parent.width
                    Layout.fillWidth: true
                    Text {
                        // anchors.left: parent.left
                        text: qsTr("stream username: ")
                    }
                    TextField {

                        background: Rectangle {
                            opacity: 0.5
                            radius: 5
                        }
                        Layout.alignment: Qt.AlignLeft
                        Layout.fillWidth: true
                        text: videoStreamSetting.streamSourceUser
                         onAccepted: editingFinished; onEditingFinished: videoStreamSetting.streamSourceUser = text
                    }
                }

                RowLayout {
                    id:r3
                    // width:  parent.width
                    Layout.fillWidth: true
                    Text {
                        // anchors.left: parent.left
                        text: qsTr("stream pass: ")
                    }
                    TextField {

                        background: Rectangle {
                            opacity: 0.5
                            radius: 5
                        }
                        Layout.alignment: Qt.AlignLeft
                        Layout.fillWidth: true
                        text: videoStreamSetting.streamSourcePass
                         onAccepted: editingFinished; onEditingFinished: videoStreamSetting.streamSourcePass = text
                    }
                }
            }
        }

    }

    Component {
        id: secComp1
        // objectName: "secComp"

        Rectangle {
            id: set
            // anchors.fill: parent
            color: colorset
            width: parent.width
            height: r0.implicitHeight + r1.implicitHeight + r2.implicitHeight + r3.implicitHeight + r4.implicitHeight + r5.implicitHeight + (6*c1.spacing)//cLayout.implicitHeight

            ColumnLayout {
                id:c1
                anchors.fill: parent


                RowLayout {
                    id:r0
                    // width:  parent.width
                    Layout.fillWidth: true
                    Text {
                        text: qsTr("Send message ")
                    }

                    Switch {
                        id: sendMsgSw
                        Layout.fillWidth: true
                        enabled: true
                        checked: messageSetting.sendMsg
                        onCheckedChanged: messageSetting.sendMsg = checked
                    }
                }

                RowLayout {
                    id:r1
                    // width:  parent.width
                    Layout.fillWidth: true
                    Text {
                        // anchors.left: parent.left
                        text: qsTr("Gender")
                        Layout.fillWidth: true
                    }
                    CheckBox {

                        background: Rectangle {
                            opacity: 0.5
                            radius: 5
                        }
                        Layout.alignment: Qt.AlignLeft
                        Layout.fillWidth: true
                        enabled: messageSetting.sendMsg
                        checked: messageSetting.dGender
                        onCheckedChanged: messageSetting.dGender = checked
                    }
                }

                RowLayout {
                    id:r2
                    // width:  parent.width
                    Layout.fillWidth: true
                    Text {
                        // anchors.left: parent.left
                        text: qsTr("Age")
                        Layout.fillWidth: true
                    }
                    CheckBox {

                        background: Rectangle {
                            opacity: 0.5
                            radius: 5
                        }
                        Layout.alignment: Qt.AlignLeft
                        Layout.fillWidth: true
                        enabled: messageSetting.sendMsg
                        checked: messageSetting.dAge
                        onCheckedChanged: messageSetting.dAge = checked
                    }
                }
                RowLayout {
                    id:r3
                    // width:  parent.width
                    Layout.fillWidth: true
                    Text {
                        Layout.fillWidth: true
                        text: qsTr("Count")
                    }
                    CheckBox {

                        background: Rectangle {
                            opacity: 0.5
                            radius: 5
                        }
                        Layout.alignment: Qt.AlignLeft
                        Layout.fillWidth: true
                        enabled: messageSetting.sendMsg
                        checked: messageSetting.dCount
                        onCheckedChanged: messageSetting.dCount = checked
                    }
                }

                RowLayout {
                    id:r4
                    // width:  parent.width
                    Layout.fillWidth: true
                    Text {
                        Layout.fillWidth: true
                        text: qsTr("Count changed event")
                    }
                    CheckBox {

                        background: Rectangle {
                            opacity: 0.5
                            radius: 5
                        }
                        Layout.alignment: Qt.AlignLeft
                        Layout.fillWidth: true
                        enabled: messageSetting.sendMsg
                        checked: messageSetting.eCount
                        onCheckedChanged: messageSetting.eCount = checked
                    }
                }

                RowLayout {
                    id:r5
                    // width:  parent.width
                    Layout.fillWidth: true
                    Text {
                        Layout.fillWidth: true
                        text: qsTr("Zero visitor event")
                    }
                    CheckBox {

                        background: Rectangle {
                            opacity: 0.5
                            radius: 5
                        }
                        Layout.alignment: Qt.AlignLeft
                        Layout.fillWidth: true
                        enabled: messageSetting.sendMsg
                        checked: messageSetting.eClear
                        onCheckedChanged: messageSetting.eClear = checked
                    }
                }

            }
        }

    }

    Component {
        id: secComp2

        Rectangle {
            id: set
            color: colorset
            width: parent.width
            height: r1.implicitHeight + r2.implicitHeight + (2*c1.spacing)

            ColumnLayout {
                id:c1
                anchors.fill: parent

                RowLayout {
                    id:r1
                    Layout.fillWidth: true
                    Text {
                        text: qsTr("IP Address: ")
                    }
                    TextField {
                        background: Rectangle {
                            // opacity: 0.5
                            radius: 5
                        }
                        Layout.alignment: Qt.AlignLeft
                        Layout.fillWidth: true
                        text: reciverSetting.ip
                         onAccepted: editingFinished; onEditingFinished: reciverSetting.ip = text
                    }
                }

                RowLayout {
                    id:r2
                    // width:  parent.width
                    Layout.fillWidth: true
                    Text {
                        text: qsTr("Port: ")
                    }
                    TextField {
                        background: Rectangle {
                            // opacity: 0.5
                            radius: 5
                        }
                        Layout.alignment: Qt.AlignLeft
                        Layout.fillWidth: true
                        text: reciverSetting.port
                         onAccepted: editingFinished; onEditingFinished: reciverSetting.port = text
                    }
                }
            }
        }

    }

    function retComp(name) {
        switch (name){
        case "secComp0":
            return secComp0;
        case "secComp1":
            return secComp1;
        case "secComp2":
            return secComp2;
        default:
            return null;
        }
    }

    function getHeight(arg0) {
        console.log("typ of arg0:",arg0,typeof(arg0))

        // var heightX = 0;
        // for (var c = 0 ; c< arg0.length;c++ ) {
        //     console.log("item at ",c," height:", arg0[c].height)
        //     heightX +=  arg0[c].height;
        // }

        // console.log("total heightX: ",heightX)
        return arg0.height;
    }

    ListModel {
        id:model1

        ListElement { txt: "Video Stream";    cmp: "secComp0" }
        ListElement { txt: "Receiver Device"; cmp: "secComp2" }
        ListElement { txt: "WS Message";      cmp: "secComp1" }
    }

    ListView {
        id:listOf
        anchors {
            fill: parent
            margins: 2
        }
        spacing: 10
        currentIndex: -1

        model: model1

        delegate: Foldable {
            idx: index
            headerTXT: txt
            sourceCmp: cmp
            selected:  ListView.isCurrentItem // index == ListView.view.currentIndex

            // onSelectedChanged: listOf.currentIndex = index
        }
    }
}
