# Security

The local API binds to `127.0.0.1`. The web UI does not own application data. Runtime paths are resolved dynamically. Project creation is restricted to the runtime projects directory. Model-generated actions are not granted arbitrary process execution in this build.

Secrets are not stored in source-controlled files. External backends are treated as untrusted and are never allowed to override CORE-AI policy.
