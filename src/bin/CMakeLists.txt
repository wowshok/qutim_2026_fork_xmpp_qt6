file(GLOB QUTIM_BIN_SRC CONFIGURE_DEPENDS src/widgets/*.cpp src/widgets/*.h src/widgets/*.ui)

add_executable(qutim main.cpp qutim.qrc ${QUTIM_BIN_SRC})

target_compile_definitions(qutim PRIVATE QUTIM_PLUGIN_NAME="qutim")
target_include_directories(qutim PRIVATE src/widgets)
target_link_libraries(qutim PRIVATE libqutim)

set_target_properties(qutim PROPERTIES
	BUILD_RPATH "$ORIGIN/../${QUTIM_LIB_DIR}"
	INSTALL_RPATH "$ORIGIN/../${QUTIM_LIB_DIR}"
)

install(TARGETS qutim RUNTIME DESTINATION ${QUTIM_BIN_DIR})
