# CORE-AI GUI

The web client is a chat-first presentation surface for the native CORE-AI service. The layout is intentionally familiar to modern AI chat applications: compact left navigation, recent conversations, minimal top bar, centered conversation column, and a floating composer.

## Composer

- Enter sends the message.
- Shift+Enter inserts a newline.
- `+` opens Plugins, Tools, Files, Images, and Library.
- Function opens Think, Codex, Maps, Research, New Chat, and Reset Chat.
- Think is a direct composer toggle.
- Microphone uses browser speech recognition only when the browser provides it; no speech result is fabricated.

## Truthfulness

The UI never invents conversation, model, provider, library, or plugin records. Unavailable services are surfaced as unavailable/errors instead of sample content.
