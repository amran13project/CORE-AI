const state={section:'chat',messages:[]};
const nav=[
 ['home','Home'],['chat','New Chat'],['think','Think'],['codex','Codex'],['recent','Recent'],
 ['projects','Projects'],['library','Library'],['maps','Maps'],['images','Images'],['memory','Memory'],
 ['files','Files'],['documents','Documents'],['git','Git'],['research','Research'],['tasks','Tasks'],
 ['agents','Agents'],['workflows','Workflows'],['scheduled','Scheduled'],['plugins','Plugins'],
 ['diagnostics','Diagnostics'],['settings','Settings']
];
const $=s=>document.querySelector(s);
const esc=s=>String(s).replace(/[&<>"']/g,c=>({'&':'&amp;','<':'&lt;','>':'&gt;','"':'&quot;',"'":'&#39;'}[c]));
async function api(path,opt={}){const r=await fetch(path,opt);const j=await r.json().catch(()=>({error:'Invalid response'}));if(!r.ok)throw new Error(j.error||`HTTP ${r.status}`);return j}
function post(path,obj){return api(path,{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify(obj)})}
function renderNav(){const n=$('#nav');n.innerHTML=nav.map(([id,t])=>`<button class="${state.section===id?'active':''}" data-id="${id}">${t}</button>`).join('');n.querySelectorAll('button').forEach(b=>b.onclick=()=>{state.section=b.dataset.id;render()})}
function card(title,body){return `<div class="card"><strong>${esc(title)}</strong>${body}</div>`}
async function render(){renderNav();const c=$('#content'),title=$('#title');title.textContent=nav.find(x=>x[0]===state.section)?.[1]||'CORE-AI';
 if(['chat','think','codex'].includes(state.section)){c.innerHTML=state.messages.map(m=>`<div class="msg ${m.role}">${esc(m.text)}</div>`).join('')||card(state.section==='chat'?'New Chat':state.section.toUpperCase(),'<div class="muted">Live service state only. No sample conversation is generated.</div>');return}
 try{
  if(state.section==='home'){const [s,caps,recent,projects]=await Promise.all([api('/status'),api('/capabilities'),api('/conversations/recent'),api('/projects')]);c.innerHTML=card('CORE-AI status',`<div class="row"><span>Version</span><span class="pill">${esc(s.version)}</span></div><div class="row"><span>Privacy</span><span class="pill">${esc(s.privacy_mode)}</span></div><div class="row"><span>Selected model</span><span class="pill">${esc(s.model)}</span></div><div class="row"><span>Storage</span><span class="pill">${s.storage?'PASS':'FAIL'}</span></div><div class="row"><span>Ollama</span><span class="pill">${s.ollama_reachable?'PASS':'UNAVAILABLE'}</span></div><div class="row"><span>Recent chats</span><span class="pill">${recent.length}</span></div><div class="row"><span>Projects</span><span class="pill">${projects.length}</span></div><div class="row"><span>Capabilities registered</span><span class="pill">${caps.length}</span></div>`);return}
  if(state.section==='recent'){const j=await api('/conversations/recent');c.innerHTML=card('Recent',j.map(x=>`<div class="row"><span>${esc(x.title)}</span><span class="pill">${esc(x.id.slice(0,8))}</span></div>`).join('')||'<span class="muted">No conversations yet.</span>');return}
  if(state.section==='projects'){const j=await api('/projects');c.innerHTML=card('Projects',j.map(x=>`<div class="row"><span>${esc(x.name)}</span><span class="muted">${esc(x.root)}</span></div>`).join('')||'<span class="muted">No projects yet.</span>');return}
  if(state.section==='library'){const j=await api('/library');c.innerHTML=card('Library',j.map(x=>`<div class="row"><span>${esc(x.name)}</span><span class="pill">${esc(x.hash.slice(0,12))}</span></div>`).join('')||'<span class="muted">No library artifacts.</span>');return}
  if(state.section==='memory'){const j=await api('/memory/search?q=');c.innerHTML=card('Memory',j.map(x=>`<div class="row"><span>${esc(x.content)}</span><span class="pill">${esc(x.scope)}</span></div>`).join('')||'<span class="muted">No memory entries.</span>');return}
  if(state.section==='tasks'){const j=await api('/tasks');c.innerHTML=card('Tasks',j.map(x=>`<div class="row"><span>${esc(x.name)}</span><span class="pill">${esc(x.status)}</span></div>`).join('')||'<span class="muted">No tasks.</span>');return}
  if(state.section==='workflows'){const j=await api('/workflows');c.innerHTML=card('Workflows',j.map(x=>`<div class="row"><span>${esc(x.name)}</span><span class="pill">${esc(x.id.slice(0,8))}</span></div>`).join('')||'<span class="muted">No workflows.</span>');return}
  if(state.section==='scheduled'){const j=await api('/scheduled');c.innerHTML=card('Scheduled',j.map(x=>`<div class="row"><span>${esc(x.name)}</span><span class="pill">${x.enabled?'enabled':'disabled'}</span></div>`).join('')||'<span class="muted">No schedules.</span>');return}
  if(state.section==='plugins'){const j=await api('/plugins');c.innerHTML=card('Plugins',j.map(x=>`<div class="row"><span>${esc(x.path)}</span><span class="pill">${esc(x.state)}</span></div>`).join('')||'<span class="muted">No native plugins discovered.</span>');return}
  if(state.section==='git'){const j=await api('/git/status');c.innerHTML=card('Git status',`<pre>${esc(j.status||'(clean)')}</pre>`);return}
  if(state.section==='research'){const j=await api('/research?q=');c.innerHTML=card('Research',j.map(x=>`<div class="row"><span>${esc(x.title)}</span><span class="pill">${esc(x.provenance)}</span></div>`).join('')||'<span class="muted">No local research sources.</span>');return}
  if(state.section==='maps'){c.innerHTML=card('Maps','<div class="muted">Map query and directions links are generated by the native maps adapter. Enter a query in the command bar is not enabled in this minimal client.</div>');return}
  if(state.section==='images'){c.innerHTML=card('Images','<div class="muted">Local image artifacts can be generated by the native service and are stored under the runtime images directory.</div>');return}
  if(state.section==='files'){c.innerHTML=card('Files','<div class="muted">Sandboxed file read/write is available through the local API.</div>');return}
  if(state.section==='documents'){c.innerHTML=card('Documents','<div class="muted">TXT, MD, JSON and CSV inspection is available through the local API.</div>');return}
  if(state.section==='agents'){c.innerHTML=card('Agents','<div class="muted">Bounded local planner and multi-agent orchestration are available through the native service.</div>');return}
  if(state.section==='diagnostics'){const [d,cap]=await Promise.all([api('/doctor'),api('/capabilities')]);c.innerHTML=card('Doctor',`<pre>${esc(JSON.stringify(d,null,2))}</pre>`)+card('Capabilities',`<pre>${esc(JSON.stringify(cap,null,2))}</pre>`);return}
  if(state.section==='settings'){const s=await api('/status');c.innerHTML=card('Settings',`<div class="row"><span>Privacy</span><span class="pill">${esc(s.privacy_mode)}</span></div><div class="row"><span>Model</span><span class="pill">${esc(s.model)}</span></div><div class="muted">Configuration is persisted by the native service.</div>`);return}
 }catch(e){c.innerHTML=card('FAILED',`<div class="muted">${esc(e.message)}</div>`)}
}
async function refresh(){try{const s=await api('/status');$('#status').textContent=`${s.ollama_reachable?'Ollama READY':'Ollama UNAVAILABLE'} · ${s.privacy_mode} · ${s.model}`}catch(e){$('#status').textContent='Core OFFLINE · '+e.message}}
$('#newBtn').onclick=async()=>{try{await post('/conversations/new',{});state.messages=[];state.section='chat';await render()}catch(e){alert(e.message)}};
$('#resetBtn').onclick=async()=>{try{await post('/conversations/reset',{});state.messages=[];await render()}catch(e){alert(e.message)}};
$('#composer').onsubmit=async e=>{e.preventDefault();const input=$('#input'),text=input.value.trim();if(!text)return;state.messages.push({role:'user',text});input.value='';await render();try{const mode=$('#mode').value;const j=await post('/chat',{mode,prompt:text});state.messages.push({role:'assistant',text:j.text||j.error||''})}catch(err){state.messages.push({role:'assistant',text:'ERROR: '+err.message})}await render()};
render();refresh();setInterval(refresh,5000);
