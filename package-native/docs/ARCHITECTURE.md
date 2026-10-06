# ARCHITECTURE

Status for this build is reported by . CORE-AI does not claim external functionality without a verified backend.


### Multi-user storage

User-owned data is isolated below `users/<sanitized-user-id>/shard-###`. `CORE_USER_ID` selects the identity. When the configured per-user shard quota is exceeded, the allocator creates the next shard for the same user. A different user always gets a different user root.
