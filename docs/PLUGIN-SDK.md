# Plugin SDK foundation

A plugin directory contains a manifest and an entry library. The current manager validates/load-checks the native library only; a stable ABI, capability declarations, signed manifests, lifecycle callbacks, and UI panels are the next extension points.

The example plugin is intentionally tiny so it can be used as the first compatibility test.
