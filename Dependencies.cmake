include(cmake/CPM.cmake)

# freetype
add_library(freetype STATIC IMPORTED)
set_target_properties(freetype PROPERTIES
        IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/libs/lib/freetype.lib
        )
target_include_directories(freetype SYSTEM INTERFACE libs/include/freetype)

# msdfgen
add_library(msdfgen STATIC IMPORTED)
set_target_properties(msdfgen PROPERTIES
        IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/libs/lib/msdfgen.lib
        )
target_include_directories(msdfgen SYSTEM INTERFACE libs/include/msdfgen)

# msdf-atlas-gen
add_library(msdf-atlas-gen STATIC IMPORTED)
set_target_properties(msdf-atlas-gen PROPERTIES
        IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/libs/lib/msdf-atlas-gen.lib
        )
target_include_directories(msdf-atlas-gen SYSTEM INTERFACE libs/include/msdf-atlas-gen)

CPMAddPackage(
        NAME glad
        GITHUB_REPOSITORY Dav1dde/glad
        VERSION 0.1.33
)

CPMAddPackage(
        NAME glm
        GITHUB_REPOSITORY g-truc/glm
        GIT_TAG 0.9.9.7
)

CPMAddPackage(
        NAME glfw
        GITHUB_REPOSITORY glfw/glfw
        GIT_TAG 3.3.2
        OPTIONS
        "GLFW_BUILD_TESTS Off"
        "GLFW_BUILD_EXAMPLES Off"
        "GLFW_BUILD_DOCS Off"
        "GLFW_INSTALL Off"
        "GLFW_USE_HYBRID_HPG On"
)

# put all external targets into a seperate folder to not pollute the project folder
set_target_properties(glad glad-generate-files glfw PROPERTIES FOLDER ExternalTargets)