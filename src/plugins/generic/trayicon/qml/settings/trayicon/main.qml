import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import org.qutim 0.4

SettingsItem {
    id: root

    property var iconOptions: [
        { "name": qsTr("Show number of new messages") },
        { "name": qsTr("Show number of chats with new messages") },
        { "name": qsTr("Show only icon") }
    ]
    property int currentOption: 0

    function save() {
        config.setValue("showNumber", currentOption);
        config.setValue("blink", blinkIconOption.checked);
        config.setValue("showIcon", showIconOption.checked);
    }

    function load() {
        root.currentOption = config.value("showNumber", 0);
        blinkIconOption.checked = config.value("blink", true);
        showIconOption.checked = config.value("showIcon", true);
    }

    ButtonGroup { id: iconGroup }
    Config {
        id: config
        path: "simpletray"
    }

    ColumnLayout {
        anchors.fill: parent

        GroupBox {
            title: qsTr("Icon")
            Layout.fillWidth: true

            ColumnLayout {
                Repeater {
                    model: root.iconOptions
                    RadioButton {
                        text: modelData.name
                        ButtonGroup.group: iconGroup
                        checked: index === root.currentOption
                        onToggled: {
                            root.currentOption = index;
                            root.modify();
                        }
                    }
                }
            }
        }

        Label {
            text: qsTr("Other")
            font.bold: true
        }
        CheckBox {
            id: showIconOption
            text: qsTr("Show mail icon if there are new messages")
            onToggled: root.modify()
        }
        CheckBox {
            id: blinkIconOption
            text: qsTr("Blink icon")
            onToggled: root.modify()
        }

        Item { Layout.fillHeight: true }
    }
}
