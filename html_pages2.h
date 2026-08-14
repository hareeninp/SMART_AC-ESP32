#pragma once

// ─────────────────────────────────────────────────────────────────────────────
// HOME PAGE
// ─────────────────────────────────────────────────────────────────────────────
static const char HOME_HTML[] PROGMEM = R"HTMLEOF(<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>SmartAC – Home</title>
<style>
:root{--bg:#0d1117;--surface:#161b22;--surface2:#1c2128;--border:#30363d;
  --accent:#00b4d8;--accent2:#0077b6;--text:#e6edf3;--muted:#8b949e;
  --on:#39d353;--off:#f85149;--warn:#e3b341;--radius:14px}
*,*::before,*::after{box-sizing:border-box;margin:0;padding:0}
body{background:var(--bg);color:var(--text);font-family:'Segoe UI',system-ui,sans-serif;
  min-height:100vh;background-image:radial-gradient(ellipse at 50% 0%,rgba(0,180,216,.12) 0%,transparent 70%)}
header{display:flex;align-items:center;justify-content:space-between;padding:1rem 1.5rem;
  border-bottom:1px solid var(--border);background:var(--surface);position:sticky;top:0;z-index:10;flex-wrap:wrap;gap:.5rem}
.brand{display:flex;align-items:center;gap:.6rem;font-weight:700;font-size:1.1rem}
nav{display:flex;gap:.5rem;align-items:center;flex-wrap:wrap}
.nav-link{padding:.4rem .85rem;border-radius:8px;font-size:.84rem;color:var(--muted);
  text-decoration:none;transition:color .18s,background .18s}
.nav-link:hover{color:var(--text);background:var(--surface2)}
.nav-link.active{color:var(--accent);background:rgba(0,180,216,.1)}
.logout-btn{padding:.4rem .9rem;border:1px solid var(--border);border-radius:8px;
  background:transparent;color:var(--muted);font-size:.82rem;cursor:pointer;
  transition:color .2s,border-color .2s;margin-left:.25rem}
.logout-btn:hover{color:var(--off);border-color:var(--off)}
main{padding:2.5rem 1.5rem;max-width:680px;margin:0 auto}
.page-title{font-size:1.5rem;font-weight:700;margin-bottom:.4rem}
.page-sub{color:var(--muted);font-size:.9rem;margin-bottom:2.5rem}
.mode-grid{display:grid;grid-template-columns:repeat(3,1fr);gap:1.25rem}
.mode-card{background:var(--surface);border:2px solid var(--border);border-radius:var(--radius);
  padding:2rem 1.5rem;cursor:pointer;transition:border-color .22s,transform .15s,box-shadow .22s;
  text-decoration:none;color:var(--text);display:flex;flex-direction:column;align-items:center;gap:1rem}
.mode-card:hover{border-color:var(--accent);transform:translateY(-3px);box-shadow:0 12px 32px rgba(0,180,216,.15)}
.mode-card:active{transform:translateY(0)}
.mode-card.auto:hover{border-color:var(--on);box-shadow:0 12px 32px rgba(57,211,83,.12)}
.mode-card.temp:hover{border-color:var(--warn);box-shadow:0 12px 32px rgba(227,179,65,.12)}
.mode-icon{width:64px;height:64px;border-radius:16px;display:flex;align-items:center;justify-content:center;font-size:2rem}
.mode-card.manual .mode-icon{background:rgba(0,180,216,.12)}
.mode-card.auto   .mode-icon{background:rgba(57,211,83,.1)}
.mode-card.temp   .mode-icon{background:rgba(227,179,65,.1)}
.mode-card h2{font-size:1.2rem;font-weight:700;text-align:center}
.mode-card p{font-size:.84rem;color:var(--muted);text-align:center;line-height:1.5}
.sensor-bar{margin-top:2.5rem;display:flex;gap:1rem}
.sensor-pill{flex:1;background:var(--surface);border:1px solid var(--border);border-radius:10px;
  padding:.9rem 1.1rem;display:flex;align-items:center;gap:.75rem}
.sensor-pill .val{font-size:1.6rem;font-weight:700;color:var(--accent)}
@media(max-width:600px){.mode-grid{grid-template-columns:1fr}.sensor-bar{flex-direction:column}}
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
  <nav>
    <a class="nav-link active" href="/home">Home</a>
    <a class="nav-link" href="/dashboard">Manual</a>
    <a class="nav-link" href="/auto">Auto</a>
    <a class="nav-link" href="/temp">Temp</a>
    <a class="nav-link" href="/settings">⚙</a>
    <button class="logout-btn" onclick="logout()">Sign Out</button>
  </nav>
</header>
<main>
  <div class="page-title">Welcome back 👋</div>
  <div class="page-sub">Choose how you want to control your AC.</div>
  <div class="mode-grid">
    <a class="mode-card manual" href="/dashboard">
      <div class="mode-icon">🕹️</div>
      <h2>Manual Control</h2>
      <p>Directly control power, temperature, fan speed, modes and special features.</p>
    </a>
    <a class="mode-card auto" href="/auto">
      <div class="mode-icon">🗓️</div>
      <h2>Automatic Mode</h2>
      <p>Set schedules to turn your AC on and off automatically throughout the day.</p>
    </a>
    <a class="mode-card temp" href="/temp">
      <div class="mode-icon">🌡️</div>
      <h2>Temperature Mode</h2>
      <p>Auto-cool when room exceeds a threshold. AC turns off once target temp is reached.</p>
    </a>
  </div>
  <div class="sensor-bar">
    <div class="sensor-pill">
      <div><div class="val" id="homeTempVal">--</div><div class="lbl">Room °C</div></div>
    </div>
    <div class="sensor-pill">
      <div><div class="val" id="homeACState" style="color:var(--off)">OFF</div><div class="lbl">AC Power</div></div>
    </div>
  </div>
</main>
<script>
async function logout(){
  await fetch('/api/logout',{method:'POST',credentials:'include'});
  window.location.href='/';
}
(async function(){
  try{
    const r=await fetch('/api/status',{credentials:'include'});
    if(r.status===401){window.location.href='/';return;}
    const d=await r.json();
    document.getElementById('homeTempVal').textContent=d.roomTemp!=null?d.roomTemp.toFixed(1):'--';
    const el=document.getElementById('homeACState');
    el.textContent=d.power?'ON':'OFF';
    el.style.color=d.power?'var(--on)':'var(--off)';
  }catch(e){}
})();
</script>
</body>
</html>
)HTMLEOF";


// ─────────────────────────────────────────────────────────────────────────────
// AUTO MODE PAGE
// ─────────────────────────────────────────────────────────────────────────────
static const char AUTO_HTML[] PROGMEM = R"HTMLEOF(<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>SmartAC – Auto Mode</title>
<style>
:root{--bg:#0d1117;--surface:#161b22;--surface2:#1c2128;--border:#30363d;
  --accent:#00b4d8;--accent2:#0077b6;--text:#e6edf3;--muted:#8b949e;
  --on:#39d353;--off:#f85149;--warn:#e3b341;--radius:14px;--gap:1rem}
*,*::before,*::after{box-sizing:border-box;margin:0;padding:0}
body{background:var(--bg);color:var(--text);font-family:'Segoe UI',system-ui,sans-serif;min-height:100vh;
  background-image:radial-gradient(ellipse at 70% 0%,rgba(57,211,83,.07) 0%,transparent 60%)}
header{display:flex;align-items:center;justify-content:space-between;padding:1rem 1.5rem;
  border-bottom:1px solid var(--border);background:var(--surface);position:sticky;top:0;z-index:10;flex-wrap:wrap;gap:.5rem}
.brand{display:flex;align-items:center;gap:.6rem;font-weight:700;font-size:1.1rem}
nav{display:flex;gap:.4rem;align-items:center;flex-wrap:wrap}
.nav-link{padding:.4rem .85rem;border-radius:8px;font-size:.84rem;color:var(--muted);text-decoration:none;transition:color .18s,background .18s}
.nav-link:hover{color:var(--text);background:var(--surface2)}
.nav-link.active{color:var(--accent);background:rgba(0,180,216,.1)}
.logout-btn{padding:.4rem .9rem;border:1px solid var(--border);border-radius:8px;background:transparent;color:var(--muted);font-size:.82rem;cursor:pointer;transition:color .2s,border-color .2s}
.logout-btn:hover{color:var(--off);border-color:var(--off)}
main{padding:1.5rem;max-width:860px;margin:0 auto;display:grid;gap:var(--gap)}
.card{background:var(--surface);border:1px solid var(--border);border-radius:var(--radius);padding:1.25rem 1.5rem}
.card-title{font-size:.72rem;text-transform:uppercase;letter-spacing:.09em;color:var(--muted);margin-bottom:1rem;display:flex;align-items:center;justify-content:space-between}
.section-badge{background:rgba(0,180,216,.12);color:var(--accent);border-radius:20px;padding:.15rem .7rem;font-size:.7rem;letter-spacing:.06em}
.settings-grid{display:grid;grid-template-columns:repeat(auto-fill,minmax(160px,1fr));gap:.75rem}
.setting-row{display:flex;flex-direction:column;gap:.4rem}
.setting-row label{font-size:.78rem;color:var(--muted)}
select.ctrl,input.ctrl{width:100%;padding:.5rem .75rem;background:var(--surface2);border:1px solid var(--border);border-radius:8px;color:var(--text);font-size:.88rem;outline:none;cursor:pointer;transition:border-color .2s}
select.ctrl:focus,input.ctrl:focus{border-color:var(--accent)}
.toggle-row{display:flex;gap:.6rem;flex-wrap:wrap;margin-top:.25rem}
.toggle-chip{padding:.4rem .9rem;border:1px solid var(--border);border-radius:20px;background:var(--surface2);color:var(--muted);font-size:.85rem;cursor:pointer;user-select:none;transition:all .18s}
.toggle-chip:hover{border-color:var(--accent);color:var(--text)}
.toggle-chip.active{background:rgba(0,180,216,.18);border-color:var(--accent);color:var(--accent);font-weight:600}
.toggle-chip.turbo.active{background:rgba(227,179,65,.15);border-color:var(--warn);color:var(--warn)}
.toggle-chip.sleep.active{background:rgba(57,211,83,.1);border-color:var(--on);color:var(--on)}
.save-btn{margin-top:1rem;padding:.6rem 1.4rem;background:linear-gradient(135deg,var(--accent),var(--accent2));border:none;border-radius:8px;color:#fff;font-size:.9rem;font-weight:600;cursor:pointer;transition:opacity .2s}
.save-btn:hover{opacity:.88}
.ntp-row{display:flex;align-items:center;gap:.6rem;font-size:.83rem;color:var(--muted);margin-bottom:.75rem}
.ntp-dot{width:8px;height:8px;border-radius:50%;background:var(--off);flex-shrink:0}
.ntp-dot.synced{background:var(--on)}
.add-btn{padding:.5rem 1.1rem;background:rgba(57,211,83,.12);border:1px solid var(--on);border-radius:8px;color:var(--on);font-size:.85rem;font-weight:600;cursor:pointer;transition:all .2s}
.add-btn:hover{background:rgba(57,211,83,.22)}
.sched-list{display:grid;gap:.65rem;margin-top:.5rem}
.sched-item{background:var(--surface2);border:1px solid var(--border);border-radius:10px;padding:.9rem 1.1rem;display:flex;align-items:center;gap:.75rem;flex-wrap:wrap}
.sched-item.enabled{border-color:rgba(0,180,216,.3)}
.sched-num{width:26px;height:26px;border-radius:50%;background:var(--border);display:flex;align-items:center;justify-content:center;font-size:.75rem;color:var(--muted);flex-shrink:0}
.sched-times{flex:1;min-width:0}
.sched-times .time-main{font-size:1rem;font-weight:700;font-variant-numeric:tabular-nums}
.sched-times .time-sub{font-size:.75rem;color:var(--muted);margin-top:.15rem}
.sched-toggle{padding:.3rem .75rem;border-radius:20px;font-size:.8rem;font-weight:600;cursor:pointer;border:1px solid;transition:all .18s}
.sched-toggle.on{background:rgba(57,211,83,.1);border-color:var(--on);color:var(--on)}
.sched-toggle.off{background:var(--surface);border-color:var(--border);color:var(--muted)}
.sched-edit,.sched-del{padding:.3rem .75rem;border-radius:8px;background:transparent;font-size:.8rem;cursor:pointer;transition:all .18s}
.sched-edit{border:1px solid var(--border);color:var(--muted)}
.sched-edit:hover{border-color:var(--accent);color:var(--accent)}
.sched-del{border:1px solid transparent;color:var(--muted)}
.sched-del:hover{border-color:var(--off);color:var(--off)}
.empty-msg{text-align:center;padding:2rem;color:var(--muted);font-size:.9rem}
.modal-overlay{position:fixed;inset:0;background:rgba(0,0,0,.7);z-index:100;display:none;align-items:center;justify-content:center;padding:1rem}
.modal-overlay.open{display:flex}
.modal{background:var(--surface);border:1px solid var(--border);border-radius:var(--radius);padding:1.75rem;width:min(400px,100%);box-shadow:0 16px 48px rgba(0,0,0,.6)}
.modal h3{font-size:1.1rem;font-weight:700;margin-bottom:1.25rem}
.modal-row{margin-bottom:1rem}
.modal-row label{display:block;font-size:.8rem;color:var(--muted);margin-bottom:.35rem}
.modal-actions{display:flex;gap:.6rem;margin-top:1.25rem;justify-content:flex-end}
.btn-cancel{padding:.55rem 1.1rem;border:1px solid var(--border);border-radius:8px;background:transparent;color:var(--muted);font-size:.88rem;cursor:pointer}
.btn-cancel:hover{border-color:var(--text);color:var(--text)}
.btn-save{padding:.55rem 1.2rem;background:linear-gradient(135deg,var(--accent),var(--accent2));border:none;border-radius:8px;color:#fff;font-size:.88rem;font-weight:600;cursor:pointer}
#toast{position:fixed;bottom:1.5rem;left:50%;transform:translateX(-50%);background:var(--surface);border:1px solid var(--accent);color:var(--text);padding:.5rem 1.2rem;border-radius:20px;font-size:.85rem;opacity:0;transition:opacity .3s;pointer-events:none;white-space:nowrap;z-index:200}
#toast.show{opacity:1}
@media(max-width:520px){.settings-grid{grid-template-columns:1fr 1fr}}
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
  <nav>
    <a class="nav-link" href="/home">Home</a>
    <a class="nav-link" href="/dashboard">Manual</a>
    <a class="nav-link active" href="/auto">Auto</a>
    <a class="nav-link" href="/temp">Temp</a>
    <a class="nav-link" href="/settings">⚙</a>
    <button class="logout-btn" onclick="logout()">Sign Out</button>
  </nav>
</header>
<main>
  <div class="card">
    <div class="card-title">Global AC Settings<span class="section-badge">SECTION A</span></div>
    <div class="ntp-row">
      <div class="ntp-dot" id="ntpDot"></div>
      <span id="ntpLabel">NTP: checking…</span>
      <span style="margin-left:auto;font-variant-numeric:tabular-nums" id="ntpTime"></span>
    </div>
    <div class="settings-grid">
      <div class="setting-row"><label>Temperature</label><select class="ctrl" id="glTemp"></select></div>
      <div class="setting-row"><label>Mode</label>
        <select class="ctrl" id="glMode">
          <option value="0">❄️ Cool</option><option value="4">🔥 Heat</option>
          <option value="1">💧 Dry</option><option value="3">🌬️ Fan</option><option value="2">🔄 Auto</option>
        </select>
      </div>
      <div class="setting-row"><label>Fan Speed</label>
        <select class="ctrl" id="glFan">
          <option value="5">🤖 Auto</option><option value="2">🌿 Low</option>
          <option value="4">💨 Medium</option><option value="1">🚀 High</option>
        </select>
      </div>
    </div>
    <div style="margin-top:.85rem">
      <div style="font-size:.78rem;color:var(--muted);margin-bottom:.5rem">Special Modes</div>
      <div class="toggle-row">
        <button class="toggle-chip"       id="glSwing" onclick="toggleGlobal('swing')">🔃 Swing</button>
        <button class="toggle-chip turbo" id="glTurbo" onclick="toggleGlobal('turbo')">⚡ Turbo</button>
        <button class="toggle-chip sleep" id="glSleep" onclick="toggleGlobal('sleep')">🌙 Sleep</button>
      </div>
    </div>
    <button class="save-btn" onclick="saveGlobal()">💾 Save Global Settings</button>
  </div>
  <div class="card">
    <div class="card-title">Schedules<span class="section-badge">SECTION B</span></div>
    <div style="display:flex;align-items:center;justify-content:space-between;margin-bottom:.75rem">
      <span style="font-size:.84rem;color:var(--muted)" id="schedCountLabel">0 schedules</span>
      <button class="add-btn" onclick="openAddModal()">+ Add Schedule</button>
    </div>
    <div class="sched-list" id="schedList">
      <div class="empty-msg">No schedules yet. Click "Add Schedule" to create one.</div>
    </div>
  </div>
</main>
<div class="modal-overlay" id="modalOverlay">
  <div class="modal">
    <h3 id="modalTitle">Add Schedule</h3>
    <div class="modal-row"><label>Start Time</label><input class="ctrl" type="time" id="modalStart" value="08:00"></div>
    <div class="modal-row"><label>Stop Time</label><input class="ctrl" type="time" id="modalStop" value="10:00"></div>
    <div class="modal-row" style="display:flex;align-items:center;gap:.6rem">
      <input type="checkbox" id="modalEnabled" checked style="width:16px;height:16px;accent-color:var(--accent)">
      <label for="modalEnabled" style="margin:0;font-size:.88rem;color:var(--text);cursor:pointer">Enable this schedule</label>
    </div>
    <div class="modal-actions">
      <button class="btn-cancel" onclick="closeModal()">Cancel</button>
      <button class="btn-save" onclick="saveModal()">Save</button>
    </div>
  </div>
</div>
<div id="toast"></div>
<script>
let globalSettings={temp:24,mode:0,fan:5,swing:false,turbo:false,sleep:false};
let schedules=[], editingIdx=-1;

(function init(){
  buildTempSelect(); fetchGlobal(); fetchSchedules(); fetchNTP();
  setInterval(fetchNTP,10000);
})();

function buildTempSelect(){
  const s=document.getElementById('glTemp');
  for(let t=17;t<=30;t++){const o=document.createElement('option');o.value=t;o.textContent=t+'°C';s.appendChild(o);}
}

async function fetchNTP(){
  try{
    const r=await fetch('/api/ntp',{credentials:'include'});
    if(r.status===401){window.location.href='/';return;}
    const d=await r.json();
    const dot=document.getElementById('ntpDot');
    dot.className='ntp-dot'+(d.synced?' synced':'');
    document.getElementById('ntpLabel').textContent='NTP: '+(d.synced?'Synced':'Not synced');
    document.getElementById('ntpTime').textContent=d.synced?d.time:'';
  }catch(e){}
}

async function fetchGlobal(){
  try{
    const r=await fetch('/api/auto/settings',{credentials:'include'});
    if(r.status===401){window.location.href='/';return;}
    applyGlobalUI(await r.json());
  }catch(e){}
}

function applyGlobalUI(d){
  globalSettings=d;
  document.getElementById('glTemp').value=d.temp;
  document.getElementById('glMode').value=d.mode;
  document.getElementById('glFan').value=d.fan;
  document.getElementById('glSwing').classList.toggle('active',!!d.swing);
  document.getElementById('glTurbo').classList.toggle('active',!!d.turbo);
  document.getElementById('glSleep').classList.toggle('active',!!d.sleep);
}

function toggleGlobal(field){
  globalSettings[field]=!globalSettings[field];
  if(globalSettings[field]){['swing','turbo','sleep'].forEach(f=>{if(f!==field)globalSettings[f]=false;});}
  document.getElementById('glSwing').classList.toggle('active',!!globalSettings.swing);
  document.getElementById('glTurbo').classList.toggle('active',!!globalSettings.turbo);
  document.getElementById('glSleep').classList.toggle('active',!!globalSettings.sleep);
}

async function saveGlobal(){
  const payload={
    temp:parseInt(document.getElementById('glTemp').value),
    mode:parseInt(document.getElementById('glMode').value),
    fan: parseInt(document.getElementById('glFan').value),
    swing:globalSettings.swing, turbo:globalSettings.turbo, sleep:globalSettings.sleep
  };
  try{
    const r=await fetch('/api/auto/settings',{method:'POST',credentials:'include',
      headers:{'Content-Type':'application/json'},body:JSON.stringify(payload)});
    if(r.status===401){window.location.href='/';return;}
    const d=await r.json();
    if(r.ok){showToast('Global settings saved ✓');applyGlobalUI(d);}
    else showToast('Error: '+(d.error||'failed'),true);
  }catch(e){showToast('Connection error',true);}
}

async function fetchSchedules(){
  try{
    const r=await fetch('/api/auto/schedules',{credentials:'include'});
    if(r.status===401){window.location.href='/';return;}
    const d=await r.json();
    schedules=d.schedules||[]; renderSchedules();
  }catch(e){}
}

function renderSchedules(){
  const list=document.getElementById('schedList');
  const lbl=document.getElementById('schedCountLabel');
  lbl.textContent=schedules.length+' schedule'+(schedules.length!==1?'s':'');
  if(schedules.length===0){list.innerHTML='<div class="empty-msg">No schedules yet.</div>';return;}
  list.innerHTML='';
  schedules.forEach((s,i)=>{
    const div=document.createElement('div');
    div.className='sched-item'+(s.enabled?' enabled':'');
    div.innerHTML=`<div class="sched-num">${i+1}</div>
      <div class="sched-times">
        <div class="time-main">${fmt(s.startHH,s.startMM)} → ${fmt(s.stopHH,s.stopMM)}</div>
        <div class="time-sub">${s.enabled?'Active':'Disabled'}</div>
      </div>
      <button class="sched-toggle ${s.enabled?'on':'off'}" onclick="toggleSchedule(${i})">${s.enabled?'ON':'OFF'}</button>
      <button class="sched-edit" onclick="openEditModal(${i})">Edit</button>
      <button class="sched-del" onclick="deleteSchedule(${i})">✕</button>`;
    list.appendChild(div);
  });
}

function fmt(hh,mm){return String(hh).padStart(2,'0')+':'+String(mm).padStart(2,'0');}

async function toggleSchedule(idx){
  const en=!schedules[idx].enabled;
  try{
    const r=await fetch('/api/auto/schedules/'+idx+'/enable',{method:'POST',credentials:'include',
      headers:{'Content-Type':'application/json'},body:JSON.stringify({enabled:en})});
    if(r.status===401){window.location.href='/';return;}
    if(r.ok){schedules[idx].enabled=en;renderSchedules();showToast('Schedule '+(en?'enabled':'disabled'));}
    else showToast('Error',true);
  }catch(e){showToast('Connection error',true);}
}

async function deleteSchedule(idx){
  if(!confirm('Delete schedule '+(idx+1)+'?')) return;
  try{
    const r=await fetch('/api/auto/schedules/'+idx,{method:'DELETE',credentials:'include'});
    if(r.status===401){window.location.href='/';return;}
    if(r.ok){await fetchSchedules();showToast('Schedule deleted');}
    else showToast('Error',true);
  }catch(e){showToast('Connection error',true);}
}

function openAddModal(){
  editingIdx=-1;
  document.getElementById('modalTitle').textContent='Add Schedule';
  document.getElementById('modalStart').value='08:00';
  document.getElementById('modalStop').value='10:00';
  document.getElementById('modalEnabled').checked=true;
  document.getElementById('modalOverlay').classList.add('open');
}

function openEditModal(idx){
  editingIdx=idx; const s=schedules[idx];
  document.getElementById('modalTitle').textContent='Edit Schedule '+(idx+1);
  document.getElementById('modalStart').value=fmt(s.startHH,s.startMM);
  document.getElementById('modalStop').value=fmt(s.stopHH,s.stopMM);
  document.getElementById('modalEnabled').checked=s.enabled;
  document.getElementById('modalOverlay').classList.add('open');
}

function closeModal(){document.getElementById('modalOverlay').classList.remove('open');}

function parseTime(val){
  /* Handles both 24-hr "17:30" and 12-hr "05:30 PM" from Windows browsers */
  if(!val) return null;
  const upper=val.trim().toUpperCase();
  const isPM=upper.indexOf('PM')!==-1;
  const isAM=upper.indexOf('AM')!==-1;
  const cleaned=upper.replace('AM','').replace('PM','').trim();
  const parts=cleaned.split(':');
  if(parts.length<2) return null;
  let h=parseInt(parts[0],10), m=parseInt(parts[1],10);
  if(isNaN(h)||isNaN(m)) return null;
  if(isPM||isAM){
    /* 12-hr conversion */
    if(isPM && h!==12) h+=12;
    if(isAM && h===12) h=0;
  }
  if(h<0||h>23||m<0||m>59) return null;
  return {h,m};
}
async function saveModal(){
  const sv=document.getElementById('modalStart').value;
  const ev=document.getElementById('modalStop').value;
  const en=document.getElementById('modalEnabled').checked;
  if(!sv||!ev){showToast('Please enter both times',true);return;}
  const st=parseTime(sv), et=parseTime(ev);
  if(!st||!et){showToast('Invalid time format',true);return;}
  const [sH,sM]=[st.h,st.m], [eH,eM]=[et.h,et.m];
  const payload={startHH:sH,startMM:sM,stopHH:eH,stopMM:eM,enabled:en};
  try{
    let r;
    if(editingIdx<0){
      r=await fetch('/api/auto/schedules',{method:'POST',credentials:'include',headers:{'Content-Type':'application/json'},body:JSON.stringify(payload)});
    }else{
      r=await fetch('/api/auto/schedules/'+editingIdx,{method:'PUT',credentials:'include',headers:{'Content-Type':'application/json'},body:JSON.stringify(payload)});
    }
    if(r.status===401){window.location.href='/';return;}
    const d=await r.json();
    if(r.ok){closeModal();await fetchSchedules();showToast(editingIdx<0?'Schedule added ✓':'Schedule updated ✓');}
    else showToast('Error: '+(d.error||'failed'),true);
  }catch(e){showToast('Connection error',true);}
}

document.getElementById('modalOverlay').addEventListener('click',function(e){if(e.target===this)closeModal();});

async function logout(){await fetch('/api/logout',{method:'POST',credentials:'include'});window.location.href='/';}

let _tt;
function showToast(msg,err){
  const t=document.getElementById('toast');t.textContent=msg;
  t.style.borderColor=err?'var(--off)':'var(--accent)';
  t.classList.add('show');clearTimeout(_tt);_tt=setTimeout(()=>t.classList.remove('show'),2200);
}
</script>
</body>
</html>
)HTMLEOF";


// ─────────────────────────────────────────────────────────────────────────────
// TEMPERATURE MODE PAGE
// ─────────────────────────────────────────────────────────────────────────────
static const char TEMP_HTML[] PROGMEM = R"HTMLEOF(<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>SmartAC – Temperature Mode</title>
<style>
:root{--bg:#0d1117;--surface:#161b22;--surface2:#1c2128;--border:#30363d;
  --accent:#00b4d8;--accent2:#0077b6;--text:#e6edf3;--muted:#8b949e;
  --on:#39d353;--off:#f85149;--warn:#e3b341;--radius:14px;--gap:1rem}
*,*::before,*::after{box-sizing:border-box;margin:0;padding:0}
body{background:var(--bg);color:var(--text);font-family:'Segoe UI',system-ui,sans-serif;min-height:100vh;
  background-image:radial-gradient(ellipse at 50% 0%,rgba(227,179,65,.1) 0%,transparent 60%)}
header{display:flex;align-items:center;justify-content:space-between;padding:1rem 1.5rem;
  border-bottom:1px solid var(--border);background:var(--surface);position:sticky;top:0;z-index:10;flex-wrap:wrap;gap:.5rem}
.brand{display:flex;align-items:center;gap:.6rem;font-weight:700;font-size:1.1rem}
nav{display:flex;gap:.4rem;align-items:center;flex-wrap:wrap}
.nav-link{padding:.4rem .85rem;border-radius:8px;font-size:.84rem;color:var(--muted);text-decoration:none;transition:color .18s,background .18s}
.nav-link:hover{color:var(--text);background:var(--surface2)}
.nav-link.active{color:var(--accent);background:rgba(0,180,216,.1)}
.logout-btn{padding:.4rem .9rem;border:1px solid var(--border);border-radius:8px;background:transparent;color:var(--muted);font-size:.82rem;cursor:pointer;transition:color .2s,border-color .2s}
.logout-btn:hover{color:var(--off);border-color:var(--off)}
main{padding:1.5rem;max-width:560px;margin:0 auto;display:grid;gap:var(--gap)}
.card{background:var(--surface);border:1px solid var(--border);border-radius:var(--radius);padding:1.5rem}
.card-title{font-size:.72rem;text-transform:uppercase;letter-spacing:.09em;color:var(--muted);margin-bottom:1.25rem}
.status-banner{display:flex;align-items:center;gap:.75rem;padding:.9rem 1.1rem;border-radius:10px;margin-bottom:1rem;font-size:.9rem;font-weight:600;border:1px solid}
.status-banner.idle{background:var(--surface2);border-color:var(--border);color:var(--muted)}
.status-banner.cooling{background:rgba(0,180,216,.1);border-color:var(--accent);color:var(--accent)}
.status-dot{width:10px;height:10px;border-radius:50%;flex-shrink:0}
.status-banner.idle .status-dot{background:var(--muted)}
.status-banner.cooling .status-dot{background:var(--accent);box-shadow:0 0 6px var(--accent)}
.field-row{margin-bottom:1.1rem}
.field-row label{display:block;font-size:.8rem;color:var(--muted);margin-bottom:.4rem}
.field-row input[type=number]{width:100%;padding:.6rem .85rem;background:var(--surface2);border:1px solid var(--border);border-radius:8px;color:var(--text);font-size:1rem;outline:none;transition:border-color .2s}
.field-row input:focus{border-color:var(--accent)}
.range-hint{font-size:.75rem;color:var(--muted);margin-top:.25rem}
.toggle-wrap{display:flex;align-items:center;gap:.85rem;margin-top:.5rem}
.toggle{position:relative;width:44px;height:24px;flex-shrink:0}
.toggle input{opacity:0;width:0;height:0}
.slider{position:absolute;inset:0;border-radius:24px;background:var(--border);cursor:pointer;transition:background .2s}
.slider:before{content:'';position:absolute;width:18px;height:18px;border-radius:50%;background:#fff;left:3px;top:3px;transition:transform .2s}
input:checked+.slider{background:var(--accent)}
input:checked+.slider:before{transform:translateX(20px)}
.save-btn{width:100%;margin-top:1.1rem;padding:.7rem;background:linear-gradient(135deg,var(--accent),var(--accent2));border:none;border-radius:8px;color:#fff;font-size:.95rem;font-weight:600;cursor:pointer;transition:opacity .2s}
.save-btn:hover{opacity:.88}
.sensor-pill{background:var(--surface2);border-radius:10px;padding:.8rem 1rem;text-align:center;margin-bottom:1.25rem}
.sensor-pill .val{font-size:1.5rem;font-weight:700;color:var(--warn)}
.sensor-pill .lbl{font-size:.72rem;color:var(--muted);margin-top:.2rem}
#toast{position:fixed;bottom:1.5rem;left:50%;transform:translateX(-50%);background:var(--surface);border:1px solid var(--accent);color:var(--text);padding:.5rem 1.2rem;border-radius:20px;font-size:.85rem;opacity:0;transition:opacity .3s;pointer-events:none;white-space:nowrap;z-index:99}
#toast.show{opacity:1}
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
  <nav>
    <a class="nav-link" href="/home">Home</a>
    <a class="nav-link" href="/dashboard">Manual</a>
    <a class="nav-link" href="/auto">Auto</a>
    <a class="nav-link active" href="/temp">Temp</a>
    <a class="nav-link" href="/settings">⚙</a>
    <button class="logout-btn" onclick="logout()">Sign Out</button>
  </nav>
</header>
<main>
  <div class="card">
    <div class="card-title">Temperature Mode</div>
    <div class="status-banner idle" id="statusBanner">
      <div class="status-dot"></div><span id="statusText">Idle</span>
    </div>
    <div class="sensor-pill">
      <div class="val" id="roomTempVal">--</div>
      <div class="lbl">Room °C</div>
    </div>
    <div class="field-row">
      <label>Threshold Temperature (°C)</label>
      <input type="number" id="threshold" min="17" max="50" value="30">
      <div class="range-hint">AC turns ON when room reaches this temperature</div>
    </div>
    <div class="field-row">
      <label>Cooling Temperature (°C)</label>
      <input type="number" id="coolingTemp" min="17" max="30" value="25">
      <div class="range-hint">AC set to this temperature; turns OFF when room reaches it</div>
    </div>
    <div class="toggle-wrap">
      <label class="toggle"><input type="checkbox" id="tmEnabled"><span class="slider"></span></label>
      <span class="toggle-label" id="toggleLabel">Disabled</span>
    </div>
    <button class="save-btn" onclick="save()">💾 Save Settings</button>
  </div>
</main>
<div id="toast"></div>
<script>
async function logout(){await fetch('/api/logout',{method:'POST',credentials:'include'});window.location.href='/';}

(async function init(){
  try{
    const [ts,st]=await Promise.all([
      fetch('/api/tempmode',{credentials:'include'}),
      fetch('/api/status',  {credentials:'include'})
    ]);
    if(ts.status===401||st.status===401){window.location.href='/';return;}
    const t=await ts.json(), s=await st.json();
    document.getElementById('threshold').value=t.threshold;
    document.getElementById('coolingTemp').value=t.coolingTemp;
    document.getElementById('tmEnabled').checked=t.enabled;
    document.getElementById('toggleLabel').textContent=t.enabled?'Enabled':'Disabled';
    document.getElementById('roomTempVal').textContent=s.roomTemp!=null?s.roomTemp.toFixed(1):'--';
    updateStatus(t.coolingActive);
  }catch(e){}
  document.getElementById('tmEnabled').addEventListener('change',function(){
    document.getElementById('toggleLabel').textContent=this.checked?'Enabled':'Disabled';
  });
  setInterval(pollStatus,5000);
})();

async function pollStatus(){
  try{
    const [ts,st]=await Promise.all([
      fetch('/api/tempmode',{credentials:'include'}),
      fetch('/api/status',  {credentials:'include'})
    ]);
    if(ts.status===401||st.status===401){window.location.href='/';return;}
    if(!ts.ok||!st.ok) return;
    const t=await ts.json(), s=await st.json();
    document.getElementById('roomTempVal').textContent=s.roomTemp!=null?s.roomTemp.toFixed(1):'--';
    updateStatus(t.coolingActive);
  }catch(e){}
}

function updateStatus(active){
  const b=document.getElementById('statusBanner'), tx=document.getElementById('statusText');
  b.className='status-banner '+(active?'cooling':'idle');
  tx.textContent=active?'Cooling Active':'Idle';
}

async function save(){
  const payload={
    threshold:parseInt(document.getElementById('threshold').value),
    coolingTemp:parseInt(document.getElementById('coolingTemp').value),
    enabled:document.getElementById('tmEnabled').checked
  };
  if(isNaN(payload.threshold)||isNaN(payload.coolingTemp)){showToast('Invalid values',true);return;}
  try{
    const r=await fetch('/api/tempmode',{method:'POST',credentials:'include',
      headers:{'Content-Type':'application/json'},body:JSON.stringify(payload)});
    if(r.status===401){window.location.href='/';return;}
    const d=await r.json();
    if(r.ok) showToast('Settings saved ✓');
    else showToast('Error: '+(d.error||'failed'),true);
  }catch(e){showToast('Connection error',true);}
}

let _tt;
function showToast(msg,err){
  const t=document.getElementById('toast');t.textContent=msg;
  t.style.borderColor=err?'var(--off)':'var(--accent)';
  t.classList.add('show');clearTimeout(_tt);_tt=setTimeout(()=>t.classList.remove('show'),2200);
}
</script>
</body>
</html>
)HTMLEOF";


// ─────────────────────────────────────────────────────────────────────────────
// SETTINGS PAGE
// ─────────────────────────────────────────────────────────────────────────────
static const char SETTINGS_HTML[] PROGMEM = R"HTMLEOF(<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>SmartAC – Settings</title>
<style>
:root{--bg:#0d1117;--surface:#161b22;--surface2:#1c2128;--border:#30363d;
  --accent:#00b4d8;--accent2:#0077b6;--text:#e6edf3;--muted:#8b949e;
  --on:#39d353;--off:#f85149;--warn:#e3b341;--radius:14px;--gap:1rem}
*,*::before,*::after{box-sizing:border-box;margin:0;padding:0}
body{background:var(--bg);color:var(--text);font-family:'Segoe UI',system-ui,sans-serif;min-height:100vh}
header{display:flex;align-items:center;justify-content:space-between;padding:1rem 1.5rem;
  border-bottom:1px solid var(--border);background:var(--surface);position:sticky;top:0;z-index:10;flex-wrap:wrap;gap:.5rem}
.brand{display:flex;align-items:center;gap:.6rem;font-weight:700;font-size:1.1rem}
nav{display:flex;gap:.4rem;align-items:center;flex-wrap:wrap}
.nav-link{padding:.4rem .85rem;border-radius:8px;font-size:.84rem;color:var(--muted);text-decoration:none;transition:color .18s,background .18s}
.nav-link:hover{color:var(--text);background:var(--surface2)}
.nav-link.active{color:var(--accent);background:rgba(0,180,216,.1)}
.logout-btn{padding:.4rem .9rem;border:1px solid var(--border);border-radius:8px;background:transparent;color:var(--muted);font-size:.82rem;cursor:pointer;transition:color .2s,border-color .2s}
.logout-btn:hover{color:var(--off);border-color:var(--off)}
main{padding:1.5rem;max-width:560px;margin:0 auto;display:grid;gap:var(--gap)}
.card{background:var(--surface);border:1px solid var(--border);border-radius:var(--radius);padding:1.5rem}
.card-title{font-size:.72rem;text-transform:uppercase;letter-spacing:.09em;color:var(--muted);margin-bottom:1.1rem}
.field{margin-bottom:.9rem}
.field label{display:block;font-size:.8rem;color:var(--muted);margin-bottom:.35rem}
.field input{width:100%;padding:.6rem .85rem;background:var(--surface2);border:1px solid var(--border);border-radius:8px;color:var(--text);font-size:.92rem;outline:none;transition:border-color .2s}
.field input:focus{border-color:var(--accent)}
.save-btn{width:100%;padding:.65rem;background:linear-gradient(135deg,var(--accent),var(--accent2));border:none;border-radius:8px;color:#fff;font-size:.92rem;font-weight:600;cursor:pointer;transition:opacity .2s;margin-top:.5rem}
.save-btn:hover{opacity:.88}
.danger-btn{width:100%;padding:.65rem;background:rgba(248,81,73,.1);border:1px solid var(--off);border-radius:8px;color:var(--off);font-size:.92rem;font-weight:600;cursor:pointer;transition:background .2s;margin-top:.5rem}
.danger-btn:hover{background:rgba(248,81,73,.2)}
.hint{font-size:.75rem;color:var(--muted);margin-top:.75rem;line-height:1.5}
#toast{position:fixed;bottom:1.5rem;left:50%;transform:translateX(-50%);background:var(--surface);border:1px solid var(--accent);color:var(--text);padding:.5rem 1.2rem;border-radius:20px;font-size:.85rem;opacity:0;transition:opacity .3s;pointer-events:none;white-space:nowrap;z-index:99}
#toast.show{opacity:1}
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
  <nav>
    <a class="nav-link" href="/home">Home</a>
    <a class="nav-link" href="/dashboard">Manual</a>
    <a class="nav-link" href="/auto">Auto</a>
    <a class="nav-link" href="/temp">Temp</a>
    <a class="nav-link active" href="/settings">⚙ Settings</a>
    <button class="logout-btn" onclick="logout()">Sign Out</button>
  </nav>
</header>
<main>
  <!-- Login Credentials -->
  <div class="card">
    <div class="card-title">Login Credentials</div>
    <div class="field"><label>New Username</label><input type="text" id="newUser" autocomplete="username" placeholder="admin"></div>
    <div class="field"><label>New Password</label><input type="password" id="newPass" autocomplete="new-password" placeholder="••••••••"></div>
    <div class="field"><label>Confirm Password</label><input type="password" id="newPass2" autocomplete="new-password" placeholder="••••••••"></div>
    <button class="save-btn" onclick="saveCredentials()">💾 Update Login</button>
    <div class="hint">After saving you will be signed out and must log in again.</div>
  </div>
  <!-- WiFi -->
  <div class="card">
    <div class="card-title">WiFi Settings</div>
    <div class="field"><label>WiFi SSID</label><input type="text" id="wifiSSID" autocomplete="off" placeholder="Network name"></div>
    <div class="field"><label>WiFi Password</label><input type="password" id="wifiPass" autocomplete="off" placeholder="••••••••"></div>
    <button class="save-btn" onclick="saveWiFi()">💾 Save &amp; Reconnect</button>
    <div class="hint">Device will reconnect. If connection fails within 60 s, it starts AP mode using the configured setup credentials.</div>
  </div>
  <!-- Cooling Verification -->
  <div class="card">
    <div class="card-title">Cooling Verification</div>
    <div id="cvBanner" style="display:flex;align-items:center;gap:.75rem;padding:.8rem 1rem;border-radius:10px;margin-bottom:1.1rem;font-size:.87rem;font-weight:600;border:1px solid var(--border);background:var(--surface2);color:var(--muted)">
      <div id="cvDot" style="width:10px;height:10px;border-radius:50%;background:var(--muted);flex-shrink:0"></div>
      <span id="cvStatus">Loading…</span>
    </div>
    <div id="cvStats" style="display:none;margin-bottom:1rem">
      <div style="display:grid;grid-template-columns:repeat(3,1fr);gap:.6rem;margin-bottom:.7rem">
        <div style="background:var(--surface2);border-radius:8px;padding:.6rem;text-align:center">
          <div style="font-size:1.3rem;font-weight:700;color:var(--warn)" id="cvBaseline">--</div>
          <div style="font-size:.7rem;color:var(--muted);margin-top:.2rem">Baseline °C</div>
        </div>
        <div style="background:var(--surface2);border-radius:8px;padding:.6rem;text-align:center">
          <div style="font-size:1.3rem;font-weight:700;color:var(--accent)" id="cvCurrent">--</div>
          <div style="font-size:.7rem;color:var(--muted);margin-top:.2rem">Current °C</div>
        </div>
        <div style="background:var(--surface2);border-radius:8px;padding:.6rem;text-align:center">
          <div style="font-size:1.3rem;font-weight:700;color:var(--on)" id="cvRetry">0</div>
          <div style="font-size:.7rem;color:var(--muted);margin-top:.2rem">Retries</div>
        </div>
      </div>
      <div style="font-size:.78rem;color:var(--muted)" id="cvElapsed"></div>
    </div>
    <div style="display:flex;align-items:center;gap:.85rem;margin-bottom:1rem">
      <label style="position:relative;width:44px;height:24px;flex-shrink:0">
        <input type="checkbox" id="cvEnabled" style="opacity:0;width:0;height:0">
        <span id="cvToggleSlider" style="position:absolute;inset:0;border-radius:24px;background:var(--border);cursor:pointer;transition:background .2s;"></span>
      </label>
      <span id="cvEnabledLabel" style="font-size:.9rem">Enable Cooling Verification</span>
    </div>
    <div class="field"><label>Verification Interval (minutes)</label><input type="number" id="cvInterval" min="1" max="120" value="10"></div>
    <div class="field"><label>Required Cooling (°C)</label><input type="number" id="cvDrop" min="0.1" max="10" step="0.1" value="1.0"></div>
    <button class="save-btn" onclick="saveCooling()">💾 Save Cooling Settings</button>
  </div>
  <!-- Factory Reset -->
  <div class="card">
    <div class="card-title">Factory Reset</div>
    <p style="font-size:.88rem;color:var(--muted);margin-bottom:1rem;line-height:1.6">Clears all stored settings: credentials, WiFi, schedules, temperature mode.<br>Device will restart with default values.</p>
    <button class="danger-btn" onclick="factoryReset()">⚠️ Factory Reset</button>
  </div>
</main>
<div id="toast"></div>
<script>
async function logout(){await fetch('/api/logout',{method:'POST',credentials:'include'});window.location.href='/';}

(async function(){
  try{
    const r=await fetch('/api/settings',{credentials:'include'});
    if(r.status===401){window.location.href='/';return;}
    const d=await r.json();
    document.getElementById('wifiSSID').value=d.wifiSSID||'';
    document.getElementById('newUser').value=d.username||'';
  }catch(e){}
})();

async function saveCredentials(){
  const u=document.getElementById('newUser').value.trim();
  const p=document.getElementById('newPass').value;
  const p2=document.getElementById('newPass2').value;
  if(!u||!p){showToast('Username and password required',true);return;}
  if(p!==p2){showToast('Passwords do not match',true);return;}
  try{
    const r=await fetch('/api/settings/credentials',{method:'POST',credentials:'include',
      headers:{'Content-Type':'application/json'},body:JSON.stringify({username:u,password:p})});
    if(r.status===401){window.location.href='/';return;}
    const d=await r.json();
    // Server clears the cookie on credential change; redirect to login.
    if(r.ok){showToast('Credentials updated – signing out…');setTimeout(()=>window.location.href='/',1800);}
    else showToast('Error: '+(d.error||'failed'),true);
  }catch(e){showToast('Connection error',true);}
}

async function saveWiFi(){
  const ssid=document.getElementById('wifiSSID').value.trim();
  const pass=document.getElementById('wifiPass').value;
  if(!ssid){showToast('SSID required',true);return;}
  try{
    const r=await fetch('/api/settings/wifi',{method:'POST',credentials:'include',
      headers:{'Content-Type':'application/json'},body:JSON.stringify({ssid,password:pass})});
    if(r.status===401){window.location.href='/';return;}
    const d=await r.json();
    if(r.ok) showToast('WiFi saved – device restarting…');
    else showToast('Error: '+(d.error||'failed'),true);
  }catch(e){showToast('WiFi saved – reconnecting…',false);}
}

async function factoryReset(){
  if(!confirm('Reset ALL settings and restart?')) return;
  try{await fetch('/api/settings/factory-reset',{method:'POST',credentials:'include'});}catch(e){}
  showToast('Resetting… device will restart');
}

// ── Cooling Verification toggle knob ──────────────────────────────────────────
(function(){
  const st=document.createElement('style');
  st.textContent='#cvToggleSlider::before{content:"";position:absolute;width:18px;height:18px;border-radius:50%;background:#fff;left:3px;top:3px;transition:transform .2s}';
  document.head.appendChild(st);
})();

document.getElementById('cvEnabled').addEventListener('change',function(){
  const s=document.getElementById('cvToggleSlider');
  s.style.background=this.checked?'var(--accent)':'var(--border)';
  document.getElementById('cvEnabledLabel').textContent=
    this.checked?'Cooling Verification Enabled':'Cooling Verification Disabled';
});

(async function initCooling(){await fetchCooling();setInterval(fetchCooling,5000);})();

async function fetchCooling(){
  try{
    const r=await fetch('/api/cooling',{credentials:'include'});
    if(r.status===401){window.location.href='/';return;}
    if(!r.ok) return;
    applyCoolingUI(await r.json());
  }catch(e){}
}

function applyCoolingUI(d){
  const cb=document.getElementById('cvEnabled');
  cb.checked=d.enabled;
  document.getElementById('cvToggleSlider').style.background=d.enabled?'var(--accent)':'var(--border)';
  document.getElementById('cvEnabledLabel').textContent=
    d.enabled?'Cooling Verification Enabled':'Cooling Verification Disabled';
  document.getElementById('cvInterval').value=d.intervalMin;
  document.getElementById('cvDrop').value=d.requiredDrop;

  const banner=document.getElementById('cvBanner');
  const dot=document.getElementById('cvDot');
  const status=document.getElementById('cvStatus');
  const base='display:flex;align-items:center;gap:.75rem;padding:.8rem 1rem;border-radius:10px;margin-bottom:1.1rem;font-size:.87rem;font-weight:600;border:1px solid ';
  if(!d.enabled){
    banner.style.cssText=base+'var(--border);background:var(--surface2);color:var(--muted)';
    dot.style.background='var(--muted)';status.textContent='Disabled';
  }else if(d.verified){
    banner.style.cssText=base+'var(--on);background:rgba(57,211,83,.08);color:var(--on)';
    dot.style.background='var(--on)';status.textContent='✓ Cooling Verified';
  }else if(d.active){
    banner.style.cssText=base+'var(--accent);background:rgba(0,180,216,.08);color:var(--accent)';
    dot.style.background='var(--accent)';status.textContent='Monitoring Cooling…';
  }else{
    banner.style.cssText=base+'var(--border);background:var(--surface2);color:var(--muted)';
    dot.style.background='var(--muted)';status.textContent=d.status||'Idle';
  }
  const stats=document.getElementById('cvStats');
  if(d.active&&d.enabled){
    stats.style.display='block';
    document.getElementById('cvBaseline').textContent=d.baselineTemp>0?d.baselineTemp.toFixed(1):'--';
    document.getElementById('cvCurrent').textContent=d.currentTemp>0?d.currentTemp.toFixed(1):'--';
    document.getElementById('cvRetry').textContent=d.retryCount;
    const remain=Math.max(0,d.intervalMin*60-(d.elapsedSec||0));
    document.getElementById('cvElapsed').textContent='Next check in '+
      String(Math.floor(remain/60)).padStart(2,'0')+':'+String(remain%60).padStart(2,'0');
  }else{stats.style.display='none';}
}

async function saveCooling(){
  const payload={
    enabled:document.getElementById('cvEnabled').checked,
    intervalMin:parseInt(document.getElementById('cvInterval').value),
    requiredDrop:parseFloat(document.getElementById('cvDrop').value)
  };
  if(isNaN(payload.intervalMin)||isNaN(payload.requiredDrop)){showToast('Invalid values',true);return;}
  try{
    const r=await fetch('/api/cooling',{method:'POST',credentials:'include',
      headers:{'Content-Type':'application/json'},body:JSON.stringify(payload)});
    if(r.status===401){window.location.href='/';return;}
    const d=await r.json();
    if(r.ok){showToast('Cooling settings saved ✓');await fetchCooling();}
    else showToast('Error: '+(d.error||'failed'),true);
  }catch(e){showToast('Connection error',true);}
}

let _tt;
function showToast(msg,err){
  const t=document.getElementById('toast');t.textContent=msg;
  t.style.borderColor=err?'var(--off)':'var(--accent)';
  t.classList.add('show');clearTimeout(_tt);_tt=setTimeout(()=>t.classList.remove('show'),2500);
}
</script>
</body>
</html>
)HTMLEOF";
