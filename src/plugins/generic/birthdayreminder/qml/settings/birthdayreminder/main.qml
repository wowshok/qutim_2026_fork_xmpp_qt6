import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import org.qutim 0.4

SettingsItem {
    id: root

    Config {
        id: config
        group: "birthdayReminder"
    }

    // The interval is stored in hours with one decimal; the spin box works in tenths of an hour
    function save() {
        config.setValue("intervalBetweenNotifications", intervalEdit.value / 10.0);
        config.setValue("daysBeforeNotification", daysEdit.value);
    }

    // The original page read "...Edit" keys here, so saved values never showed up
    function load() {
        intervalEdit.value = Math.round(config.value("intervalBetweenNotifications", 24.0) * 10);
        daysEdit.value = config.value("daysBeforeNotification", 3);
    }

    GridLayout {
        anchors.fill: parent
        columns: 3

        Label { text: qsTr("Show notifications every: ") }
        SpinBox {
            id: intervalEdit
            from: 1
            to: 24 * 7 * 10
            stepSize: 10
            editable: true
            Layout.fillWidth: true
            textFromValue: function(value, locale) { return Number(value / 10).toLocaleString(locale, 'f', 1); }
            valueFromText: function(text, locale) { return Math.round(Number.fromLocaleString(locale, text) * 10); }
            onValueModified: root.modify()
        }
        Label { text: qsTr("hours.") }

        Label { text: qsTr("starting from: ") }
        SpinBox {
            id: daysEdit
            from: 0
            to: 365
            editable: true
            Layout.fillWidth: true
            onValueModified: root.modify()
        }
        Label { text: qsTr("days before birthday.") }

        Item {
            Layout.columnSpan: 3
            Layout.fillHeight: true
        }
    }
}
