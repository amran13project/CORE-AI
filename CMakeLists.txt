cmake_minimum_required(VERSION 3.20)
project(CoreAI VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(CORE_BUILD_TESTS "Build CORE AI tests" ON)
option(CORE_ENABLE_WIN32_GUI "Build the native Windows GUI" ON)

add_library(coreai_lib
    src/core/CoreApp.cpp
    src/core/Config.cpp
    src/core/Logger.cpp
    src/core/EventBus.cpp
    src/ai/OllamaProvider.cpp
    src/ai/ModelRouter.cpp
    src/memory/MemoryStore.cpp
    src/tools/FileTool.cpp
    src/tools/ProcessTool.cpp
    src/tools/ToolRegistry.cpp
    src/security/PermissionManager.cpp
    src/audit/AuditLog.cpp
    src/agents/Planner.cpp
    src/agents/AgentManager.cpp
    src/verification/VerificationEngine.cpp
    src/plugins/PluginManager.cpp
    src/projects/ProjectManager.cpp
    src/automation/WorkflowEngine.cpp
    src/ui/ConsoleUI.cpp
)

target_include_directories(coreai_lib PUBLIC src)
target_compile_definitions(coreai_lib PRIVATE CORE_VERSION=\"${PROJECT_VERSION}\")

if (WIN32)
    target_link_libraries(coreai_lib PUBLIC user32 gdi32 advapi32 shell32)
    if (CORE_ENABLE_WIN32_GUI)
        target_sources(coreai_lib PRIVATE src/ui/Win32UI.cpp)
        target_compile_definitions(coreai_lib PUBLIC CORE_HAS_WIN32_GUI=1)
    endif()
else()
    target_compile_definitions(coreai_lib PUBLIC CORE_HAS_WIN32_GUI=0)
endif()

add_executable(core-ai src/main.cpp)
target_link_libraries(core-ai PRIVATE coreai_lib)

if (WIN32 AND CORE_ENABLE_WIN32_GUI)
    set_target_properties(core-ai PROPERTIES WIN32_EXECUTABLE TRUE)
endif()

install(TARGETS core-ai RUNTIME DESTINATION bin)
install(DIRECTORY plugins/ DESTINATION share/core-ai/plugins)

if (CORE_BUILD_TESTS)
    enable_testing()
    add_executable(core-ai-tests
        tests/core_tests.cpp
    )
    target_link_libraries(core-ai-tests PRIVATE coreai_lib)
    add_test(NAME CoreAITests COMMAND core-ai-tests)
endif()
