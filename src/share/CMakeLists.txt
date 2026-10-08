# Runtime data: copied into the build tree so qutim runs from it,
# and installed to the same relative location.
set(SHARE_OUT "${CMAKE_BINARY_DIR}/${QUTIM_SHARE_DIR}")
set(ART "${CMAKE_SOURCE_DIR}/artwork")
set(PLUGINS "${CMAKE_SOURCE_DIR}/src/plugins/generic")

# <source dir> <destination relative to the share dir>
set(QUTIM_SHARE_DIRS
	"${CMAKE_SOURCE_DIR}/config/generic|config"
	"${CMAKE_CURRENT_SOURCE_DIR}/qutim/ca-certs|ca-certs"
	"${ART}/sounds|sounds"
	"${ART}/icons/qutim-default|icons/qutim-default"
	"${ART}/icons/user-status|icons/qutim-default/user-status"
	"${ART}/icons/humanity/hicolor|icons/hicolor"
	"${ART}/icons/clients/hicolor/16x16/apps|icons/hicolor/16x16/apps"
	"${ART}/icons/user-status/hicolor/scalable/status|icons/hicolor/scalable/status"
	"${ART}/icons/tray/ubuntu-mono-light|icons/ubuntu-mono-light/scalable/status"
	"${ART}/icons/tray/ubuntu-mono-dark|icons/ubuntu-mono-dark/scalable/status"
	"${PLUGINS}/kineticpopups/src/quickpopup|quickpopup"
	"${PLUGINS}/xsettingsdialog/qml|qml"
	"${PLUGINS}/trayicon/qml|qml"
)
# qticons' QML settings page uses QtQuick.Controls 1, removed in Qt 6

foreach(entry ${QUTIM_SHARE_DIRS})
	string(REPLACE "|" ";" entry "${entry}")
	list(GET entry 0 src)
	list(GET entry 1 dst)
	if(NOT EXISTS "${src}")
		message(WARNING "Missing share data: ${src}")
		continue()
	endif()
	file(COPY "${src}/" DESTINATION "${SHARE_OUT}/${dst}" PATTERN "CMakeLists.txt" EXCLUDE)
	install(DIRECTORY "${src}/" DESTINATION "${QUTIM_SHARE_DIR}/${dst}" PATTERN "CMakeLists.txt" EXCLUDE)
endforeach()

install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/applications/" DESTINATION share/applications)
