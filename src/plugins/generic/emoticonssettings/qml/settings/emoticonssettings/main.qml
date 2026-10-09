import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import org.qutim 0.4

SettingsItem {
    id: root

    Emoticons {
        id: emoticons
    }

    EmoticonsTheme {
        id: emoticonsTheme
        // Index 0 is "No emoticons"
        themeName: comboBox.currentIndex > 0 ? comboBox.currentText : ""
    }

    function save() {
        var index = comboBox.currentIndex;
        emoticons.themeName = index > 0 ? themesModel.get(index).text : "";
    }

    function load() {
        themesModel.clear();
        themesModel.append({ text: qsTr("No emoticons") })

        var index = 0;

        // Lists coming from C++ are read-only in Qt 6, copy before sorting
        var themes = Array.from(emoticons.themeList);
        themes.sort();
        for (var i = 0; i < themes.length; ++i) {
            if (themes[i] === emoticons.themeName)
                index = i + 1;

            themesModel.append({ text: themes[i] });
        }

        comboBox.currentIndex = index;
    }

    ListModel {
        id: themesModel
    }

    ColumnLayout {
        anchors.fill: parent

        ComboBox {
            id: comboBox
            Layout.fillWidth: true
            model: themesModel
            textRole: "text"
            onActivated: root.modify()
        }
        ScrollView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true

            GridView {
                cellWidth: 32
                cellHeight: 32
                model: emoticonsTheme.emoticons
                delegate: Item {
                    width: 32
                    height: 32
                    AnimatedImage {
                        anchors.centerIn: parent
                        source: modelData.url
                    }
                }
            }
        }
    }
}
