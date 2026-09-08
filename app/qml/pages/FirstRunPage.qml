import QtQuick 2.0
import Sailfish.Silica 1.0

Page {
    id: page
    allowedOrientations: Orientation.All

    SilicaFlickable {
        anchors.fill: parent
        contentHeight: content.height

        Column {
            id: content
            width: parent.width
            spacing: Theme.paddingLarge

            PageHeader {
                title: "First Run"
            }

            Label {
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * Theme.horizontalPageMargin
                text: "Sailfish Link announces this phone on the local Wi-Fi network so the mother PC can find the current SSH and service endpoints after the IP address changes."
                wrapMode: Text.Wrap
                color: Theme.primaryColor
            }

            Label {
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * Theme.horizontalPageMargin
                text: "Discovery starts disabled. Review the pairing token, confirm the SSH settings, then enable discovery when the phone is on a trusted LAN."
                wrapMode: Text.Wrap
                color: Theme.primaryColor
            }

            Label {
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * Theme.horizontalPageMargin
                text: "The broadcast payload contains only a token hint. Use the full token shown in the app to pair the mother PC, and rotate it here if trust needs to be reset."
                wrapMode: Text.Wrap
                color: Theme.primaryColor
            }

            Button {
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * Theme.horizontalPageMargin
                text: "Done"
                onClicked: {
                    linkModel.markFirstRunSeen()
                    pageStack.pop()
                }
            }
        }
    }
}
