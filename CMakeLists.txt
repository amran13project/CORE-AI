cmake_minimum_required(VERSION 3.20)
project(COREAI VERSION 0.3.2 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(COREAI_BUILD_TESTS "Build tests" ON)
option(COREAI_BUILD_NATIVE_GUI "Build native Windows GUI" ON)

add_library(coreai STATIC
    src/core/RuntimePaths.cpp
    src/core/UserStorage.cpp
    src/core/Logger.cpp
    src/core/Config.cpp
    src/core/Database.cpp
    src/core/Capabilities.cpp
    src/providers/OllamaProvider.cpp
    src/services/CoreApp.cpp
    src/services/LocalApiServer.cpp
    src/services/LocalFeatures.cpp
)

target_include_directories(coreai PUBLIC include src)

if(WIN32)
    target_link_libraries(coreai PUBLIC ws2_32 user32 shell32 advapi32)
else()
    target_link_libraries(coreai PUBLIC dl pthread)
endif()

add_executable(core-ai src/main.cpp)
target_link_libraries(core-ai PRIVATE coreai)

if(WIN32 AND COREAI_BUILD_NATIVE_GUI)
    target_sources(core-ai PRIVATE src/gui/Win32Gui.cpp)
    target_compile_definitions(core-ai PRIVATE COREAI_HAS_NATIVE_GUI=1)
endif()

if(COREAI_BUILD_TESTS)
    enable_testing()
    add_executable(core-ai-tests tests/core_tests.cpp)
    target_link_libraries(core-ai-tests PRIVATE coreai)
    add_test(NAME COREAI.UnitIntegration COMMAND core-ai-tests)
endif()

install(TARGETS core-ai RUNTIME DESTINATION bin)
install(DIRECTORY web/ DESTINATION web)
install(DIRECTORY docs/ DESTINATION docs)
install(FILES README.md BUILD.md SECURITY.md CONFIGURATION.md DESTINATION .)

set(CPACK_PACKAGE_NAME "CORE-AI")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_GENERATOR ZIP)
include(CPack)
