cmake_minimum_required(VERSION 3.8)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
    cmake_policy(SET CMP0141 NEW)
    set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif ()

project("msdf_text_rendering")

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

add_subdirectory(app)