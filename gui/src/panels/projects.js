import { CORE_API } from '../api.js';
export async function projectList(){ const r=await fetch(CORE_API+'/projects'); return r.json(); }
