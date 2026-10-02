# Security Rules

1. Deny process execution by default.
2. Separate read/write permissions.
3. Record privileged actions in `audit.log`.
4. Treat plugin DLLs as trusted code only after explicit user approval.
5. Keep GUI communication localhost-only.
6. Never claim an external task succeeded without a real result.
7. Add sandboxing before enabling autonomous process execution.
