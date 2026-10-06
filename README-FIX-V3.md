# CORE-AI GUI — Fix V3

This build fixes the two browser ReferenceErrors reported by the user:
- `Cannot access 'googleCsePromise' before initialization`
- `Cannot access 'toastTimer' before initialization`

It also restores programmatic Google Search execution with `element.execute(query)` and adds a cache-busting query to `app.js` so a browser is less likely to reuse the broken cached script.

Validation:
- `node --check gui/app.js` passed.
- Static declaration/order checks passed.
- ZIP integrity check passed.

Use this ZIP as a replacement for the previous GUI package. After unzipping, do a hard refresh (`Ctrl+F5`) once.
