(() => {
  'use strict';

  const K = { chats:'coreai.nav.chats.v1', current:'coreai.nav.current.v1', settings:'coreai.nav.settings.v1', draft:'coreai.nav.draft.v1', users:'coreai.auth.users.v1', session:'coreai.auth.session.v1', googleCx:'coreai.google.cx.v1' };
  const state = { chats:load(K.chats,[]), current:localStorage.getItem(K.current)||'', model:'chat', think:false, busy:false, files:[], settings:{rememberDraft:true}, authMode:'login' };
  const $=id=>document.getElementById(id);
  const el={
    sidebar:$('sidebar'),chatList:$('chatList'),emptyHistory:$('emptyHistory'),messageScroll:$('messageScroll'),welcome:$('welcome'),messages:$('messages'),
    input:$('messageInput'),send:$('sendBtn'),think:$('thinkToggle'),thinkState:$('thinkState'),loader:$('loadingBanner'),loaderLabel:$('loadingLabel'),
    attachments:$('attachments'),file:$('fileInput'),statusPill:$('statusPill'),statusText:$('statusText'),backendHint:$('backendHint'),chatView:$('chatView'),workspaceView:$('workspaceView'),
    workspaceKicker:$('workspaceKicker'),workspaceTitle:$('workspaceTitle'),workspaceDescription:$('workspaceDescription'),workspaceBody:$('workspaceBody'),toast:$('toast'),profileName:$('profileName'),profileStatus:$('profileStatus'),authOverlay:$('authOverlay'),authForm:$('authForm'),authUsername:$('authUsername'),authEmail:$('authEmail'),authEmailWrap:$('authEmailWrap'),authPassword:$('authPassword'),authMessage:$('authMessage'),authTitle:$('authTitle'),authSubtitle:$('authSubtitle'),authSubmit:$('authSubmit'),loginTab:$('loginTab'),registerTab:$('registerTab'),googleSetupOverlay:$('googleSetupOverlay'),googleCxInput:$('googleCxInput'),googleSetupMessage:$('googleSetupMessage')
  };

  let googleCsePromise=null;
  let toastTimer=0;

  init();

  function load(key,fallback){try{const v=JSON.parse(localStorage.getItem(key)||'');return v==null?fallback:v}catch(_){return fallback}}
  function save(){localStorage.setItem(K.chats,JSON.stringify(state.chats));localStorage.setItem(K.current,state.current)}
  function makeId(){return Date.now().toString(36)+'-'+Math.random().toString(36).slice(2,9)}
  function makeChat(){return{id:makeId(),title:'New chat',createdAt:Date.now(),updatedAt:Date.now(),messages:[]}}
  function currentChat(){return state.chats.find(c=>c.id===state.current)}
  function ensureCurrent(){if(state.current&&currentChat())return;if(!state.chats.length){const c=makeChat();state.chats=[c];state.current=c.id}else{state.chats.sort((a,b)=>b.updatedAt-a.updatedAt);state.current=state.chats[0].id}save()}

  function init(){
    Object.assign(state.settings,load(K.settings,{}));
    ensureCurrent(); bind(); setAuthMode('login'); syncProfile(); render(); health(); preloadGoogleSearch();
  }

  function bind(){
    $('newChatBtn').onclick=()=>newChat(true);
    $('brandBtn').onclick=()=>showView('chat');
    $('searchChatsBtn').onclick=openSearch;
    $('clearChatsBtn').onclick=clearChats;
    $('moreWorkspaceBtn').onclick=toggleMoreWorkspace;
    $('recentCollapseBtn').onclick=toggleRecentCollapse;
    $('profileBtn').onclick=openProfile; $('loginTab').onclick=()=>setAuthMode('login'); $('registerTab').onclick=()=>setAuthMode('register'); $('authClose').onclick=closeAuth; $('authForm').onsubmit=submitAuth; $('closeGoogleSetup').onclick=closeGoogleSetup; $('saveGoogleCx').onclick=saveGoogleCx; $('googleDocsBtn').onclick=()=>window.open('https://programmablesearchengine.google.com/','_blank','noopener');
    $('openSidebar').onclick=()=>el.sidebar.classList.add('open');
    $('closeSidebar').onclick=()=>el.sidebar.classList.remove('open');
    $('workspaceNav').onclick=e=>{const b=e.target.closest('.nav-item[data-view]');if(!b)return;showView(b.dataset.view);el.sidebar.classList.remove('open')};
    $('composer').onsubmit=e=>{e.preventDefault();send()};
    el.input.oninput=onInput;
    el.input.onkeydown=e=>{if(e.key==='Enter'&&!e.shiftKey){e.preventDefault();send()}};
    el.think.onclick=()=>{state.think=!state.think;syncThink();toast(state.think?'Think is ON':'Think is OFF')};
    $('attachBtn').onclick=()=>el.file.click();
    el.file.onchange=()=>{state.files=Array.from(el.file.files||[]);renderFiles()};
    $('modelBtn').onclick=()=>togglePopover($('modelPop'));
    $('moreBtn').onclick=()=>togglePopover($('morePop'));
    document.querySelectorAll('#modelPop [data-model]').forEach(b=>b.onclick=()=>{state.model=b.dataset.model;const label={chat:'Auto',think:'Think',code:'Code',agent:'Agent'}[state.model];$('modelName').textContent=label;if(state.model==='think')state.think=true;syncThink();closePopovers();toast('Mode: '+label)});
    $('renameBtn').onclick=rename;
    $('exportBtn').onclick=exportChat;
    $('resetBtn').onclick=resetChat;
    $('closeSearch').onclick=closeSearch;
    $('searchOverlay').onclick=e=>{if(e.target.id==='searchOverlay')closeSearch()};
    $('searchInput').oninput=renderSearch;
    document.querySelectorAll('[data-quick]').forEach(b=>b.onclick=()=>showView(b.dataset.quick));
    document.addEventListener('click',e=>{if(!e.target.closest('.popover')&&!e.target.closest('#modelBtn')&&!e.target.closest('#moreBtn'))closePopovers()});
    document.addEventListener('keydown',e=>{if((e.ctrlKey||e.metaKey)&&e.key.toLowerCase()==='k'){e.preventDefault();openSearch()}if(e.key==='Escape'){closeSearch();closePopovers()}});
  }

  function toggleMoreWorkspace(){
    const btn=$('moreWorkspaceBtn');
    const box=$('moreWorkspace');
    const open=box.classList.contains('hidden');
    box.classList.toggle('hidden',!open);
    btn.classList.toggle('expanded',open);
    btn.setAttribute('aria-expanded',String(open));
  }
  function toggleRecentCollapse(){
    const section=$('recentSection');
    const btn=$('recentCollapseBtn');
    const collapsed=section.classList.toggle('collapsed');
    btn.setAttribute('aria-expanded',String(!collapsed));
  }

  function render(){renderChats();renderMessages();renderFiles();syncThink();onInput()}
  function renderChats(){el.chatList.innerHTML='';const list=[...state.chats].sort((a,b)=>b.updatedAt-a.updatedAt);el.emptyHistory.classList.toggle('hidden',list.length>0);list.slice(0,100).forEach(c=>{const b=document.createElement('button');b.className='chat-item'+(c.id===state.current?' active':'');b.textContent=c.title||'New chat';b.title=c.title||'New chat';b.onclick=()=>switchChat(c.id);el.chatList.appendChild(b)})}
  function renderMessages(){
    const c=currentChat();
    const msgs=c?c.messages:[];
    el.welcome.classList.toggle('hidden',msgs.length>0);
    el.messages.innerHTML='';
    msgs.forEach(m=>{
      const a=document.createElement('article');a.className='message '+m.role;
      const inner=document.createElement('div');inner.className='message-inner';
      const av=document.createElement('div');av.className='message-avatar';av.textContent=m.role==='user'?'U':'C';
      const box=document.createElement('div');box.className='message-box';
      const author=document.createElement('div');author.className='message-author';author.textContent=m.role==='user'?'You':'CORE-AI';box.appendChild(author);
      if(m.image){
        const shell=document.createElement('div');shell.className='chat-tool-shell';
        const head=document.createElement('div');head.className='chat-tool-head';
        const ic=document.createElement('span');ic.className='chat-tool-icon';ic.textContent='IM';
        const label=document.createElement('strong');label.textContent=m.image.status==='generating'?'Creating image…':m.image.status==='ready'?'Image ready':'Image generation';
        head.append(ic,label);shell.appendChild(head);
        if(m.image.status==='generating'){const l=document.createElement('div');l.className='tool-loading';l.innerHTML='<span class="search-spinner"></span><span>Generating image…</span>';shell.appendChild(l)}
        if(m.image.url){const img=document.createElement('img');img.className='generated-image';img.src=m.image.url;img.alt=m.image.prompt||'Generated image';img.loading='lazy';img.onerror=()=>{img.replaceWith(makeText('The image URL could not be displayed in this browser.'))};shell.appendChild(img);const row=document.createElement('div');row.className='artifact-actions';const dl=action('Download image',()=>downloadImage(m.image.url,'coreai-image'));row.appendChild(dl);shell.appendChild(row)}
        if(m.image.status==='error'&&m.image.error){const err=document.createElement('div');err.className='core-inline-no-sources';err.textContent=m.image.error;shell.appendChild(err)}
        box.appendChild(shell);
      }else if(m.map){
        const shell=document.createElement('div');shell.className='chat-tool-shell';
        const head=document.createElement('div');head.className='chat-tool-head';const ic=document.createElement('span');ic.className='chat-tool-icon';ic.textContent='MP';const label=document.createElement('strong');label.textContent=m.map.status==='searching'?'Searching Maps…':'Maps';head.append(ic,label);shell.appendChild(head);
        if(m.map.status==='searching'){const l=document.createElement('div');l.className='tool-loading';l.innerHTML='<span class="search-spinner"></span><span>Finding places…</span>';shell.appendChild(l)}
        if(m.map.url){const a=document.createElement('a');a.className='core-inline-source';a.href=m.map.url;a.target='_blank';a.rel='noopener noreferrer';const fav=document.createElement('div');fav.className='core-inline-source-icon map-favicon';fav.textContent='M';const tw=document.createElement('span');tw.className='core-inline-source-text';const t=document.createElement('strong');t.textContent=m.map.query;const d=document.createElement('small');d.textContent='Google Maps';tw.append(t,d);a.append(fav,tw);shell.appendChild(a)}
        box.appendChild(shell);
      }else if(m.plugin){
        const shell=document.createElement('div');shell.className='chat-tool-shell';const head=document.createElement('div');head.className='chat-tool-head';const ic=document.createElement('span');ic.className='chat-tool-icon';ic.textContent=m.plugin.name.slice(0,2).toUpperCase();const label=document.createElement('strong');label.textContent=m.plugin.status==='searching'||m.plugin.status==='checking'?m.plugin.name+'…':m.plugin.name;head.append(ic,label);shell.appendChild(head);
        if(m.plugin.results?.length){const list=document.createElement('div');list.className='core-inline-sources';m.plugin.results.forEach(r=>{const a=document.createElement('a');a.className='core-inline-source';a.href=r.url;a.target='_blank';a.rel='noopener noreferrer';const fav=document.createElement('img');fav.className='core-inline-source-icon';fav.src='https://github.githubassets.com/favicons/favicon.svg';fav.alt='';const tw=document.createElement('span');tw.className='core-inline-source-text';const t=document.createElement('strong');t.textContent=r.name;const d=document.createElement('small');d.textContent=r.description||'GitHub repository';tw.append(t,d);a.append(fav,tw);list.appendChild(a)});shell.appendChild(list)}
        if(m.plugin.raw){const pre=document.createElement('pre');pre.className='tool-output';pre.textContent=m.plugin.raw;shell.appendChild(pre)}
        if(m.plugin.models?.length){const p=document.createElement('div');p.className='tool-output';p.textContent='Models: '+m.plugin.models.join(', ');shell.appendChild(p)}
        if(m.plugin.status==='error'&&m.plugin.error){const err=document.createElement('div');err.className='core-inline-no-sources';err.textContent=m.plugin.error;shell.appendChild(err)}
        box.appendChild(shell);
      }else if(m.search){
        const searchShell=document.createElement('div');searchShell.className='chat-search-shell';
        const line=document.createElement('div');line.className='chat-inline-search-line';
        const icon=document.createElement('span');icon.className='chat-search-google-icon';icon.textContent='G';
        const label=document.createElement('span');label.textContent=m.searchLoading!==false?(m.searchStatus||'Searching Google…'):(m.searchStatus||'Web sources');
        line.append(icon,label);
        if(m.searchLoading!==false){
          const spin=document.createElement('span');spin.className='search-spinner';line.appendChild(spin);
        }
        searchShell.appendChild(line);

        if(Array.isArray(m.searchResults)&&m.searchResults.length){
          const list=document.createElement('div');list.className='core-inline-sources';
          m.searchResults.slice(0,6).forEach(r=>{
            const a=document.createElement('a');a.className='core-inline-source';a.href=r.url;a.target='_blank';a.rel='noopener noreferrer';
            const fav=document.createElement('img');fav.className='core-inline-source-icon';fav.src='https://www.google.com/s2/favicons?domain='+encodeURIComponent(r.host)+'&sz=32';fav.alt='';fav.loading='lazy';
            fav.onerror=()=>{fav.style.visibility='hidden'};
            const textWrap=document.createElement('span');textWrap.className='core-inline-source-text';
            const t=document.createElement('strong');t.textContent=r.title||r.host;
            const d=document.createElement('small');d.textContent=r.host;
            textWrap.append(t,d);
            if(r.snippet){const sn=document.createElement('em');sn.textContent=r.snippet;sn.className='core-inline-source-snippet';textWrap.append(sn)}
            a.append(fav,textWrap);list.appendChild(a);
          });
          searchShell.appendChild(list);
        }else if(m.searchLoading===false){
          const none=document.createElement('div');none.className='core-inline-no-sources';none.textContent=m.searchError||'No web sources found.';searchShell.appendChild(none);
        }
        if(m.pending){
          const l=document.createElement('div');l.className='loading-state inline-answer-loading';
          if(state.think){const t=document.createElement('span');t.textContent='Thinking';t.style.fontSize='11px';l.appendChild(t)}
          const d=document.createElement('span');d.className='loading-dots';d.innerHTML='<i></i><i></i><i></i>';l.appendChild(d);
          searchShell.appendChild(l);
        }
        box.appendChild(searchShell);
      }else if(m.pending){
        const l=document.createElement('div');l.className='loading-state';
        if(state.think){const t=document.createElement('span');t.textContent='Thinking';t.style.fontSize='11px';l.appendChild(t)}
        const d=document.createElement('span');d.className='loading-dots';d.innerHTML='<i></i><i></i><i></i>';l.appendChild(d);box.appendChild(l);
      }else{
        const t=document.createElement('div');t.className='message-text';t.innerHTML=renderRichLinks(m.content||'');box.appendChild(t);
        const artifacts=extractDownloadableArtifacts(m.content||'');
        if(artifacts.length){
          const bar=document.createElement('div');bar.className='artifact-actions';
          artifacts.slice(0,4).forEach(a=>{
            const b=document.createElement('button');b.type='button';b.className='tool-btn tiny';b.textContent='Download '+a.name;
            b.onclick=()=>downloadText(a.name,a.content,a.type);bar.appendChild(b);
          });
          box.appendChild(bar);
        }
      }
      inner.appendChild(av);inner.appendChild(box);a.appendChild(inner);el.messages.appendChild(a);
    });
    requestAnimationFrame(()=>el.messageScroll.scrollTop=el.messageScroll.scrollHeight);
  }
  function renderFiles(){el.attachments.innerHTML='';state.files.forEach((f,i)=>{const x=document.createElement('div');x.className='attachment';const s=document.createElement('span');s.textContent=f.name;const b=document.createElement('button');b.type='button';b.textContent='x';b.onclick=()=>{state.files.splice(i,1);renderFiles()};x.append(s,b);el.attachments.appendChild(x)})}
  function onInput(){el.input.style.height='auto';el.input.style.height=Math.min(el.input.scrollHeight,180)+'px';el.send.disabled=state.busy||!el.input.value.trim();if(state.settings.rememberDraft)localStorage.setItem(K.draft,el.input.value)}
  function syncThink(){el.think.classList.toggle('on',state.think);el.think.setAttribute('aria-pressed',String(state.think));el.thinkState.textContent=state.think?'ON':'OFF'}

  function detectRoute(text){
    const raw=String(text||'').trim();
    const low=raw.toLowerCase();
    const explicit=low.match(/^#([a-z0-9_-]+)\b\s*(.*)$/i);
    if(explicit){
      const tag=explicit[1].toLowerCase();
      const map={github:'github',google:'search',maps:'maps',map:'maps',ollama:'ollama',git:'git',image:'image',images:'image',codex:'codex',code:'code',game:'game'};
      if(map[tag])return {type:map[tag],label:tag==='search'?'Google Search':tag[0].toUpperCase()+tag.slice(1),query:explicit[2].trim()||raw};
    }
    const image=/\b(create|make|generate|draw|design|buat|jana|hasilkan)\b[\s\w-]*(image|picture|photo|poster|wallpaper|gambar|imej)\b|\b(image|gambar|imej)\b.*\b(create|buat|generate|jana|hasil)\b/i.test(raw);
    if(image)return {type:'image',label:'Image',query:raw};
    const map=/\b(map|maps|directions|direction|route|location|near me|nearby|peta|arah|lokasi|jalan ke|cari sekolah|restaurant near|kedai dekat)\b/i.test(raw);
    if(map)return {type:'maps',label:'Maps',query:raw};
    const code=/\b(codex|code|coding|program|programming|javascript|typescript|python|c\+\+|cmake|html|css|script|debug|bug|fix|compile|compiler|repository|repo|git|github|make a game|buat game|create a game)\b/i.test(raw);
    if(code)return {type:low.includes('codex')?'codex':(low.includes('game')||low.includes('buat game')||low.includes('create a game')?'game':'code'),label:low.includes('game')?'Game Studio':(low.includes('codex')?'Codex':'Code'),query:raw};
    const web=/\b(search|find|look up|latest|today|news|tutorial|youtube|video|website|link|source|research|recent|current|semasa|terkini|berita|tutorial|carikan|cari)\b/i.test(raw);
    if(web)return {type:'search',label:'Web Search',query:raw};
    return {type:'chat',label:'CORE-AI',query:raw};
  }

  async function send(){
    if(state.busy)return;const text=el.input.value.trim();if(!text)return;const c=currentChat();if(!c)return;
    c.messages.push({role:'user',content:text,createdAt:Date.now()});if(c.messages.filter(m=>m.role==='user').length===1)c.title=text.slice(0,48)||'New chat';c.updatedAt=Date.now();save();el.input.value='';localStorage.removeItem(K.draft);
    const route=detectRoute(text);
    state.busy=true;el.loaderLabel.textContent=route.label;el.loader.classList.remove('hidden');const pending={role:'assistant',content:'',pending:true,createdAt:Date.now(),route:route.type};c.messages.push(pending);setBusy(true);renderMessages();onInput();
    try{
      if(route.type==='image'){
        await runImageGeneration(route.query,pending);
      }else if(route.type==='maps'){
        await runMapRoute(route.query,pending);
      }else if(route.type==='search'){
        pending.search=true;pending.query=route.query;pending.searchLoading=true;pending.searchProvider='Google';pending.searchStatus='Searching Google…';save();renderMessages();
        runAutomaticGoogleSearch(pending).catch(()=>{});
        await chatAnswer(route.query,'chat',pending);
      }else if(route.type==='github'){
        await runGitHubRoute(route.query,pending);
      }else if(route.type==='ollama'){
        await runOllamaRoute(route.query,pending);
      }else if(route.type==='git'){
        await runGitRoute(route.query,pending);
      }else{
        const mode=state.think||state.model==='think'?'think':(route.type==='code'||route.type==='codex'||route.type==='game'||state.model==='code'?'code':state.model==='agent'?'agent':'chat');
        await chatAnswer(route.query,mode,pending,route.type==='game');
      }
    }catch(err){pending.content='CORE-AI error.\n\n'+(err&&err.message?err.message:String(err));pending.error=true;pending.pending=false}
    finally{c.updatedAt=Date.now();save();state.busy=false;state.files=[];el.file.value='';el.loader.classList.add('hidden');setBusy(false);render()}
  }

  async function chatAnswer(prompt,mode,pending,preferGame=false){
    const r=await fetch('/chat',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({prompt,mode}),cache:'no-store'});
    const raw=await r.text();if(!r.ok)throw new Error(raw||('HTTP '+r.status));
    const data=parse(raw);const answer=extract(data,raw);if(!answer)throw new Error('Backend returned no assistant message.');
    pending.content=answer;pending.pending=false;pending.error=false;pending.route=preferGame?'game':pending.route;
  }

  async function runImageGeneration(prompt,pending){
    pending.image={status:'generating',prompt};pending.pending=true;pending.content='';save();renderMessages();
    const attempts=[
      {path:'/image',payload:{prompt}},
      {path:'/image-create',payload:{prompt}}
    ];
    let last='Image generation endpoint is not available.';
    for(const attempt of attempts){
      try{
        const r=await fetch(attempt.path,{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify(attempt.payload),cache:'no-store'});
        const raw=await r.text();
        if(r.status===404){last='No image generation route was exposed by the backend.';continue}
        if(!r.ok)throw new Error(raw||('HTTP '+r.status));
        const d=parse(raw);
        const src=extractImageSource(d,raw);
        if(!src)throw new Error('Backend returned no usable image URL/data.');
        pending.image={status:'ready',prompt,url:src};pending.pending=false;pending.content='';pending.error=false;save();renderMessages();return;
      }catch(e){last=e.message||String(e)}
    }
    pending.image={status:'error',prompt,error:last};pending.content='Image generation is not available in the current CORE-AI backend.\n\n'+last;pending.pending=false;pending.error=true;save();renderMessages();
  }

  function extractImageSource(d,raw){
    const vals=[];
    if(d){
      vals.push(d.url,d.image_url,d.imageUrl,d.data,d.output,d.path,d.file,d.artifact&&d.artifact.url,d.result&&d.result.url,d.result&&d.result.image_url,d.result&&d.result.data);
      if(d.result&&typeof d.result==='string')vals.push(d.result);
      if(Array.isArray(d.images))vals.push(d.images[0]);
    }
    vals.push(raw.trim());
    for(const v of vals){
      if(typeof v!=='string'||!v.trim())continue;
      const t=v.trim();
      if(/^data:image\//i.test(t)||/^https?:\/\//i.test(t)||/^\/(?!\/)/.test(t))return t;
    }
    return '';
  }

  async function runMapRoute(query,pending){
    const q=query.replace(/^\s*(where|find|search|cari|show|tunjuk)\s+/i,'').trim()||query;
    pending.map={status:'searching',query:q};pending.pending=true;pending.content='';save();renderMessages();
    try{
      const r=await fetch('/maps?q='+encodeURIComponent(q),{cache:'no-store'});const raw=await r.text();
      if(!r.ok)throw new Error(raw||('HTTP '+r.status));
      pending.map={status:'ready',query:q,url:'https://www.google.com/maps/search/?api=1&query='+encodeURIComponent(q),raw};pending.content='I found a map result for “'+q+'”.';pending.pending=false;save();renderMessages();
    }catch(e){
      pending.map={status:'fallback',query:q,url:'https://www.google.com/maps/search/?api=1&query='+encodeURIComponent(q),error:e.message};pending.content='Maps backend is unavailable, so here is a direct Google Maps link for “'+q+'”.';pending.pending=false;pending.error=false;save();renderMessages();
    }
  }

  async function runGitHubRoute(query,pending){
    const q=query.trim()||'CORE-AI';pending.plugin={name:'GitHub',status:'searching',query:q};pending.pending=true;save();renderMessages();
    try{
      const r=await fetch('https://api.github.com/search/repositories?q='+encodeURIComponent(q)+'&per_page=6',{headers:{Accept:'application/vnd.github+json'},cache:'no-store'});const raw=await r.text();if(!r.ok)throw new Error(raw||('HTTP '+r.status));
      const d=JSON.parse(raw);pending.plugin={name:'GitHub',status:'ready',query:q,results:(d.items||[]).map(x=>({name:x.full_name,url:x.html_url,description:x.description||''}))};pending.content=`GitHub results for “${q}”.`;pending.pending=false;save();renderMessages();
    }catch(e){pending.plugin={name:'GitHub',status:'error',error:e.message};pending.content='GitHub search failed.\n\n'+e.message;pending.pending=false;pending.error=true;save();renderMessages()}
  }

  async function runOllamaRoute(query,pending){
    const model=query.trim();pending.plugin={name:'Ollama',status:'checking',query:model};pending.pending=true;save();renderMessages();
    try{const r=await fetch('http://127.0.0.1:11434/api/tags',{cache:'no-store'});if(!r.ok)throw new Error('HTTP '+r.status);const d=await r.json();const names=(d.models||[]).map(x=>x.name);pending.plugin={name:'Ollama',status:'ready',model,models:names};pending.content='Ollama is connected.'+(model?` Requested model: ${model}.`:'');pending.pending=false;save();renderMessages()}
    catch(e){pending.plugin={name:'Ollama',status:'error',error:e.message};pending.content='Ollama is unavailable on this device.\n\n'+e.message;pending.pending=false;pending.error=true;save();renderMessages()}
  }

  async function runGitRoute(query,pending){
    pending.plugin={name:'Git',status:'checking'};pending.pending=true;save();renderMessages();
    try{const r=await fetch('/git-status',{cache:'no-store'});const raw=await r.text();if(!r.ok)throw new Error(raw||('HTTP '+r.status));pending.plugin={name:'Git',status:'ready',raw};pending.content='Git status is ready.';pending.pending=false;save();renderMessages()}
    catch(e){pending.plugin={name:'Git',status:'error',error:e.message};pending.content='Git integration is unavailable.\n\n'+e.message;pending.pending=false;pending.error=true;save();renderMessages()}
  }

  function setBusy(v){el.input.disabled=v;$('attachBtn').disabled=v;if(v){el.statusPill.classList.remove('ok','err');el.statusText.textContent='Working'}else health()}
  async function health(){try{const r=await fetch('/status',{cache:'no-store'});if(!r.ok)throw new Error();const d=await r.json();el.statusPill.className='status-pill ok';el.statusText.textContent='Connected';el.backendHint.textContent=d.model?'Connected - '+d.model:'Local CORE-AI backend'}catch(_){el.statusPill.className='status-pill err';el.statusText.textContent='Offline';el.backendHint.textContent='Backend: not connected'}}

  function showView(name){
    el.chatView.classList.toggle('hidden',name!=='chat');el.workspaceView.classList.toggle('hidden',name==='chat');document.querySelectorAll('.nav-item').forEach(b=>b.classList.toggle('active',b.dataset.view===name));
    if(name!=='chat')renderWorkspace(name);else setTimeout(()=>el.input.focus(),0)
  }

  const META={
    think:['THINK','Think','Reasoning mode uses the real CORE-AI /chat think route.'],code:['CODING','Code','Coding mode uses the real CORE-AI /chat code route.'],codex:['CODEX','Codex','Codex workspace entry for coding tasks.'],agents:['AGENTS','Agents','Agent workspace for deterministic agent operations exposed by the native backend.'],projects:['PROJECTS','Projects','Project creation and listing from the native CORE-AI backend.'],memory:['MEMORY','Memory','Search the persistent memory store.'],library:['LIBRARY','Library','Library workspace for saved knowledge and documents.'],plugins:['PLUGINS','Plugins','Plugin discovery workspace.'],maps:['MAPS','Maps','Map and directions workspace.'],games:['GAME STUDIO','Game Studio','Create a playable browser game and download it as an HTML file.'],search:['SEARCH','Google Search','Run Google web search and keep results inside the chat.'],images:['IMAGES','Images','Image artifact workspace.'],files:['FILES','Files','Project file workspace.'],tasks:['TASKS','Tasks','Task workspace.'],workflows:['WORKFLOWS','Workflows','Workflow workspace.'],scheduled:['SCHEDULED','Scheduled','Scheduled jobs workspace.'],research:['RESEARCH','Research','Local research workspace.'],settings:['SETTINGS','Settings','Connection, capabilities and UI settings.']
  };

  const DEFAULT_GOOGLE_CX='776e66dac6c6c4221';
  const GOOGLE_SCRIPT_TIMEOUT_MS=12000;
  const GOOGLE_SEARCH_TIMEOUT_MS=12000;
  const googleChatSearches=new Map();
  function getGoogleCx(){return (localStorage.getItem(K.googleCx)||DEFAULT_GOOGLE_CX).trim()}

  function withTimeout(promise,ms,message){
    let timer=0;
    const timeout=new Promise((_,reject)=>{timer=window.setTimeout(()=>reject(new Error(message)),ms)});
    return Promise.race([promise,timeout]).finally(()=>window.clearTimeout(timer));
  }

  function preloadGoogleSearch(){
    ensureGoogleCse().catch(()=>{});
  }

  function installGoogleCallbacks(){
    window.__gcse=window.__gcse||{};
    window.__gcse.parsetags='explicit';
    window.__gcse.searchCallbacks={
      web:{
        starting:(gname,query)=>{
          const rec=googleChatSearches.get(gname);
          if(rec?.message){
            rec.message.searchLoading=true;
            rec.message.searchStatus='Searching Google…';
            save();
            renderMessages();
          }
          return query;
        },
        ready:(gname,q,promos,results)=>{
          const rec=googleChatSearches.get(gname);
          if(!rec?.message)return true;
          const mapped=Array.isArray(results)?results.slice(0,8).map(result=>{
            let url='';
            try{url=new URL(result.url||result.link||result.contextUrl||'').href}catch(_){return null}
            let u;
            try{u=new URL(url)}catch(_){return null}
            if(!/^https?:$/i.test(u.protocol))return null;
            const host=(String(result.visibleUrl||u.hostname||'').replace(/^www\./i,'').split('/')[0]||u.hostname.replace(/^www\./i,''));
            const title=String(result.titleNoFormatting||result.title||host||'Website').replace(/\s+/g,' ').trim();
            const snippet=String(result.content||result.snippet||'').replace(/\s+/g,' ').trim();
            return {title,url,host,snippet};
          }).filter(Boolean):[];
          rec.message.searchResults=mapped;
          rec.message.searchLoading=false;
          rec.message.searchStatus=mapped.length?`Found ${mapped.length} source${mapped.length===1?'':'s'}`:'No sources found';
          rec.message.searchError='';
          save();
          renderMessages();
          window.setTimeout(()=>googleChatSearches.delete(gname),0);
          return true; // Do not render Google's separate results panel.
        },
        rendered:()=>true
      }
    };
  }

  function ensureGoogleCse(){
    if(googleCsePromise)return googleCsePromise;
    googleCsePromise=new Promise((resolve,reject)=>{
      const cx=getGoogleCx();
      if(!cx){reject(new Error('Google Search Engine ID is not configured.'));return;}
      installGoogleCallbacks();

      const ready=()=>{
        if(window.google?.search?.cse?.element){resolve();return true}
        return false;
      };
      if(ready())return;

      const existing=document.querySelector('script[data-core-google-cse]');
      const script=existing||document.createElement('script');
      let settled=false;
      const finishOk=()=>{if(settled)return;settled=true;if(ready())resolve();else reject(new Error('Google Search loaded without the Search Element API.'))};
      const finishErr=()=>{if(settled)return;settled=true;reject(new Error('Google Search could not be loaded.'))};
      script.addEventListener('load',finishOk,{once:true});
      script.addEventListener('error',finishErr,{once:true});
      if(!existing){
        script.async=true;
        script.src='https://cse.google.com/cse.js?cx='+encodeURIComponent(cx);
        script.dataset.coreGoogleCse='1';
        document.head.appendChild(script);
      }
      // Some browser/cache paths do not fire a useful load event after the API has initialized.
      const pollStart=Date.now();
      const poll=()=>{
        if(settled)return;
        if(ready()){settled=true;resolve();return}
        if(Date.now()-pollStart>=GOOGLE_SCRIPT_TIMEOUT_MS){settled=true;reject(new Error('Google Search initialization timed out.'));return}
        window.setTimeout(poll,100);
      };
      poll();
    }).catch(err=>{
      googleCsePromise=null;
      throw err;
    });
    return googleCsePromise;
  }

  async function runAutomaticGoogleSearch(message){
    if(!message?.query)return;
    const gname='corechat_'+String(message.createdAt||Date.now())+'_'+Math.random().toString(36).slice(2,8);
    googleChatSearches.set(gname,{message});
    message.search=true;
    message.searchLoading=true;
    message.searchStatus='Searching Google…';
    message.searchError='';
    save();
    renderMessages();

    let helper=null;
    let completed=false;
    const fail=(status,error)=>{
      if(completed)return;
      completed=true;
      message.searchLoading=false;
      message.searchStatus=status;
      message.searchError=error;
      save();
      renderMessages();
      googleChatSearches.delete(gname);
    };
    try{
      await withTimeout(ensureGoogleCse(),GOOGLE_SCRIPT_TIMEOUT_MS,'Google Search initialization timed out.');
      if(!googleChatSearches.has(gname))return;
      helper=document.createElement('div');
      helper.id='core-google-helper-'+gname;
      helper.className='core-google-helper';
      helper.setAttribute('aria-hidden','true');
      document.body.appendChild(helper);

      google.search.cse.element.render({div:helper,tag:'searchresults-only',gname});
      const element=google.search.cse.element.getElement(gname);
      if(!element)throw new Error('Google Search element failed to initialize.');
      try {
        element.execute(message.query);
      } catch (_) {
        throw new Error('Google Search query could not be started.');
      }

      await withTimeout(new Promise((resolve,reject)=>{
        const check=()=>{
          if(!googleChatSearches.has(gname)){resolve();return}
          if(message.searchLoading===false){resolve();return}
          window.setTimeout(check,50);
        };
        check();
      }),GOOGLE_SEARCH_TIMEOUT_MS,'Google did not return results in time.');
      if(message.searchLoading===true)fail('Search timed out','Google did not return results in time.');
      else completed=true;
    }catch(err){
      fail('Search unavailable',err?.message||'Google Search is unavailable.');
    }finally{
      window.setTimeout(()=>helper?.remove(),500);
    }
  }

  function extractYouTubeId(url){
    try{
      if(url.hostname==='youtu.be') return url.pathname.slice(1).split('/')[0]||'';
      if(/youtube\.com$/i.test(url.hostname.replace(/^www\./,''))){
        if(url.pathname==='/watch') return url.searchParams.get('v')||'';
        const parts=url.pathname.split('/').filter(Boolean);
        if(parts[0]==='shorts' || parts[0]==='embed') return parts[1]||'';
      }
    }catch(_){ }
    return '';
  }

  function escapeHtml(s){return s.replace(/[&<>"']/g,c=>({'&':'&amp;','<':'&lt;','>':'&gt;','"':'&quot;',"'":'&#39;'}[c]))}
  function renderRichLinks(raw){
    const value=String(raw||'');
    const re=/(https?:\/\/[^\s<]+)/g;
    let out='';
    let last=0;
    let m;
    while((m=re.exec(value))!==null){
      out+=escapeHtml(value.slice(last,m.index));
      const clean=m[1].replace(/[),.;!?]+$/,'');
      let u;
      try{u=new URL(clean)}catch{out+=escapeHtml(m[1]);last=m.index+m[1].length;continue}
      const host=u.hostname.replace(/^www\./,'');
      const icon='https://www.google.com/s2/favicons?domain='+encodeURIComponent(u.hostname)+'&sz=64';
      out+='<a class="link-card" href="'+escapeHtml(u.href)+'" target="_blank" rel="noopener noreferrer">'
        +'<img class="link-icon" src="'+icon+'" alt="">'
        +'<span class="link-meta"><strong>'+escapeHtml(host)+'</strong><small>'+escapeHtml(u.href)+'</small></span>'
        +'</a>';
      last=m.index+m[1].length;
    }
    out+=escapeHtml(value.slice(last));
    return out;
  }
  function extractDownloadableArtifacts(raw){
    const src=String(raw||'');
    const out=[];
    const re=/```([\w.+-]*)\s*\n([\s\S]*?)```/g;
    let m;
    while((m=re.exec(src))!==null){
      const lang=(m[1]||'text').toLowerCase();
      const content=m[2].trimEnd();
      if(!content)continue;
      let name='code.txt',type='text/plain';
      if(lang==='html'||lang==='htm'){name='coreai-game.html';type='text/html'}
      else if(lang==='css'){name='coreai-style.css';type='text/css'}
      else if(lang==='js'||lang==='javascript'){name='coreai-script.js';type='text/javascript'}
      else if(lang==='json'){name='coreai-data.json';type='application/json'}
      else if(lang==='py'||lang==='python'){name='coreai-script.py';type='text/plain'}
      else if(lang==='cpp'||lang==='c++'){name='coreai-program.cpp';type='text/plain'}
      out.push({name,content,type});
    }
    return out;
  }
  function downloadText(name,content,type='text/plain'){
    const blob=new Blob([content],{type});
    const u=URL.createObjectURL(blob);
    const a=document.createElement('a');a.href=u;a.download=name;a.rel='noopener';document.body.appendChild(a);a.click();a.remove();
    window.setTimeout(()=>URL.revokeObjectURL(u),1500);
    toast('Downloaded '+name);
  }

  async function downloadImage(src,name){
    try{
      if(/^data:image\//i.test(src)){const a=document.createElement('a');a.href=src;a.download=name+'.png';document.body.appendChild(a);a.click();a.remove();toast('Downloaded image');return}
      const r=await fetch(src,{mode:'cors'});if(!r.ok)throw Error('HTTP '+r.status);const blob=await r.blob();const ext=(blob.type||'image/png').split('/')[1]||'png';const u=URL.createObjectURL(blob);const a=document.createElement('a');a.href=u;a.download=name+'.'+ext;document.body.appendChild(a);a.click();a.remove();window.setTimeout(()=>URL.revokeObjectURL(u),1500);toast('Downloaded image');
    }catch(e){window.open(src,'_blank','noopener');toast('Provider blocked direct download; image opened in a new tab.')}
  }

  function extractHtmlGame(text){
    const blocks=extractDownloadableArtifacts(text).filter(x=>x.name.endsWith('.html'));
    return blocks[0]?.content||'';
  }
  function starterGameHtml(title){
    const safe=String(title||'CORE-AI Game').replace(/[<&>]/g,'');
    return `<!doctype html><html lang="en"><head><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1"><title>${safe}</title><style>html,body{margin:0;height:100%;overflow:hidden;background:#0b0d10;color:#fff;font-family:system-ui}canvas{display:block;width:100%;height:100%}#ui{position:fixed;top:14px;left:14px;padding:10px 12px;border-radius:12px;background:rgba(0,0,0,.45);backdrop-filter:blur(8px)}small{opacity:.7}</style></head><body><div id="ui"><strong>${safe}</strong><br><small>WASD / Arrow Keys · collect the gold orbs</small><div>Score: <span id="score">0</span></div></div><canvas id="c"></canvas><script>const c=document.querySelector('#c'),x=c.getContext('2d'),scoreEl=document.querySelector('#score');let w,h,p={x:0,y:0,r:16,s:6},orb={x:0,y:0,r:10},score=0,keys={};function resize(){w=c.width=innerWidth;h=c.height=innerHeight;p.x=Math.max(p.x,20);p.y=Math.max(p.y,20);place()}function place(){orb.x=40+Math.random()*(w-80);orb.y=60+Math.random()*(h-100)}addEventListener('resize',resize);addEventListener('keydown',e=>keys[e.key.toLowerCase()]=1);addEventListener('keyup',e=>keys[e.key.toLowerCase()]=0);function reset(){p.x=w/2;p.y=h/2;place()}function loop(){requestAnimationFrame(loop);if(keys.w||keys.arrowup)p.y-=p.s;if(keys.s||keys.arrowdown)p.y+=p.s;if(keys.a||keys.arrowleft)p.x-=p.s;if(keys.d||keys.arrowright)p.x+=p.s;p.x=Math.max(p.r,Math.min(w-p.r,p.x));p.y=Math.max(p.r,Math.min(h-p.r,p.y));if(Math.hypot(p.x-orb.x,p.y-orb.y)<p.r+orb.r){score++;scoreEl.textContent=score;place()}x.clearRect(0,0,w,h);x.fillStyle='#10141b';x.fillRect(0,0,w,h);x.fillStyle='#f1c75b';x.beginPath();x.arc(orb.x,orb.y,orb.r,0,Math.PI*2);x.fill();x.fillStyle='#65a9ff';x.beginPath();x.arc(p.x,p.y,p.r,0,Math.PI*2);x.fill()}resize();reset();loop();</script></body></html>`;
  }

  async function hashPassword(value){const data=new TextEncoder().encode(value);const digest=await crypto.subtle.digest('SHA-256',data);return Array.from(new Uint8Array(digest)).map(x=>x.toString(16).padStart(2,'0')).join('')}
  function users(){return load(K.users,[])}
  function saveUsers(v){localStorage.setItem(K.users,JSON.stringify(v))}
  function openAuth(){el.authOverlay.classList.remove('hidden');el.authMessage.textContent='';setTimeout(()=>el.authUsername.focus(),0)}
  function closeAuth(){el.authOverlay.classList.add('hidden');el.authMessage.textContent='';el.authForm.reset()}
  function setAuthMode(mode){state.authMode=mode;const reg=mode==='register';el.loginTab.classList.toggle('active',!reg);el.registerTab.classList.toggle('active',reg);el.authTitle.textContent=reg?'Create your CORE-AI account':'Log in to CORE-AI';el.authSubtitle.textContent=reg?'Create a local browser account.':'Use your local browser account.';el.authSubmit.textContent=reg?'Register':'Log in';el.authEmail.required=reg;el.authEmailWrap.classList.toggle('hidden',!reg);el.authPassword.autocomplete=reg?'new-password':'current-password';el.authMessage.textContent=''}
  async function submitAuth(e){e.preventDefault();const username=el.authUsername.value.trim();const password=el.authPassword.value;const email=el.authEmail.value.trim();if(!username||!password){el.authMessage.textContent='Username and password are required.';return}if(state.authMode==='register'){if(!email){el.authMessage.textContent='Email is required.';return}const list=users();if(list.some(u=>u.username.toLowerCase()===username.toLowerCase())){el.authMessage.textContent='Username already exists.';return}const passwordHash=await hashPassword(password);list.push({username,email,passwordHash,createdAt:Date.now()});saveUsers(list);localStorage.setItem(K.session,username);closeAuth();syncProfile();toast('Account created');return}const passwordHash=await hashPassword(password);const found=users().find(u=>u.username.toLowerCase()===username.toLowerCase()&&u.passwordHash===passwordHash);if(!found){el.authMessage.textContent='Incorrect username or password.';return}localStorage.setItem(K.session,found.username);closeAuth();syncProfile();toast('Logged in')}
  function openProfile(){if(localStorage.getItem(K.session)){if(confirm('Log out of CORE-AI?')){localStorage.removeItem(K.session);syncProfile();toast('Logged out')}}else{openAuth();setAuthMode('login')}}
  function syncProfile(){const user=localStorage.getItem(K.session);el.profileName.textContent=user||'CORE user';el.profileStatus.textContent=user?'Logged in':'Log in / Register'}
  function renderGoogleSearch(card){
    const cx=localStorage.getItem(K.googleCx)||'';
    const stateLine=document.createElement('div');stateLine.className='google-status '+(cx?'ready':'');
    const dot=document.createElement('i');const label=document.createElement('span');label.textContent=cx?'Google Search Engine configured':'Direct Google links available';stateLine.append(dot,label);card.appendChild(stateLine);
    const input=textbox('Google Search...');
    input.onkeydown=e=>{if(e.key==='Enter'){e.preventDefault();runGoogleLink(input.value.trim(),card)}};
    const search=action('Google Search',()=>runGoogleLink(input.value.trim(),card));
    const setup=action(cx?'Change Google connection':'Connect Google Search',openGoogleSetup);setup.classList.add('secondary');
    card.append(input,search,setup);
    const box=result();box.textContent=cx?'Google results will appear below using your configured Search Engine ID.':'Use a direct Google result link now, or connect a Programmable Search Engine for in-app results.';card.appendChild(box);
    if(cx)loadGoogleCse(card,cx);
  }
  function runGoogleLink(q,card){
    const box=card.querySelector('.result-box');
    if(!q){box.textContent='Enter a search query.';return}
    box.innerHTML='<div class="google-searching"><span class="search-spinner"></span><span>Searching Google…</span></div>';
    const cx=getGoogleCx();
    if(!cx){box.textContent='Google Search Engine ID is not configured.';return}
    const msg={query:q,search:true,searchLoading:true,createdAt:Date.now()};
    runAutomaticGoogleSearch(msg).finally(()=>{
      const results=Array.isArray(msg.searchResults)?msg.searchResults:[];
      box.innerHTML='';
      if(results.length){
        const list=document.createElement('div');list.className='core-inline-sources';
        results.forEach(r=>{
          const a=document.createElement('a');a.className='core-inline-source';a.href=r.url;a.target='_blank';a.rel='noopener noreferrer';
          const fav=document.createElement('img');fav.className='core-inline-source-icon';fav.src='https://www.google.com/s2/favicons?domain='+encodeURIComponent(r.host)+'&sz=32';fav.alt='';
          const wrap=document.createElement('span');wrap.className='core-inline-source-text';
          const t=document.createElement('strong');t.textContent=r.title||r.host;
          const d=document.createElement('small');d.textContent=r.host;wrap.append(t,d);a.append(fav,wrap);list.appendChild(a);
        });
        box.appendChild(list);
      }else{box.textContent=msg.searchError||'No web sources found.'}
    });
  }
  function loadGoogleCse(card,cx,q=''){
    const host=document.createElement('div');host.className='search-cse gcse-search';host.style.marginTop='12px';if(q)host.setAttribute('data-query',q);card.appendChild(host);
    if(document.querySelector('script[data-core-google-cse]')){try{if(window.google&&google.search&&google.search.cse&&google.search.cse.element)google.search.cse.element.go()}catch(_){ }return}
    const script=document.createElement('script');script.async=true;script.src='https://cse.google.com/cse.js?cx='+encodeURIComponent(cx);script.dataset.coreGoogleCse='1';document.head.appendChild(script);
  }
  function openGoogleSetup(){el.googleCxInput.value=localStorage.getItem(K.googleCx)||'';el.googleSetupMessage.textContent='';el.googleSetupOverlay.classList.remove('hidden');setTimeout(()=>el.googleCxInput.focus(),0)}
  function closeGoogleSetup(){el.googleSetupOverlay.classList.add('hidden')}
  function saveGoogleCx(){const cx=el.googleCxInput.value.trim();if(!cx){el.googleSetupMessage.textContent='Enter your Search Engine ID (cx).';return}localStorage.setItem(K.googleCx,cx);googleCsePromise=null;closeGoogleSetup();preloadGoogleSearch();showView('search');toast('Google Search configured')}

  function renderGameStudio(card){
    const note=makeText('Describe the game. CORE-AI will ask its real backend for a self-contained HTML game, then create a real downloadable file. A local starter is used only when the backend is unavailable.');card.appendChild(note);
    const input=textarea('Example: Make a 2D space shooter with WASD movement, enemies and score.');card.appendChild(input);
    const row=document.createElement('div');row.className='tool-row';const generate=action('Create game',()=>createGameFromPrompt(input.value.trim(),card));const starter=action('Use starter game',()=>showGeneratedGame(starterGameHtml(input.value.trim()||'CORE-AI Game'),card,'coreai-starter-game.html'));row.append(generate,starter);card.appendChild(row);
    const out=result();out.textContent='No game generated yet.';card.appendChild(out);
  }
  async function createGameFromPrompt(prompt,card){
    const box=card.querySelector('.result-box');if(!prompt){toast('Describe the game first');return}
    box.innerHTML='<div class="google-searching"><span class="search-spinner"></span><span>Creating game…</span></div>';
    try{
      const r=await fetch('/chat',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({prompt:'Create a complete playable browser game based on this request. Return ONE self-contained HTML document inside a ```html code fence. No explanations outside the code fence. Request: '+prompt,mode:'code'}),cache:'no-store'});
      const raw=await r.text();if(!r.ok)throw Error(raw||('HTTP '+r.status));const d=parse(raw);const answer=extract(d,raw);const html=extractHtmlGame(answer);if(!html)throw Error('The backend did not return an HTML game code block.');showGeneratedGame(html,card,'coreai-game.html');
    }catch(e){box.innerHTML='';const p=makeText('Backend game generation unavailable: '+e.message);box.appendChild(p);const b=action('Use playable starter',()=>showGeneratedGame(starterGameHtml(prompt),card,'coreai-starter-game.html'));box.appendChild(b)}
  }
  function showGeneratedGame(html,card,name){
    const box=card.querySelector('.result-box');box.innerHTML='';const frame=document.createElement('iframe');frame.className='game-preview';frame.sandbox='allow-scripts';frame.srcdoc=html;box.appendChild(frame);const row=document.createElement('div');row.className='artifact-actions';const open=action('Open game',()=>{const w=window.open();if(!w)return;w.document.open();w.document.write(html);w.document.close()});const dl=action('Download game',()=>downloadText(name,html,'text/html'));row.append(open,dl);box.appendChild(row);toast('Game ready')
  }

  function renderWorkspace(name){
    const m=META[name]||['WORKSPACE',name,''];el.workspaceKicker.textContent=m[0];el.workspaceTitle.textContent=m[1];el.workspaceDescription.textContent=m[2];el.workspaceBody.innerHTML='';
    const card=document.createElement('div');card.className='tool-card';el.workspaceBody.appendChild(card);
    if(name==='think'){card.appendChild(makeText('Think is OFF or ON from the chat composer.'));card.appendChild(action('Turn Think ON',()=>{state.think=true;syncThink();showView('chat')}));return}
    if(name==='code'||name==='codex'){const input=textarea(name==='code'?'Describe your coding task':'Describe your Codex task');card.appendChild(input);card.appendChild(action('Run',()=>runChatMode(input.value.trim(),name==='code'?'code':'chat')));card.appendChild(result());return}
    if(name==='agents'){const input=textarea('Describe an agent task');card.appendChild(input);card.appendChild(action('Run Agent',()=>runChatMode(input.value.trim(),'agent')));card.appendChild(action('Run Multi-Agent',()=>runCommand('/multi-agent-run',input.value.trim(),'POST')));card.appendChild(result());return}
    if(name==='projects'){card.appendChild(makeText('Use the real project APIs already exposed by the native server.'));const input=textbox('Project name');card.appendChild(input);card.appendChild(action('Create project',()=>runCommand('/project-create',input.value.trim(),'POST',{name:input.value.trim()})));card.appendChild(action('List projects',()=>runCommand('/projects','','GET')));card.appendChild(result());return}
    if(name==='memory'){const input=textbox('Search memory');card.appendChild(input);card.appendChild(action('Search',()=>runCommand('/memory-search?q='+encodeURIComponent(input.value.trim()),'','GET')));card.appendChild(result());return}
    if(name==='plugins'){renderPluginExamples(card);return}
    if(name==='maps'){const input=textbox('Place to search');card.appendChild(input);card.appendChild(action('Search Maps',()=>runCommand('/maps?q='+encodeURIComponent(input.value.trim()),'','GET')));card.appendChild(action('Open Google Maps',()=>window.open('https://www.google.com/maps/search/?api=1&query='+encodeURIComponent(input.value.trim()),'_blank','noopener')));card.appendChild(result());return}
    if(name==='games'){renderGameStudio(card);return}
    if(name==='search'){renderGoogleSearch(card);return}
    if(name==='library'){card.appendChild(makeText('Library UI is present. Backend-specific library endpoints are called only if exposed.'));card.appendChild(action('Check capabilities',()=>runCommand('/capabilities','','GET')));card.appendChild(result());return}
    if(name==='images'){card.appendChild(makeText('Image workspace. Backend-specific image generation endpoint is called only when exposed.'));card.appendChild(action('Check capabilities',()=>runCommand('/capabilities','','GET')));card.appendChild(result());return}
    if(name==='files'){card.appendChild(makeText('Files workspace. Use the native file APIs through the backend when exposed.'));card.appendChild(action('Check capabilities',()=>runCommand('/capabilities','','GET')));card.appendChild(result());return}
    if(name==='tasks'||name==='workflows'||name==='scheduled'||name==='research'){card.appendChild(makeText('This workspace is visible and connected to CORE-AI routing. When the current native HTTP server does not expose a route, the result area reports the actual HTTP status rather than faking success.'));card.appendChild(action('Check capabilities',()=>runCommand('/capabilities','','GET')));card.appendChild(result());return}
    if(name==='settings'){const row=document.createElement('div');row.className='tool-row';row.innerHTML='<div><strong>Remember draft</strong><small>Keep unsent text in browser storage.</small></div>';const checkbox=document.createElement('input');checkbox.type='checkbox';checkbox.checked=state.settings.rememberDraft;checkbox.onchange=()=>{state.settings.rememberDraft=checkbox.checked;localStorage.setItem(K.settings,JSON.stringify(state.settings));if(!checkbox.checked)localStorage.removeItem(K.draft)};row.appendChild(checkbox);card.appendChild(row);card.appendChild(action('Check status',health));card.appendChild(action('Load capabilities',()=>runCommand('/capabilities','','GET')));card.appendChild(result());return}
  }

  async function renderPluginExamples(card){
    const intro=document.createElement('div');intro.className='google-note';intro.textContent='Real built-in connectors. Each action calls the real provider/browser API and reports failure instead of faking a connection.';card.appendChild(intro);
    const grid=document.createElement('div');grid.className='plugin-grid';card.appendChild(grid);
    const plugins=[
      {id:'github',name:'GitHub',icon:'GH',description:'Search public GitHub repositories directly from CORE-AI.',run:async()=>{
        const q=window.prompt('GitHub repository search'); if(!q)return;
        const box=card.querySelector('.plugin-result')||makePluginResult(card); box.textContent='GitHub: searching…';
        try{const r=await fetch('https://api.github.com/search/repositories?q='+encodeURIComponent(q)+'&per_page=8',{headers:{Accept:'application/vnd.github+json'}});if(!r.ok)throw Error('HTTP '+r.status);const d=await r.json();box.innerHTML='';(d.items||[]).slice(0,8).forEach(repo=>{const a=document.createElement('a');a.href=repo.html_url;a.target='_blank';a.rel='noopener noreferrer';a.className='plugin-result-link';a.textContent=repo.full_name+' — '+(repo.description||'No description');box.appendChild(a)})}catch(e){box.textContent='GitHub unavailable: '+e.message}
      }},
      {id:'google-search',name:'Google Search',icon:'G',description:'Use the configured Programmable Search Engine inside CORE-AI chat.',run:()=>openGoogleSetup()},
      {id:'google-maps',name:'Google Maps',icon:'MP',description:'Search a place with a real Maps URL.',run:()=>{const q=window.prompt('Maps search');if(q)window.open('https://www.google.com/maps/search/?api=1&query='+encodeURIComponent(q),'_blank','noopener')}},
      {id:'ollama',name:'Ollama',icon:'OL',description:'Check a local Ollama server and list installed models.',run:async()=>{const box=card.querySelector('.plugin-result')||makePluginResult(card);box.textContent='Ollama: connecting…';try{const r=await fetch('http://127.0.0.1:11434/api/tags',{cache:'no-store'});if(!r.ok)throw Error('HTTP '+r.status);const d=await r.json();box.textContent=(d.models||[]).map(m=>m.name).join('\n')||'Ollama connected; no local models reported.'}catch(e){box.textContent='Ollama unavailable: '+e.message}}},
      {id:'git',name:'Git',icon:'GIT',description:'Use the native CORE-AI Git endpoint when the local backend exposes it.',run:async()=>{const box=card.querySelector('.plugin-result')||makePluginResult(card);box.textContent='Git: checking native backend…';try{const r=await fetch('/git-status',{cache:'no-store'});const t=await r.text();if(!r.ok)throw Error('HTTP '+r.status+' '+t);box.textContent=t}catch(e){box.textContent='Git backend unavailable: '+e.message}}}
    ];
    plugins.forEach(p=>{
      const item=document.createElement('article');item.className='plugin-card';
      const ic=document.createElement('div');ic.className='plugin-icon';ic.textContent=p.icon;
      const body=document.createElement('div');body.style.minWidth='0';
      const h=document.createElement('h3');h.textContent=p.name;const d=document.createElement('p');d.textContent=p.description;
      const m=document.createElement('small');m.textContent='Built-in action';const acts=document.createElement('div');acts.className='plugin-actions';const b=document.createElement('button');b.className='tool-btn tiny';b.textContent='Run';b.onclick=p.run;acts.appendChild(b);body.append(h,d,m,acts);item.append(ic,body);grid.appendChild(item)
    });
    const out=makePluginResult(card);out.style.marginTop='12px';
  }
  function makePluginResult(card){let d=card.querySelector('.plugin-result');if(!d){d=document.createElement('div');d.className='result-box plugin-result';d.textContent='Ready.';card.appendChild(d)}return d}

  function makeText(t){const p=document.createElement('p');p.textContent=t;return p}
  function textbox(ph){const i=document.createElement('input');i.className='tool-input';i.placeholder=ph;return i}
  function textarea(ph){const i=document.createElement('textarea');i.className='tool-input';i.rows=5;i.placeholder=ph;return i}
  function action(label,fn){const b=document.createElement('button');b.className='tool-btn';b.textContent=label;b.onclick=fn;return b}
  function result(){const d=document.createElement('div');d.className='result-box';d.textContent='Ready.';return d}
  function runChatMode(prompt,mode){if(!prompt)return toast('Enter a task first');el.input.value=prompt;state.model=mode;state.think=mode==='think';syncThink();showView('chat');send()}
  async function runCommand(path,value,method='GET',payload=null){const box=el.workspaceBody.querySelector('.result-box');if(!box)return;box.textContent='Loading...';try{const options={method,cache:'no-store'};if(method==='POST'){options.headers={'Content-Type':'application/json'};options.body=JSON.stringify(payload||{prompt:value,mode:'chat'})}let p=path;if(method==='GET'&&value)p+=encodeURIComponent(value);const r=await fetch(p,options);const t=await r.text();if(!r.ok)throw new Error('HTTP '+r.status+' - '+t);box.textContent=pretty(t)}catch(e){box.textContent='CORE-AI route unavailable.\n\n'+e.message}}
  function parse(t){try{return JSON.parse(t)}catch(_){return null}}
  function extract(d,raw){if(typeof d==='string')return d.trim();if(!d)return raw.trim();const a=[d.text,d.response,d.content,d.message&&d.message.content,d.message,d.result&&d.result.text,d.result&&d.result.content];for(const x of a)if(typeof x==='string'&&x.trim())return x.trim();return ''}

  function switchChat(id){if(!state.chats.some(c=>c.id===id))return;state.current=id;el.input.value=state.settings.rememberDraft?localStorage.getItem(K.draft)||'':'';save();showView('chat');render();el.sidebar.classList.remove('open')}
  function newChat(focus){const c=makeChat();state.chats.unshift(c);state.current=c.id;state.files=[];el.file.value='';el.input.value='';localStorage.removeItem(K.draft);save();showView('chat');render();if(focus)el.input.focus()}
  function resetChat(){const c=currentChat();if(!c)return;c.messages=[];c.title='New chat';c.updatedAt=Date.now();save();showView('chat');render();closePopovers();toast('Chat reset')}
  function clearChats(){const c=makeChat();state.chats=[c];state.current=c.id;save();render();toast('Recent chats cleared')}
  function rename(){const c=currentChat();if(!c)return;const v=window.prompt('Rename chat',c.title||'New chat');if(v&&v.trim()){c.title=v.trim().slice(0,80);c.updatedAt=Date.now();save();renderChats()}closePopovers()}
  function exportChat(){const c=currentChat();if(!c)return;const blob=new Blob([JSON.stringify(c,null,2)],{type:'application/json'});const u=URL.createObjectURL(blob);const a=document.createElement('a');a.href=u;a.download=(c.title||'core-chat').replace(/[^a-z0-9-_]+/gi,'-')+'.json';a.click();URL.revokeObjectURL(u);closePopovers()}
  function togglePopover(p){const open=p.classList.contains('hidden');closePopovers();p.classList.toggle('hidden',!open)}
  function closePopovers(){$('modelPop').classList.add('hidden');$('morePop').classList.add('hidden')}
  function openSearch(){$('searchOverlay').classList.remove('hidden');$('searchInput').value='';renderSearch();setTimeout(()=>$('searchInput').focus(),0)}
  function closeSearch(){$('searchOverlay').classList.add('hidden')}
  function renderSearch(){const q=$('searchInput').value.trim().toLowerCase();const out=$('searchResults');out.innerHTML='';state.chats.filter(c=>!q||(c.title||'').toLowerCase().includes(q)||(c.messages||[]).some(m=>(m.content||'').toLowerCase().includes(q))).slice(0,50).forEach(c=>{const b=document.createElement('button');b.className='result-item';b.innerHTML='<strong></strong><small></small>';b.children[0].textContent=c.title||'New chat';b.children[1].textContent=new Date(c.updatedAt).toLocaleString();b.onclick=()=>{switchChat(c.id);closeSearch()};out.appendChild(b)});if(!out.children.length)out.textContent='No chats found.'}
  function pretty(t){const d=parse(t);return d?JSON.stringify(d,null,2):t}
  function toast(t){clearTimeout(toastTimer);el.toast.textContent=t;el.toast.classList.add('show');toastTimer=setTimeout(()=>el.toast.classList.remove('show'),2200)}
})();
