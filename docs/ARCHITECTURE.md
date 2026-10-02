# CORE AI 0.2 Architecture

CORE is layered so the GUI is replaceable and the native core does not depend on the web UI.

- **Core**: lifecycle, config, event bus.
- **AI**: provider interface + model router + Ollama adapter.
- **Agents**: deterministic planning foundation; model-backed execution is provider/tool dependent.
- **Memory**: persistent local text store; replaceable later with SQLite/vector/RAG.
- **Tools**: registry with explicit permission boundaries.
- **Security**: permission gates.
- **Audit**: evidence log.
- **Verification**: result/exit-code truth boundary.
- **Projects**: isolated project metadata.
- **Automation**: event → action definitions.
- **Plugins**: dynamic loading foundation + manifest/SDK example.
- **GUI**: HTML/CSS/JS, replaceable by the owner.

The architecture deliberately separates implemented functionality from adapters and roadmap features. A UI state never counts as proof of completion.
