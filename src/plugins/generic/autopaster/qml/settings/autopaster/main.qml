import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import org.qutim 0.4
import org.qutim.autopaster 0.4

SettingsItem {
    id: root

    Config {
        id: config
        group: "autoPaster"
    }

    Plugin {
        id: autopaster
        name: "autopaster"
    }

    readonly property QtObject handler: autopaster.object ? autopaster.object.handler : null

    function save() {
        config.setValue("autoSubmit", autoSubmitEdit.checked);
        config.setValue("defaultLocation", defaultLocationEdit.currentIndex);
        config.setValue("lineCount", lineCountEdit.value);
    }

    function load() {
        itemsModel.clear();

        var pasters = root.handler ? root.handler.pasters : [];
        for (var i = 0; i < pasters.length; ++i) {
            itemsModel.append({
                text: pasters[i],
                value: pasters[i]
            });
        }

        autoSubmitEdit.checked = config.value("autoSubmit", false);
        defaultLocationEdit.currentIndex = config.value("defaultLocation", 0);
        lineCountEdit.value = config.value("lineCount", 5);
    }

    ListModel {
        id: itemsModel
    }
    GridLayout {
        anchors.fill: parent
        columns: 2

        Label { text: qsTr("Autosubmit:") }
        CheckBox {
            id: autoSubmitEdit
            Layout.fillWidth: true
            onToggled: root.modify()
        }

        Label { text: qsTr("Default paste:") }
        ComboBox {
            id: defaultLocationEdit
            Layout.fillWidth: true
            model: itemsModel
            textRole: "text"
            onActivated: root.modify()
        }

        Label { text: qsTr("The number of rows to trigger:") }
        SpinBox {
            id: lineCountEdit
            from: 1
            to: 1000
            editable: true
            Layout.fillWidth: true
            onValueModified: root.modify()
        }

        Item {
            Layout.columnSpan: 2
            Layout.fillHeight: true
        }
    }
}
