# QML module org.qutim: C++ types (Config, Service, Action, ...) plus QML components,
# loaded by the engine from <share>/imports (see DeclarativeView::globalEngine)
set(QUTIM_QML_MODULE_DIR "${CMAKE_BINARY_DIR}/${QUTIM_SHARE_DIR}/imports/org/qutim")

file(GLOB QUTIM_QML_SRC CONFIGURE_DEPENDS src/*.cpp src/*.h)
add_library(qutimplugin MODULE ${QUTIM_QML_SRC})
set_target_properties(qutimplugin PROPERTIES
	PREFIX "lib"
	LIBRARY_OUTPUT_DIRECTORY "${QUTIM_QML_MODULE_DIR}"
	CXX_VISIBILITY_PRESET hidden
	BUILD_RPATH "$ORIGIN/../../../../../../${QUTIM_LIB_DIR}"
	INSTALL_RPATH "$ORIGIN/../../../../../../${QUTIM_LIB_DIR}"
)
target_compile_definitions(qutimplugin PRIVATE QUTIM_PLUGIN_NAME="qutimplugin")
target_link_libraries(qutimplugin PRIVATE libqutim Qt6::Qml Qt6::Quick)
target_link_options(qutimplugin PRIVATE "LINKER:--no-undefined")

file(GLOB QUTIM_QML_FILES CONFIGURE_DEPENDS
	"${CMAKE_CURRENT_SOURCE_DIR}/imports/org/qutim/*.qml"
	"${CMAKE_CURRENT_SOURCE_DIR}/imports/org/qutim/qmldir")
add_custom_target(qutimplugin_qml ALL
	COMMAND ${CMAKE_COMMAND} -E copy_if_different ${QUTIM_QML_FILES} "${QUTIM_QML_MODULE_DIR}"
	DEPENDS qutimplugin
	COMMENT "Copying org.qutim QML files")

install(TARGETS qutimplugin LIBRARY DESTINATION "${QUTIM_SHARE_DIR}/imports/org/qutim")
install(FILES ${QUTIM_QML_FILES} DESTINATION "${QUTIM_SHARE_DIR}/imports/org/qutim")
