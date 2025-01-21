/*

  this file includes main user interface.
  beside that, media playback and websocket messaging logics are also implemented here.
*/


// pragma Singleton

import QtCore
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtMultimedia
import QtWebSockets
import CVcustomDetector



Window {
    id: mainWindow
    width: 818
    height: 480
    visible: true
    title: qsTr(" Age & Gender Detector ")



    property alias videoStreamSet: videoStreamSetting
    property alias reciverSet:     reciverSetting
    property alias messageSet:     messageSetting

    property bool drawing: false


    Settings {
        id: videoStreamSetting
        category: "videoStream"
        property bool preview: true
        property string videoInput:"rtsp"
        property int cameraID: 0
        property string streamSource: "rtsp://192.168.1.36:8554/live"
        property string streamSourceUser: ""
        property string streamSourcePass: ""

        onVideoInputChanged: {
            resetBoundingBoxes()
            console.log("mainWindow videoInputs:",videoInput)
            switch(videoInput){
            case "camera":
                videoStreamer.stop()
                captureSession.videoOutput = video
                camera.start()
                break;
            case "rtsp":
                camera.stop()
                videoStreamer.videoOutput = video
                videoStreamer.source = videoStreamSetting.streamSource
                break;
            default:
                console.log("an Error Occurred, videoInput: ", videoInput)

                break;
            }
        }


        onStreamSourceChanged: {
            if (videoInput === "rtsp") {
                videoStreamer.stop()
                videoStreamer.videoOutput = video
                videoStreamer.source = videoStreamSetting.streamSource
            }

        }

    }

    Settings {
        id: messageSetting
        category: "message"
        property bool sendMsg: true
        property bool dGender: true
        property bool dAge:    true
        property bool dCount:  true
        property bool eCount:  true
        property bool eClear:  true

        onSendMsgChanged: sendMsg ? (
                                        wsClient.active = true ,
                                        socketTimer.start()
                                    )

                                  : (
                                        wsClient.active = false ,
                                        socketTimer.stop()
                                    )
    }

    Settings {
        id: reciverSetting
        category: "reciver"
        property bool secure: false
        property string ip: ""
        property string port: "80"
        onSecureChanged: {
            wsClient.url = reciverSetting.secure ? "wss://"+reciverSetting.ip+":"+reciverSetting.port :
                                                   "ws://" +reciverSetting.ip+":"+reciverSetting.port
        }
        onIpChanged: {
            wsClient.url = reciverSetting.secure ? "wss://"+reciverSetting.ip+":"+reciverSetting.port :
                                                   "ws://" +reciverSetting.ip+":"+reciverSetting.port
        }
        onPortChanged: {
            wsClient.url = reciverSetting.secure ? "wss://"+reciverSetting.ip+":"+reciverSetting.port :
                                                   "ws://" +reciverSetting.ip+":"+reciverSetting.port
        }
    }


    function resetBoundingBoxes() {
        for (var i = 0; i < boundingBoxesHolder.count; ++i)
            boundingBoxesHolder.itemAt(i).visible = false;
    }

    function delay(delayTime) {
        timer = new Timer()
        timer.interval = delayTime;
        timer.repeat = false;
        timer.start();
    }

    function delay2(duration) { // In milliseconds
        var timeStart = new Date().getTime();

        while (new Date().getTime() - timeStart < duration) {
            // console.log(" retry in: ", new Date().getTime() - timeStart )
        }
        // Duration has passed
    }

    function sleep(ms) {
        return new Promise(resolve => setTimeout(resolve, ms));
    }

    function resetSource () {
        videoStreamer.stop();
        videoStreamer.source = ""
        sleep(2500)
        // delay2(5)
        videoStreamer.source = videoStreamSetting.streamSource
        videoStreamer.videoOutput = video
        sleep(2500)
    }


    Component.onCompleted: {
        if (videoStreamSetting.videoInput === /*"camera"*/ "rtsp"){
            camera.stop()
            // videoStreamer.stop()
            videoStreamer.videoOutput = video
            videoStreamer.source = videoStreamSetting.streamSource
        }
        cvLdr.sourceComponent = cvComp
    }


    Timer{
        id: drawingTimer
        interval: 100
        onTriggered: {
            drawing = false;
        }
    }

    Timer {
        id: socketTimer
        interval: 1000
        running: true
        repeat: true
        onTriggered: {
            wsClient.active = false;
            wsClient.active = true;
        }
    }

    Timer{
        id: resetTimer
        running: false
        onTriggered: {
            videoStreamer.stop();
            videoStreamer.source = ""

            videoStreamer.videoOutput = video
            videoStreamer.source = videoStreamSetting.streamSource
        }
    }


    Loader {
        id: cvLdr
    }

    Component {
        id:cvComp
        CustomDetector
        {
            id: testFilter
            videoSink : video.videoSink

            onObjectsDetected: function (faceData){

                var total = 0;
                var mc= 0;
                var fc= 0;
                var mina = 0;
                var maxa = 0;
                var ageBrkt = "";

                if(drawing) return;

                drawing = true;

                resetBoundingBoxes();

                var JsonData = JSON.parse(faceData);
                var contentRect = video.contentRect;


                for(let i = 0; i < JsonData.length; i++){
                    var currentPerson = JsonData[i];
                    var boundingBox = boundingBoxesHolder.itemAt(i);

                    if(currentPerson.gender === "Male")
                        mc++;
                    else
                        fc++;


                    var ageD = currentPerson.age.split('-')[0].split('(')[1]
                    mina += Number(ageD);
                    var ageU = currentPerson.age.split('-')[1].split(')')[0]
                    maxa += Number(ageU);

                    var r = {
                        x: currentPerson.x * contentRect.width,
                        y: currentPerson.y * contentRect.height,
                        width: currentPerson.w * contentRect.width,
                        height: currentPerson.h * contentRect.height,
                        gen: currentPerson.gender,
                        age: currentPerson.age
                    };

                    boundingBox.x = r.x;
                    boundingBox.y = r.y;
                    boundingBox.width = r.width;
                    boundingBox.height = r.height;
                    boundingBox.gen = r.gen;
                    boundingBox.age = r.age;
                    boundingBox.visible = true;
                }

                total = mc+fc;
                mina =  Math.round( mina/total );
                maxa =  Math.round( maxa/total );
                ageBrkt = mina + " - " + maxa;

                infoRect.minAge = mina;
                infoRect.maxAge = maxa;
                infoRect.tCount = total;
                infoRect.mCount = mc;
                infoRect.fCount = fc;
                infoRect.ageBracket = ageBrkt;

                var wsMsg={}
                wsMsg.Message = "Data"


                if(messageSetting.dGender){
                    wsMsg.Gender =  (mc>0 && fc>0) ?  "Male & Female" : ((mc>0)?  "Male": "Female")
                }
                if(messageSetting.dAge){
                    wsMsg.MinAge= mina
                    wsMsg.MaxAge= mina
                }
                if(messageSetting.dCount){
                    wsMsg.MaleCount = mc
                    wsMsg.FemaleCount = fc
                    wsMsg.TotalCount= total
                }

                // var wsstr = wsMsg.toString()
                // console.log(JSON.stringify(wsMsg))
                wsClient.message = JSON.stringify(wsMsg)
                wsClient.count = total;


                drawingTimer.start();

            }

            onNoObject: {

                var wsMsg={}
                wsMsg.Message = "Reset"
                // console.log(JSON.stringify(wsMsg))
                wsClient.message = JSON.stringify(wsMsg)
                wsClient.count = 0;

                infoRect.minAge = 0;
                infoRect.maxAge = 0;
                infoRect.tCount = 0;
                infoRect.mCount = 0;
                infoRect.fCount = 0;
                infoRect.ageBracket = "";
                resetBoundingBoxes();
            }

        }
    }


    WebSocket {
        id:wsClient
        property int count: 0
        property var message
        active: messageSetting.sendMsg
        url:  reciverSetting.secure ? "wss://"+reciverSetting.ip+":"+reciverSetting.port :
                                      "ws://"+reciverSetting.ip+":"+reciverSetting.port
        onStatusChanged: function (status){
            if (status === WebSocket.Error) {
                console.log(qsTr("Client error: %1").arg(wsClient.errorString));
                if(!socketTimer.running) socketTimer.start()
            } else if (status === WebSocket.Closed) {
                console.log(qsTr("Client socket closed."));
                if (messageSetting.sendMsg){
                    if(!socketTimer.running) socketTimer.start()
                }
            } else if (status === WebSocket.Open) {
                console.log(qsTr("Client socket openned."));
                if(socketTimer.running) socketTimer.stop()
            }
        }

        onCountChanged: {
            if(count>0){
                if(messageSetting.eCount)
                    sendTextMessage(message)
            }else{
                if(messageSetting.eClear)
                    sendTextMessage(message)
            }
        }
    }



    MediaPlayer {
        id: videoStreamer

        autoPlay: false
        // playbackRate: 0
        audioOutput: AudioOutput{
            muted: true
        }

        onErrorOccurred: {
            resetBoundingBoxes()
            console.log("an Error Occurred while playing RTSP stream: ", videoStreamer.error, videoStreamer.errorString )
            if (!resetTimer.running){
                resetTimer.interval = 5000
                resetTimer.start()
            }
        }

        onSourceChanged: play()
        onDurationChanged: {
            position = videoStreamer.duration
        }
        onPlayingChanged: {
            var stateString
            switch(playbackState){
            case 0:
                stateString = 'stopped';
                break;
            case 1:
                stateString = 'playing';
                break;
            case 2:
                stateString = 'paused';
                break;
            default:
                stateString = 'unDefained';
                break;
            }

            console.log("playBack state: ", stateString)
            if(playbackState !== MediaPlayer.PlayingState  &&  playbackState !== MediaPlayer.PausedState  && !resetTimer.running) {
                console.log("an Error Occurred in playing RTSP stream: ")
                errorOccurred(99, "may be source have been losted")
            }
            // if(playbackState === MediaPlayer.PausedState){
            //     play()
            // }

        }
    }

    MediaDevices {
        id: mediaDevices
    }

    CaptureSession {
        id: captureSession

        camera: Camera {
            id: camera
            cameraDevice: mediaDevices.videoInputs[videoStreamSetting.cameraID]
            focusMode: Camera.FocusModeAuto
        }
        imageCapture: ImageCapture {
            id: imageCapture

        }
        recorder: MediaRecorder {
            id: recorder
        }
    }


    Rectangle {
        id: main_frame
        antialiasing: true
        anchors.fill: parent

        Rectangle {
            id:logoRect
            width: contentItem.width
            height: 10
            Text {

                text: qsTr("Company logo")
            }
            anchors {
                top: parent.top
                left: parent.left
                margins: 15
            }
        }

        Rectangle {
            id:settingsRect
            width: txt.width + 5
            height: txt.height + 2
            border.width: 0
            border.color: "black"
            radius: 2
            Text {
                id:txt
                text: qsTr("Settings")
                anchors.centerIn: parent
            }
            anchors {
                top: parent.top
                right: parent.right
                margins: 15
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    settingPopup.open()
                }
            }
        }



        Popup {
            id:settingPopup
            // anchors.centerIn: parent
            x: main_frame.width/2 - width/2
            y: videoHolder.y + 10

            implicitWidth: parent.width /3
            width: 300
            implicitHeight: parent.height /3
            height: 200
            background: Rectangle {
                opacity: 0.7
                radius: 10
            }

            // modal: true
            focus: true
            dim: true
            Overlay.modeless: Rectangle {
                     color: "darkGray"
                     opacity: 0.8
            }


            // closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutsideParent | Popup.CloseOnPressOutsideParent

            enter: Transition {
                NumberAnimation { property: "opacity"; from: 0.0; to: 1.0 }
            }
            exit : Transition {
                NumberAnimation { property: "opacity"; from: 1.0; to: 0.0 }
            }

            onClosed:  {
                videoStreamSetting.sync();
                messageSetting.sync();
                reciverSetting.sync();
            }


            SettingsScreen {
                clip: true
                anchors.centerIn: parent
            }
        }

        GridLayout {
            id:grid
            anchors {
                margins: 10
                top: logoRect.bottom
                left: parent.left
                right: parent.right
                bottom: parent.bottom
            }

            columns: width > height ? 2 : 1
            Rectangle {

                id:videoHolder
                Layout.fillHeight: true
                Layout.margins: 5
                Layout.alignment: Qt.AlignLeft
                Layout.preferredWidth: mainWindow.width/3*2
                Layout.fillWidth: true

                border.color: "black"
                border.width: 1
                radius: 10


                Switch {
                    id: videoPreview
                    anchors {
                        // left: parent.left
                        right: parent.right
                        top: parent.top
                        margins: 10
                    }
                    z:99
                    // text: "Video Preview"
                    enabled: true
                    checked: videoStreamSetting.preview
                    checkable: true
                    onCheckedChanged: videoStreamSetting.preview = checked
                }

                VideoOutput
                {
                    id: video
                    anchors {
                        fill: parent
                        margins: 5
                    }
                    clip: true
                    // implicitHeight:  mainWindow.height/2
                    // source: videoStreamer //camera
                    // autoOrientation: true
                    focus : visible
                    visible: videoPreview.checked
                    fillMode: VideoOutput.PreserveAspectFit

                    MouseArea{
                        id: clickArea
                        anchors.fill: parent
                        onClicked: {

                            if( videoStreamSetting.videoInput === "camera" && mediaDevices.videoInputs.length > 1){
                                if(videoStreamSetting.cameraID<mediaDevices.videoInputs.length-1)
                                    videoStreamSetting.cameraID = videoStreamSetting.cameraID+1
                                else
                                    videoStreamSetting.cameraID = 0
                                camera.cameraDevice = mediaDevices.videoInputs[videoStreamSetting.cameraID]
                                console.log("Active camera changed: ",camera.cameraDevice)
                            }
                        }
                        // onPressAndHold: {
                        //     videoStreamSetting.videoInput = videoStreamSetting.videoInput === "camera" ? "rtsp":"camera"
                        // }
                    }

                    Item {
                        width: video.contentRect.width
                        height: video.contentRect.height
                        anchors.centerIn: parent

                        Repeater{
                            id: boundingBoxesHolder
                            model: 100

                            Rectangle{
                                id:drect
                                property var gen
                                property var age

                                border.width: 3
                                border.color: gen==="Female" ? "#FFC0CB" : "#3399ff"
                                visible: false
                                color: "transparent"
                                radius: 2
                                Label{
                                    anchors.bottom: drect.top
                                    text: drect.gen +" "+drect.age
                                    color: drect.border.color
                                }
                            }
                        }
                    }
                }
            }
            Rectangle {
                id: infoRect

                property int mCount
                property int fCount
                property int tCount: mCount + fCount
                property int minAge
                property int maxAge
                property string ageBracket

                Layout.fillHeight: true
                Layout.margins: 5
                Layout.alignment: Qt.AlignLeft
                Layout.preferredWidth: mainWindow.width/3
                Layout.fillWidth: true


                border.color: "black"
                border.width: 1
                radius: 10


                Column {
                    id:c1
                    anchors.fill: parent
                    anchors.margins: 10

                    RowLayout {
                        id:r1
                        Layout.fillWidth: true
                        Text {
                            text: qsTr("Number of People in Frame:")
                        }
                        Text {
                            Layout.alignment: Qt.AlignLeft
                            Layout.fillWidth: true
                            text: infoRect.tCount
                        }
                    }

                    RowLayout {
                        id:r2
                        // width:  parent.width
                        Layout.fillWidth: true
                        Text {
                            text: qsTr("Males:")
                        }
                        Text {
                            Layout.alignment: Qt.AlignLeft
                            Layout.fillWidth: true
                            text: infoRect.mCount
                        }
                    }

                    RowLayout {
                        id:r3
                        // width:  parent.width
                        Layout.fillWidth: true
                        Text {
                            text: qsTr("Females:")
                        }
                        Text {
                            Layout.alignment: Qt.AlignLeft
                            Layout.fillWidth: true
                            text: infoRect.fCount
                        }
                    }

                    RowLayout {
                        id:r4
                        // width:  parent.width
                        Layout.fillWidth: true
                        Text {
                            text: qsTr("Age Bracket:")
                        }
                        Text {

                            Layout.alignment: Qt.AlignLeft
                            Layout.fillWidth: true
                            text: infoRect.ageBracket
                        }
                    }

                }
            }
        }
    }
}
