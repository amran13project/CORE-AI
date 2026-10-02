import { CORE_API } from '../api.js';
export async function memorySearch(query){ const r=await fetch(CORE_API+'/memory-search?q='+encodeURIComponent(query)); return r.json(); }
