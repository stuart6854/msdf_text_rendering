file(GLOB_RECURSE APP_SOURCES CONFIGURE_DEPENDS src/*.cpp)
add_executable(app ${APP_SOURCES})
target_include_directories(app PRIVATE include)
target_link_libraries(app PRIVATE freetype msdfgen msdf-atlas-gen glad glfw glm)

set_target_properties(app PROPERTIES CXX_STANDARD 20)
