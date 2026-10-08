file(GLOB_RECURSE LIBQUTIM_SRC CONFIGURE_DEPENDS qutim/*.cpp qutim/*.h qutim/*.ui)

set(THIRDPARTY_DIR "${CMAKE_SOURCE_DIR}/src/3rdparty")

add_library(libqutim SHARED
	${LIBQUTIM_SRC}
	${THIRDPARTY_DIR}/slidingstackedwidget/slidingstackedwidget.cpp
	${THIRDPARTY_DIR}/slidingstackedwidget/fingerswipegesture.cpp
	${THIRDPARTY_DIR}/flowlayout/flowlayout.cpp
)

set_target_properties(libqutim PROPERTIES
	PREFIX ""
	VERSION ${PROJECT_VERSION}
	SOVERSION ${PROJECT_VERSION_MAJOR}
)

set(QUTIM_VERSION_GENERATED_DIR "${CMAKE_CURRENT_BINARY_DIR}/include")
set(CMAKE_QUTIM_VERSION_STRING   "${QUTIM_VERSION_STRING}")
set(CMAKE_QUTIM_VERSION_MAJOR    "${PROJECT_VERSION_MAJOR}")
set(CMAKE_QUTIM_VERSION_MINOR    "${PROJECT_VERSION_MINOR}")
set(CMAKE_QUTIM_VERSION_SECMINOR "${PROJECT_VERSION_PATCH}")
set(CMAKE_QUTIM_VERSION_PATCH    "${PROJECT_VERSION_TWEAK}")
configure_file(qutim/version.h.cmake "${QUTIM_VERSION_GENERATED_DIR}/qutim/libqutim_version.h")

target_compile_definitions(libqutim
	PRIVATE
		LIBQUTIM_LIBRARY
		QUTIM_SHARE_DIR="${QUTIM_SHARE_DIR}"
		QUTIM_PLUGIN_NAME="libqutim"
	PUBLIC
		QUTIM_SINGLE_PROFILE
)

target_include_directories(libqutim
	PRIVATE
		qutim
		"${QUTIM_VERSION_GENERATED_DIR}/qutim"
	PUBLIC
		"${CMAKE_CURRENT_SOURCE_DIR}"
		"${QUTIM_VERSION_GENERATED_DIR}"
		"${QUTIM_VERSION_GENERATED_DIR}/qutim"
		"${THIRDPARTY_DIR}"
		"${THIRDPARTY_DIR}/slidingstackedwidget"
		"${THIRDPARTY_DIR}/flowlayout"
)

target_link_libraries(libqutim
	PUBLIC Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Network Qt6::Qml Qt6::Quick
	PRIVATE k8json qtsolutions
)

install(TARGETS libqutim LIBRARY DESTINATION ${QUTIM_LIB_DIR})
