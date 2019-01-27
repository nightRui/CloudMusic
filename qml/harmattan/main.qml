import QtQuick 1.1
import com.nokia.meego 1.0
import com.nokia.extras 1.1
import com.yeatse.cloudmusic 1.0

import "../js/api.js" as Api
import "../js/util.js" as Util

PageStackWindow {
    id: app

    platformStyle: PageStackWindowStyle {
        backgroundFillMode: Image.PreserveAspectCrop
        Image {
            id: backgroundImage
            width: app.inPortrait ? screen.displayHeight : screen.displayWidth
            height: app.inPortrait ? screen.displayWidth : screen.displayHeight
            asynchronous: true
            fillMode: Image.PreserveAspectCrop
            source: player.coverImageUrl
            visible: false
            onStatusChanged: if (status == Image.Ready) {
                                 bgProvider.refresh(backgroundImage)
                                 platformStyle.background = "image://appBackground/" + Math.random()
                             }
        }
    }

    initialPage: MainPage {}

    QtObject {
        id: internal

        function initialize() {
            Api.qmlApi = qmlApi
            resetBackground()
            user.initialize()
            checkForUpdate()
        }

        function resetBackground() {
            theme.inverted = true

            var tbar = pageStack.toolBar
            for (var i = 0; i < tbar.children.length; i++) {
                if (tbar.children[i].hasOwnProperty("verticalTileMode")) {
                    tbar.children[i].opacity = 0.7
                    break
                }
            }
        }

        function checkForUpdate() {
            var xhr = new XMLHttpRequest
            xhr.onreadystatechange = function() {
                        if (xhr.readyState == XMLHttpRequest.DONE) {
                            if (xhr.status == 200) {
                                var resp = JSON.parse(xhr.responseText)
                                if (Util.verNameToVerCode(appVersion) < Util.verNameToVerCode(resp.ver)) {
                                    var diag = updateDialogComp.createObject(initialPage)
                                    diag.message = "当前版本: %1\n最新版本: %2\n%3".arg(appVersion).arg(resp.ver).arg(resp.desc)
                                    diag.downUrl = resp.url
                                    diag.open()
                                }
                            }
                        }
                    }
            xhr.open("GET", "http://yeatse.com/cloudmusicqt/harmattan.ver")
            xhr.send(null)
        }

        property Component updateDialogComp: Component {
            QueryDialog {
                id: dialog
                property bool closing: false
                property string downUrl
                titleText: "目测新版本粗现"
                acceptButtonText: "下载"
                rejectButtonText: "取消"
                onAccepted: Qt.openUrlExternally(downUrl)
                onStatusChanged: {
                    if (status == DialogStatus.Closing)
                        closing = true
                    else if (status == DialogStatus.Closed && closing)
                        dialog.destroy(500)
                }
            }
        }
    }

    Connections {
        target: qmlApi
        onProcessCommand: {
            console.log("qml api: process command", commandId)
            if (commandId == 1) {
                player.bringToFront()
            }
            else if (commandId == 2) {
                if (pageStack.currentPage == null
                        || pageStack.currentPage.objectName != player.callerTypeDownload)
                    pageStack.push(Qt.resolvedUrl("DownloadPage.qml"))
            }
        }
    }

    Connections {
        target: downloader
        onDownloadCompleted: {
            var msg = success ? "下载完成:" : "下载失败:"
            msg += musicName
            infoBanner.showMessage(msg)
//            qmlApi.showNotification("网易云音乐", msg, 2)
        }
    }

    CloudMusicUser {
        id: user
    }

    CountDownTimer {
        id: cdTimer
        onTriggered: Qt.quit()
    }

    InfoBanner {
        id: infoBanner

        y: 36

        function showMessage(msg) {
            infoBanner.text = msg
            infoBanner.show()
        }

        function showDevelopingMsg() {
            showMessage("此功能正在建设中...> <")
        }
    }

    PlayerPage {
        id: player
    }

    Component.onCompleted: internal.initialize()
}
