const state={section:'chat',messages:[],think:false,busy:false,mic:false,workingPhase:0,lastRequest:null,recent:[]};
const nav=[
  ['chat','New chat','✦'],['think','Think','◌'],['codex','Codex','⌘'],['projects','Projects','▦'],['library','Library','▤'],['maps','Maps','⌖'],['images','Images','◉'],['memory','Memory','∞'],['files','Files','□'],['documents','Documents','▥'],['git','Git','◫'],['research','Research','⌁'],['tasks','Tasks','✓'],['agents','Agents','◈'],['workflows','Workflows','◇'],['scheduled','Scheduled','◷'],['plugins','Plugins','◆'],['diagnostics','Diagnostics','!']
];
const phases=['Connecting to provider…','Generating response…','Finishing response…'];
const $=s=>document.querySelector(s);
const esc=s=>String(s??'').replace(/[&<>"']/g,c=>({'&':'&amp;','<':'&lt;','>':'&gt;','"':'&quot;',"'":'&#39;'}[c]));
async function api(path,opt={}){
  let r;try{r=await fetch(path,opt)}catch(err){const e=new Error('CORE-AI connection failed. Check that the local core is running.');e.code='COREAI_CONNECTION_FAILED';e.retryable=true;throw e}
  const j=await r.json().catch(()=>({error:'Invalid response from CORE-AI',error_code:'COREAI_INVALID_RESPONSE'}));
  if(!r.ok){const e=new Error(j.error||`HTTP ${r.status}`);e.code=j.error_code||`HTTP_${r.status}`;e.retryable=Boolean(j.retryable);e.status=r.status;throw e}
  return j;
}
const post=(path,obj)=>api(path,{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify(obj)});
function card(title,body){return `<div class="card"><strong>${esc(title)}</strong>${body}</div>`}
function renderNav(){
  const n=$('#nav');n.innerHTML=nav.map(([id,t,ico])=>`<button class="${state.section===id?'active':''}" data-id="${id}"><span class="nav-ico">${ico}</span><span>${esc(t)}</span></button>`).join('');
  n.querySelectorAll('button').forEach(b=>b.onclick=async()=>{if(state.busy)return;state.section=b.dataset.id;if(state.section==='chat')state.think=false;await render();closeSidebarMobile()});
}
function renderComposer(){
  const think=$('#thinkBtn'),send=$('#send'),input=$('#input');
  think.classList.toggle('active',state.think);think.setAttribute('aria-pressed',String(state.think));think.textContent=state.think?'Think ON':'Think';
  send.disabled=state.busy||!input.value.trim();input.disabled=state.busy;think.disabled=state.busy;
}
function workingMarkup(){return `<div class="working-indicator" role="status" aria-live="polite"><div class="working-track" aria-hidden="true"><span class="runner"></span><span class="obstacle obstacle-a"></span><span class="obstacle obstacle-b"></span><span class="obstacle obstacle-c"></span></div><div class="working-copy"><strong>CORE is working</strong><span id="workingPhase">${esc(phases[state.workingPhase])}</span></div></div>`}
function errorMarkup(err,index){const retry=Boolean(err.retryable||err.code==='COREAI_CONNECTION_FAILED');return `<div class="error-card" role="alert"><div class="error-head"><span class="error-icon">!</span><div><strong>CORE-AI error</strong><span>${esc(err.code||'COREAI_ERROR')}</span></div></div><p>${esc(err.message||'The operation failed.')}</p><div class="error-actions">${retry?`<button class="retry-btn" data-retry="${index}">Retry</button>`:''}<span class="muted">No successful result was fabricated.</span></div></div>`}
function renderChat(){
  const c=$('#content');
  if(!state.messages.length){c.innerHTML=`<div class="empty-state"><div class="empty-state-inner"><div class="core-orb">C</div><h1>How can I help you?</h1><p>Ask CORE-AI anything, or choose a workspace from the sidebar.</p></div></div>`;return}
  c.innerHTML=state.messages.map((m,i)=>{
    if(m.kind==='error')return errorMarkup(m.error,i);
    if(m.kind==='working')return workingMarkup();
    const user=m.role==='user';return `<article class="msg-row ${user?'user':'assistant'}">${user?'':`<div class="avatar">C</div>`}<div class="message-body"><div class="msg-meta">${user?'You':'CORE-AI'}</div><div class="msg-text">${esc(m.text)}</div></div></article>`;
  }).join('');
  requestAnimationFrame(()=>{const w=$('#chatScroll');w.scrollTop=w.scrollHeight});
}
function startWorking(){
  state.busy=true;state.workingPhase=0;state.messages=state.messages.filter(m=>m.kind!=='working');state.messages.push({kind:'working',role:'assistant'});render();
  const timer=setInterval(()=>{if(!state.busy){clearInterval(timer);return}state.workingPhase=(state.workingPhase+1)%phases.length;const p=$('#workingPhase');if(p)p.textContent=phases[state.workingPhase]},1100);return timer;
}
async function renderWorkspace(){
  renderNav();renderComposer();
  $('#title').textContent=state.section==='chat'?'New chat':(nav.find(x=>x[0]===state.section)?.[1]||'CORE-AI');
  if(['chat','think','codex'].includes(state.section)){renderChat();return}
  const c=$('#content');
  try{
    if(state.section==='projects'){const j=await api('/projects');c.innerHTML=card('Projects',j.map(x=>`<div class="row"><span>${esc(x.name)}</span><span class="muted">${esc(x.root)}</span></div>`).join('')||'<span class="muted">No projects yet.</span>');return}
    if(state.section==='recent'){await loadRecent();c.innerHTML=card('Recent',state.recent.map(x=>`<div class="row"><span>${esc(x.title)}</span><span class="pill">${esc(x.id.slice(0,8))}</span></div>`).join('')||'<span class="muted">No conversations yet.</span>');return}
    if(state.section==='library'){const j=await api('/library');c.innerHTML=card('Library',j.map(x=>`<div class="row"><span>${esc(x.name)}</span><span class="pill">${esc(x.hash?.slice(0,12)||'artifact')}</span></div>`).join('')||'<span class="muted">No library artifacts.</span>');return}
    if(state.section==='memory'){const j=await api('/memory/search?q=');c.innerHTML=card('Memory',j.map(x=>`<div class="row"><span>${esc(x.content)}</span><span class="pill">${esc(x.scope)}</span></div>`).join('')||'<span class="muted">No memory entries.</span>');return}
    if(state.section==='tasks'){const j=await api('/tasks');c.innerHTML=card('Tasks',j.map(x=>`<div class="row"><span>${esc(x.name)}</span><span class="pill">${esc(x.status)}</span></div>`).join('')||'<span class="muted">No tasks yet.</span>');return}
    if(state.section==='workflows'){const j=await api('/workflows');c.innerHTML=card('Workflows',j.map(x=>`<div class="row"><span>${esc(x.name)}</span><span class="pill">${esc(x.id.slice(0,8))}</span></div>`).join('')||'<span class="muted">No workflows yet.</span>');return}
    if(state.section==='scheduled'){const j=await api('/scheduled');c.innerHTML=card('Scheduled',j.map(x=>`<div class="row"><span>${esc(x.name)}</span><span class="pill">${x.enabled?'enabled':'disabled'}</span></div>`).join('')||'<span class="muted">No schedules yet.</span>');return}
    if(state.section==='plugins'){const j=await api('/plugins');c.innerHTML=card('Plugins',j.map(x=>`<div class="row"><span>${esc(x.path)}</span><span class="pill">${esc(x.state)}</span></div>`).join('')||'<span class="muted">No native plugins discovered.</span>');return}
    if(state.section==='git'){const j=await api('/git/status');c.innerHTML=card('Git status',`<pre>${esc(j.status||'(clean)')}</pre>`);return}
    if(state.section==='research'){const j=await api('/research?q=');c.innerHTML=card('Research',j.map(x=>`<div class="row"><span>${esc(x.title)}</span><span class="pill">${esc(x.provenance)}</span></div>`).join('')||'<span class="muted">No local research sources.</span>');return}
    if(state.section==='diagnostics'){const [d,cap]=await Promise.all([api('/doctor'),api('/capabilities')]);c.innerHTML=card('Doctor',`<pre>${esc(JSON.stringify(d,null,2))}</pre>`)+card('Capabilities',`<pre>${esc(JSON.stringify(cap,null,2))}</pre>`);return}
    const labels={maps:'Maps',images:'Images',files:'Files',documents:'Documents',agents:'Agents',settings:'Settings'};
    c.innerHTML=card(labels[state.section]||'CORE-AI',`<div class="muted">This workspace uses the same native CORE-AI service authority. No sample data is generated when a backend is unavailable.</div>`);
  }catch(e){c.innerHTML=card('Failed',`<div class="error-card"><div class="error-head"><span class="error-icon">!</span><div><strong>${esc(e.code||'COREAI_ERROR')}</strong><span>${esc(e.message)}</span></div></div></div>`)}
}
async function loadRecent(){try{const j=await api('/conversations/recent');state.recent=Array.isArray(j)?j:[]}catch{state.recent=[]}renderRecentList()}
function renderRecentList(){const box=$('#recentList');box.innerHTML=state.recent.slice(0,18).map(x=>`<button class="recent-item" title="${esc(x.title)}" data-recent-id="${esc(x.id)}">${esc(x.title||'Untitled conversation')}</button>`).join('')||'<div class="muted" style="padding:6px 10px">No recent chats</div>';box.querySelectorAll('[data-recent-id]').forEach(b=>b.onclick=()=>{state.section='chat';render()})}
async function refresh(){
  try{
    const s=await api('/status');
    const publicMode=location.hostname.endsWith('onrender.com') || location.hostname!=='127.0.0.1' && location.hostname!=='localhost';

    const providerStatus=s.ollama_reachable
      ? `AI READY � ${s.model||'model'}`
      : (publicMode ? 'AI provider not configured' : 'AI provider unavailable');

    $('#status').textContent=`CORE-AI Online � ${providerStatus}`;

    $('#storageLabel').textContent=
      publicMode ? 'Public web'
      : (s.storage_shard ? `Local � ${s.storage_shard}` : 'Local storage');

    const model=$('#model');
    model.innerHTML='';

    const opt=document.createElement('option');

    if(s.ollama_reachable && s.model){
      opt.value=s.model;
      opt.textContent=s.model;
      opt.disabled=false;
    }else{
      opt.value='';
      opt.textContent='No AI provider';
      opt.disabled=true;
    }

    model.appendChild(opt);
  }catch(e){
    $('#status').textContent=`CORE-AI Offline � ${e.message}`;
    $('#storageLabel').textContent='Connection unavailable';

    const model=$('#model');
    model.innerHTML='';
    const opt=document.createElement('option');
    opt.textContent='Backend offline';
    opt.disabled=true;
    model.appendChild(opt);
  }
}
async function sendMessage(){
  if(state.busy)return;const input=$('#input'),text=input.value.trim();if(!text)return;
  const mode=state.think?'think':'chat';state.lastRequest={prompt:text,mode};state.messages.push({role:'user',text});input.value='';input.style.height='auto';renderComposer();
  const timer=startWorking();
  try{const j=await post('/chat',{mode,prompt:text});state.messages=state.messages.filter(m=>m.kind!=='working');state.messages.push({role:'assistant',text:j.text||''})}
  catch(err){state.messages=state.messages.filter(m=>m.kind!=='working');state.messages.push({kind:'error',error:{message:err.message,code:err.code,retryable:err.retryable}})}
  finally{clearInterval(timer);state.busy=false;await renderWorkspace()}
}
async function retryRequest(index){const item=state.messages[index];if(!item||item.kind!=='error'||!state.lastRequest)return;state.messages.splice(index,1);$('#input').value=state.lastRequest.prompt;await sendMessage()}
function closeMenus(except){document.querySelectorAll('.popup-menu').forEach(m=>{if(m!==except)m.hidden=true});document.querySelectorAll('[aria-haspopup="menu"]').forEach(b=>{if(!except||b.nextElementSibling!==except)b.setAttribute('aria-expanded','false')})}
function openMenu(button,menu){const willOpen=menu.hidden;closeMenus(willOpen?menu:null);menu.hidden=!willOpen;button.setAttribute('aria-expanded',String(willOpen))}
function closeSidebarMobile(){document.body.classList.remove('sidebar-open')}
$('#plusBtn').onclick=e=>{e.stopPropagation();openMenu($('#plusBtn'),$('#plusMenu'))};
$('#functionBtn').onclick=e=>{e.stopPropagation();openMenu($('#functionBtn'),$('#functionMenu'))};
document.addEventListener('click',()=>closeMenus());
document.querySelectorAll('#plusMenu [data-plus-action]').forEach(b=>b.onclick=async e=>{e.stopPropagation();closeMenus();state.section={plugin:'plugins',tools:'tools',files:'files',images:'images',library:'library'}[b.dataset.plusAction]||'chat';await renderWorkspace()});
document.querySelectorAll('#functionMenu [data-fn]').forEach(b=>b.onclick=async e=>{e.stopPropagation();closeMenus();const a=b.dataset.fn;if(a==='think'){state.think=true;state.section='think'}else if(a==='new-chat'){$('#newBtn').click();return}else if(a==='reset-chat'){$('#resetBtn').click();return}else{state.think=false;state.section=a}await renderWorkspace()});
function setupMic(){const btn=$('#micBtn');const Speech=window.SpeechRecognition||window.webkitSpeechRecognition;if(!Speech){btn.title='Voice input unavailable';btn.onclick=()=>{$('#status').textContent='Voice input unavailable in this browser.'};return}const rec=new Speech();rec.lang=navigator.language||'en-US';rec.interimResults=true;rec.continuous=false;rec.onstart=()=>{state.mic=true;btn.textContent='■';btn.classList.add('active')};rec.onresult=e=>{let t='';for(let i=e.resultIndex;i<e.results.length;i++)t+=e.results[i][0].transcript;$('#input').value=t;$('#input').dispatchEvent(new Event('input'))};rec.onerror=e=>{$('#status').textContent=`Voice input error: ${e.error}`};rec.onend=()=>{state.mic=false;btn.textContent='⌁';btn.classList.remove('active');$('#input').focus()};btn.onclick=()=>state.mic?rec.stop():rec.start()}
setupMic();
$('#newBtn').onclick=async()=>{if(state.busy)return;try{await post('/conversations/new',{});state.messages=[];state.section='chat';state.think=false;$('#input').value='';await renderWorkspace()}catch(e){state.messages.push({kind:'error',error:e});renderChat()}};
$('#resetBtn').onclick=async()=>{if(state.busy)return;try{await post('/conversations/reset',{});state.messages=[];state.think=false;await renderWorkspace()}catch(e){state.messages.push({kind:'error',error:e});renderChat()}};
$('#settingsShortcut').onclick=()=>{state.section='settings';renderWorkspace()};
$('#searchBtn').onclick=()=>{$('#input').focus();$('#status').textContent='Search your recent chats from the sidebar.';$('#chatScroll').scrollTop=0};
$('#openSidebar').onclick=()=>document.body.classList.add('sidebar-open');
$('#closeSidebar').onclick=closeSidebarMobile;$('#sidebarBackdrop').onclick=closeSidebarMobile;
$('#composer').onsubmit=e=>{e.preventDefault();sendMessage()};
$('#input').addEventListener('keydown',e=>{if(e.key==='Enter'&&!e.shiftKey){e.preventDefault();sendMessage()}});
$('#input').addEventListener('input',()=>{const el=$('#input');el.style.height='auto';el.style.height=Math.min(el.scrollHeight,190)+'px';renderComposer()});
$('#thinkBtn').onclick=()=>{if(state.busy)return;state.think=!state.think;state.section=state.think?'think':'chat';renderWorkspace()};
$('#content').addEventListener('click',e=>{const b=e.target.closest('[data-retry]');if(b)retryRequest(Number(b.dataset.retry))});
document.addEventListener('keydown',e=>{if((e.ctrlKey||e.metaKey)&&e.key.toLowerCase()==='k'){e.preventDefault();$('#input').focus()}});
(async()=>{await loadRecent();await renderWorkspace();await refresh();setInterval(refresh,5000)})();
