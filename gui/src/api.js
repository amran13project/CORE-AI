export const CORE_API = 'http://127.0.0.1:47821';
export async function getJSON(path, options = {}) {
  const r = await fetch(CORE_API + path, options);
  if (!r.ok) throw new Error(`HTTP ${r.status}`);
  return r.json();
}
export async function chat(mode, prompt) {
  return getJSON('/chat', { method:'POST', headers:{'Content-Type':'application/json'}, body:JSON.stringify({mode,prompt}) });
}
