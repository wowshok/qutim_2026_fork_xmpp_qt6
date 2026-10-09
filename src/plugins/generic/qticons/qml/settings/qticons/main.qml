import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import org.qutim 0.4

SettingsItem {
    id: root

    Config {
        id: config
        group: "qticons"
    }

    Service {
        id: iloader
        name: "IconLoader"
    }

    function save() {
        config.setValue("qutim-default", defaultThemeCheckBox.checked)
    }

    function load() {
        defaultThemeCheckBox.checked = config.value("qutim-default", true);
        missingIconsBox.text = iloader.object ? iloader.object.iconsList : "";
    }

    ColumnLayout {
        anchors.fill: parent

        CheckBox {
            id: defaultThemeCheckBox
            text: qsTr("Enable qutim-default icon theme")
            onToggled: root.modify()
        }

        Label {
            text: qsTr("List of missing icons in your theme:")
        }

        ScrollView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            TextArea {
                id: missingIconsBox
                readOnly: true
            }
        }
    }
}
