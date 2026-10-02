# CORE AI Architecture

## Runtime layers

1. **CoreApp** — lifecycle and composition root.
2. **ModelRouter** — provider abstraction; v0.1 includes Ollama CLI.
3. **MemoryStore** — personal/project memory persistence.
4. **ToolRegistry** — capability registry.
5. **PermissionManager** — explicit action gates.
6. **AgentManager + Planner** — task decomposition and model-assisted planning.
7. **VerificationEngine** — truth boundary for actual process results.
8. **AuditLog** — action history.
9. **PluginManager** — dynamically loaded extensions.
10. **ProjectManager** — project-local metadata.
11. **WorkflowEngine** — reusable event/action definitions.
12. **UI** — Win32 GUI on Windows; console elsewhere.

## Design rule

No subsystem may claim functionality based only on UI state. A completion status must originate from an actual result, process exit code, test, or explicit external evidence.
