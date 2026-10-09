import QtQuick 2.15
import QtQuick.Controls 2.15 as Controls
import org.qutim 0.4

Controls.Menu {
    id: menu
    property alias controller: container.controller
    property var objects: []

    MenuContainer {
        id: container
        onActionAdded: function(index, action) {
            var item = action.separator
                    ? separatorComponent.createObject(null)
                    : menuItemComponent.createObject(null, { myAction: action });
            menu.insertItem(index, item);
            menu.objects.splice(index, 0, item);
        }
        onActionRemoved: function(index) {
            var item = menu.itemAt(index);
            menu.objects.splice(index, 1);
            menu.removeItem(item);
            item.destroy();
        }
    }

    Action {
        id: defaultAction
        text: '<NULL Action>'
    }

    Component {
        id: separatorComponent
        Controls.MenuSeparator {}
    }

    Component {
        id: menuItemComponent
        Controls.MenuItem {
            property Action myAction: null
            function tryAction() {
                return myAction !== null ? myAction : defaultAction;
            }
            checkable: tryAction().checkable
            checked: tryAction().checked
            enabled: tryAction().enabled
            icon.name: tryAction().iconName
            icon.source: tryAction().iconSource
            text: tryAction().text
            // Controls 2 menus do not collapse invisible items by themselves
            visible: tryAction().visible
            height: visible ? implicitHeight : 0
            onToggled: tryAction().checked = checked
            onTriggered: tryAction().trigger()
        }
    }
}
