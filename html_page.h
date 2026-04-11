#pragma once

const char HTML_PAGE[] PROGMEM = R"=====(
<!DOCTYPE html>
<html lang="ar" dir="rtl">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1,maximum-scale=1">
<title>فقاسة بيض ذكية</title>
<style>
  @import url('https://fonts.googleapis.com/css2?family=Cairo:wght@300;400;600;700;900&display=swap');

  :root {
    --bg:       #0a0c10;
    --surface:  #111520;
    --card:     #161c2a;
    --border:   #1e2840;
    --accent:   #f5a623;
    --accent2:  #e05a1e;
    --blue:     #3b82f6;
    --green:    #10b981;
    --red:      #ef4444;
    --text:     #e2e8f0;
    --muted:    #64748b;
    --glow:     0 0 20px rgba(245,166,35,.25);
  }

  * { margin:0; padding:0; box-sizing:border-box; }

  body {
    font-family: 'Cairo', sans-serif;
    background: var(--bg);
    color: var(--text);
    min-height: 100vh;
    overflow-x: hidden;
  }

  /* ── TOP BAR ── */
  header {
    background: var(--surface);
    border-bottom: 1px solid var(--border);
    padding: 14px 20px;
    display: flex;
    align-items: center;
    justify-content: space-between;
    position: sticky; top: 0; z-index: 100;
    backdrop-filter: blur(12px);
  }
  .logo {
    display: flex; align-items: center; gap: 10px;
    font-size: 20px; font-weight: 900; letter-spacing: -.5px;
  }
  .logo-icon { font-size: 26px; }
  .logo span { color: var(--accent); }

  .status-bar {
    display: flex; align-items: center; gap: 8px;
    font-size: 12px; color: var(--muted);
  }
  .dot {
    width: 8px; height: 8px; border-radius: 50%;
    background: var(--green);
    box-shadow: 0 0 8px var(--green);
    animation: pulse 2s infinite;
  }
  .dot.error { background: var(--red); box-shadow: 0 0 8px var(--red); }
  @keyframes pulse { 0%,100%{opacity:1} 50%{opacity:.4} }

  /* ── MAIN GRID ── */
  main {
    max-width: 900px;
    margin: 0 auto;
    padding: 20px 16px 40px;
    display: grid;
    gap: 16px;
    grid-template-columns: 1fr 1fr;
  }

  @media(max-width:600px) {
    main { grid-template-columns: 1fr; }
  }

  /* ── CARDS ── */
  .card {
    background: var(--card);
    border: 1px solid var(--border);
    border-radius: 16px;
    padding: 20px;
    position: relative;
    overflow: hidden;
    transition: border-color .3s;
  }
  .card:hover { border-color: var(--accent); }
  .card::before {
    content: '';
    position: absolute; top:0; left:0; right:0; height:2px;
    background: linear-gradient(90deg, var(--accent), var(--accent2));
    opacity: 0;
    transition: opacity .3s;
  }
  .card:hover::before { opacity: 1; }

  .card-title {
    font-size: 13px; font-weight: 600;
    color: var(--muted); text-transform: uppercase;
    letter-spacing: 1px; margin-bottom: 14px;
    display: flex; align-items: center; gap: 6px;
  }

  /* ── BIG METRIC ── */
  .metric-big {
    font-size: 52px; font-weight: 900;
    line-height: 1;
    background: linear-gradient(135deg, #fff, var(--accent));
    -webkit-background-clip: text;
    -webkit-text-fill-color: transparent;
    margin-bottom: 4px;
  }
  .metric-unit { font-size: 18px; font-weight: 400; }
  .metric-sub  { font-size: 13px; color: var(--muted); margin-top: 4px; }
  .metric-range {
    display: flex; gap: 12px; margin-top: 10px;
    font-size: 12px; color: var(--muted);
  }
  .metric-range span { color: var(--text); }

  /* ── PROGRESS BAR ── */
  .progress-wrap {
    background: var(--border);
    border-radius: 99px;
    height: 6px; overflow: hidden;
    margin-top: 12px;
  }
  .progress-bar {
    height: 100%; border-radius: 99px;
    background: linear-gradient(90deg, var(--accent2), var(--accent));
    transition: width .5s ease;
  }

  /* ── STATUS BADGES ── */
  .badge {
    display: inline-flex; align-items: center; gap: 6px;
    padding: 5px 12px; border-radius: 99px;
    font-size: 12px; font-weight: 700;
    letter-spacing: .5px;
  }
  .badge-on  { background: rgba(16,185,129,.15); color: var(--green); border: 1px solid rgba(16,185,129,.3); }
  .badge-off { background: rgba(100,116,139,.1);  color: var(--muted);  border: 1px solid var(--border); }
  .badge-warn{ background: rgba(245,166,35,.15);  color: var(--accent); border: 1px solid rgba(245,166,35,.3); }

  /* ── TOGGLE SWITCH ── */
  .toggle-row {
    display: flex; align-items: center; justify-content: space-between;
    padding: 12px 0;
    border-bottom: 1px solid var(--border);
  }
  .toggle-row:last-child { border-bottom: none; }
  .toggle-label {
    display: flex; flex-direction: column; gap: 2px;
  }
  .toggle-label strong { font-size: 14px; }
  .toggle-label small  { font-size: 11px; color: var(--muted); }

  .sw { position: relative; display: inline-block; width: 48px; height: 26px; }
  .sw input { opacity: 0; width: 0; height: 0; }
  .slider {
    position: absolute; inset: 0;
    background: var(--border); border-radius: 26px;
    cursor: pointer; transition: .3s;
  }
  .slider::before {
    content: ''; position: absolute;
    height: 20px; width: 20px; left: 3px; bottom: 3px;
    background: var(--muted); border-radius: 50%;
    transition: .3s;
  }
  input:checked + .slider { background: var(--green); }
  input:checked + .slider::before { transform: translateX(22px); background: #fff; }

  /* ── BUTTONS ── */
  .btn {
    border: none; border-radius: 10px;
    padding: 10px 18px;
    font-family: 'Cairo', sans-serif;
    font-size: 13px; font-weight: 700;
    cursor: pointer; transition: all .2s;
    display: inline-flex; align-items: center; gap: 6px;
  }
  .btn-primary {
    background: var(--accent); color: #000;
  }
  .btn-primary:hover { background: #ffb83d; transform: translateY(-1px); box-shadow: var(--glow); }
  .btn-secondary {
    background: var(--border); color: var(--text);
  }
  .btn-secondary:hover { background: #263045; }
  .btn-danger {
    background: rgba(239,68,68,.15); color: var(--red);
    border: 1px solid rgba(239,68,68,.3);
  }
  .btn-danger:hover { background: rgba(239,68,68,.25); }
  .btn-blue {
    background: rgba(59,130,246,.15); color: var(--blue);
    border: 1px solid rgba(59,130,246,.3);
  }
  .btn-blue:hover { background: rgba(59,130,246,.3); }
  .btn:active { transform: scale(.96); }
  .btn:disabled { opacity: .4; cursor: not-allowed; transform: none; }

  /* ── INPUTS ── */
  .input-group {
    display: flex; flex-direction: column; gap: 6px;
    margin-bottom: 14px;
  }
  .input-group label {
    font-size: 12px; color: var(--muted); font-weight: 600;
  }
  .input-group input, .input-group select {
    background: var(--surface);
    border: 1px solid var(--border);
    border-radius: 10px;
    color: var(--text);
    padding: 10px 14px;
    font-family: 'Cairo', sans-serif;
    font-size: 14px;
    transition: border-color .2s;
    width: 100%;
  }
  .input-group input:focus, .input-group select:focus {
    outline: none;
    border-color: var(--accent);
    box-shadow: 0 0 0 3px rgba(245,166,35,.1);
  }

  /* ── INCUBATION DAY PROGRESS ── */
  .day-card { grid-column: 1 / -1; }
  .day-progress {
    display: flex; align-items: center; gap: 12px;
    margin-bottom: 10px;
  }
  .day-num {
    font-size: 48px; font-weight: 900;
    color: var(--accent);
    line-height: 1;
    min-width: 70px;
  }
  .day-info { flex: 1; }
  .day-info h3 { font-size: 18px; margin-bottom: 4px; }
  .day-info p  { font-size: 12px; color: var(--muted); }

  .days-track {
    display: flex; gap: 3px; flex-wrap: wrap; margin-top: 12px;
  }
  .day-dot {
    width: 10px; height: 10px; border-radius: 50%;
    background: var(--border);
    transition: background .3s;
  }
  .day-dot.done   { background: var(--accent); }
  .day-dot.today  { background: var(--green); box-shadow: 0 0 6px var(--green); }
  .day-dot.future { background: var(--border); }

  /* ── SERVO VISUAL ── */
  .servo-visual {
    display: flex; flex-direction: column; align-items: center;
    padding: 10px 0;
  }
  .egg-tray {
    width: 120px; height: 60px;
    background: var(--surface);
    border: 2px solid var(--border);
    border-radius: 12px;
    display: flex; align-items: center; justify-content: center;
    font-size: 28px;
    transition: transform .5s ease;
    margin-bottom: 12px;
  }
  .egg-tray.pos-b { transform: rotate(25deg); }

  /* ── GRID SPAN ── */
  .full-width { grid-column: 1 / -1; }

  /* ── TOAST ── */
  #toast {
    position: fixed; bottom: 20px; left: 50%; transform: translateX(-50%) translateY(80px);
    background: var(--surface); border: 1px solid var(--border);
    border-radius: 10px; padding: 10px 20px;
    font-size: 14px; font-weight: 600;
    transition: transform .3s;
    z-index: 999;
    min-width: 200px; text-align: center;
  }
  #toast.show { transform: translateX(-50%) translateY(0); }
  #toast.success { border-color: var(--green); color: var(--green); }
  #toast.error   { border-color: var(--red);   color: var(--red); }

  /* ── SECTION DIVIDER ── */
  .sect-label {
    font-size: 11px; font-weight: 700;
    color: var(--muted); text-transform: uppercase;
    letter-spacing: 1.5px;
    padding: 6px 0 2px;
    border-top: 1px solid var(--border);
    margin-top: 6px;
  }

  /* ── ALERT ── */
  .alert {
    border-radius: 10px; padding: 10px 14px;
    font-size: 13px; margin-top: 10px;
    display: none;
  }
  .alert-danger { background: rgba(239,68,68,.1); border: 1px solid rgba(239,68,68,.3); color: var(--red); }
  .alert.show { display: block; }

  .row-2 { display: grid; grid-template-columns: 1fr 1fr; gap: 10px; }
</style>
</head>
<body>

<header>
  <div class="logo">
    <span class="logo-icon">🥚</span>
    فقاسة <span>ذكية</span>
  </div>
  <div class="status-bar">
    <div class="dot" id="connDot"></div>
    <span id="connLabel">متصل</span>
    &nbsp;|&nbsp;
    <span id="uptimeLabel">--</span>
  </div>
</header>

<main>

  <!-- ── TEMPERATURE CARD ── -->
  <div class="card" id="tempCard">
    <div class="card-title">🌡️ درجة الحرارة</div>
    <div class="metric-big" id="tempVal">--<span class="metric-unit">°C</span></div>
    <div class="metric-sub">الهدف: <span id="tempTarget">--</span>°C</div>
    <div class="metric-range">
      <span>↓ أدنى: <span id="minTemp">--</span>°C</span>
      <span>↑ أقصى: <span id="maxTemp">--</span>°C</span>
    </div>
    <div class="progress-wrap">
      <div class="progress-bar" id="tempBar" style="width:0%"></div>
    </div>
    <div style="margin-top:12px">
      <span class="badge badge-off" id="heaterBadge">🔌 السخان: متوقف</span>
    </div>
    <div class="alert alert-danger" id="tempAlert">⚠️ خطأ في قراءة الحساس!</div>
  </div>

  <!-- ── HUMIDITY CARD ── -->
  <div class="card" id="humCard">
    <div class="card-title">💧 الرطوبة</div>
    <div class="metric-big" id="humVal">--<span class="metric-unit">%</span></div>
    <div class="metric-sub">الهدف: <span id="humTarget">--</span>%</div>
    <div class="metric-range">
      <span>↓ أدنى: <span id="minHum">--</span>%</span>
      <span>↑ أقصى: <span id="maxHum">--</span>%</span>
    </div>
    <div class="progress-wrap">
      <div class="progress-bar" id="humBar" style="width:0%;background:linear-gradient(90deg,#3b82f6,#06b6d4)"></div>
    </div>
    <div style="margin-top:12px">
      <span class="badge badge-off" id="fanBadge">💨 المروحة: متوقفة</span>
    </div>
  </div>

  <!-- ── DAY PROGRESS CARD ── -->
  <div class="card day-card">
    <div class="card-title">📅 تقدم الحضانة</div>
    <div class="day-progress">
      <div class="day-num" id="dayNum">0</div>
      <div class="day-info">
        <h3 id="dayLabel">بداية الحضانة</h3>
        <p id="dayPct">0% مكتمل</p>
      </div>
    </div>
    <div class="days-track" id="dayTrack"></div>
  </div>

  <!-- ── CONTROLS CARD ── -->
  <div class="card">
    <div class="card-title">🎛️ التحكم</div>

    <div class="toggle-row">
      <div class="toggle-label">
        <strong>وضع تلقائي</strong>
        <small>تحكم ذكي في الحرارة والرطوبة</small>
      </div>
      <label class="sw">
        <input type="checkbox" id="autoToggle" checked onchange="setAuto(this.checked)">
        <span class="slider"></span>
      </label>
    </div>

    <div id="manualControls" style="opacity:.4;pointer-events:none">
      <div class="sect-label">تحكم يدوي</div>
      <div class="toggle-row">
        <div class="toggle-label">
          <strong>🔥 السخان</strong>
          <small>لمبة التسخين</small>
        </div>
        <label class="sw">
          <input type="checkbox" id="heaterToggle" onchange="setDevice('heater',this.checked)">
          <span class="slider"></span>
        </label>
      </div>
      <div class="toggle-row">
        <div class="toggle-label">
          <strong>💨 المروحة</strong>
          <small>مروحة التبريد والتهوية</small>
        </div>
        <label class="sw">
          <input type="checkbox" id="fanToggle" onchange="setDevice('fan',this.checked)">
          <span class="slider"></span>
        </label>
      </div>
    </div>
  </div>

  <!-- ── SERVO / EGG TURNER CARD ── -->
  <div class="card">
    <div class="card-title">🥚 تقليب البيض</div>
    <div class="servo-visual">
      <div class="egg-tray" id="eggTray">🥚🥚🥚</div>
      <div style="display:flex;gap:6px;align-items:center;font-size:12px;color:var(--muted)">
        وضع:
        <span class="badge badge-warn" id="servoBadge">A</span>
      </div>
    </div>
    <div style="margin-top:10px;font-size:12px;color:var(--muted);text-align:center">
      آخر تقليب: <span id="lastTurn">--</span> دقيقة<br>
      التقليب القادم: <span id="nextTurn">--</span> دقيقة
    </div>
    <div style="margin-top:14px;display:flex;justify-content:center">
      <button class="btn btn-blue" onclick="turnNow()">🔄 تقليب الآن</button>
    </div>
  </div>

  <!-- ── SETTINGS CARD ── -->
  <div class="card full-width">
    <div class="card-title">⚙️ الإعدادات</div>
    <div class="row-2">
      <div class="input-group">
        <label>درجة الحرارة المستهدفة (°C)</label>
        <input type="number" id="setTemp" step="0.1" min="30" max="42" value="37.8">
      </div>
      <div class="input-group">
        <label>الرطوبة المستهدفة (%)</label>
        <input type="number" id="setHum" step="1" min="20" max="90" value="60">
      </div>
      <div class="input-group">
        <label>فترة التقليب (دقيقة)</label>
        <input type="number" id="setTurn" step="30" min="30" max="1440" value="240">
      </div>
      <div class="input-group">
        <label>أيام الحضانة الكاملة</label>
        <select id="setDays">
          <option value="21">دجاج - 21 يوم</option>
          <option value="28">بط / إوز - 28 يوم</option>
          <option value="17">حمام / سمان - 17 يوم</option>
          <option value="18">سمان - 18 يوم</option>
          <option value="30">رومي - 28 يوم</option>
        </select>
      </div>
    </div>
    <div style="display:flex;gap:10px;flex-wrap:wrap;margin-top:6px">
      <button class="btn btn-primary" onclick="saveSettings()">💾 حفظ الإعدادات</button>
      <button class="btn btn-danger" onclick="resetStats()">🗑️ إعادة ضبط الإحصائيات</button>
    </div>
  </div>

</main>

<div id="toast"></div>

<script>
  let autoMode = true;
  let pollTimer;

  // ── FETCH STATUS ──────────────────────────────────────────────
  async function fetchStatus() {
    try {
      const r = await fetch('/api/status', { cache: 'no-store' });
      if (!r.ok) throw new Error('HTTP ' + r.status);
      const d = await r.json();
      updateUI(d);
      setConnected(true);
    } catch(e) {
      setConnected(false);
    }
  }

  // ── UPDATE UI ─────────────────────────────────────────────────
  function updateUI(d) {
    // Temp
    const t = d.temp.toFixed(1);
    document.getElementById('tempVal').innerHTML = t + '<span class="metric-unit">°C</span>';
    document.getElementById('tempTarget').textContent = d.targetTemp.toFixed(1);
    document.getElementById('minTemp').textContent = d.minTemp.toFixed(1);
    document.getElementById('maxTemp').textContent = d.maxTemp.toFixed(1);
    const tPct = Math.min(100, Math.max(0, ((d.temp - 30) / 14) * 100));
    document.getElementById('tempBar').style.width = tPct + '%';
    document.getElementById('tempCard').style.borderColor = d.heaterOn ? 'rgba(245,166,35,.5)' : '';

    // Hum
    document.getElementById('humVal').innerHTML = d.hum.toFixed(0) + '<span class="metric-unit">%</span>';
    document.getElementById('humTarget').textContent = d.targetHum.toFixed(0);
    document.getElementById('minHum').textContent = d.minHum.toFixed(0);
    document.getElementById('maxHum').textContent = d.maxHum.toFixed(0);
    document.getElementById('humBar').style.width = d.hum + '%';

    // Badges
    const hb = document.getElementById('heaterBadge');
    hb.className = 'badge ' + (d.heaterOn ? 'badge-warn' : 'badge-off');
    hb.textContent = d.heaterOn ? '🔥 السخان: يعمل' : '🔌 السخان: متوقف';

    const fb = document.getElementById('fanBadge');
    fb.className = 'badge ' + (d.fanOn ? 'badge-on' : 'badge-off');
    fb.textContent = d.fanOn ? '💨 المروحة: تعمل' : '💨 المروحة: متوقفة';

    // DHT Error
    document.getElementById('tempAlert').classList.toggle('show', d.dhtError);

    // Auto toggle
    autoMode = d.autoMode;
    document.getElementById('autoToggle').checked = d.autoMode;
    const mc = document.getElementById('manualControls');
    if (d.autoMode) { mc.style.opacity = '.4'; mc.style.pointerEvents = 'none'; }
    else            { mc.style.opacity = '1';  mc.style.pointerEvents = 'auto'; }

    // Manual toggles
    document.getElementById('heaterToggle').checked = d.heaterOn;
    document.getElementById('fanToggle').checked    = d.fanOn;

    // Day progress
    const day = d.day;
    const total = d.totalDays;
    const pct = total > 0 ? Math.min(100, (day / total) * 100) : 0;
    document.getElementById('dayNum').textContent = day;
    document.getElementById('dayPct').textContent = pct.toFixed(0) + '% مكتمل';

    const labels = ['بداية الحضانة','مرحلة النمو الأولى','تطور الأجنة','مرحلة النضج','قريباً من الفقس 🐣','تم الفقس! 🐥'];
    const labelIdx = Math.min(labels.length - 1, Math.floor((day / total) * labels.length));
    document.getElementById('dayLabel').textContent = labels[labelIdx];

    // Day track
    const track = document.getElementById('dayTrack');
    if (track.children.length !== total) {
      track.innerHTML = '';
      for (let i = 1; i <= total; i++) {
        const dot = document.createElement('div');
        dot.className = 'day-dot';
        track.appendChild(dot);
      }
    }
    Array.from(track.children).forEach((dot, i) => {
      const dayN = i + 1;
      if (dayN < day)       dot.className = 'day-dot done';
      else if (dayN === day) dot.className = 'day-dot today';
      else                   dot.className = 'day-dot future';
    });

    // Servo
    const pos = d.servoPos;
    document.getElementById('servoBadge').textContent = pos;
    document.getElementById('eggTray').className = 'egg-tray' + (pos === 'B' ? ' pos-b' : '');
    document.getElementById('lastTurn').textContent = d.lastTurn;
    document.getElementById('nextTurn').textContent = Math.max(0, d.nextTurn);

    // Settings (first time only)
    if (!window._settingsLoaded) {
      window._settingsLoaded = true;
      document.getElementById('setTemp').value  = d.targetTemp;
      document.getElementById('setHum').value   = d.targetHum;
      document.getElementById('setTurn').value  = d.turnInterval;
      const sel = document.getElementById('setDays');
      for (let o of sel.options) if (o.value == d.totalDays) { o.selected = true; break; }
    }

    // Uptime
    const up = d.uptime;
    const h = Math.floor(up / 3600), m = Math.floor((up%3600)/60), s = up%60;
    document.getElementById('uptimeLabel').textContent =
      (h ? h+'h ' : '') + (m ? m+'m ' : '') + s + 's';
  }

  // ── CONNECTED / DISCONNECTED ──────────────────────────────────
  function setConnected(ok) {
    document.getElementById('connDot').className   = 'dot' + (ok ? '' : ' error');
    document.getElementById('connLabel').textContent = ok ? 'متصل' : 'انقطع الاتصال';
  }

  // ── CONTROLS ─────────────────────────────────────────────────
  async function setAuto(v) {
    await api('/api/control', { autoMode: v });
    const mc = document.getElementById('manualControls');
    if (v) { mc.style.opacity = '.4'; mc.style.pointerEvents = 'none'; }
    else   { mc.style.opacity = '1';  mc.style.pointerEvents = 'auto'; }
    toast(v ? '✅ وضع تلقائي' : '🎛️ وضع يدوي', 'success');
  }

  async function setDevice(dev, val) {
    await api('/api/control', { [dev]: val });
  }

  async function turnNow() {
    const btn = event.target;
    btn.disabled = true;
    btn.textContent = '⏳ جاري التقليب...';
    await api('/api/turn-now', {});
    toast('✅ تم تقليب البيض', 'success');
    setTimeout(() => { btn.disabled = false; btn.innerHTML = '🔄 تقليب الآن'; }, 2000);
  }

  async function saveSettings() {
    const t  = parseFloat(document.getElementById('setTemp').value);
    const h  = parseFloat(document.getElementById('setHum').value);
    const ti = parseInt(document.getElementById('setTurn').value);
    const td = parseInt(document.getElementById('setDays').value);

    if (t < 30 || t > 42) return toast('❌ حرارة غير صحيحة (30-42)', 'error');
    if (h < 20 || h > 90) return toast('❌ رطوبة غير صحيحة (20-90)', 'error');

    const ok = await api('/api/settings', { targetTemp: t, targetHum: h, turnInterval: ti, incubationDays: td });
    if (ok) { toast('✅ تم الحفظ', 'success'); window._settingsLoaded = false; }
  }

  async function resetStats() {
    if (!confirm('هل تريد إعادة ضبط جميع الإحصائيات وعداد الأيام؟')) return;
    const ok = await api('/api/reset-stats', {});
    if (ok) { toast('✅ تمت إعادة الضبط', 'success'); window._settingsLoaded = false; }
  }

  // ── API HELPER ────────────────────────────────────────────────
  async function api(url, body) {
    try {
      const r = await fetch(url, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify(body)
      });
      const d = await r.json();
      return d.ok;
    } catch(e) {
      toast('❌ فشل الاتصال', 'error');
      return false;
    }
  }

  // ── TOAST ─────────────────────────────────────────────────────
  let toastT;
  function toast(msg, type='success') {
    const el = document.getElementById('toast');
    el.textContent = msg;
    el.className = 'show ' + type;
    clearTimeout(toastT);
    toastT = setTimeout(() => { el.className = ''; }, 2500);
  }

  // ── START ─────────────────────────────────────────────────────
  fetchStatus();
  setInterval(fetchStatus, 3000);
</script>

</body>
</html>
)=====";
