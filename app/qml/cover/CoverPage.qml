import QtQuick 2.0
import Sailfish.Silica 1.0

CoverBackground {
    Column {
        anchors.centerIn: parent
        width: parent.width - 2 * Theme.paddingLarge
        spacing: Theme.paddingSmall

        Label {
            width: parent.width
            text: "Sailfish Link"
            horizontalAlignment: Text.AlignHCenter
            truncationMode: TruncationMode.Fade
        }

        Label {
            width: parent.width
            text: linkModel.statusState
            horizontalAlignment: Text.AlignHCenter
            color: Theme.secondaryColor
            truncationMode: TruncationMode.Fade
        }
    }

    CoverActionList {
        CoverAction {
            iconSource: "image://theme/icon-cover-refresh"
            onTriggered: linkModel.refresh()
        }
    }
}
