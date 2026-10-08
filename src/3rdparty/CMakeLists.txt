add_subdirectory(jreen)

add_library(k8json STATIC k8json/k8json.cpp)
target_compile_definitions(k8json PUBLIC K8JSON_INCLUDE_GENERATOR K8JSON_INCLUDE_COMPLEX_GENERATOR)
target_link_libraries(k8json PUBLIC Qt6::Core)

add_library(qtsolutions STATIC
	qtsolutions/qtlocalpeer.cpp
	qtsolutions/qtlockedfile.cpp
	qtsolutions/qtlockedfile_unix.cpp
)
target_link_libraries(qtsolutions PUBLIC Qt6::Core Qt6::Network)

set_target_properties(k8json qtsolutions PROPERTIES POSITION_INDEPENDENT_CODE ON)
