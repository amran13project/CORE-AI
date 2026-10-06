# CORE-AI — ChatGPT-style GUI

This is the requested CORE-AI frontend layout:

- Left fixed sidebar
- CORE-AI branding
- + New chat
- Search chats
- WORKSPACE: New chat / Think / Settings
- Recent chat history
- CORE user profile
- Main chat/model header
- Model selector and ⋮ menu
- Large scrollable chat area
- Bottom-fixed composer
- + attachment button
- Message input
- Think ON/OFF
- Send ↑ button
- Local browser chat history
- `/api/chat` integration
- `/api/health` connection indicator
- No fake assistant response when the backend is unavailable

## Integration

Copy `index.html`, `style.css`, and `app.js` into the GUI folder used by your existing CORE-AI server.

The UI expects:

GET `/api/health`
POST `/api/chat`

POST body:

```json
{
  "message": "Hello",
  "conversationId": "...",
  "model": "auto",
  "think": true,
  "attachments": [
    {"name":"file.txt","type":"text/plain","size":123}
  ]
}
```

The response can be a common JSON shape such as:

```json
{"content":"Hello!"}
```

or `{ "response": "..." }`, `{ "reply": "..." }`, or `{ "message": { "content": "..." } }`.

No external libraries are required.
