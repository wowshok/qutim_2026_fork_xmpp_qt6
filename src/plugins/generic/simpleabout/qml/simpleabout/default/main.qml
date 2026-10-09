/****************************************************************************
**
** qutIM - instant messenger
**
** Copyright © 2014 Ruslan Nigmatullin <euroelessar@yandex.ru>
**
*****************************************************************************
**
** $QUTIM_BEGIN_LICENSE$
** This program is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
** See the GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program.  If not, see http://www.gnu.org/licenses/.
** $QUTIM_END_LICENSE$
**
****************************************************************************/

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Window 2.15
import QtQuick.Layouts 1.15
import org.qutim.simpleabout 0.4

// Shown through QuickDialog, which toggles the "visible" property
ApplicationWindow {
    id: root
    width: 520
    height: 480
    minimumWidth: 400
    minimumHeight: 360
    visible: false

    title: qsTr("About qutIM")

    AboutInfo {
        id: info
    }

    readonly property string translators: info.translators

    component InfoText: ScrollView {
        property alias text: area.text
        clip: true
        TextArea {
            id: area
            readOnly: true
            textFormat: TextEdit.RichText
            wrapMode: TextEdit.Wrap
            onLinkActivated: function(link) { Qt.openUrlExternally(link) }
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 10

        RowLayout {
            Layout.fillWidth: true

            Label {
                text: "qutIM"
                font.bold: true
                font.pointSize: qutimVersion.font.pointSize * 4
            }
            Label {
                id: qutimVersion
                text: info.qutimVersion
                Layout.alignment: Qt.AlignBottom
            }
        }

        Label {
            Layout.fillWidth: true
            wrapMode: Text.Wrap
            text: qsTr("Module based instant messenger. Based on Qt %1 (%2 bits)").arg(info.qtVersion).arg(info.wordSize)
        }

        GridLayout {
            columns: 2
            // qutim.org and trac.qutim.org are gone since 2023
            Label {
                text: qsTr("Source code repository:")
            }
            Label {
                text: "<a href=\"https://github.com/wowshok/qutim_2026_fork_xmpp_qt6\">github.com/wowshok/qutim_2026_fork_xmpp_qt6</a>"
                onLinkActivated: function(link) { Qt.openUrlExternally(link) }
            }
            Label {
                text: qsTr("Original project:")
            }
            Label {
                text: "<a href=\"https://github.com/euroelessar/qutim\">github.com/euroelessar/qutim</a>"
                onLinkActivated: function(link) { Qt.openUrlExternally(link) }
            }
        }

        TabBar {
            id: tabs
            Layout.fillWidth: true
            TabButton { text: qsTr("Developers") }
            TabButton {
                text: qsTr("Translators")
                visible: root.translators.length > 0
                width: visible ? implicitWidth : 0
            }
            TabButton { text: qsTr("License") }
        }

        StackLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            currentIndex: tabs.currentIndex

            InfoText { text: info.developers }
            InfoText { text: root.translators }
            InfoText { text: info.license }
        }

        DialogButtonBox {
            Layout.fillWidth: true
            standardButtons: DialogButtonBox.Ok
            onAccepted: root.close()
        }
    }
}
