'use strict';

// ── Soft auth ────────────────────────────────────────────────────────────
// Trigger: switching to any non-race tab when not yet authed.
// The password lives only in NVS on the web node — never in the JS bundle.
// Session-scoped (sessionStorage) so a shared device doesn't stay authed
// across a browser-close.

function openLoginModal(){
  var m=document.getElementById('loginModal');if(!m)return;
  var pw=document.getElementById('loginPw');var err=document.getElementById('loginErr');
  if(pw)pw.value='';if(err)err.style.display='none';
  m.style.display='flex';
  setTimeout(()=>{if(pw)pw.focus();},50);
}
function closeLoginModal(){
  var m=document.getElementById('loginModal');if(m)m.style.display='none';
}
function cancelLogin(){pendingTab=null;closeLoginModal();}

async function submitLogin(){
  var pw=document.getElementById('loginPw');var err=document.getElementById('loginErr');
  var v=pw?pw.value:'';
  try{
    var r=await fetch('/api/auth/login',{method:'POST',headers:{'Content-Type':'application/json'},
      body:JSON.stringify({password:v})});
    if(r.ok){
      isAuthed=true;sessionStorage.setItem('authed','1');
      closeLoginModal();applyAuthState();
      var t=pendingTab;pendingTab=null;
      if(t)switchTab(t);
      toast('🔓 ログインしました');
    }else{
      if(err){err.style.display='block';err.textContent='パスワードが違います';}
    }
  }catch(e){if(err){err.style.display='block';err.textContent='通信エラー';}}
}

function logout(){
  isAuthed=false;sessionStorage.removeItem('authed');
  applyAuthState();
  // Bounce back to Race tab so viewers can't keep seeing admin panes.
  switchTab('race');
  toast('🔒 ログアウトしました');
}

// Reflect auth state across the UI: race controls, tab buttons (lock icons),
// and the Config-tab admin card visibility (the card is *inside* a tab that
// only the authed user can reach, so we just need to populate it).
function applyAuthState(){
  // Race control buttons — viewers see them disabled with a hint.
  var hint=document.getElementById('authHint');
  if(hint)hint.style.display=isAuthed?'none':'block';
  if(!isAuthed){
    ['btnStart','btnStop','btnClear'].forEach(function(id){
      var b=document.getElementById(id);if(b){b.disabled=true;b.title='管理者ログインが必要です';}
    });
  }else{
    // Restore normal disable-state for race buttons from race state.
    if(typeof setBtns==='function')setBtns(raceRunning);
    ['btnStart','btnStop','btnClear'].forEach(function(id){
      var b=document.getElementById(id);if(b)b.title='';
    });
  }
  // Lock icon on gated tab buttons.
  document.querySelectorAll('.tab-btn').forEach(function(b,i){
    var name=['race','config','calib','sd'][i];
    if(name==='race')return;
    b.classList.toggle('locked',!isAuthed);
  });
}

async function changePassword(){
  var cur=document.getElementById('pwCurrent');
  var nw =document.getElementById('pwNew');
  var cf =document.getElementById('pwConfirm');
  if(!cur||!nw||!cf)return;
  if(nw.value.length<1||nw.value.length>32){toast('⚠️ パスワードは1〜32文字');return;}
  if(nw.value!==cf.value){toast('⚠️ 新パスワードと確認が一致しません');return;}
  try{
    var r=await fetch('/api/auth/password',{method:'POST',headers:{'Content-Type':'application/json'},
      body:JSON.stringify({current:cur.value,new:nw.value})});
    if(r.ok){cur.value=nw.value=cf.value='';toast('✅ パスワードを変更しました');}
    else if(r.status===401)toast('⚠️ 現在のパスワードが違います');
    else toast('⚠️ 変更エラー');
  }catch(e){toast('⚠️ 通信エラー');}
}
