# Developer tool, not installed: checks that QML files compile against org.qutim
add_executable(qmlcheck main.cpp)
target_link_libraries(qmlcheck PRIVATE Qt6::Gui Qt6::Qml)
