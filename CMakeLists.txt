cmake_minimum_required(VERSION 3.20)
project(xmod_windows LANGUAGES C CXX)

# The Omni-Bot Windows DLL exposes a C++ interface, including GameEntity
# returns. MinGW's ABI is incompatible with its MSVC build on x86 AND x64.
if(NOT WIN32 OR NOT MSVC)
    message(FATAL_ERROR "This build requires MSVC on Windows. Use GNUmakefile on other platforms.")
endif()
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
find_package(Python3 REQUIRED COMPONENTS Interpreter)
file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/base")
execute_process(COMMAND "${Python3_EXECUTABLE}" project/info.py -h project/info.db
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
    OUTPUT_FILE "${CMAKE_BINARY_DIR}/base/project.h"
    COMMAND_ERROR_IS_FATAL ANY)
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS project/info.py project/info.db)

include_directories("${CMAKE_BINARY_DIR}" src pak src/sqlite3)
add_compile_definitions(_CRT_SECURE_NO_WARNINGS _CRT_NONSTDC_NO_WARNINGS)
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    add_compile_definitions(XMOD_WINDOWS64)
    set(module_arch x64)
else()
    add_compile_definitions(XMOD_WINDOWS)
    set(module_arch x86)
endif()

# Share the authoritative source lists with GNU make rather than maintaining
# another stale list in the legacy Visual Studio projects. Only the srcs
# section precedes MODULE.<name>.objs in each .defs file.
function(module_sources name output)
    file(READ "${CMAKE_SOURCE_DIR}/src/${name}.defs" definitions)
    string(TOUPPER "${name}" upper)
    string(FIND "${definitions}" "MODULE.${upper}.objs" end)
    if(end LESS 0)
        message(FATAL_ERROR "Missing source section in src/${name}.defs")
    endif()
    string(SUBSTRING "${definitions}" 0 ${end} definitions)
    string(REPLACE [[$(PROJECT.platspecific)]] "win32" definitions "${definitions}")
    string(REPLACE [[$(PROJECT/)]] "" definitions "${definitions}")
    string(REGEX MATCHALL "src/[A-Za-z0-9_./*+-]+\\.(cpp|c)" patterns "${definitions}")
    set(sources)
    foreach(pattern IN LISTS patterns)
        file(GLOB matches CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/${pattern}")
        list(APPEND sources ${matches})
    endforeach()
    list(REMOVE_DUPLICATES sources)
    set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "src/${name}.defs")
    set(${output} ${sources} PARENT_SCOPE)
endfunction()

module_sources(lua lua_sources)
# GNU make compiles Lua with g++; its consumers use C++ linkage as well.
set_source_files_properties(${lua_sources} PROPERTIES LANGUAGE CXX)
add_library(lua STATIC ${lua_sources})
module_sources(sqlite3 sqlite_sources)
add_library(sqlite3 STATIC ${sqlite_sources})
target_compile_definitions(sqlite3 PRIVATE SQLITE_THREADSAFE=0 SQLITE_OMIT_LOAD_EXTENSION)
module_sources(base base_sources)
add_library(base STATIC ${base_sources})
target_precompile_headers(base PRIVATE <base/public.h>)

# The standalone loader uses CRT formatting functions; q_shared.h deliberately
# poisons these names on MSVC, so do not inject the game PCH into this file.
set_source_files_properties(src/omnibot/common/BotLoadLibrary.cpp
    PROPERTIES SKIP_PRECOMPILE_HEADERS ON)

foreach(module game cgame ui)
    module_sources(${module} sources)
    set(compile_target ${module})
    if(module STREQUAL "game")
        # Reuse the production server objects in the authentication regression
        # host; only the engine syscall boundary is replaced by that test.
        add_library(game_objects OBJECT ${sources})
        add_library(game SHARED $<TARGET_OBJECTS:game_objects>)
        set(compile_target game_objects)
        set(module_define GAMEDLL)
        set(module_name qagame)
    elseif(module STREQUAL "cgame")
        set(module_define CGAMEDLL)
        set(module_name cgame)
    else()
        set(module_define UIDLL)
        set(module_name ui)
    endif()
    if(NOT module STREQUAL "game")
        add_library(${module} SHARED ${sources})
    endif()
    target_compile_definitions(${compile_target} PRIVATE ${module_define})
    target_precompile_headers(${compile_target} PRIVATE <bgame/impl.h>)
    target_link_libraries(${module} PRIVATE base lua sqlite3 ws2_32 iphlpapi advapi32)
    set_target_properties(${module} PROPERTIES
        OUTPUT_NAME "${module_name}_mp_${module_arch}"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${module}/$<CONFIG>")
endforeach()

include(CTest)
if(BUILD_TESTING)
    add_executable(auth_mapchange_test tests/auth_mapchange_test.cpp $<TARGET_OBJECTS:game_objects>)
    target_compile_definitions(auth_mapchange_test PRIVATE GAMEDLL)
    target_link_libraries(auth_mapchange_test PRIVATE base lua sqlite3 ws2_32 iphlpapi advapi32)
    add_test(NAME auth_mapchange COMMAND auth_mapchange_test)
    add_executable(windows_module_load tests/windows_module_load.cpp)
    add_dependencies(windows_module_load game cgame ui)
    add_test(NAME windows_module_load COMMAND windows_module_load
        "$<TARGET_FILE:game>" "$<TARGET_FILE:cgame>" "$<TARGET_FILE:ui>")
endif()
