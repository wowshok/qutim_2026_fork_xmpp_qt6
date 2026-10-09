import QtQuick 2.15
import "../default" as Default

// The original theme used Windows DWM glass (QtWinExtras); this keeps its look
// with a translucent dark background that works everywhere
Default.Controller {
    popupComponent: Default.PopupBase {
        id: window

        textColor: "black"
        textStyle: Text.Outline
        textStyleColor: "white"

        Rectangle {
            anchors.fill: window.contentItem
            radius: 8
            color: "#b0dce6f0"
            border.color: "#80ffffff"
        }
    }
}
