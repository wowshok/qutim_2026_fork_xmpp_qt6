import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import org.qutim 0.4

SettingsItem {
    id: root

    Config {
        id: config
        group: "auto-away"
    }

    function save() {
        config.setValue("away-enabled", awayCheckBox.checked)
        config.setValue("na-enabled", naCheckBox.checked)
        config.setValue("away-secs", awaySpinBox.value * 60)
        config.setValue("na-secs", naSpinBox.value * 60)
        config.setValue("away-text", awayTextBox.text)
        config.setValue("na-text", naTextBox.text)
    }

    function load() {
        awayCheckBox.checked = config.value("away-enabled", true);
        naCheckBox.checked = config.value("na-enabled", true);
        awaySpinBox.value = config.value("away-secs", 180) / 60;
        naSpinBox.value = config.value("na-secs", 600) / 60;
        awayTextBox.text = config.value("away-text", "");
        naTextBox.text = config.value("na-text", "");
    }

    component MinutesSpinBox: SpinBox {
        from: 1
        to: 24 * 60
        editable: true
        Layout.preferredWidth: 200
        textFromValue: function(value, locale) { return qsTr("%1 min.").arg(value); }
        valueFromText: function(text, locale) { return parseInt(text, 10) || from; }
        onValueModified: root.modify()
    }

    ColumnLayout {
        anchors.fill: parent

        CheckBox {
            id: awayCheckBox
            text: qsTr("Set status \"Away\" after:")
            onToggled: root.modify()
        }
        MinutesSpinBox {
            id: awaySpinBox
            enabled: awayCheckBox.checked
        }
        TextArea {
            id: awayTextBox
            enabled: awayCheckBox.checked
            Layout.fillWidth: true
            onTextChanged: root.modify()
        }
        CheckBox {
            id: naCheckBox
            text: qsTr("Set status \"Not available\" after:")
            onToggled: root.modify()
        }
        MinutesSpinBox {
            id: naSpinBox
            enabled: naCheckBox.checked
        }
        TextArea {
            id: naTextBox
            enabled: naCheckBox.checked
            Layout.fillWidth: true
            onTextChanged: root.modify()
        }
        Item {
            Layout.fillHeight: true
        }
    }
}
