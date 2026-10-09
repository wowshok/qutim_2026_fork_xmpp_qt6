import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import org.qutim 0.4

SettingsItem {
    id: root

    Config {
        id: config
        group: "urlPreview"
    }

    function save() {
        config.setValue("maxFileSize", maxFileSizeEdit.value);
        config.setValue("maxWidth", maxWidthEdit.value);
        config.setValue("maxHeight", maxHeightEdit.value);
        config.setValue("youtubePreview", youtubePreviewEdit.checked);
        config.setValue("imagesPreview", imagesPreviewEdit.checked);
        config.setValue("HTML5Audio", html5AudioEdit.checked);
        config.setValue("HTML5Video", html5VideoEdit.checked);
        var exceptions = exceptionListEdit.text.split(';').filter(function(word) { return word.length > 0; });
        config.setValue("exceptionList", exceptions);
    }

    // Defaults must match UrlPreview::UrlHandler::loadSettings()
    function load() {
        maxFileSizeEdit.value = config.value("maxFileSize", 100000);
        maxWidthEdit.value = config.value("maxWidth", 800);
        maxHeightEdit.value = config.value("maxHeight", 600);
        youtubePreviewEdit.checked = config.value("youtubePreview", true);
        imagesPreviewEdit.checked = config.value("imagesPreview", true);
        html5AudioEdit.checked = config.value("HTML5Audio", true);
        html5VideoEdit.checked = config.value("HTML5Video", true);
        exceptionListEdit.text = config.value("exceptionList", []).join(';');
    }

    component UnitSpinBox: SpinBox {
        property string unit
        editable: true
        Layout.fillWidth: true
        textFromValue: function(value, locale) { return value + unit; }
        valueFromText: function(text, locale) { return parseInt(text, 10) || from; }
        onValueModified: root.modify()
    }

    GridLayout {
        anchors.fill: parent
        columns: 2

        Label { text: qsTr("Max. file size:") }
        UnitSpinBox {
            id: maxFileSizeEdit
            unit: qsTr(" bytes")
            stepSize: 1000
            from: 1000
            to: 1000000000
        }
        Label { text: qsTr("Max. width:") }
        UnitSpinBox {
            id: maxWidthEdit
            unit: qsTr(" px.")
            stepSize: 10
            from: 50
            to: 2000
        }
        Label { text: qsTr("Max. height:") }
        UnitSpinBox {
            id: maxHeightEdit
            unit: qsTr(" px.")
            stepSize: 10
            from: 50
            to: 2000
        }
        CheckBox {
            id: youtubePreviewEdit
            text: qsTr("Enable youtube preview")
            Layout.columnSpan: 2
            onToggled: root.modify()
        }
        CheckBox {
            id: imagesPreviewEdit
            text: qsTr("Enable images preview")
            Layout.columnSpan: 2
            onToggled: root.modify()
        }
        CheckBox {
            id: html5AudioEdit
            text: qsTr("Enable HTML5 Audio")
            Layout.columnSpan: 2
            onToggled: root.modify()
        }
        CheckBox {
            id: html5VideoEdit
            text: qsTr("Enable HTML5 Video")
            Layout.columnSpan: 2
            onToggled: root.modify()
        }
        Label {
            text: qsTr("Do not preview links containing these words (separated by ';'):")
            Layout.columnSpan: 2
            Layout.fillWidth: true
            wrapMode: Text.Wrap
        }
        TextArea {
            id: exceptionListEdit
            Layout.columnSpan: 2
            Layout.fillWidth: true
            wrapMode: TextEdit.Wrap
            onTextChanged: root.modify()
        }
        Item {
            Layout.columnSpan: 2
            Layout.fillHeight: true
        }
    }
}
