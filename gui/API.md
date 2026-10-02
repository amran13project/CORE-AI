# GUI ↔ CORE API contract

The starter GUI expects a localhost-only service at `http://127.0.0.1:47821`.

`GET /status` returns `{ "online": true, "model": "..." }`.

`POST /chat` accepts `{ "mode": "chat|think|code|agent", "prompt": "..." }` and returns `{ "text": "..." }` or `{ "error": "..." }`.

The GUI is intentionally replaceable. You can rewrite `index.html`, `style.css`, and `app.js` without changing the core contracts.
