'use strict';

var actx=null;
function ensureAudio(){
  if(!actx)actx=new(window.AudioContext||window.webkitAudioContext)();
  if(actx.state==='suspended')actx.resume();
  warmUpSpeech();
}
function beep(freq,dur,type,vol){if(!actx)return;type=type||'sine';vol=vol===undefined?0.4:vol;var o=actx.createOscillator(),g=actx.createGain();o.connect(g);g.connect(actx.destination);o.frequency.value=freq;o.type=type;var t=actx.currentTime;g.gain.setValueAtTime(vol,t);g.gain.exponentialRampToValueAtTime(.001,t+dur);o.start(t);o.stop(t+dur);}
function beepSeq(notes){notes.forEach(n=>setTimeout(()=>beep(n[0],n[1],n[2]),n[3]||0));}

var sfx={
  // RotorHazard leader tone
  lap:  ()=>beepSeq([[1200,.075,'square',0],[1800,.1,'square',75]]),
  // RotorHazard winner tone
  best: ()=>beepSeq([
    [1200,.05,'square',0],  [1800,.075,'square',50],
    [1200,.05,'square',125],[1800,.075,'square',175],
    [1200,.05,'square',250],[1800,.1,  'square',300]
  ]),
  // RotorHazard staging + start
  count: ()=>beepSeq([
    [440,.1,'triangle',0],
    [440,.1,'triangle',1000],
    [440,.1,'triangle',2000],
    [880,.7,'triangle',3000]
  ]),
  enter: ()=>beep(880,.2,'sine'),
  exit:  ()=>beep(1100,.07,'sine')
};

var speechQ=[],speechBusy=false,speechWarmedUp=false,lastSpeechStart=0;

// Resume paused speech (Chrome background tab) and recover from stuck state.
// Debounce: skip recovery for 1.5s after speak() to avoid double-firing while
// the TTS engine is still initialising (speaking may briefly read false).
setInterval(()=>{
  if(typeof speechSynthesis==='undefined')return;
  if(speechSynthesis.paused)speechSynthesis.resume();
  if(speechBusy&&!speechSynthesis.speaking&&Date.now()-lastSpeechStart>1500){
    speechBusy=false;nextSpeech();
  }
},1000);
var cachedJaVoice=null;

function getJaVoice(){
  if(cachedJaVoice)return cachedJaVoice;
  var voices=speechSynthesis.getVoices();
  cachedJaVoice=voices.find(v=>v.lang&&v.lang.startsWith('ja'))||null;
  return cachedJaVoice;
}
if(typeof speechSynthesis!=='undefined'&&'onvoiceschanged' in speechSynthesis){
  speechSynthesis.onvoiceschanged=function(){cachedJaVoice=null;};
}

function warmUpSpeech(){
  if(speechWarmedUp||typeof speechSynthesis==='undefined')return;
  speechWarmedUp=true;
  var u=new SpeechSynthesisUtterance('​');
  u.volume=0;
  speechSynthesis.speak(u);
}

function speak(text){
  if(!voiceEnabled||announceMode==='none'){sfx.lap();return;}
  if(announceMode==='beep'){sfx.lap();return;}
  if(speechQ.length<8)speechQ.push(text);
  if(!speechBusy)nextSpeech();
}
function nextSpeech(){
  if(!speechQ.length){speechBusy=false;return;}
  speechBusy=true;
  var text=speechQ.shift();
  var u=new SpeechSynthesisUtterance(text);
  u.lang='ja-JP';u.rate=speechRate;
  var jaVoice=getJaVoice();
  if(jaVoice)u.voice=jaVoice;
  // Generous timeout accounting for speech rate; safety net only
  var ms=Math.max(5000,text.length*500/Math.max(0.5,speechRate));
  var timeout=setTimeout(()=>{speechSynthesis.cancel();speechBusy=false;nextSpeech();},ms);
  u.onend=()=>{clearTimeout(timeout);setTimeout(nextSpeech,80);};
  u.onerror=()=>{clearTimeout(timeout);speechBusy=false;nextSpeech();};
  lastSpeechStart=Date.now();
  speechSynthesis.speak(u);
}
function getSpokenName(p){return (p.yomi&&p.yomi!=='')?p.yomi:p.name;}
function buildSpeech(p,lapCount,lapMs){
  var spokenName=getSpokenName(p);
  var s=Math.floor(lapMs/1000),ms=Math.floor((lapMs%1000)/100);
  var m=Math.floor(s/60);s=s%60;
  var tStr=m>0?m+'分'+s+'秒'+ms:s+'秒'+ms;
  if(announceMode==='lap_laptime'){var lapLabel=lapMode==='immediate'?lapCount+'周':(lapCount===1?'ホールショット':(lapCount-1)+'周');return spokenName+'、'+lapLabel+'、'+tStr;}
  return spokenName+'、'+tStr;
}
function testVoice(){
  ensureAudio();sfx.lap();
  if(voiceEnabled&&announceMode!=='beep'&&announceMode!=='none'){
    var u=new SpeechSynthesisUtterance('テスト、1分23秒4');u.lang='ja-JP';u.rate=speechRate;
    var voices=speechSynthesis.getVoices();
    var jaVoice=voices.find(v=>v.lang&&v.lang.startsWith('ja'));
    if(jaVoice)u.voice=jaVoice;
    speechSynthesis.speak(u);
  }
}
function toggleVoice(){
  ensureAudio();
  voiceEnabled=!voiceEnabled;
  localStorage.setItem('voice',voiceEnabled?'1':'0');
  refreshVoiceBtns();
}
function refreshVoiceBtns(){
  ['voiceToggle','voiceToggle2'].forEach(id=>{
    var el=document.getElementById(id);if(!el)return;
    el.checked=voiceEnabled;
  });
}
