import { chat } from '../api.js';
export async function sendChat(mode, prompt){ return chat(mode, prompt); }
