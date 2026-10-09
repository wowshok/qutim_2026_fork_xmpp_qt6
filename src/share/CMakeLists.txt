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
	"${ART}/icons/oxygen|icons/oxygen"
	"${ART}/icons/humanity/hicolor|icons/hicolor"
	"${ART}/icons/clients/hicolor/16x16/apps|icons/hicolor/16x16/apps"
	# Application and tray status icons (qutim, qutim-online, qutim-offline, ...)
	"${ART}/icons/qutim/icons/hicolor|icons/hicolor"
	# Mood icons, laid out as a hicolor theme
	"${ART}/icons/user-status/hicolor|icons/hicolor"
	"${ART}/icons/tray/ubuntu-mono-light|icons/ubuntu-mono-light/scalable/status"
	"${ART}/icons/tray/ubuntu-mono-dark|icons/ubuntu-mono-dark/scalable/status"
	"${PLUGINS}/kineticpopups/src/quickpopup|quickpopup"
)

set(index 0)
foreach(entry ${QUTIM_SHARE_DIRS})
	math(EXPR index "${index} + 1")
	string(REPLACE "|" ";" entry "${entry}")
	list(GET entry 0 src)
	list(GET entry 1 dst)
	if(NOT EXISTS "${src}")
		message(WARNING "Missing share data: ${src}")
		continue()
	endif()
	add_custom_target(share_data_${index} ALL
		COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different "${src}" "${SHARE_OUT}/${dst}"
		COMMAND ${CMAKE_COMMAND} -E rm -f "${SHARE_OUT}/${dst}/CMakeLists.txt"
		COMMENT "Copying ${dst}")
	# Several sources share icons/hicolor and each brings its own index.theme:
	# copy in list order so the last (and most complete) one wins every time
	if(previous_share_target)
		add_dependencies(share_data_${index} ${previous_share_target})
	endif()
	set(previous_share_target share_data_${index})
	install(DIRECTORY "${src}/" DESTINATION "${QUTIM_SHARE_DIR}/${dst}" PATTERN "CMakeLists.txt" EXCLUDE)
endforeach()

install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/applications/" DESTINATION share/applications)
# The desktop file refers to the application icon by name, so it goes to the system theme too
install(DIRECTORY "${ART}/icons/qutim/icons/" DESTINATION share/icons)
install(DIRECTORY "${ART}/icons/qutim/pixmaps/" DESTINATION share/pixmaps)
