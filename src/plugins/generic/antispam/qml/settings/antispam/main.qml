import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import org.qutim 0.4

SettingsItem {
    id: root

    Config {
        id: config
        group: "antispam"
    }

    function save() {
        config.setValue("enabled", enabledBoxEdit.checked);
        config.setValue("handleAuth", handleAuthEdit.checked);
        config.setValue("answers", answerEdit.text);
        config.setValue("success", successEdit.text);
        config.setValue("question", questionEdit.text);
    }

    // Defaults must match Antispam::Handler::loadSettings()
    function load() {
        enabledBoxEdit.checked = config.value("enabled", false);
        handleAuthEdit.checked = config.value("handleAuth", true);
        answerEdit.text = config.value("answers", qsTr("vodka;Vodka"));
        successEdit.text = config.value("success", qsTr("We are ready to drink with you!"));
        questionEdit.text = config.value("question", qsTr("Beer, wine, vodka, champagne: after which drink in this sequence I should stop?"));
    }

    ColumnLayout {
        anchors.fill: parent

        CheckBox {
            id: enabledBoxEdit
            text: qsTr("Enabled")
            onToggled: root.modify()
        }
        CheckBox {
            id: handleAuthEdit
            text: qsTr("Handle auth requests")
            enabled: enabledBoxEdit.checked
            onToggled: root.modify()
        }

        Label { text: qsTr("Question:") }
        TextArea {
            id: questionEdit
            Layout.fillWidth: true
            wrapMode: TextEdit.Wrap
            enabled: enabledBoxEdit.checked
            onTextChanged: root.modify()
        }

        Label { text: qsTr("Answers (semicolon as a separator):") }
        TextArea {
            id: answerEdit
            Layout.fillWidth: true
            wrapMode: TextEdit.Wrap
            enabled: enabledBoxEdit.checked
            onTextChanged: root.modify()
        }

        Label { text: qsTr("Message on correct answer:") }
        TextArea {
            id: successEdit
            Layout.fillWidth: true
            wrapMode: TextEdit.Wrap
            enabled: enabledBoxEdit.checked
            onTextChanged: root.modify()
        }

        Item { Layout.fillHeight: true }
    }
}
