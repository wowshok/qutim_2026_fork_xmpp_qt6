import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Window 2.15
import QtQuick.Layouts 1.15
import org.qutim 0.4
import org.qutim.autopaster 0.4

// Shown through QuickDialog and by the handler when a long message is about to be sent
ApplicationWindow {
    id: root
    width: 450
    height: 160
    visible: false

    title: qsTr("Autopaster")

    Plugin {
        id: autopaster
        name: "autopaster"
    }

    readonly property QtObject handler: autopaster.object ? autopaster.object.handler : null

    Connections {
        target: root.handler
        function onMessageReceived() { root.visible = true; }
    }

    onVisibleChanged: {
        if (visible && root.handler)
            pasterItems.currentIndex = root.handler.currentPasterIndex;
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 10

        GridLayout {
            Layout.fillWidth: true
            columns: 2

            Label { text: qsTr("Pastebins: ") }
            ComboBox {
                id: pasterItems
                Layout.fillWidth: true
                textRole: "text"
                model: ListModel { id: pastersModel }
            }

            Label { text: qsTr("Languages: ") }
            ComboBox {
                id: syntaxItems
                Layout.fillWidth: true
                textRole: "text"
                model: ListModel { id: syntaxesModel }
            }
        }

        Item { Layout.fillHeight: true }

        DialogButtonBox {
            Layout.fillWidth: true
            standardButtons: DialogButtonBox.Ok | DialogButtonBox.Cancel
            onAccepted: {
                var paster = pastersModel.get(pasterItems.currentIndex).value;
                var syntax = syntaxesModel.get(syntaxItems.currentIndex).value;
                root.handler.upload(paster, syntax);
                root.close();
            }
            onRejected: {
                root.handler.cancel();
                root.close();
            }
        }
    }

    Component.onCompleted: {
        if (!root.handler)
            return;

        var i;
        var pasters = root.handler.pasters;
        for (i = 0; i < pasters.length; ++i) {
            pastersModel.append({
                text: pasters[i],
                value: pasters[i]
            });
        }

        var syntaxes = root.handler.syntaxes;
        for (i = 0; i < syntaxes.length; ++i)
            syntaxesModel.append(syntaxes[i]);
    }
}
