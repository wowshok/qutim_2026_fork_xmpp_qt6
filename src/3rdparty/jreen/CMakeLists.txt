cmake_minimum_required(VERSION 3.19)
project(Jreen VERSION 1.3.0 LANGUAGES CXX)

include(GNUInstallDirs)

find_package(Qt6 REQUIRED COMPONENTS Core Network)
find_package(PkgConfig REQUIRED)
pkg_check_modules(GSASL REQUIRED IMPORTED_TARGET libgsasl)
pkg_check_modules(SPEEX IMPORTED_TARGET speex)
find_package(ZLIB REQUIRED)

file(GLOB_RECURSE SRC CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
# jdns-based resolver is only used by the Qt 4 build, Qt 6 has QDnsLookup
list(REMOVE_ITEM SRC "${CMAKE_CURRENT_SOURCE_DIR}/src/sjdns.cpp")

qt_add_library(jreen SHARED ${SRC})

set_target_properties(jreen PROPERTIES
	VERSION ${PROJECT_VERSION}
	SOVERSION ${PROJECT_VERSION_MAJOR}
	OUTPUT_NAME jreen-qt6
	AUTOMOC ON
	CXX_VISIBILITY_PRESET hidden
	DEFINE_SYMBOL J_BUILD_LIBRARY
)

target_include_directories(jreen PRIVATE
	"${CMAKE_CURRENT_SOURCE_DIR}"
	"${CMAKE_CURRENT_SOURCE_DIR}/src"
	"${CMAKE_CURRENT_SOURCE_DIR}/3rdparty"
)

target_link_libraries(jreen
	PUBLIC Qt6::Core Qt6::Network
	PRIVATE PkgConfig::GSASL ZLIB::ZLIB
)

if(SPEEX_FOUND)
	target_link_libraries(jreen PRIVATE PkgConfig::SPEEX)
	target_compile_definitions(jreen PRIVATE JREEN_HAVE_SPEEX=1)
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
	target_compile_options(jreen PRIVATE -Wall -Wextra)
endif()

# Public headers are included as <jreen/foo.h>; generate forwarding headers
set(JREEN_INCLUDE_DIR "${CMAKE_CURRENT_BINARY_DIR}/include")
file(GLOB_RECURSE JREEN_HEADERS RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}/src"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/*.h")
foreach(header ${JREEN_HEADERS})
	if(NOT header MATCHES ".*_p\\.h")
		file(CONFIGURE OUTPUT "${JREEN_INCLUDE_DIR}/jreen/${header}"
			CONTENT "#include \"${CMAKE_CURRENT_SOURCE_DIR}/src/${header}\"\n")
	endif()
endforeach()

target_include_directories(jreen PUBLIC "$<BUILD_INTERFACE:${JREEN_INCLUDE_DIR}>")

install(TARGETS jreen
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
