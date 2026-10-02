import { getJSON, chat } from './api.js';
import { state, setState } from './state.js';
import { bindNavigation } from './navigation.js';
const $=s=>document.querySelector(s);
function add(role,text){const el=document.createElement('div');el.className='msg '+role;el.textContent=text;$('#messages').append(el);$('#messages').scrollTop=$('#messages').scrollHeight;}
async function refresh(){try{const s=await getJSON('/status');setState({online:true,model:s.model});$('#status').textContent='Online · '+(s.model||'model');}catch{setState({online:false});$('#status').textContent='Core offline';}}
$('#composer').addEventListener('submit',async e=>{e.preventDefault();const input=$('#input'),text=input.value.trim();if(!text)return;add('user',text);input.value='';try{const j=await chat($('#mode').value.toLowerCase(),text);add('ai',j.text||j.error||'No response');}catch(err){add('ai','CORE connection error: '+err.message);}});
$('#mode').addEventListener('change',e=>setState({mode:e.target.value.toLowerCase()}));
bindNavigation(); refresh(); setInterval(refresh,5000);
