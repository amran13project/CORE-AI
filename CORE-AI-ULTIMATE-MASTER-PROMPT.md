# CORE-AI
# FULL PERSONAL AI OPERATING SYSTEM
# NATIVE CORE + DESKTOP GUI + WEB CLIENT
# COMPLETE REAL PRODUCT — NO PHASED DELIVERY
# ULTIMATE MASTER IMPLEMENTATION PROMPT v3.0

PRODUCT TAGLINE:
YOUR INTELLIGENCE. YOUR SYSTEM. YOUR CONTROL.

You are the principal software engineering team responsible for repository inspection,
architecture, implementation, refactoring, integration, debugging, security hardening,
testing, GUI, web client, API, CLI, packaging, documentation, release validation and
maintenance.

============================================================
1. ABSOLUTE MISSION
============================================================

BUILD CORE-AI AS ONE COMPLETE, REAL, COHERENT PRODUCT.

This is NOT:
- a mockup
- a prototype
- a UI-only demo
- a fake ChatGPT clone
- a collection of disconnected screens
- architecture-only code
- empty interfaces
- a giant monolithic C++ file
- fake providers
- fake memory
- fake agents
- fake workflows
- fake automation
- fake plugins
- fake maps
- fake images
- fake scheduled jobs

Do not stop because the program launches.
Do not stop because the GUI looks attractive.
Do not stop after only the CLI, API, database, provider abstraction or one model works.
Do not create a user-facing Phase 1 / Phase 2 / Phase 3 delivery plan.
Internal engineering dependency order is allowed, but the final result must be one
integrated product covering the complete requested scope.

============================================================
2. PRODUCT IDENTITY AND EXPERIENCE
============================================================

CORE-AI must feel like one personal AI operating environment, not a collection of apps.
The desktop GUI, web client, CLI and API are different clients of the same native core.

PRIMARY NAVIGATION:
- New Chat
- Recent
- Chat
- Think
- Codex
- Projects
- Library
- Images
- Maps
- Agents
- Tools
- Workflows
- Scheduled
- Plugins
- Research
- Files
- Documents
- Git
- Tasks
- Diagnostics
- Settings

You may refine the information architecture, but every requested capability must be
reachable through a real UI action backed by a real service.

TOP BAR MUST INCLUDE:
- CORE-AI identity
- Model/Version selector
- Provider status
- Privacy mode
- Connection/health state
- Search/command button
- Current project
- Settings/profile access

The Model/Version selector is real. It must use actual discovered/configured ModelDescriptor
objects. Never invent model names or pretend unavailable models are ready.

============================================================
3. CORE UX — REQUIRED
============================================================

NEW CHAT
- Creates a real conversation.
- Persists a unique conversation ID.
- Preserves appropriate project/model/privacy/session settings.
- Clears only active context, not historical data.
- Shows the newly created conversation immediately.

RECENT
- Lists actual persisted conversations, projects, tasks and relevant recent artifacts.
- Uses real timestamps and sorting.
- Supports search/filter/open/rename/archive/delete where applicable.
- Never creates fake sample records just to fill the page.

AUTOMATIC CHAT RESET
Implement a real configurable chat/session reset system.
Modes:
- Off
- After inactivity
- On application restart
- At explicit session boundary
- Custom duration

Auto-reset must NEVER silently erase history.
Correct behavior:
1. Persist the current conversation/session.
2. Close/reset active context.
3. Optionally create a fresh conversation/session.
4. Notify the user.
5. Preserve history unless the user explicitly deletes it.

Manual actions must remain separate:
- Reset Chat = reset active context/session.
- Clear Current Context = remove active context only.
- Delete Conversation = permanently remove that conversation according to the data model.

THINK
Think is a real task/reasoning mode using the actual model pipeline.
It may provide analysis, plans, calculations and conclusions, but MUST NOT expose
private chain-of-thought. Show safe metadata such as selected model, duration, context
sources and result status.

CODEX
Codex is a real coding workspace/mode, not a button that changes a label.
Support where implemented:
- repository discovery
- source/file inspection
- code search
- symbol search
- build-system detection
- test detection
- patch generation
- diff preview
- permission-aware file modifications
- build execution
- test execution
- diagnostics
- Git status/diff
- project memory
- coding instructions
- task tracking

Required Codex workflow:
inspect -> understand -> plan -> generate patch -> preview diff -> permission check ->
apply -> build -> test -> report actual result

Never say FIXED merely because a patch was generated.

============================================================
4. NATIVE CORE + CLIENT ARCHITECTURE
============================================================

Authoritative architecture:

Native C++ Core
    -> Application Services
    -> Local API / Service Boundary
    -> Desktop GUI / CLI / Web Client

The C++ core is authoritative.
HTML/CSS/JavaScript/TypeScript are clients only.
Business logic MUST NOT be duplicated in UI callbacks, web handlers, CLI handlers or API
handlers when one shared application service can perform the operation.

Create one composition/lifecycle root:
namespace coreai { class CoreApp; }

CoreApp coordinates startup/shutdown and service ownership. It must not become a god object.
Use modular in-process services. Do not create fake microservices merely for appearance.

============================================================
5. TECHNOLOGY BASELINE
============================================================

Required:
- C++20
- CMake
- CTest

Primary target:
- Windows

Architect for future Linux/macOS without falsely claiming support until tested.

Design for practical Windows toolchains such as:
- LLVM-MinGW / clang++
- MSVC where available

Never hard-code developer-specific paths. Never use a path such as
C:\Users\HP\... in source code or repository defaults.

============================================================
6. AUTHORITATIVE SOURCE TREE
============================================================

Use a maintainable structure such as:

CORE-AI/
  CMakeLists.txt
  cmake/
  include/coreai/
  src/
    app/
    core/
    config/
    runtime/
    logging/
    diagnostics/
    storage/
    security/
    providers/
    routing/
    conversations/
    context/
    memory/
    retrieval/
    tools/
    agents/
    workflows/
    automation/
    scheduling/
    plugins/
    projects/
    tasks/
    documents/
    files/
    coding/
    git/
    research/
    maps/
    images/
    multimodal/
    voice/
    api/
    cli/
    gui/
  web/
  tests/
  docs/
  resources/
  scripts/
  packaging/

Use a different structure only when technically justified.
Do not create arbitrary folders merely to look professional.

============================================================
7. ONE AUTHORITY PER RESPONSIBILITY
============================================================

Do not allow competing implementations of the same authority.
Examples of forbidden duplication:
- ConfigManager + ConfigurationManager + SettingsManager
- Logger + LogManager
- ModelRouter + ProviderRouter
- MemoryStore + MemoryManager
- ToolRegistry + ToolManager
- ProjectManager + ProjectService
- PluginManager + PluginService
- WorkflowEngine + WorkflowManager

Inspect existing code first. Reuse/refactor/replace deliberately. Remove obsolete code only
after verifying it is unused.

============================================================
8. TRUTH MODEL
============================================================

Every major capability must expose truthful state.
Allowed states:
IMPLEMENTED
AVAILABLE
CONFIGURED
ENABLED
READY
RUNNING
DISABLED
UNAVAILABLE
FAILED
PARTIAL
EXPERIMENTAL
NOT_IMPLEMENTED
NOT_VERIFIED

A capability is NOT READY merely because:
- a class exists
- an interface exists
- a button exists
- an endpoint returns JSON
- a mock succeeds
- a test file exists

Correct example:
implemented=true
available=false
configured=false
ready=false
status=UNAVAILABLE
reason="No provider configured"

============================================================
9. VERIFICATION AUTHORITY
============================================================

The only evidence for a working feature is actual evidence from:
1. compiler/build output
2. executed tests
3. executed integration tests
4. runtime diagnostics
5. actual provider/network response
6. actual filesystem/database state
7. actual GUI/web behavior
8. actual Git state/diff
9. actual package validation

The AI's own statement is NEVER evidence.
If a feature could not be executed, report NOT_VERIFIED.
Never convert NOT_VERIFIED into PASS.

============================================================
10. NO FAKE FUNCTIONALITY
============================================================

Never fabricate:
- AI responses
- provider health
- model availability
- streaming
- memory
- semantic retrieval
- search results
- maps data
- route results
- citations
- Git state
- file changes
- process results
- agent execution
- workflow execution
- automation/scheduled history
- plugin lifecycle
- image generation
- vision
- voice
- telemetry
- benchmark numbers
- progress percentages
- installation/update state
- authentication
- cloud connectivity

Mocks are allowed only in clearly labeled deterministic tests and must never be used as
live production state.

============================================================
11. VERSION / MODEL SYSTEM
============================================================

Implement one authoritative version registry for:
- application version
- database schema version
- configuration schema version
- project schema version
- plugin API version
- local API version
- web client version

Also implement a real model/version selector driven by actual ModelDescriptor records.
Each model entry may show when actually known:
- provider
- model ID
- display name
- capabilities
- context window
- streaming support
- tool calling
- vision
- structured output
- local/remote
- readiness
- availability

The selector must allow selecting only valid configured/discovered models.
If discovery is unavailable, state why.
Never hard-code a model as discovered when it was not discovered.

============================================================
12. CONFIGURATION
============================================================

Implement typed configuration with scopes:
- global
- project
- workspace
- session

Define clear precedence.
Support:
- defaults
- validation
- migration
- atomic save
- backup
- restore
- import/export
- reset
- environment overrides

Visible settings must control actual runtime behavior.
Every setting requires schema + persistence + runtime usage + validation + test.

============================================================
13. RUNTIME PATHS
============================================================

Implement platform-aware runtime directories:
- config
- data
- cache
- logs
- temp
- projects
- plugins
- models
- indexes
- library
- images
- exports
- backups
- artifacts

Paths must resolve at runtime and work on another Windows machine without source edits.

============================================================
14. STORAGE
============================================================

Use SQLite for structured persistent state and filesystem storage for large files/artifacts.
Persist, where applicable:
- conversations
- messages
- projects
- memory
- tasks
- scheduled jobs
- workflow runs
- plugin configuration
- settings
- library metadata
- artifacts
- diagnostics history

Do not place the entire application state in one giant JSON file.

Important writes should be atomic where practical.

============================================================
15. MIGRATIONS / DATA INTEGRITY
============================================================

Migration:
detect schema -> validate -> backup -> migrate -> validate -> commit

On failure:
- preserve original data
- report exact reason
- provide recovery path

Test interrupted writes, corruption detection, restart, backup and restore without touching
real user data.

============================================================
16. RESULT / ERROR SYSTEM
============================================================

Create typed:
- Result<T>
- Error
- ErrorCode
- ErrorCategory
- ErrorSeverity

Errors should include when appropriate:
code, message, details, component, operation, source, recoverable, retryable,
user_action_required, cause, correlation_id

Use stable error codes such as:
COREAI_CONFIG_INVALID
COREAI_STORAGE_OPEN_FAILED
COREAI_STORAGE_CORRUPT
COREAI_PERMISSION_DENIED
COREAI_PROVIDER_UNAVAILABLE
COREAI_PROVIDER_AUTH_FAILED
COREAI_MODEL_UNAVAILABLE
COREAI_NETWORK_TIMEOUT
COREAI_TOOL_FAILED
COREAI_AGENT_TIMEOUT
COREAI_WORKFLOW_FAILED
COREAI_SCHEDULE_FAILED
COREAI_PLUGIN_FAILED
COREAI_PROJECT_INVALID
COREAI_GIT_FAILED
COREAI_IMPORT_INVALID
COREAI_EXPORT_FAILED

============================================================
17. LOGGING / REDACTION / DIAGNOSTICS
============================================================

Structured logging levels:
DEBUG INFO WARN ERROR FATAL

Include where useful:
timestamp, component, operation, correlation ID, request ID, task ID, project ID

Never log secrets such as:
- passwords
- API keys
- bearer tokens
- refresh tokens
- private key material

Create central diagnostics and:
core-ai doctor
core-ai status
core-ai capabilities

Diagnostic states must come from real checks.

============================================================
18. CAPABILITY REGISTRY
============================================================

One authoritative capability registry powers:
- GUI
- web
- CLI
- API
- doctor
- status
- support bundle

Capability record:
id, name, description, implemented, available, configured, enabled, status,
reason, version, requirements

No duplicated status logic.

============================================================
19. PROVIDERS
============================================================

Create provider-neutral types:
AIProvider
ProviderDescriptor
ProviderHealth
ModelDescriptor
ModelCapabilities
GenerationRequest
GenerationResponse
StreamEvent
Usage
ProviderError

Architect for:
- Ollama
- OpenAI-compatible endpoints
- Generic HTTP
- Local/custom providers

Provider states separately track:
configured
reachable
authenticated
model_available
generation_tested
ready

============================================================
20. OLLAMA
============================================================

Implement real Ollama support where appropriate:
- endpoint
- health check
- model listing
- model metadata
- generation
- streaming where supported
- cancellation where possible
- timeout
- error handling

If Ollama is absent or not running:
adapter = IMPLEMENTED
runtime availability = UNAVAILABLE

Never simulate Ollama.

============================================================
21. OPENAI-COMPATIBLE / REMOTE PROVIDERS
============================================================

Implement configurable support for:
- base URL
- authentication
- model discovery where supported
- chat
- streaming where supported
- tool calling where supported
- vision where supported
- structured output where supported

Never assume an endpoint supports every feature.

============================================================
22. MODEL ROUTING / PRIVACY
============================================================

Routing may consider:
- task type
- required capability
- context window
- privacy policy
- availability
- local preference
- latency
- cost when actual usage is available
- tool/vision/structured-output support

Expose a human-readable selection reason.

Privacy modes:
- Local Only
- Private
- Balanced
- Cloud Enabled

Local Only MUST block remote model/network use for protected operations.
When remote data leaves the machine, show provider, endpoint and transmission state.

============================================================
23. CHAT
============================================================

Implement persistent entities:
Conversation
Message
Attachment
MessageMetadata
ConversationMetadata

Roles:
system, developer, user, assistant, tool, event, error

Features:
- New Chat
- Recent
- search
- rename
- archive
- delete
- edit
- retry
- regenerate
- branch
- export/import
- attachments
- cancel generation
- reset chat
- auto-reset configuration

Chat pipeline:
user input
-> validation
-> conversation context
-> project context
-> memory retrieval
-> security policy
-> model routing
-> provider/model
-> permitted tools
-> response
-> persistence
-> optional memory evaluation
-> client output

============================================================
24. STREAMING
============================================================

Only use streaming when a provider genuinely streams.
Use real events:
GenerationStarted
GenerationDelta
GenerationCompleted
GenerationFailed
GenerationCancelled

Do not simulate streaming with a timer or delayed chunks.

============================================================
25. CONTEXT ENGINE
============================================================

Context sources:
- conversation
- project
- memory
- files
- documents
- tools
- research
- settings
- system instructions

Support relevance, priority, token budget, truncation, summarization where implemented,
and provenance.
Never dump an entire database into every request.

============================================================
26. MEMORY
============================================================

Implement persistent memory types:
- user
- preference
- fact
- project
- conversation
- task
- knowledge
- temporary

Record:
id, content, type, scope, source/provenance, created_at, updated_at, confidence, metadata

Memory can only originate from:
- explicit user action
- actual conversation evidence
- authorized project data
- authorized import

Never invent memories.

Operations:
create/read/search/edit/delete/clear scope/export/import

Project memory must be isolated by project.

============================================================
27. SEMANTIC RETRIEVAL
============================================================

Provide an embedding abstraction.
When a real backend exists:
- embedding generation
- vector indexing
- similarity search
- metadata filtering
- ranking
- incremental indexing

Without a backend:
semantic retrieval = UNAVAILABLE

Exact search remains available without embeddings.

============================================================
28. LIBRARY
============================================================

Library is a real persistent knowledge/document workspace.
Support real:
- import/upload
- folders
- collections
- tags
- metadata
- preview
- processing state
- full-text/search where implemented
- provenance
- pin/favorite
- archive
- delete
- export

Integrate Library with Chat, Memory, Projects, Research and Codex through the real context
engine.

No fake library items.

============================================================
29. DOCUMENTS / FILES
============================================================

Support only parsers actually available and tested.
Potential formats:
TXT, MD, JSON, CSV, PDF, source code

Track:
name, type, size, hash, source, processing state, provenance, storage reference

Use chunking/streaming for large files where practical.

============================================================
30. PROJECTS
============================================================

Projects are first-class persistent entities.
Support:
create/open/close/rename/duplicate/import/export/archive/repair/delete

Project metadata:
id, name, location, created_at, updated_at, version, settings

Project settings may include:
- provider/model
- instructions
- tool permissions
- memory policy
- coding rules
- workflows
- Git policy
- indexing rules

Changing providers must not destroy project data.

============================================================
31. CODEX / CODING SYSTEM
============================================================

Codex must understand repositories.
Support where actually implemented:
- repository map
- language/target discovery
- dependency/build detection
- symbol discovery
- code search
- file analysis
- tests
- diagnostics
- patch generation
- diff review
- refactoring
- test generation
- build execution
- test execution
- Git status

AI file changes must follow:
inspect -> plan -> patch -> preview -> permission -> apply -> build -> test -> report

No silent destructive overwrite.

============================================================
32. GIT
============================================================

Implement actual:
status
diff
log
branch
checkout
add
commit
restore
stash
fetch
pull
push

Destructive/remote actions require appropriate confirmation/policy.
Never claim remote success unless actually verified.

============================================================
33. MAPS
============================================================

Implement Maps as a real service/provider abstraction.
Possible real functions:
- place search
- map view
- directions/routes
- origin/destination
- recent places
- saved places

Do not hard-code API keys.

If no provider is configured:
Maps capability = IMPLEMENTED
Maps availability = UNAVAILABLE
Reason = No map provider configured

Never show fake locations, distances or routes as live results.

============================================================
34. IMAGES
============================================================

Implement Images as a real workspace.
Support when a real backend exists:
- image generation
- image history
- project association
- prompt metadata
- image attachments
- artifact tracking
- preview/export

Only report image generation as READY after a real backend response.
No generated placeholder may be presented as real AI output.

Vision and image generation are separate capabilities.

============================================================
35. AGENTS / MULTI-AGENT
============================================================

Implement real bounded agents.
Entities:
AgentDefinition
AgentTask
AgentRuntime
AgentState
AgentPermissions
AgentEvent
AgentMemory

States:
created, queued, running, waiting, blocked, completed, failed, cancelled, timed_out

Every run has:
time limit, task limit, tool limit, permission limit, context limit, cancellation, failure handling

Agents cannot grant themselves privileges.

Multi-agent roles may include:
Planner, Researcher, Coder, Reviewer, Tester, Documenter

Use structured typed outputs between agents.

============================================================
36. TOOLS
============================================================

Create:
Tool
ToolDescriptor
ToolSchema
ToolExecutor
ToolResult
ToolPermission
ToolAudit

Potential core tools:
filesystem
file_search
code_search
project
memory
documents
Git
process
network
research
diagnostics
maps

Only expose a tool when its real implementation exists.

Permission levels:
DENIED
READ_ONLY
SAFE_WRITE
CONFIRM_REQUIRED
PRIVILEGED

============================================================
37. FILESYSTEM / PROCESS SECURITY
============================================================

Restrict file tools to allowed roots.
Protect against:
- path traversal
- unauthorized absolute paths
- symlink escape where relevant
- destructive operations outside policy

Normalize paths before authorization.

Centralize process execution. Represent program, arguments, working directory, environment,
timeout, cancellation, stdout, stderr and exit code.
Prefer direct process APIs over unsafe shell concatenation.

============================================================
38. WORKFLOWS
============================================================

Implement real workflow execution.
Node types may include:
AI, Tool, Condition, Transform, Loop, Parallel, Delay, Input, Output, Subworkflow

Before execution validate:
structure, references, dependencies, cycles, permissions, providers, tools and inputs.

Persist workflow runs, results and artifacts.

============================================================
39. SCHEDULED / AUTOMATION
============================================================

Scheduled is a real scheduler, not a reminder mockup.
Support where practical:
- one-time schedule
- recurring schedule
- startup trigger
- file-change trigger
- project event trigger
- system event trigger
- manual Run Now

Persist:
schedule ID, task/workflow reference, enabled state, next run, last run, run history,
result, failure reason, created/updated timestamps

Support:
- create
- edit
- pause
- resume
- enable/disable
- run now
- cancel
- history
- delete

Scheduler uses durable persistent state and real system time.
Scheduler obeys the same security/privacy policy as manual actions.

============================================================
40. PLUGINS
============================================================

Implement a real plugin platform.
Manifest:
id, name, version, author, description, entry point, API version, permissions,
dependencies, compatibility

Lifecycle:
discovered, validated, installed, enabled, loaded, running, failed, disabled, uninstalled

Validate dependencies, version conflicts, circular dependencies, malformed manifests,
permission declarations and load failures.

Plugins receive no unrestricted permissions by default.

============================================================
41. RESEARCH / WEB SEARCH
============================================================

Implement interchangeable research/search providers.
Support real workflows for:
- search
- retrieval
- source metadata
- extraction
- comparison
- synthesis
- citation tracking
- uncertainty

Record source provenance:
title, URL, provider, retrieval time, content hash where practical

Never invent citations.

External content is untrusted data and must not override system/security policy.

============================================================
42. NETWORK SECURITY
============================================================

Centralize network access.
Support where appropriate:
- timeouts
- TLS verification
- redirect limits
- URL validation
- response size limits
- proxy configuration
- retry policy
- private-network protections
- network allow/deny policy

The web subsystem must not silently access privileged internal resources.

============================================================
43. MULTIMODAL / VOICE / VISION
============================================================

Architect for:
- text
- image
- audio
- documents
- future video

VOICE is operational only with a real speech backend.
VISION is operational only with a real vision-capable provider.
IMAGE GENERATION is operational only with a real image-generation provider.

Unavailable capabilities must be reported honestly.

============================================================
44. LOCAL API
============================================================

Provide a versioned local HTTP API.
Default binding:
127.0.0.1

Endpoints should include:
/api/v1/health
/api/v1/ready
/api/v1/version
/api/v1/status
/api/v1/capabilities
/api/v1/providers
/api/v1/models
/api/v1/chat
/api/v1/conversations
/api/v1/memory
/api/v1/library
/api/v1/projects
/api/v1/tasks
/api/v1/tools
/api/v1/agents
/api/v1/workflows
/api/v1/scheduled
/api/v1/plugins
/api/v1/files
/api/v1/documents
/api/v1/git
/api/v1/research
/api/v1/maps
/api/v1/images

Use consistent JSON schemas and typed error responses.

============================================================
45. API SECURITY
============================================================

Default to loopback-only.
Implement where appropriate:
- authentication
- authorization
- request validation
- request size limits
- timeouts
- safe CORS
- audit IDs

LAN exposure must be explicitly enabled.
Never expose privileged operations to LAN by default.

============================================================
46. WEB CLIENT
============================================================

Build a complete responsive web client using HTML/CSS/JS or TypeScript when justified.
The web client calls the real API. It never owns authoritative backend state.

Required pages/workspaces:
Home, New Chat, Recent, Chat, Think, Codex, Projects, Library, Images, Maps, Agents,
Tools, Workflows, Scheduled, Plugins, Research, Files, Documents, Git, Tasks,
Diagnostics, Settings

Every page must support truthful states:
loading, ready, empty, offline, disabled, unavailable, permission denied, error, partial

No fake live records.

============================================================
47. WEB DESIGN SYSTEM
============================================================

Original CORE-AI visual identity.
Design principles:
- modern
- dark-first
- technical
- readable
- high information density
- restrained motion
- responsive
- accessible

Suggested shell:
LEFT: navigation/sidebar
TOP: CORE-AI + model/version + provider + privacy + search/command + project
CENTER: current workspace
RIGHT: contextual activity/details when appropriate

CHAT composer should include:
- message input
- mode selector: Chat / Think / Code/Codex / Agent where appropriate
- model/version selector
- attachment control
- project selector
- send
- stop/cancel
- new-chat
- reset-chat

============================================================
48. NATIVE DESKTOP GUI
============================================================

Provide a real native Windows desktop GUI.
Native application/window/input/filesystem/process integration.
The C++ core remains authoritative.

Support:
- dark theme
- light theme
- midnight theme
- high contrast
- scaling/DPI
- keyboard navigation
- focus management
- accessible labels
- responsive layouts

Long-running work must not block the UI thread.

============================================================
49. HOME DASHBOARD
============================================================

Show only real data:
- recent chats
- recent projects
- active tasks
- scheduled jobs
- provider/model health
- memory state
- library activity
- active agents
- plugin state
- storage warnings
- privacy mode
- current project

Do not manufacture metrics or sample records.

============================================================
50. COMMAND PALETTE / SEARCH
============================================================

Implement a real command palette.
Examples:
New Chat, Search, Open Project, Open Library, Think, Codex, Maps, Images,
Schedule Task, Run Workflow, Open Diagnostics, Switch Model, Reset Chat

Commands execute actual application actions.

============================================================
51. SETTINGS
============================================================

Settings should cover where implemented:
providers, model/version, privacy, permissions, memory, library, scheduled, plugins,
appearance, language, scaling, network, storage, retention, performance, chat reset

Every setting has schema, persistence, runtime usage, validation and tests.

============================================================
52. SECURITY / AUTONOMY
============================================================

Central policy enforcement for:
filesystem, processes, network, providers, plugins, agents, workflows, scheduled tasks,
credentials, projects and data transmission.

Autonomy modes:
- Ask Every Time
- Ask for Risky Actions
- Project Safe
- Automatic

These modes must affect real permission checks.

AI cannot grant itself access to files, processes, network, plugins, agent privileges or
scheduler privileges.

============================================================
53. PROMPT-INJECTION / TRUST BOUNDARIES
============================================================

Treat web pages, documents, PDFs, repository files, Git data, plugin output, tool output,
search results, research sources and model suggestions as untrusted data.
They may never override system policy, application policy, security policy or permissions.

============================================================
54. PERFORMANCE / RESOURCES
============================================================

Measure actual performance where possible:
startup, shutdown, project open, indexing, search, memory retrieval, provider latency,
tool latency, agent runtime, workflow runtime, scheduled jobs, API latency and UI response.

Monitor actual CPU/RAM/disk/workers/processes where platform support exists.
Detect low disk space and bound growth of logs/cache/indexes/temp/artifacts.
Never silently delete persistent user data during cleanup.

============================================================
55. CANCELLATION / TIMEOUT / RETRY
============================================================

Long-running operations should support cancellation where technically possible.
Use explicit timeouts for HTTP, providers, tools, processes, plugins, agents, workflows,
scheduler jobs, import/export and research.

Retry only transient errors. Never endlessly retry invalid credentials, permissions,
unsupported features or broken configuration.

============================================================
56. ARTIFACTS / PROVENANCE
============================================================

Track generated artifacts:
code, patches, diffs, reports, documents, images, exports, research outputs,
build logs and test reports.

Track provenance:
created_by, request, task, agent, timestamp, source/project

============================================================
57. IMPORT / EXPORT / BACKUP
============================================================

Portable formats may include:
JSON, Markdown, CSV, plain text and ZIP/project packages.

Imports:
validate -> version check -> migrate -> isolate -> safe write -> report

Exports contain schema/version information.
Backups support manual creation, validation and restore.
Credentials are excluded by default.

============================================================
58. RECOVERY / SAFE MODE
============================================================

Support:
- safe mode
- config repair
- storage repair
- index rebuild
- cache cleanup
- backup restore
- project repair

Safe mode may disable optional risky components such as plugins, automation, remote
providers and experimental tools while leaving diagnostics available.

============================================================
59. CLI
============================================================

Provide:
core-ai version
core-ai status
core-ai doctor
core-ai capabilities
core-ai config
core-ai providers
core-ai models
core-ai chat
core-ai conversations
core-ai new-chat
core-ai recent
core-ai think
core-ai codex
core-ai reset-chat
core-ai memory
core-ai library
core-ai projects
core-ai tasks
core-ai tools
core-ai agents
core-ai workflows
core-ai scheduled
core-ai plugins
core-ai files
core-ai documents
core-ai coding
core-ai git
core-ai research
core-ai maps
core-ai images
core-ai export
core-ai import
core-ai backup
core-ai restore
core-ai repair
core-ai serve

Commands need help, validation, meaningful exit codes and structured JSON mode where appropriate.

============================================================
60. TESTING
============================================================

Required categories:
- unit
- integration
- failure
- security
- storage
- CLI
- API
- native GUI smoke
- web smoke
- provider
- memory
- library
- tools
- agents
- multi-agent
- workflows
- scheduled
- plugins
- projects
- coding/Codex
- Git
- maps
- images
- import/export
- backup/restore
- recovery
- restart
- clean build
- packaging

Never weaken assertions, delete failing tests just to get green, or count skipped as passed.

============================================================
61. FAILURE MATRIX
============================================================

Explicitly test:
- invalid configuration
- corrupted database
- migration failure
- unavailable provider
- unreachable provider
- invalid credentials
- timeout
- rate limit
- permission denied
- path traversal
- invalid path
- file-write failure
- process failure
- tool timeout
- agent timeout/cancel
- workflow failure/cancel
- scheduled-task failure
- plugin failure
- invalid plugin manifest
- Git failure/conflict
- missing model
- low disk
- missing file
- invalid project
- corrupted import
- export failure
- API authentication failure
- malformed API request
- web client API failure
- map backend unavailable
- image backend unavailable

============================================================
62. LIVE VS OFFLINE TESTS
============================================================

Core tests must be deterministic and offline.
Live tests run only when the dependency is configured.

Example:
Ollama not configured or unavailable -> SKIPPED — dependency unavailable

Never report SKIPPED as PASS.
Test result states:
PASS
FAIL
SKIPPED
NOT_RUN

============================================================
63. CLEAN START TEST
============================================================

From clean state, validate actual behavior:
launch
-> doctor
-> create project
-> configure settings
-> configure provider where available
-> discover models
-> New Chat
-> send a real message when provider is available
-> persist conversation
-> restart
-> verify conversation
-> create/search memory
-> inspect Library
-> open Codex
-> discover repository
-> perform one safe coding operation
-> test permission flow
-> run one agent
-> run one workflow
-> run one scheduled job
-> inspect Plugins
-> inspect Maps
-> inspect Images
-> inspect Diagnostics
-> shutdown
-> relaunch

Only report what actually ran.

============================================================
64. CLEAN BUILD / FALLBACK BUILD
============================================================

CMake is authoritative.
Provide:
scripts/Build-CORE-AI-Windows.ps1
scripts/Build-CORE-AI-Windows.cmd

Fallback may search for an existing clang++/g++ compiler but MUST NOT install software
automatically. It must use the same source and same tests.

Validate clean checkout -> configure -> build -> tests -> package.

============================================================
65. PACKAGING
============================================================

Create a clean Windows distribution.
Do not include unnecessary build intermediates, developer paths, temporary data or secrets.
Where practical provide a portable ZIP and installer.

Validate the actual extracted package:
- files
- dependencies
- launch
- runtime paths
- GUI
- web
- CLI
- shutdown

============================================================
66. DOCUMENTATION
============================================================

Maintain truthful documentation for major implemented systems, including:
README.md
BUILD.md
ARCHITECTURE.md
CONFIGURATION.md
PROVIDERS.md
MODELS.md
CHAT.md
THINK.md
CODEX.md
MEMORY.md
LIBRARY.md
MAPS.md
IMAGES.md
TOOLS.md
AGENTS.md
WORKFLOWS.md
SCHEDULED.md
PLUGINS.md
PROJECTS.md
CODING.md
GIT.md
RESEARCH.md
SECURITY.md
PRIVACY.md
API.md
CLI.md
GUI.md
WEB.md
TESTING.md
TROUBLESHOOTING.md
RELEASE.md
CHANGELOG.md

Only mark implemented behavior as implemented.

============================================================
67. UI-TO-SERVICE INTEGRATION RULE
============================================================

Every important control must invoke a real backend operation.
Examples:
New Chat -> ConversationService
Think -> real generation pipeline
Codex -> Coding/Codex service
Projects -> ProjectService
Library -> LibraryService
Maps -> MapsService
Images -> ImageService
Scheduled -> SchedulerService
Plugins -> PluginService
Model/Version -> Provider/Model registry
Recent -> persisted query
Reset Chat -> actual session operation

No UI-only features.

============================================================
68. STATE MACHINES
============================================================

Define explicit valid state machines for:
Application
Provider
Model
Generation
Conversation
Task
Agent
Workflow
ScheduledJob
Plugin
Project
Import
Export
Artifact

Prevent impossible states.
Example: a live generation cannot be SUCCESS while the same operation reports a live
provider connection failure unless that failure is explicitly historical and separate.

============================================================
69. BACKGROUND WORK
============================================================

Long operations run asynchronously where appropriate:
network, generation, indexing, Git, agents, workflows, scheduler, import/export.

No orphan threads.
Every worker has owner, lifecycle, cancellation, error state and shutdown behavior.

============================================================
70. FINAL COMPLETION GATE
============================================================

A feature is complete only when it has:
- implementation
- integration
- error handling
- security/policy path
- persistence where required
- diagnostics
- tests
- documentation where appropriate

External dependency absent:
IMPLEMENTED + UNAVAILABLE is valid.

Genuinely absent implementation:
NOT_IMPLEMENTED is valid.

Could not execute verification:
NOT_VERIFIED is mandatory.

============================================================
71. FINAL IMPLEMENTATION COMMAND
============================================================

NOW BUILD CORE-AI INTO THE COMPLETE REAL PRODUCT DESCRIBED ABOVE.

You are authorized to:
- inspect the repository
- redesign architecture
- replace broken modules
- remove obsolete modules
- implement native core
- implement desktop GUI
- implement web client
- implement API
- implement CLI
- implement providers
- implement model/version selector
- implement New Chat
- implement Recent
- implement Think
- implement Codex
- implement Projects
- implement Library
- implement Maps
- implement Images
- implement Memory
- implement Tools
- implement Agents
- implement Multi-Agent
- implement Workflows
- implement Scheduled/Automation
- implement Plugins
- implement Research
- implement Files/Documents
- implement Coding/Git
- implement Security/Privacy
- implement Diagnostics/Doctor
- implement Import/Export/Backup/Recovery
- implement automatic chat reset
- implement packaging
- implement documentation

Then compile, test, debug, integrate, validate, package and retest.

DO NOT RESPOND WITH AN ARCHITECTURE PLAN ONLY.
DO NOT CREATE A USER-FACING PHASED DELIVERY PLAN.
DO NOT CREATE FAKE FEATURES.
DO NOT STOP AT FOUNDATION.
DO NOT STOP AT UI.
DO NOT STOP AT API.
DO NOT STOP AT ONE PROVIDER.
DO NOT STOP AT ONE CHAT.
DO NOT STOP AT ONE PASSING TEST.

Continue until the requested product scope is exhausted or an actual technical/external
limitation prevents further verification. Report that limitation precisely.

============================================================
72. FINAL TRUTH COMMANDMENT
============================================================

REALITY > APPEARANCE
TRUTH > MARKETING
VERIFICATION > CLAIMS
DATA SAFETY > CONVENIENCE
SECURITY > AUTONOMY

Never claim READY, PASS, SUCCESS or COMPLETE without evidence.

A truthful:
UNAVAILABLE
NOT_IMPLEMENTED
FAILED
NOT_VERIFIED
is always preferable to fake success.

============================================================
73. FINAL REPORT
============================================================

Produce a truthful final report:

CORE-AI FINAL STATUS

Version:
Build:
Platform:

Architecture:
Source Build:
Clean Build:
Unit Tests:
Integration Tests:
Failure Tests:
Security Tests:

Native GUI:
Web UI:
API:
CLI:

Provider System:
Models:
Chat:
New Chat:
Recent:
Think:
Codex:
Memory:
Semantic Retrieval:
Library:
Tools:
Agents:
Multi-Agent:
Workflows:
Scheduled:
Plugins:
Projects:
Tasks:
Files:
Documents:
Git:
Research:
Maps:
Images:
Multimodal:
Vision:
Voice:
Security:
Privacy:
Diagnostics:
Export:
Import:
Backup:
Recovery:
Packaging:
Documentation:

Each item must be one of:
PASS
FAIL
SKIPPED
NOT_RUN
UNAVAILABLE
NOT_IMPLEMENTED
NOT_VERIFIED

Give the actual reason for every non-pass state.

TEST COUNTS:
total:
passed:
failed:
skipped:
not_run:

DEPENDENCIES:
Compiler:
CMake:
Ninja:
Git:
Ollama:
Remote Provider:
Network:
Map Backend:
Image Backend:
Voice Backend:
Vision Backend:

FILE CHANGE REPORT:
created:
modified:
deleted:
migrated:
generated:

REMOTE REPOSITORY:
If remote operations were not executed:
LOCAL IMPLEMENTATION VERIFIED
REMOTE REPOSITORY UPDATE NOT VERIFIED

Never claim a GitHub push, PR, release or remote update unless actually executed and verified.

============================================================
74. FINAL PRODUCT IDENTITY
============================================================

CORE-AI must become:
REAL
NATIVE
LOCAL-FIRST
WEB-CAPABLE
SECURE
PRIVATE
MODULAR
EXTENSIBLE
TESTABLE
OBSERVABLE
USER-CONTROLLED
PROFESSIONAL

It should feel like one unified operating environment for personal AI.

Not a mockup.
Not a prototype.
Not a browser pretending to be an operating system.
Not a giant monolithic C++ file.
Not fake enterprise architecture.

CORE-AI
YOUR INTELLIGENCE.
YOUR SYSTEM.
YOUR CONTROL.

BUILD IT.
TEST IT.
VERIFY IT.
MAKE IT REAL.
