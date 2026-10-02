export const state = { panel: 'chat', mode: 'chat', online: false };
export function setState(patch){ Object.assign(state, patch); window.dispatchEvent(new CustomEvent('core-state')); }
