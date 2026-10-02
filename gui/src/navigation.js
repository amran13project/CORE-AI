import { setState } from './state.js';
export function bindNavigation(){ document.querySelectorAll('[data-panel]').forEach(b=>b.addEventListener('click',()=>setState({panel:b.dataset.panel}))); }
