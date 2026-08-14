#pragma once

// ─────────────────────────────────────────────────────────────────────────────
// LOGIN PAGE
// ─────────────────────────────────────────────────────────────────────────────
static const char LOGIN_HTML[] PROGMEM = R"HTMLEOF(<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>SmartAC – Login</title>
<style>
  :root{--bg:#0d1117;--surface:#161b22;--border:#30363d;--accent:#00b4d8;--accent2:#0077b6;--text:#e6edf3;--muted:#8b949e;--danger:#f85149;--radius:12px}
  *,*::before,*::after{box-sizing:border-box;margin:0;padding:0}
  body{background:var(--bg);color:var(--text);font-family:'Segoe UI',system-ui,sans-serif;min-height:100vh;display:flex;align-items:center;justify-content:center;background-image:radial-gradient(ellipse at 50% 0%,rgba(0,180,216,.12) 0%,transparent 70%)}
  .card{background:var(--surface);border:1px solid var(--border);border-radius:var(--radius);padding:2.5rem 2rem;width:min(380px,94vw);box-shadow:0 8px 32px rgba(0,0,0,.5)}
  .logo{text-align:center;margin-bottom:2rem}
  .logo svg{display:block;margin:0 auto .75rem}
  .logo h1{font-size:1.6rem;font-weight:700;letter-spacing:-.03em}
  .logo p{color:var(--muted);font-size:.85rem;margin-top:.25rem}
  label{display:block;font-size:.82rem;color:var(--muted);margin-bottom:.4rem;margin-top:1.1rem}
  input{width:100%;padding:.65rem .9rem;background:#0d1117;border:1px solid var(--border);border-radius:8px;color:var(--text);font-size:.95rem;outline:none;transition:border-color .2s}
  input:focus{border-color:var(--accent)}
  .btn{margin-top:1.6rem;width:100%;padding:.75rem;background:linear-gradient(135deg,var(--accent),var(--accent2));border:none;border-radius:8px;color:#fff;font-size:1rem;font-weight:600;cursor:pointer;transition:opacity .2s,transform .1s}
  .btn:hover{opacity:.9}.btn:active{transform:scale(.98)}
  .err{margin-top:1rem;padding:.65rem .9rem;background:rgba(248,81,73,.1);border:1px solid var(--danger);border-radius:8px;color:var(--danger);font-size:.85rem;display:none}
  .spinner{display:none;width:18px;height:18px;border:2px solid rgba(255,255,255,.3);border-top-color:#fff;border-radius:50%;animation:spin .6s linear infinite;margin:0 auto}
  @keyframes spin{to{transform:rotate(360deg)}}
</style>
</head>
<body>
<div class="card">
  <div class="logo">
    <svg width="48" height="48" viewBox="0 0 48 48" fill="none">
      <rect width="48" height="48" rx="12" fill="url(#lg)"/>
      <defs><linearGradient id="lg" x1="0" y1="0" x2="48" y2="48"><stop stop-color="#00b4d8"/><stop offset="1" stop-color="#0077b6"/></linearGradient></defs>
      <rect x="10" y="16" width="28" height="16" rx="4" fill="white" opacity=".9"/>
      <circle cx="37" cy="24" r="3" fill="url(#lg)"/>
      <rect x="14" y="20" width="16" height="2" rx="1" fill="#0077b6"/>
      <rect x="14" y="25" width="10" height="2" rx="1" fill="#0077b6" opacity=".6"/>
    </svg>
    <h1>SmartAC</h1>
    <p>Sign in to control your AC</p>
  </div>
  <label for="usr">Username</label>
  <input id="usr" type="text" autocomplete="username" placeholder="admin">
  <label for="pwd">Password</label>
  <input id="pwd" type="password" autocomplete="current-password" placeholder="••••••••">
  <button class="btn" onclick="doLogin()">
    <span id="btnTxt">Sign In</span>
    <div class="spinner" id="spinner"></div>
  </button>
  <div class="err" id="errMsg">Invalid username or password.</div>
</div>
<script>
// ── Auto-redirect if a valid session cookie already exists (Requirement 6) ──
// Check by hitting a protected endpoint. If NOT 401, we are already logged in.
(async function checkExistingSession(){
  try{
    const r = await fetch('/api/status', {credentials:'include'});
    if(r.status !== 401){
      window.location.replace('/home');
    }
  }catch(e){/* network error – show login form normally */}
})();

document.addEventListener('keydown', e => { if(e.key === 'Enter') doLogin(); });

async function doLogin(){
  const u   = document.getElementById('usr').value.trim();
  const p   = document.getElementById('pwd').value;
  const err = document.getElementById('errMsg');
  const txt = document.getElementById('btnTxt');
  const sp  = document.getElementById('spinner');
  if(!u || !p){
    err.style.display = 'block';
    err.textContent   = 'Please enter credentials.';
    return;
  }
  err.style.display  = 'none';
  txt.style.display  = 'none';
  sp.style.display   = 'block';
  try{
    // credentials:'include' so the browser accepts and stores the Set-Cookie
    // header that the server sends on successful login.
    const r = await fetch('/api/login', {
      method      : 'POST',
      headers     : {'Content-Type':'application/json'},
      credentials : 'include',
      body        : JSON.stringify({username:u, password:p})
    });
    if(r.ok){
      window.location.href = '/home';
    } else {
      err.textContent  = 'Invalid username or password.';
      err.style.display = 'block';
    }
  }catch(e){
    err.textContent  = 'Connection error.';
    err.style.display = 'block';
  } finally {
    txt.style.display = 'block';
    sp.style.display  = 'none';
  }
}
</script>
</body>
</html>
)HTMLEOF";

// ─────────────────────────────────────────────────────────────────────────────
// DASHBOARD PAGE  (Manual control)
// ─────────────────────────────────────────────────────────────────────────────
static const char DASHBOARD_HTML[] PROGMEM = R"HTMLEOF(<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>SmartAC – Manual</title>
<style>
:root{
  --bg:#0d1117;--surface:#161b22;--surface2:#1c2128;
  --border:#30363d;--accent:#00b4d8;--accent2:#0077b6;
  --text:#e6edf3;--muted:#8b949e;
  --on:#39d353;--off:#f85149;--warn:#e3b341;
  --radius:14px;--gap:1rem;
}
*,*::before,*::after{box-sizing:border-box;margin:0;padding:0}
body{background:var(--bg);color:var(--text);font-family:'Segoe UI',system-ui,sans-serif;min-height:100vh;
  background-image:radial-gradient(ellipse at 70% 0%,rgba(0,180,216,.09) 0%,transparent 60%)}
header{display:flex;align-items:center;justify-content:space-between;padding:1rem 1.5rem;
  border-bottom:1px solid var(--border);background:var(--surface);position:sticky;top:0;z-index:10;flex-wrap:wrap;gap:.5rem}
.brand{display:flex;align-items:center;gap:.6rem;font-weight:700;font-size:1.1rem}
.wifi-badge{display:flex;align-items:center;gap:.4rem;font-size:.8rem;color:var(--muted)}
.wifi-dot{width:8px;height:8px;border-radius:50%;background:var(--on);transition:background .4s}
.nav-link{padding:.4rem .85rem;border-radius:8px;font-size:.84rem;color:var(--muted);
  text-decoration:none;transition:color .18s,background .18s}
.nav-link:hover{color:var(--text);background:var(--surface2)}
.nav-link.active{color:var(--accent);background:rgba(0,180,216,.1)}
.logout-btn{padding:.4rem .9rem;border:1px solid var(--border);border-radius:8px;
  background:transparent;color:var(--muted);font-size:.82rem;cursor:pointer;transition:color .2s,border-color .2s}
.logout-btn:hover{color:var(--off);border-color:var(--off)}
main{padding:1.5rem;max-width:860px;margin:0 auto;display:grid;gap:var(--gap)}
.card{background:var(--surface);border:1px solid var(--border);border-radius:var(--radius);padding:1.25rem 1.5rem}
.card-title{font-size:.72rem;text-transform:uppercase;letter-spacing:.09em;color:var(--muted);margin-bottom:1rem}
.status-row{display:grid;grid-template-columns:repeat(2,1fr);gap:.75rem}
.stat{text-align:center;padding:.6rem;background:var(--surface2);border-radius:10px}
.stat-val{font-size:1.9rem;font-weight:700;line-height:1}
.stat-label{font-size:.72rem;color:var(--muted);margin-top:.25rem}
.on-state{color:var(--on)}.off-state{color:var(--off)}
.power-row{display:flex;gap:.75rem}
.btn-power{flex:1;padding:.9rem;border:2px solid var(--border);border-radius:var(--radius);
  font-size:.95rem;font-weight:700;cursor:pointer;transition:all .2s;background:var(--surface2);color:var(--text)}
.btn-power.on:hover,.btn-power.on.active{background:rgba(57,211,83,.15);border-color:var(--on);color:var(--on)}
.btn-power.off:hover,.btn-power.off.active{background:rgba(248,81,73,.15);border-color:var(--off);color:var(--off)}
.temp-display{text-align:center;font-size:3.6rem;font-weight:800;letter-spacing:-.04em;
  color:var(--accent);line-height:1;padding:.4rem 0 .6rem}
.temp-display span{font-size:1.6rem;color:var(--muted)}
.temp-btns{display:flex;align-items:center;gap:1rem;justify-content:center}
.btn-temp{width:50px;height:50px;border-radius:50%;font-size:1.5rem;font-weight:300;
  border:2px solid var(--border);background:var(--surface2);color:var(--text);cursor:pointer;
  display:flex;align-items:center;justify-content:center;transition:all .15s;flex-shrink:0}
.btn-temp:hover{border-color:var(--accent);color:var(--accent)}
.btn-temp:active{transform:scale(.92)}
.slider-wrap{margin-top:.85rem}
.slider-labels{display:flex;justify-content:space-between;font-size:.72rem;color:var(--muted);margin-bottom:.35rem}
input[type=range]{-webkit-appearance:none;appearance:none;width:100%;height:6px;
  background:linear-gradient(to right,var(--accent) var(--pct,0%),var(--border) var(--pct,0%));
  border-radius:3px;outline:none;cursor:pointer}
input[type=range]::-webkit-slider-thumb{-webkit-appearance:none;width:20px;height:20px;
  background:var(--accent);border-radius:50%;border:2px solid var(--bg);cursor:pointer;
  box-shadow:0 0 0 3px rgba(0,180,216,.25)}
select{width:100%;margin-top:.6rem;padding:.55rem .8rem;background:var(--surface2);
  border:1px solid var(--border);border-radius:8px;color:var(--text);font-size:.9rem;outline:none;cursor:pointer}
select:focus{border-color:var(--accent)}
.btn-group{display:flex;flex-wrap:wrap;gap:.5rem}
.btn-mode{padding:.45rem .9rem;border:1px solid var(--border);border-radius:8px;
  background:var(--surface2);color:var(--muted);font-size:.85rem;cursor:pointer;transition:all .18s}
.btn-mode:hover{border-color:var(--accent);color:var(--text)}
.btn-mode.active{background:rgba(0,180,216,.18);border-color:var(--accent);color:var(--accent);font-weight:600}
.toggle-row{display:flex;gap:.6rem;flex-wrap:wrap}
.toggle-chip{padding:.45rem 1rem;border:1px solid var(--border);border-radius:20px;
  background:var(--surface2);color:var(--muted);font-size:.85rem;cursor:pointer;
  user-select:none;transition:all .18s}
.toggle-chip:hover{border-color:var(--accent);color:var(--text)}
.toggle-chip.active{background:rgba(0,180,216,.18);border-color:var(--accent);color:var(--accent);font-weight:600}
.toggle-chip.turbo.active{background:rgba(227,179,65,.15);border-color:var(--warn);color:var(--warn)}
.toggle-chip.sleep.active{background:rgba(57,211,83,.1);border-color:var(--on);color:var(--on)}
.chart-wrap{position:relative;height:160px;margin-top:.5rem}
canvas{width:100%!important}
#toast{position:fixed;bottom:1.5rem;left:50%;transform:translateX(-50%);
  background:var(--surface);border:1px solid var(--accent);color:var(--text);
  padding:.5rem 1.2rem;border-radius:20px;font-size:.85rem;opacity:0;
  transition:opacity .3s;pointer-events:none;white-space:nowrap;z-index:99}
#toast.show{opacity:1}
@media(max-width:500px){.temp-display{font-size:2.8rem}.btn-group{gap:.4rem}}
</style>
</head>
<body>
<header>
  <div class="brand">
    <svg width="26" height="26" viewBox="0 0 48 48" fill="none">
      <rect width="48" height="48" rx="10" fill="url(#hg)"/>
      <defs><linearGradient id="hg" x1="0" y1="0" x2="48" y2="48"><stop stop-color="#00b4d8"/><stop offset="1" stop-color="#0077b6"/></linearGradient></defs>
      <rect x="10" y="16" width="28" height="16" rx="4" fill="white" opacity=".9"/>
      <circle cx="37" cy="24" r="3" fill="url(#hg)"/>
      <rect x="14" y="20" width="16" height="2" rx="1" fill="#0077b6"/>
      <rect x="14" y="25" width="10" height="2" rx="1" fill="#0077b6" opacity=".6"/>
    </svg>
    SmartAC
  </div>
  <div style="display:flex;align-items:center;gap:.4rem;flex-wrap:wrap">
    <a class="nav-link" href="/home">Home</a>
    <a class="nav-link active" href="/dashboard">Manual</a>
    <a class="nav-link" href="/auto">Auto</a>
    <a class="nav-link" href="/temp">Temp</a>
    <a class="nav-link" href="/settings">⚙</a>
    <div class="wifi-badge" style="margin-left:.5rem">
      <div class="wifi-dot" id="wifiDot"></div>
      <span id="wifiLabel">Connected</span>
    </div>
    <button class="logout-btn" onclick="logout()">Sign Out</button>
  </div>
</header>
<main>
  <div class="card">
    <div class="card-title">Status</div>
    <div class="status-row">
      <div class="stat"><div class="stat-val" id="acPowerState">OFF</div><div class="stat-label">AC Power</div></div>
      <div class="stat"><div class="stat-val" id="roomTempVal">--</div><div class="stat-label">Room °C</div></div>
    </div>
  </div>
  <div class="card">
    <div class="card-title">Power</div>
    <div class="power-row">
      <button class="btn-power on"  id="btnOn"  onclick="sendCmd('/api/poweron')">⏻ Turn ON</button>
      <button class="btn-power off" id="btnOff" onclick="sendCmd('/api/poweroff')">⏹ Turn OFF</button>
    </div>
  </div>
  <div class="card">
    <div class="card-title">Set Temperature</div>
    <div class="temp-display" id="setTempDisp">24<span>°C</span></div>
    <div class="temp-btns">
      <button class="btn-temp" onclick="sendCmd('/api/tempdown')" title="−1°C">−</button>
      <div style="flex:1">
        <div class="slider-wrap">
          <div class="slider-labels"><span>17°C</span><span>30°C</span></div>
          <input type="range" id="tempSlider" min="17" max="30" value="24"
                 oninput="onSliderInput(this.value)" onchange="sendSetTemp(this.value)">
        </div>
        <select id="tempDrop" onchange="sendSetTemp(this.value)"></select>
      </div>
      <button class="btn-temp" onclick="sendCmd('/api/tempup')" title="+1°C">+</button>
    </div>
  </div>
  <div class="card">
    <div class="card-title">Mode</div>
    <div class="btn-group" id="modeGroup">
      <button class="btn-mode" data-mode="0" onclick="sendMode(0)">❄️ Cool</button>
      <button class="btn-mode" data-mode="4" onclick="sendMode(4)">🔥 Heat</button>
      <button class="btn-mode" data-mode="1" onclick="sendMode(1)">💧 Dry</button>
      <button class="btn-mode" data-mode="3" onclick="sendMode(3)">🌬️ Fan</button>
      <button class="btn-mode" data-mode="2" onclick="sendMode(2)">🔄 Auto</button>
    </div>
  </div>
  <div class="card">
    <div class="card-title">Fan Speed</div>
    <div class="btn-group" id="fanGroup">
      <button class="btn-mode" data-fan="5" onclick="sendFan(5)">🤖 Auto</button>
      <button class="btn-mode" data-fan="2" onclick="sendFan(2)">🌿 Low</button>
      <button class="btn-mode" data-fan="4" onclick="sendFan(4)">💨 Medium</button>
      <button class="btn-mode" data-fan="1" onclick="sendFan(1)">🚀 High</button>
    </div>
  </div>
  <div class="card">
    <div class="card-title">Special Modes</div>
    <div class="toggle-row">
      <button class="toggle-chip"       id="chipSwing" onclick="sendCmd('/api/swing')">🔃 Swing</button>
      <button class="toggle-chip turbo" id="chipTurbo" onclick="sendCmd('/api/turbo')">⚡ Turbo</button>
      <button class="toggle-chip sleep" id="chipSleep" onclick="sendCmd('/api/sleep')">🌙 Sleep</button>
    </div>
  </div>
  <div class="card">
    <div class="card-title">Room Temperature History (last 5 min)</div>
    <div class="chart-wrap"><canvas id="histChart"></canvas></div>
  </div>
</main>
<div id="toast"></div>
<script>
const POLL_MS=4000, HIST_MS=15000;
let state={}, chart=null, histData=[];

(function init(){
  buildTempDropdown(); initChart();
  fetchStatus(); fetchHistory();
  setInterval(fetchStatus, POLL_MS);
  setInterval(fetchHistory, HIST_MS);
})();

function buildTempDropdown(){
  const s=document.getElementById('tempDrop');
  for(let t=17;t<=30;t++){const o=document.createElement('option');o.value=t;o.textContent=t+'°C';s.appendChild(o);}
}

// credentials:'include' ensures the session cookie is sent on every request.
async function fetchStatus(){
  try{
    const r=await fetch('/api/status',{credentials:'include'});
    if(r.status===401){window.location.href='/';return;}
    if(!r.ok) return;
    applyState(await r.json());
  }catch(e){console.error('fetchStatus:',e);}
}

function applyState(d){
  state=d;
  const ps=document.getElementById('acPowerState');
  ps.textContent=d.power?'ON':'OFF';
  ps.className='stat-val '+(d.power?'on-state':'off-state');
  document.getElementById('btnOn').classList.toggle('active',!!d.power);
  document.getElementById('btnOff').classList.toggle('active',!d.power);
  const t=d.temp??24;
  document.getElementById('setTempDisp').innerHTML=t+'<span>°C</span>';
  const sl=document.getElementById('tempSlider');
  sl.value=t; updateSliderGradient(sl);
  document.getElementById('tempDrop').value=t;
  document.getElementById('roomTempVal').textContent=d.roomTemp!=null?d.roomTemp.toFixed(1):'--';
  const wok=d.wifiOk!==false;
  document.getElementById('wifiDot').style.background=wok?'var(--on)':'var(--off)';
  document.getElementById('wifiLabel').textContent=wok?'Connected':'Disconnected';
  document.querySelectorAll('#modeGroup .btn-mode').forEach(b=>b.classList.toggle('active',parseInt(b.dataset.mode)===d.mode));
  document.querySelectorAll('#fanGroup .btn-mode').forEach(b=>b.classList.toggle('active',parseInt(b.dataset.fan)===d.fan));
  document.getElementById('chipSwing').classList.toggle('active',!!d.swing);
  document.getElementById('chipTurbo').classList.toggle('active',!!d.turbo);
  document.getElementById('chipSleep').classList.toggle('active',!!d.sleep);
}

async function sendCmd(url,body){
  try{
    const opts={method:'POST',credentials:'include'};
    if(body){opts.headers={'Content-Type':'application/json'};opts.body=JSON.stringify(body);}
    const r=await fetch(url,opts);
    if(r.status===401){window.location.href='/';return;}
    const d=await r.json();
    applyState(d); showToast(d.message||'Done');
  }catch(e){showToast('Connection error',true);}
}

function sendMode(m){sendCmd('/api/setmode',{mode:m});}
function sendFan(f){sendCmd('/api/setfan',{fan:f});}
function sendSetTemp(v){sendCmd('/api/settemp',{temp:parseInt(v)});}
function onSliderInput(val){
  document.getElementById('setTempDisp').innerHTML=val+'<span>°C</span>';
  updateSliderGradient(document.getElementById('tempSlider'));
  document.getElementById('tempDrop').value=val;
}
function updateSliderGradient(el){
  const pct=((+el.value-+el.min)/(+el.max-+el.min)*100).toFixed(1)+'%';
  el.style.setProperty('--pct',pct);
}

async function logout(){
  await fetch('/api/logout',{method:'POST',credentials:'include'});
  window.location.href='/';
}

function initChart(){
  const canvas=document.getElementById('histChart');
  chart={canvas,ctx:canvas.getContext('2d')};
  drawChart();
}

async function fetchHistory(){
  try{
    const r=await fetch('/api/history',{credentials:'include'});
    if(!r.ok) return;
    histData=await r.json(); drawChart();
  }catch(e){}
}

function drawChart(){
  const {canvas,ctx}=chart;
  const W=canvas.offsetWidth||canvas.parentElement.offsetWidth||400, H=160;
  canvas.width=W; canvas.height=H;
  ctx.fillStyle='#0d1117'; ctx.fillRect(0,0,W,H);
  if(!histData||histData.length<2){
    ctx.fillStyle='#8b949e';ctx.font='13px system-ui';ctx.textAlign='center';
    ctx.fillText('Collecting data…',W/2,H/2);return;
  }
  const PAD={t:12,r:12,b:28,l:38};
  const cW=W-PAD.l-PAD.r, cH=H-PAD.t-PAD.b;
  const temps=histData.map(d=>d.t);
  const tMin=Math.floor(Math.min(...temps)-1), tMax=Math.ceil(Math.max(...temps)+1);
  const n=histData.length;
  const xOf=i=>PAD.l+(i/(n-1))*cW;
  const yOfT=v=>PAD.t+cH-((v-tMin)/(tMax-tMin))*cH;
  ctx.strokeStyle='#30363d'; ctx.lineWidth=1;
  for(let g=0;g<=4;g++){const y=PAD.t+g*(cH/4);ctx.beginPath();ctx.moveTo(PAD.l,y);ctx.lineTo(PAD.l+cW,y);ctx.stroke();}
  ctx.save(); ctx.beginPath();
  histData.forEach((d,i)=>{i===0?ctx.moveTo(xOf(i),yOfT(d.t)):ctx.lineTo(xOf(i),yOfT(d.t));});
  ctx.lineTo(xOf(n-1),PAD.t+cH); ctx.lineTo(xOf(0),PAD.t+cH); ctx.closePath();
  const grad=ctx.createLinearGradient(0,PAD.t,0,PAD.t+cH);
  grad.addColorStop(0,'rgba(0,180,216,.25)'); grad.addColorStop(1,'rgba(0,180,216,0)');
  ctx.fillStyle=grad; ctx.fill(); ctx.restore();
  ctx.strokeStyle='#00b4d8'; ctx.lineWidth=2; ctx.beginPath();
  histData.forEach((d,i)=>{i===0?ctx.moveTo(xOf(i),yOfT(d.t)):ctx.lineTo(xOf(i),yOfT(d.t));});
  ctx.stroke();
  ctx.fillStyle='#00b4d8'; ctx.font='10px system-ui'; ctx.textAlign='right';
  ctx.fillText(tMax+'°',PAD.l-4,PAD.t+4); ctx.fillText(tMin+'°',PAD.l-4,PAD.t+cH);
  ctx.fillStyle='#8b949e'; ctx.textAlign='center';
  const fmtAgo=s=>s<60?s+'s ago':Math.round(s/60)+'m ago';
  ctx.fillText(fmtAgo(histData[0].ts),xOf(0),H-6); ctx.fillText('now',xOf(n-1),H-6);
  ctx.textAlign='left'; ctx.fillStyle='#00b4d8'; ctx.fillText('● Temp °C',PAD.l,H-6);
}

let _tt;
function showToast(msg,err){
  const t=document.getElementById('toast');
  t.textContent=msg; t.style.borderColor=err?'var(--off)':'var(--accent)';
  t.classList.add('show'); clearTimeout(_tt); _tt=setTimeout(()=>t.classList.remove('show'),2200);
}
</script>
</body>
</html>
)HTMLEOF";
