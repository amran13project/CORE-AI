# CORE-AI Public Web Deployment

1. Host the contents of `gui/` on a static web host.
2. Open the site.
3. Use the chat composer with `#Google <query>`.
4. Search results render directly in the chat timeline with a loading animation, favicon, title, URL and snippet.

No Google API key is stored in the frontend. The build uses the public Programmable Search Engine ID (`cx`) in the browser.


### Inline video results
YouTube results that include a playable YouTube URL are rendered directly in the chat as an inline video player, with a source header linking to YouTube and a snippet below when available. Other web results remain compact clickable source cards.
