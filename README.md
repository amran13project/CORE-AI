# CORE-AI Public Auto-Web-Search GUI

- Workspace and Recent share one sidebar scroll area.
- Workspace keeps a short primary list.
- More expands the secondary workspace tools.
- Recent can collapse/expand.
- Sidebar header and CORE user profile remain fixed.
- Main chat scroll and bottom composer remain separate from sidebar scrolling.
- Think remains in the composer, not in the left sidebar.
- C++/src is not changed by this package.


V5 GOOGLE + AUTH
- Adds Google Search under More. It builds a direct Google results URL; it does not fabricate search-result content.
- Typing /search <query> in chat returns a real clickable Google search link.
- URLs returned by CORE-AI assistant messages are rendered as clickable links.
- Adds local Log in / Register UI with SHA-256 password hashing in browser storage.
- No C++ source changes. Local auth is for the personal/local prototype, not production authentication.


V6 WEBSITE ICONS
- URLs in assistant messages render as clickable link cards with website favicons.
- Google search links show the Google favicon.
- The target website still opens directly in a new tab.
- No C++ source is changed.


PUBLIC AUTO GOOGLE SEARCH
- Uses the provided Programmable Search Engine ID (cx) without requiring a Google API key for the embedded Search Element.
- Search UI stays inside CORE-AI.
- Shows a real “Searching Google…” spinner before results load.
- Shows a clickable Google result card with Google favicon and query.
- Loads the configured Programmable Search Element to render the actual search results inside the app.
- This package does not include or expose any secret API key.


## Public inline Google Search

This build is designed for a public static web deployment. The public Google Programmable Search Engine ID is embedded in `gui/app.js` as `DEFAULT_GOOGLE_CX` and is **not a secret**.

Use the normal CORE-AI chat box. There is no `#Google` command and no `/search` command.

Example:

- `How do I install Roblox Studio?`
- `What is the latest CMake documentation?`
- `Find me C++ tutorials`

CORE-AI automatically sends the natural-language prompt to the public Google Programmable Search Element, shows a `Searching Google…` animation, and keeps the clickable result cards inside the same chat timeline. While Google loads, the assistant message shows a searching animation. When results are rendered, each result stays clickable and the UI adds a website favicon, title, visible URL, and snippet using Google's Search Element.

Google documents the Search Element's `searchresults-only` renderer plus `execute(query)` and search/result callbacks. See the official documentation before changing the integration.

### Important

This public GUI does not contain a Google API key. The Programmable Search Element is loaded using the public Search Engine ID (`cx`). Publishing the `cx` is expected for this client-side integration; do not put secret API keys into frontend files.

### Deploy

Upload the contents of `gui/` to a static host such as your own web hosting, Cloudflare Pages, Netlify, GitHub Pages, or another static web host. The inline `#Google` search works from the browser. The normal CORE-AI chat routes still require the native/backend service when those are used.


## ChatGPT-like source links
The search UI is presented as compact clickable source cards inside the chat timeline. YouTube results are also shown as inline 16:9 video players when a YouTube watch/shorts URL is returned. Each source shows a website favicon, title, domain/URL, and external-link arrow. The raw Google result layout is hidden so the experience is closer to a ChatGPT-style Sources section.


UI behavior: web search results are rendered inline in the chat timeline. During loading, result metadata is shown progressively with real site favicons as soon as Google exposes it; there is no separate source panel to close.

## Automatic tool routing v4
Normal chat text is routed automatically. Explicit #GitHub/#Maps/#Ollama/#Git/#Image/#Codex tags are supported, but not required. The normal route order is: explicit plugin tag, image generation intent, maps/location intent, coding/Codex/game intent, current/web-search intent, then normal chat.

Image generation calls the backend /image or /image-create endpoint when exposed and renders a returned image URL/data URI inside the chat; it never claims success when the backend does not provide a usable image.
