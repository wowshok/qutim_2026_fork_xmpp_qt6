import QtQuick 2.15
import QtQuick.Window 2.15
import org.qutim.kineticpopups 0.4

Window {
	id: window
    
    readonly property Item main: window.contentItem

	width: 300
    height: subject.height + body.height + actions.height + 5
	color: "transparent"

    property var notifies: {[]}
    property color textColor: "white"
    property int textStyle: Text.Sunken
    property color textStyleColor: "black"

    // Wayland refuses tool tip windows without a transient parent; there the
    // compositor places the window anyway, so use a plain frameless tool window
    flags: Qt.platform.pluginName === "wayland"
           ? (Qt.Tool | Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint | Qt.WindowDoesNotAcceptFocus)
           : Qt.ToolTip

	Image {
		id: image
        z: 5
        source: "images/qutim.svg"
		width: 30
		fillMode: Image.PreserveAspectFit

		anchors {
			top: main.top
			left: main.left
			topMargin: 15
			leftMargin: 5
		}


	}

	Text {
		id: subject
		textFormat: Text.PlainText
        z: 5
		//text: "Title"

		anchors {
			top: main.top
			left: image.right
			right: main.right
			topMargin: 5
			leftMargin: 5
			rightMargin: 5
		}

		elide: Text.ElideMiddle
		color: window.textColor
		font.bold: true
		style: window.textStyle
		styleColor: window.textStyleColor
		font.pixelSize: body.font.pixelSize + 2
	}

	Text {
		id: body
		// Only our own <br /> survives escapeHtml()
		textFormat: Text.StyledText
		wrapMode: Text.Wrap
        z: 5
		//text: "Body Вот тестирую текст, много разного текста, он должен нормально отображаться"

		anchors {
			top: subject.bottom
			left: subject.left
			right: subject.right
			//topMargin: -15
		}

		color: window.textColor
        style: window.textStyle
		styleColor: window.textStyleColor
	}

	ListView {
		id: actions

		z: 10
		height: 30
		orientation: ListView.Horizontal
		spacing: 10

		anchors {
			top: body.bottom
			left: body.left
			right: body.right
		}
		delegate: Text {
			id: actionDelegate
			text: model.modelData.text

			color: window.textColor
            style: window.textStyle
            styleColor: window.textStyleColor
			font.italic: true
			font.underline: true

			MouseArea {
				id: actionArea
				anchors.fill: parent

				hoverEnabled: true
				acceptedButtons: Qt.LeftButton

				onClicked: function(mouse) {
					model.modelData.trigger();
					mouse.accepted = true;
				}
			}

			states: [
				State {
					name: "hovered"
					when: actionArea.containsMouse
					PropertyChanges {
						target: actionDelegate
						font.underline: false
					}
				},
				State {
					name: "leaved"
					when: !actionArea.containsMouse
					PropertyChanges {
						target: actionDelegate
						font.underline: true
					}
				},
				State {
					name: "pressed"
					when: actionArea.pressed
					PropertyChanges {
						target: actionDelegate
						font.underline: false
						font.bold: true
					}
				}
			]
		}
	}

	MouseArea {
		id: acceptIgnoreArea
		anchors.fill: main
		acceptedButtons: Qt.LeftButton | Qt.RightButton
		onClicked: function(mouse) {
            for (var i = 0; i < window.notifies.length; ++i) {
                var notify = window.notifies[i];
                if (notify && mouse.button === Qt.RightButton)
                    notify.ignore();
                else if (notify)
                    notify.accept();
            }
		}
	}

    // Notification texts come from contacts: show them as plain text, never as
    // markup (an <img> tag there would make the popup load a remote image)
    function escapeHtml(text) {
        return String(text).replace(/&/g, "&amp;").replace(/</g, "&lt;")
                           .replace(/>/g, "&gt;").replace(/"/g, "&quot;");
    }

    property var plainLines: []

    function addNotify(notify) {
        var trimLength = 80;

        window.notifies.push(notify);

        var lines = window.plainLines.slice();
        lines.push(notify.text);
        var str = lines.join("\n");
        if (str.length > trimLength)
            str = str.substring(0, trimLength - 3) + "...";
        window.plainLines = lines;
        body.text = escapeHtml(str).replace(/\n/g, "<br />");

        subject.text = notify.title;
        actions.model = notify.actions;

        if (notify.avatar)
            image.source = notify.avatar;
    }

    function removeNotify(notify) {
        for (var i = 0; i < window.notifies.length; ++i) {
            var windowNotify = window.notifies[i];
            if (windowNotify === notify) {
                window.notifies.splice(i, 1);
                return true;
            }
        }
        return false;
    }
    
    function removeAll() {
        var notifies = window.notifies;
        window.notifies = [];

        for (var i = 0; i < notifies.length; ++i) {
            var notify = notifies[i];
            notify.remove();
        }
    }
}
