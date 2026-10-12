# Debian package: cmake --build build --target package (or cpack in the build dir).
# Configure the build with -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_BUILD_TYPE=Release.
set(QUTIM_PACKAGE_RELEASE "" CACHE STRING
	"Suffix of the package version, for example ~2026.10.12 (Debian sorts ~ before release)")

set(CPACK_GENERATOR "DEB")
set(CPACK_PACKAGE_NAME "qutim")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}${QUTIM_PACKAGE_RELEASE}")
set(CPACK_PACKAGE_VENDOR "qutIM 2026 fork")
set(CPACK_PACKAGE_CONTACT "wowshok <mrneefu@gmail.com>")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "modular Jabber/XMPP instant messenger (Qt 6)")
set(CPACK_DEBIAN_PACKAGE_DESCRIPTION "modular Jabber/XMPP instant messenger (Qt 6)
 qutIM is a plugin based instant messenger. This is the 2026 fork, ported to
 Qt 6 and limited to XMPP: roster groups, metacontacts, tabbed chats with
 Adium message styles (Qt WebEngine) or a light text view, group chats with
 bookmarks, OTR, delivery receipts, system keychain, desktop notifications,
 tray icon, idle detection on X11 and Wayland, spell checking.")
set(CPACK_DEBIAN_PACKAGE_SECTION "net")
set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
set(CPACK_DEBIAN_PACKAGE_HOMEPAGE "https://github.com/wowshok/qutim_2026_fork_xmpp_qt6")
set(CPACK_DEBIAN_FILE_NAME DEB-DEFAULT)

# Linked libraries are found by dpkg-shlibdeps; the rest is loaded at run time
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
# dpkg-shlibdeps runs in the staging tree: our own libraries are there, not in the system
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS_PRIVATE_DIRS "${CMAKE_INSTALL_PREFIX}/${QUTIM_LIB_DIR}")
set(CPACK_DEBIAN_PACKAGE_DEPENDS
	"libqt6webenginecore6-bin, qt6-svg-plugins, qml6-module-qtquick, qml6-module-qtquick-controls, qml6-module-qtquick-templates, qml6-module-qtquick-layouts, qml6-module-qtquick-window, qml6-module-qtqml-workerscript")
set(CPACK_DEBIAN_PACKAGE_RECOMMENDS "qt6-wayland, qt6-image-formats-plugins")
set(CPACK_DEBIAN_PACKAGE_SUGGESTS "hunspell-ru | hunspell-dictionary, aspell-ru | aspell-dictionary")

set(CPACK_STRIP_FILES ON)
set(CPACK_DEBIAN_PACKAGE_CONTROL_STRICT_PERMISSION ON)

include(CPack)
