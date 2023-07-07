file(GLOB_RECURSE APP_SOURCES src/*.cpp)
add_executable(app ${APP_SOURCES})
target_include_directories(app PRIVATE include)
target_link_libraries(app PRIVATE freetype msdfgen msdf-atlas-gen)

set_target_properties(app PROPERTIES CXX_STANDARD 17)
