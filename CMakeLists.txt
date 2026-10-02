cmake_minimum_required(VERSION 3.20)
project(COREAI VERSION 0.2.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
option(CORE_BUILD_TESTS "Build tests" ON)
option(CORE_BUILD_SINGLE_FILE "Build single-file CORE-AI.cpp" OFF)

add_library(coreai STATIC
  src/core/CoreApp.cpp
  src/core/Config.cpp
  src/core/Logger.cpp
  src/core/EventBus.cpp
  src/ai/ModelRouter.cpp
  src/ai/OllamaProvider.cpp
  src/memory/MemoryStore.cpp
  src/tools/ToolRegistry.cpp
  src/tools/FileTool.cpp
  src/tools/ProcessTool.cpp
  src/agents/Planner.cpp
  src/agents/AgentManager.cpp
  src/security/PermissionManager.cpp
  src/audit/AuditLog.cpp
  src/verification/VerificationEngine.cpp
  src/projects/ProjectManager.cpp
  src/automation/WorkflowEngine.cpp
  src/plugins/PluginManager.cpp
  src/server/LocalApiServer.cpp
  src/core/CoreCAPI.cpp
  src/adapters/web/WebSearchAdapter.cpp
  src/adapters/maps/MapsAdapter.cpp
  src/adapters/vision/VisionAdapter.cpp
  src/adapters/image/ImageGenerationAdapter.cpp
  src/adapters/voice/VoiceAdapter.cpp
)
target_include_directories(coreai PUBLIC include src)
if(WIN32)
  target_link_libraries(coreai PUBLIC ws2_32 shell32 user32 advapi32)
else()
  target_link_libraries(coreai PUBLIC dl)
endif()
add_executable(core-ai src/main.cpp)
target_link_libraries(core-ai PRIVATE coreai)

if(CORE_BUILD_SINGLE_FILE)
  add_executable(core-ai-single CORE-AI.cpp)
  target_compile_definitions(core-ai-single PRIVATE CORE_SINGLE_FILE_BUILD=1)
  if(WIN32)
    target_link_libraries(core-ai-single PRIVATE winhttp shell32 advapi32 user32 gdi32)
  endif()
endif()

if(CORE_BUILD_TESTS)
  enable_testing()
  add_executable(core-ai-tests tests/core_tests.cpp)
  target_link_libraries(core-ai-tests PRIVATE coreai)
  add_test(NAME CoreAITests COMMAND core-ai-tests)
  add_executable(core-ai-adapter-tests tests/adapter_tests.cpp src/adapters/web/WebSearchAdapter.cpp src/adapters/maps/MapsAdapter.cpp)
  add_test(NAME CoreAIAdapterTests COMMAND core-ai-adapter-tests)
endif()
