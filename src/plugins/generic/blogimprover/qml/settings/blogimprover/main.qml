import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import org.qutim 0.4

SettingsItem {
    id: root

    Config {
        id: config
        group: "BlogImprover"
    }

    function save() {
        config.setValue("enablePointIntegration", enablePointIntegration.checked);
        config.setValue("enableJuickIntegration", enableJuickIntegration.checked);
        config.setValue("enableBnwIntegration", enableBnwIntegration.checked);
    }

    function load() {
        enablePointIntegration.checked = config.value("enablePointIntegration", true);
        enableJuickIntegration.checked = config.value("enableJuickIntegration", true);
        enableBnwIntegration.checked = config.value("enableBnwIntegration", true);
    }

    ColumnLayout {
        anchors.fill: parent
        CheckBox {
            id: enablePointIntegration
            text: qsTr("Enable Point integration")
            onToggled: root.modify()
        }
        CheckBox {
            id: enableJuickIntegration
            text: qsTr("Enable Juick integration")
            onToggled: root.modify()
        }
        CheckBox {
            id: enableBnwIntegration
            text: qsTr("Enable Bnw integration")
            onToggled: root.modify()
        }
        Item { Layout.fillHeight: true }
    }
}
