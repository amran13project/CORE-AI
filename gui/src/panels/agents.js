import { sendChat } from './chat.js';
export async function runAgent(prompt){ return sendChat('agent', prompt); }
