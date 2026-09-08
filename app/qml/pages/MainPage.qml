import QtQuick 2.0
import Sailfish.Silica 1.0

Page {
    id: page
    allowedOrientations: Orientation.All

    Component.onCompleted: {
        linkModel.refresh()
        if (linkModel.firstRun)
            pageStack.push(Qt.resolvedUrl("FirstRunPage.qml"))
    }

    SilicaFlickable {
        anchors.fill: parent
        contentHeight: content.height

        PullDownMenu {
            MenuItem {
                text: "Clear generated files"
                onClicked: linkModel.clearGeneratedFiles()
            }
            MenuItem {
                text: "Rotate pairing token"
                onClicked: linkModel.rotatePairingToken()
            }
            MenuItem {
                text: "Refresh"
                onClicked: linkModel.refresh()
            }
        }

        Column {
            id: content
            width: parent.width
            spacing: Theme.paddingMedium

            PageHeader {
                title: "Sailfish Link"
            }

            SectionHeader {
                text: "Status"
            }

            TextSwitch {
                id: enabledSwitch
                text: "Discovery enabled"
                description: linkModel.enabled ? "Service may announce on local Wi-Fi" : "Service will stay quiet"
                checked: linkModel.enabled
                onClicked: linkModel.setDiscoveryEnabled(!linkModel.enabled)
            }

            Label {
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * Theme.horizontalPageMargin
                text: linkModel.statusSummary
                wrapMode: Text.Wrap
                color: Theme.primaryColor
            }

            DetailItem {
                label: "Current IP"
                value: linkModel.currentIp || "None"
            }

            DetailItem {
                label: "Interface"
                value: linkModel.interfaceName || "None"
            }

            DetailItem {
                label: "Last announce"
                value: linkModel.lastAnnounceAt || "Never"
            }

            DetailItem {
                label: "Last error"
                value: linkModel.lastError || "None"
            }

            SectionHeader {
                text: "Pairing"
            }

            Label {
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * Theme.horizontalPageMargin
                text: "Token hint: " + linkModel.tokenHint
                wrapMode: Text.Wrap
                color: Theme.highlightColor
            }

            Label {
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * Theme.horizontalPageMargin
                text: linkModel.pairingToken
                wrapMode: Text.WrapAnywhere
                font.pixelSize: Theme.fontSizeSmall
                color: Theme.primaryColor
            }

            Button {
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * Theme.horizontalPageMargin
                text: "Rotate token"
                onClicked: linkModel.rotatePairingToken()
            }

            SectionHeader {
                text: "Settings"
            }

            TextField {
                id: deviceNameField
                width: parent.width
                label: "Device name"
                text: linkModel.deviceName
                EnterKey.iconSource: "image://theme/icon-m-enter-accept"
            }

            TextField {
                id: sshUserField
                width: parent.width
                label: "SSH user"
                text: linkModel.sshUser
                EnterKey.iconSource: "image://theme/icon-m-enter-next"
            }

            TextField {
                id: sshPortField
                width: parent.width
                label: "SSH port"
                text: linkModel.sshPort
                inputMethodHints: Qt.ImhDigitsOnly
                EnterKey.iconSource: "image://theme/icon-m-enter-next"
            }

            TextField {
                id: announcePortField
                width: parent.width
                label: "Announcement UDP port"
                text: linkModel.announcementPort
                inputMethodHints: Qt.ImhDigitsOnly
                EnterKey.iconSource: "image://theme/icon-m-enter-accept"
            }

            Button {
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * Theme.horizontalPageMargin
                text: "Save settings"
                onClicked: linkModel.saveSettings(deviceNameField.text,
                                                  sshUserField.text,
                                                  parseInt(sshPortField.text),
                                                  parseInt(announcePortField.text),
                                                  enabledSwitch.checked)
            }

            SectionHeader {
                text: "Files"
            }

            DetailItem {
                label: "Config"
                value: linkModel.configPath
            }

            DetailItem {
                label: "Status"
                value: linkModel.statusPath
            }

            DetailItem {
                label: "Log"
                value: linkModel.logPath
            }

            DetailItem {
                label: "Generated size"
                value: linkModel.generatedSize
            }

            Button {
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * Theme.horizontalPageMargin
                text: "Delete generated files"
                onClicked: linkModel.clearGeneratedFiles()
            }

            SectionHeader {
                text: "Announcement Payload"
            }

            Label {
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * Theme.horizontalPageMargin
                text: linkModel.payloadPreview || "No payload has been announced yet."
                wrapMode: Text.WrapAnywhere
                font.pixelSize: Theme.fontSizeExtraSmall
                color: Theme.secondaryColor
            }

            Label {
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * Theme.horizontalPageMargin
                text: linkModel.message
                wrapMode: Text.Wrap
                color: Theme.highlightColor
                visible: text.length > 0
            }

            Item {
                width: 1
                height: Theme.paddingLarge
            }
        }
    }
}
