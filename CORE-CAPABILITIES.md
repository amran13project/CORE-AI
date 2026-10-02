# CORE AI capability matrix

Legend:
- IMPLEMENTED = code exists and is exercised by this build.
- ADAPTER = interface/architecture exists; external backend still required.
- ROADMAP = deliberately not implemented yet.

| Area | Status | Notes |
|---|---|---|
| Native C++20 Core | IMPLEMENTED | CMake project and core lifecycle |
| Local LLM adapter | IMPLEMENTED | Ollama CLI adapter |
| Model routing | IMPLEMENTED | Provider abstraction + router |
| Chat | IMPLEMENTED | Local model completion |
| Memory | IMPLEMENTED | Persistent simple text store |
| File tools | IMPLEMENTED | Read/write UTF-8 text |
| Process tools | IMPLEMENTED | Permission-gated shell execution |
| Agent manager | IMPLEMENTED | Planner + model-assisted guidance |
| Verification | IMPLEMENTED | Real process exit-code verification |
| Audit log | IMPLEMENTED | Action log |
| Permissions | IMPLEMENTED | Basic capability gates |
| Plugin manager | IMPLEMENTED | Dynamic loading interface |
| Plugin SDK | IMPLEMENTED | Example plugin source |
| Project manager | IMPLEMENTED | Project-local metadata |
| Workflow registry | IMPLEMENTED | Workflow definitions |
| Native Windows UI | IMPLEMENTED | Win32 shell included; Windows validation required |
| Voice | ROADMAP | whisper.cpp adapter planned |
| Vision | ROADMAP | Local vision adapter planned |
| RAG/embeddings | ROADMAP | Vector/index backend planned |
| Browser automation | ROADMAP | Permissioned browser plugin planned |
| Native code editor | ROADMAP | Dedicated editor workspace planned |
| 2D/3D creation | ROADMAP | Creator plugins planned |
| Advanced sandboxing | ROADMAP | OS-level isolation required |
| Cloud providers | ADAPTER | Provider interface permits additional backends |
