# qutim_add_plugin(<name> DIR <plugin dir> [SOURCE_DIR <dir>] [LINK <libs>...]
#                  [DEFINES <defs>...] [INCLUDES <dirs>...] [EXCLUDE <regex>])
#
# Mirrors the old qbs Plugin rule: sources are globbed recursively from
# SOURCE_DIR (DIR/src by default), and if DIR contains a *.plugin.json the
# plugin class is generated from plugintemplate.cpp.
function(qutim_add_plugin name)
	cmake_parse_arguments(P "" "DIR;SOURCE_DIR;EXCLUDE" "LINK;DEFINES;INCLUDES" ${ARGN})
	if(NOT P_DIR)
		message(FATAL_ERROR "qutim_add_plugin(${name}): DIR is required")
	endif()
	get_filename_component(P_DIR "${P_DIR}" ABSOLUTE)
	if(NOT DEFINED P_SOURCE_DIR)
		set(P_SOURCE_DIR "${P_DIR}/src")
	elseif(NOT IS_ABSOLUTE "${P_SOURCE_DIR}")
		set(P_SOURCE_DIR "${P_DIR}/${P_SOURCE_DIR}")
	endif()

	file(GLOB_RECURSE sources CONFIGURE_DEPENDS
		"${P_SOURCE_DIR}/*.cpp" "${P_SOURCE_DIR}/*.h" "${P_SOURCE_DIR}/*.ui" "${P_SOURCE_DIR}/*.c")
	if(P_EXCLUDE)
		list(FILTER sources EXCLUDE REGEX "${P_EXCLUDE}")
	endif()

	file(GLOB json "${P_DIR}/*.plugin.json")
	if(json)
		list(GET json 0 json)
		file(READ "${json}" data)
		set(productName "${name}")
		foreach(key pluginIcon pluginName pluginDescription extensionHeader extensionClass)
			string(JSON value ERROR_VARIABLE err GET "${data}" ${key})
			if(err)
				set(value "")
			endif()
			set(${key} "${value}")
		endforeach()
		set(extensionHeader "${P_DIR}/${extensionHeader}")
		set(includeQmlTypes "")
		set(registerQmlTypes "")
		string(JSON qmlCount ERROR_VARIABLE err LENGTH "${data}" qmlTypes)
		if(NOT err AND qmlCount GREATER 0)
			math(EXPR last "${qmlCount} - 1")
			foreach(i RANGE ${last})
				string(JSON header MEMBER "${data}" qmlTypes ${i})
				string(JSON code GET "${data}" qmlTypes "${header}")
				string(APPEND includeQmlTypes "#include \"${P_DIR}/${header}\"\n")
				string(APPEND registerQmlTypes "${code}\n")
			endforeach()
		endif()
		file(READ "${QUTIM_PLUGIN_TEMPLATE}" template)
		string(CONFIGURE "${template}" generated)
		set(genfile "${CMAKE_CURRENT_BINARY_DIR}/${name}/${name}genplugin.cpp")
		file(CONFIGURE OUTPUT "${genfile}" CONTENT "${generated}")
		list(APPEND sources "${genfile}")
	endif()

	add_library(${name} MODULE ${sources})
	set_target_properties(${name} PROPERTIES
		PREFIX "lib"
		LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${QUTIM_PLUGIN_DIR}"
		CXX_VISIBILITY_PRESET hidden
		BUILD_RPATH "$ORIGIN/../.."
		INSTALL_RPATH "$ORIGIN/../.."
	)
	string(MD5 hash "${name}")
	string(SUBSTRING "${hash}" 0 16 pluginId)
	target_compile_definitions(${name} PRIVATE
		QUTIM_PLUGIN_ID=${pluginId}
		QUTIM_PLUGIN_NAME="${name}"
		${P_DEFINES}
	)
	target_include_directories(${name} PRIVATE "${P_SOURCE_DIR}" ${P_INCLUDES})
	target_link_libraries(${name} PRIVATE libqutim ${P_LINK})
	# Catch missing symbols at link time instead of at plugin load
	target_link_options(${name} PRIVATE "LINKER:--no-undefined")
	install(TARGETS ${name} LIBRARY DESTINATION ${QUTIM_PLUGIN_DIR})

	# QML pages and dialogs of the plugin go to <share>/qml, like the old qbs did
	if(IS_DIRECTORY "${P_DIR}/qml")
		# Copied on every build, so edited QML reaches the build tree without re-running CMake
		add_custom_target(${name}_qml ALL
			COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
				"${P_DIR}/qml" "${CMAKE_BINARY_DIR}/${QUTIM_SHARE_DIR}/qml"
			COMMENT "Copying QML of ${name}")
		install(DIRECTORY "${P_DIR}/qml/" DESTINATION "${QUTIM_SHARE_DIR}/qml")
	endif()
endfunction()

# Shared library used by a group of plugins; its public headers are
# included as <qutim/${subdir}/header.h>
function(qutim_add_framework name)
	cmake_parse_arguments(P "" "DIR;HEADER_DIR;DEFINE" "" ${ARGN})
	get_filename_component(P_DIR "${P_DIR}" ABSOLUTE)
	file(GLOB sources CONFIGURE_DEPENDS "${P_DIR}/*.cpp" "${P_DIR}/*.h" "${P_DIR}/*.ui")
	add_library(${name} SHARED ${sources})
	set_target_properties(${name} PROPERTIES
		PREFIX "lib"
		CXX_VISIBILITY_PRESET hidden
		VERSION ${PROJECT_VERSION}
		SOVERSION ${PROJECT_VERSION_MAJOR}
	)
	target_compile_definitions(${name} PRIVATE ${P_DEFINE} QUTIM_PLUGIN_NAME="${name}")

	set(incdir "${CMAKE_CURRENT_BINARY_DIR}/${name}-include")
	file(GLOB headers "${P_DIR}/*.h")
	foreach(header ${headers})
		get_filename_component(fn "${header}" NAME)
		if(NOT fn MATCHES "_p\\.h$")
			file(CONFIGURE OUTPUT "${incdir}/qutim/${P_HEADER_DIR}/${fn}"
				CONTENT "#include \"${header}\"\n")
		endif()
	endforeach()
	target_include_directories(${name} PUBLIC "${incdir}" PRIVATE "${P_DIR}")
	target_link_libraries(${name} PUBLIC libqutim)
	target_link_options(${name} PRIVATE "LINKER:--no-undefined")
	install(TARGETS ${name} LIBRARY DESTINATION ${QUTIM_LIB_DIR})
endfunction()

set(QUTIM_PLUGIN_TEMPLATE "${CMAKE_CURRENT_SOURCE_DIR}/plugintemplate.cpp")
set(G "${CMAKE_CURRENT_SOURCE_DIR}/generic")

find_package(Qt6 REQUIRED COMPONENTS Xml DBus)
find_package(PkgConfig REQUIRED)
find_package(Qt6Keychain REQUIRED)

# Minimal XMPP client: without these qutim crashes or cannot create an account
foreach(p jsonconfig nocryptoservice qticons password authdialog dataformsbackend
          simplerosterstorage comparators xsettingsdialog accountcreator)
	qutim_add_plugin(${p} DIR "${G}/${p}" SOURCE_DIR .)
endforeach()
qutim_add_plugin(keychain DIR "${G}/keychain" LINK qt6keychain)
qutim_add_plugin(contactmodel DIR "${G}/contactmodel")

qutim_add_framework(qutim-simplecontactlist DIR "${G}/simplecontactlist/lib"
	HEADER_DIR simplecontactlist DEFINE SIMPLECONTACTLIST_LIBRARY)
qutim_add_plugin(simplecontactlist DIR "${G}/simplecontactlist" SOURCE_DIR contactlist
	LINK qutim-simplecontactlist)
qutim_add_plugin(simplecontactdelegate DIR "${G}/simplecontactlist/simplecontactdelegate"
	SOURCE_DIR . LINK qutim-simplecontactlist)
qutim_add_plugin(simplecontactlistwidget DIR "${G}/simplecontactlist/widgets/simplecontactlistwidget"
	SOURCE_DIR . LINK qutim-simplecontactlist)
qutim_add_plugin(torycontactlistwidget DIR "${G}/simplecontactlist/widgets/torycontactlistwidget"
	SOURCE_DIR . LINK qutim-simplecontactlist)

qutim_add_framework(qutim-adiumchat DIR "${G}/adiumchat/lib"
	HEADER_DIR adiumchat DEFINE ADIUMCHAT_LIBRARY)
qutim_add_plugin(adiumchat DIR "${G}/adiumchat" SOURCE_DIR chatlayer LINK qutim-adiumchat)
qutim_add_plugin(textchat DIR "${G}/adiumchat/chatfactories/textchat" SOURCE_DIR . LINK qutim-adiumchat)
qutim_add_plugin(tabbedchatform DIR "${G}/adiumchat/chatforms/tabbedchatform" SOURCE_DIR . LINK qutim-adiumchat)
qutim_add_plugin(stackedchatform DIR "${G}/adiumchat/chatforms/stackedchatform" SOURCE_DIR . LINK qutim-adiumchat)

# Nice to have
foreach(p jsonhistory localization chatnotificationsbackend addcontactdlg contactinfo
          joingroupchatdlg metacontacts)
	qutim_add_plugin(${p} DIR "${G}/${p}" SOURCE_DIR .)
endforeach()
qutim_add_plugin(kineticpopups DIR "${G}/kineticpopups" EXCLUDE "/quickpopup/")
qutim_add_plugin(trayicon DIR "${G}/trayicon")

# Standard desktop features: contact actions, dialog buttons, protocol icons,
# emoticons, sound theme, settings pages, file transfer, history viewer, search
foreach(p simpleactions simpleactionbox adiumsrvicons emoticonssettings
          notificationssettings notificationfilter proxysettings shortcutsettings filetransfer
          filetransfersettings servicechooser simpleabout joinchatdialog soundthemeselector)
	qutim_add_plugin(${p} DIR "${G}/${p}" SOURCE_DIR .)
endforeach()
foreach(p kopeteemoticonsbackend oldsoundtheme)
	qutim_add_plugin(${p} DIR "${G}/${p}" SOURCE_DIR . LINK Qt6::Xml)
endforeach()
qutim_add_plugin(searchdialog DIR "${G}/searchdialog" SOURCE_DIR .)
qutim_add_plugin(histview DIR "${G}/histview")
qutim_add_plugin(unreadmessageskeeper DIR "${G}/unreadmessageskeeper")

# Idle time for auto-away (X11 screensaver extension or D-Bus on Wayland)
find_package(Qt6 REQUIRED COMPONENTS DBus)
find_package(PkgConfig REQUIRED)
pkg_check_modules(XCB_SCREENSAVER REQUIRED IMPORTED_TARGET xcb xcb-screensaver)
qutim_add_plugin(idledetector DIR "${G}/idledetector" SOURCE_DIR .
	LINK Qt6::DBus PkgConfig::XCB_SCREENSAVER)

# Notification sounds through Qt Multimedia (replaces the Phonon and SDL backends)
find_package(Qt6 REQUIRED COMPONENTS Multimedia)
qutim_add_plugin(qtmultimediasound DIR "${G}/qtmultimediasound" SOURCE_DIR . LINK Qt6::Multimedia)

# Optional chat features
foreach(p highlighter antispam autoreply urlpreview massmessaging birthdayreminder clconf logger
          emoedit floaties antiboss formula blogimprover autopaster)
	qutim_add_plugin(${p} DIR "${G}/${p}")
endforeach()
foreach(p idlestatuschanger chatspellchecker sessionhelper)
	qutim_add_plugin(${p} DIR "${G}/${p}" SOURCE_DIR .)
endforeach()
qutim_add_plugin(oldcontactdelegate DIR "${G}/oldcontactdelegate" SOURCE_DIR . LINK qutim-simplecontactlist Qt6::Xml)
qutim_add_plugin(dbusapi DIR "${G}/dbusapi" LINK Qt6::DBus)
# Restores account status on start and reconnects after network or connection loss
qutim_add_plugin(bearermanager DIR "${G}/bearermanager")
qutim_add_plugin(nowplaying DIR "${G}/nowplaying" SOURCE_DIR .
	LINK Qt6::DBus INCLUDES "${G}/nowplaying/include" "${G}/nowplaying/src")

# Spell checking backends
pkg_check_modules(HUNSPELL REQUIRED IMPORTED_TARGET hunspell)
qutim_add_plugin(hunspeller DIR "${G}/hunspeller" LINK PkgConfig::HUNSPELL)
find_library(ASPELL_LIBRARY aspell REQUIRED)
qutim_add_plugin(aspeller DIR "${G}/aspeller" LINK ${ASPELL_LIBRARY})

# Encryption: OTR (libotr 4) and qutim's own AES scheme (QCA)
pkg_check_modules(LIBOTR REQUIRED IMPORTED_TARGET libotr>=4.0.0)
qutim_add_plugin(offtherecord DIR "${G}/offtherecord" LINK PkgConfig::LIBOTR)
find_package(Qca-qt6 REQUIRED)
qutim_add_plugin(aescrypto DIR "${G}/aescrypto" LINK qca-qt6)

# Linux desktop integration: freedesktop notifications and TCP keepalive
set(I "${CMAKE_CURRENT_SOURCE_DIR}/integrations")
qutim_add_plugin(dbusnotifications DIR "${I}/dbusnotifications" LINK Qt6::DBus)
qutim_add_plugin(linuxintegration DIR "${I}/linuxintegration" SOURCE_DIR .)

# XMPP, Jingle (VoIP) stays disabled
qutim_add_plugin(jabber DIR "${CMAKE_CURRENT_SOURCE_DIR}/protocols/jabber" LINK jreen)
