import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import org.qutim 0.4

SettingsItem {
    id: root

    Config {
        id: config
        path: "autoreply"
    }

    function save() {
        var messages = [];
        for (var i = 0; i < messagesModel.count; ++i)
            messages.push(messagesModel.get(i).message);

        config.setValue("automatic", automaticEdit.checked);
        config.setValue("timeOut", timeOutEdit.value * 60);
        config.setValue("deltaTime", deltaTimeEdit.value * 60);
        config.setValue("message", messageEdit.text);
        config.setValue("messages", messages);
    }

    function load() {
        automaticEdit.checked = config.value("automatic", true);
        timeOutEdit.value = config.value("timeOut", 60 * 15) / 60;
        deltaTimeEdit.value = config.value("deltaTime", 60 * 15) / 60;
        messageEdit.text = config.value("message", "");
        var messages = config.value("messages", []);

        messagesModel.clear();
        for (var i = 0; i < messages.length; ++i)
            messagesModel.append({ message: messages[i] });
    }

    ListModel {
        id: messagesModel
    }

    component MinutesSpinBox: SpinBox {
        from: 1
        to: 3600
        editable: true
        Layout.fillWidth: true
        textFromValue: function(value, locale) { return qsTr("%1 min.").arg(value); }
        valueFromText: function(text, locale) { return parseInt(text, 10) || from; }
        onValueModified: root.modify()
    }

    GridLayout {
        anchors.fill: parent
        columns: 2

        CheckBox {
            id: automaticEdit
            text: qsTr("Enable autoreply by idle")
            Layout.columnSpan: 2
            onToggled: root.modify()
        }

        Label {
            text: qsTr("Idle timeout:")
            enabled: automaticEdit.checked
        }
        MinutesSpinBox {
            id: timeOutEdit
            enabled: automaticEdit.checked
        }

        Label {
            text: qsTr("Delta time between messages:")
            enabled: automaticEdit.checked
        }
        MinutesSpinBox {
            id: deltaTimeEdit
        }

        // Automatic mode: one message sent on idle
        ScrollView {
            Layout.columnSpan: 2
            Layout.fillHeight: true
            Layout.fillWidth: true
            visible: automaticEdit.checked

            TextArea {
                id: messageEdit
                text: "Hello!"
                textFormat: TextEdit.PlainText
                wrapMode: TextEdit.Wrap
                onTextChanged: root.modify()
            }
        }

        // Manual mode: a list of answers to choose from
        ColumnLayout {
            Layout.columnSpan: 2
            Layout.fillHeight: true
            Layout.fillWidth: true
            visible: !automaticEdit.checked

            ListView {
                Layout.fillHeight: true
                Layout.fillWidth: true
                clip: true
                spacing: 4
                model: messagesModel
                ScrollBar.vertical: ScrollBar {}
                delegate: RowLayout {
                    width: ListView.view.width
                    height: 100
                    TextArea {
                        Layout.fillHeight: true
                        Layout.fillWidth: true
                        wrapMode: TextEdit.Wrap
                        text: message
                        onTextChanged: {
                            if (messagesModel.get(index).message !== text) {
                                messagesModel.setProperty(index, "message", text);
                                root.modify();
                            }
                        }
                    }
                    Button {
                        text: qsTr("Remove")
                        onClicked: {
                            messagesModel.remove(index);
                            root.modify();
                        }
                    }
                }
            }
            Button {
                Layout.alignment: Qt.AlignRight
                text: qsTr("Add answer")
                onClicked: {
                    messagesModel.append({ message: qsTr("Enter your text here") });
                    root.modify();
                }
            }
        }
    }
}
