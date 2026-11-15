/**
 * ================================================================
 *  APP.JS — Application Logic
 *  Hospital Patient Priority Queue Management System
 *  ADSA Mini Project — B.Tech CSE Sem 3
 * ================================================================
 */

// ═══════════════ GLOBAL STATE ══════════════════════════════════
const heap    = new PatientHeap();   // Live priority queue (Binary Max-Heap)
let treated   = [];                   // Treated patients history
let nextIdNum = 1;                    // Numeric part of patient IDs
let nextArrival = 1;                  // Arrival order counter (tie-breaker)
let removeTargetId = null;           // Pending remove confirmation

// ═══════════════ URGENCY HELPERS ═══════════════════════════════
const URGENCY_LABEL = { 5:'Critical', 4:'High', 3:'Moderate', 2:'Low', 1:'Minor' };
const URGENCY_COLOR = { 5:'#dc2626', 4:'#ea580c', 3:'#d97706', 2:'#2563eb', 1:'#16a34a' };

function urgencyLabel(level) { return URGENCY_LABEL[level] || 'Unknown'; }

function urgencyBadge(level) {
  return `<span class="badge badge-${level}">${urgencyLabel(level)}</span>`;
}

function generateId(num) {
  return 'P' + String(num).padStart(3, '0');
}

// ═══════════════ PERSISTENCE (localStorage) ════════════════════
function saveData() {
  localStorage.setItem('hpq_heap',    JSON.stringify(heap.toArray()));
  localStorage.setItem('hpq_treated', JSON.stringify(treated));
  localStorage.setItem('hpq_nextId',  nextIdNum);
  localStorage.setItem('hpq_nextArr', nextArrival);
}

function loadData() {
  const rawHeap    = localStorage.getItem('hpq_heap');
  const rawTreated = localStorage.getItem('hpq_treated');
  const rawId      = localStorage.getItem('hpq_nextId');
  const rawArr     = localStorage.getItem('hpq_nextArr');

  if (rawHeap)    { const arr = JSON.parse(rawHeap); heap.loadFromArray(arr); }
  if (rawTreated) { treated = JSON.parse(rawTreated); }
  if (rawId)      { nextIdNum   = parseInt(rawId,  10) || 1; }
  if (rawArr)     { nextArrival = parseInt(rawArr, 10) || 1; }
}

// ═══════════════ TOAST NOTIFICATION ════════════════════════════
let _toastTimer = null;
function toast(msg, type = 'info') {
  const el = document.getElementById('toast');
  el.textContent = msg;
  el.className = `toast show ${type}`;
  if (_toastTimer) clearTimeout(_toastTimer);
  _toastTimer = setTimeout(() => { el.className = 'toast'; }, 3200);
}

// ═══════════════ PANEL NAVIGATION ══════════════════════════════
const PANEL_TITLES = {
  register: 'Register New Patient',
  queue:    'Patient Priority Queue',
  treat:    'Treat Next Patient',
  search:   'Search Patient',
  update:   'Update Patient Priority',
  remove:   'Remove Patient',
  stats:    'Hospital Statistics',
  history:  'Treated Patients History',
};

function showPanel(name) {
  // Deactivate all panels and nav buttons
  document.querySelectorAll('.panel').forEach(p => p.classList.remove('active'));
  document.querySelectorAll('.nav-btn').forEach(b => b.classList.remove('active'));

  // Activate selected
  document.getElementById(`panel-${name}`).classList.add('active');
  document.getElementById(`nav-${name}`).classList.add('active');
  document.getElementById('page-title').textContent = PANEL_TITLES[name];

  // Render panel content
  if (name === 'queue')   renderQueue();
  if (name === 'treat')   renderTreatPreview();
  if (name === 'stats')   renderStats();
  if (name === 'history') renderHistory();
  if (name === 'register') prefillId();
}

// ═══════════════ TOPBAR: NEXT PATIENT ══════════════════════════
function updateTopbar() {
  const next   = heap.peek();
  const nameEl = document.getElementById('next-name');
  const badge  = document.getElementById('next-badge');
  const sbWait = document.getElementById('sb-waiting');
  const sbTreat= document.getElementById('sb-treated');

  sbWait.textContent  = heap.size;
  sbTreat.textContent = treated.length;

  if (next) {
    nameEl.textContent = next.name;
    badge.className  = `next-badge badge-${next.urgencyLevel}`;
    badge.textContent = urgencyLabel(next.urgencyLevel);
  } else {
    nameEl.textContent = '—';
    badge.textContent  = '';
    badge.className    = 'next-badge';
  }
}

// ═══════════════ 1. REGISTER PATIENT ═══════════════════════════
function prefillId() {
  document.getElementById('reg-id').value = generateId(nextIdNum);
}

function clearRegisterForm() {
  document.getElementById('reg-name').value      = '';
  document.getElementById('reg-age').value       = '';
  document.getElementById('reg-complaint').value = '';
  document.querySelectorAll('input[name="urgency"]').forEach(r => r.checked = false);
  prefillId();
}

function registerPatient() {
  const name      = document.getElementById('reg-name').value.trim();
  const ageVal    = document.getElementById('reg-age').value;
  const complaint = document.getElementById('reg-complaint').value.trim();
  const urgRadio  = document.querySelector('input[name="urgency"]:checked');

  // Validation
  if (!name)         { toast('⚠️ Patient name is required.', 'error');     return; }
  if (!ageVal)       { toast('⚠️ Age is required.',          'error');     return; }
  const age = parseInt(ageVal, 10);
  if (isNaN(age) || age < 0 || age > 150) { toast('⚠️ Age must be between 0 and 150.', 'error'); return; }
  if (!complaint)    { toast('⚠️ Medical complaint is required.', 'error'); return; }
  if (!urgRadio)     { toast('⚠️ Please select an urgency level.',  'error'); return; }

  const patient = {
    id:           generateId(nextIdNum),
    name,
    age,
    complaint,
    urgencyLevel: parseInt(urgRadio.value, 10),
    arrivalOrder: nextArrival,
    treatedAt:    null,
  };

  heap.insert(patient);    // O(log n) — heapifyUp
  nextIdNum++;
  nextArrival++;
  saveData();
  updateTopbar();

  toast(`✅ ${patient.id} · ${patient.name} registered as ${urgencyLabel(patient.urgencyLevel)}!`, 'success');
  clearRegisterForm();
}

// ═══════════════ 2. VIEW QUEUE ════════════════════════════════
function renderQueue() {
  const tbody = document.getElementById('queue-tbody');
  const wrap  = document.getElementById('queue-table-wrap');
  const empty = document.getElementById('queue-empty');

  if (heap.isEmpty()) {
    wrap.style.display  = 'none';
    empty.style.display = 'block';
    return;
  }

  wrap.style.display  = 'block';
  empty.style.display = 'none';

  const sorted = heap.getSortedPatients();  // O(n log n)
  tbody.innerHTML = sorted.map((p, i) => {
    const rank = i + 1;
    const rankClass = rank <= 3 ? ` rank-${rank}` : '';
    return `
      <tr>
        <td><span class="rank-num${rankClass}">${rank}</span></td>
        <td><strong>${p.id}</strong></td>
        <td>${esc(p.name)}</td>
        <td>${p.age}</td>
        <td>${urgencyBadge(p.urgencyLevel)}</td>
        <td>${esc(p.complaint)}</td>
        <td>${p.arrivalOrder}</td>
      </tr>`;
  }).join('');
}

// ═══════════════ 3. TREAT NEXT ════════════════════════════════
function renderTreatPreview() {
  const container = document.getElementById('treat-preview');
  const next = heap.peek();   // O(1)

  if (!next) {
    container.innerHTML = `
      <div class="treat-card">
        <div class="treat-avatar">🏁</div>
        <div class="treat-info">
          <div class="treat-empty">No patients in the queue.<br>All patients have been treated!</div>
        </div>
      </div>`;
    return;
  }

  container.innerHTML = `
    <div class="treat-card">
      <div class="treat-avatar" style="background:${URGENCY_COLOR[next.urgencyLevel]}">
        ${next.name.charAt(0).toUpperCase()}
      </div>
      <div class="treat-info">
        <div class="patient-name">${esc(next.name)} &nbsp; ${urgencyBadge(next.urgencyLevel)}</div>
        <div class="patient-meta">
          <span>ID: ${next.id}</span>
          <span>Age: ${next.age}</span>
          <span>${esc(next.complaint)}</span>
          <span>Arrival #${next.arrivalOrder}</span>
        </div>
      </div>
    </div>
    <p style="margin-top:14px;color:#64748b;font-size:.85rem">
      ℹ️ Clicking <strong>Treat Now</strong> will perform <code>extractMax()</code>:
      swap root ↔ last element, pop, then <strong>heapifyDown()</strong>.
    </p>`;
}

function treatNext() {
  if (heap.isEmpty()) { toast('ℹ️ Queue is empty!', 'info'); return; }

  const patient     = heap.extractMax();   // O(log n)
  patient.treatedAt = new Date().toLocaleTimeString();
  treated.push(patient);
  saveData();
  updateTopbar();
  renderTreatPreview();

  toast(`💊 ${patient.name} (${patient.id}) is now being treated!`, 'success');
}

// ═══════════════ 4. SEARCH ════════════════════════════════════
function searchPatient() {
  const raw = document.getElementById('search-id').value.trim().toUpperCase();
  const id  = raw.startsWith('P') ? raw : 'P' + raw;
  const container = document.getElementById('search-result');

  if (!raw) { container.innerHTML = ''; toast('⚠️ Enter a Patient ID.', 'error'); return; }

  // Search waiting queue — O(n)
  const waiting = heap.searchById(id);
  if (waiting) {
    container.innerHTML = resultCard(waiting, 'waiting', '🟢 Found in Waiting Queue');
    return;
  }

  // Search treated history
  const done = treated.find(p => p.id === id);
  if (done) {
    container.innerHTML = resultCard(done, 'treated', '✅ Found in Treated History');
    return;
  }

  container.innerHTML = `
    <div class="result-card">
      <div class="result-header notfound">❌ Patient "${id}" Not Found</div>
      <div class="result-body" style="color:#64748b;font-size:.88rem">
        No patient with this ID exists in the waiting queue or treated history.
      </div>
    </div>`;
}

function resultCard(p, type, header) {
  return `
    <div class="result-card">
      <div class="result-header ${type}">${header}</div>
      <div class="result-body">
        <div class="detail-grid">
          <div class="detail-item"><label>Patient ID</label><span>${p.id}</span></div>
          <div class="detail-item"><label>Name</label><span>${esc(p.name)}</span></div>
          <div class="detail-item"><label>Age</label><span>${p.age}</span></div>
          <div class="detail-item"><label>Urgency</label><span>${urgencyBadge(p.urgencyLevel)}</span></div>
          <div class="detail-item"><label>Arrival #</label><span>${p.arrivalOrder}</span></div>
          <div class="detail-item"><label>Status</label><span>${type === 'treated' ? '✅ Treated at ' + p.treatedAt : '⏳ Waiting'}</span></div>
          <div class="detail-item" style="grid-column:1/-1"><label>Complaint</label><span>${esc(p.complaint)}</span></div>
        </div>
      </div>
    </div>`;
}

// ═══════════════ 5. UPDATE PRIORITY ═══════════════════════════
let updateTargetId = null;

function loadPatientForUpdate() {
  const raw = document.getElementById('update-id').value.trim().toUpperCase();
  const id  = raw.startsWith('P') ? raw : (raw ? 'P' + raw : '');
  const container = document.getElementById('update-current');
  const form      = document.getElementById('update-form');

  if (!id) { toast('⚠️ Enter a Patient ID.', 'error'); return; }

  const p = heap.searchById(id);
  if (!p) {
    container.innerHTML = `<div class="result-card"><div class="result-header notfound">❌ "${id}" not found in waiting queue.</div></div>`;
    form.style.display = 'none';
    updateTargetId = null;
    return;
  }

  updateTargetId = id;
  container.innerHTML = resultCard(p, 'waiting', `🟢 ${p.name} · Current: ${urgencyLabel(p.urgencyLevel)}`);
  form.style.display = 'block';

  // Pre-select current urgency
  document.querySelectorAll('input[name="update-urgency"]').forEach(r => {
    r.checked = parseInt(r.value, 10) === p.urgencyLevel;
  });
}

function applyUpdate() {
  if (!updateTargetId) { toast('⚠️ Load a patient first.', 'error'); return; }
  const radio = document.querySelector('input[name="update-urgency"]:checked');
  if (!radio)          { toast('⚠️ Select a new urgency level.', 'error'); return; }
  const newLevel = parseInt(radio.value, 10);
  const oldP = heap.searchById(updateTargetId);
  if (!oldP) { toast('⚠️ Patient no longer in queue.', 'error'); return; }
  if (newLevel === oldP.urgencyLevel) { toast('ℹ️ Urgency level is unchanged.', 'info'); return; }

  const oldLabel = urgencyLabel(oldP.urgencyLevel);
  heap.updatePriority(updateTargetId, newLevel);  // O(n + log n)
  saveData();
  updateTopbar();

  toast(`✏️ ${updateTargetId}: ${oldLabel} → ${urgencyLabel(newLevel)}. Heap reorganized!`, 'success');
  cancelUpdate();
}

function cancelUpdate() {
  updateTargetId = null;
  document.getElementById('update-current').innerHTML = '';
  document.getElementById('update-form').style.display = 'none';
  document.getElementById('update-id').value = '';
}

// ═══════════════ 6. REMOVE PATIENT ════════════════════════════
function loadPatientForRemove() {
  const raw = document.getElementById('remove-id').value.trim().toUpperCase();
  const id  = raw.startsWith('P') ? raw : (raw ? 'P' + raw : '');
  const container = document.getElementById('remove-current');
  const confirmDiv= document.getElementById('remove-confirm');

  if (!id) { toast('⚠️ Enter a Patient ID.', 'error'); return; }

  const p = heap.searchById(id);
  if (!p) {
    container.innerHTML = `<div class="result-card"><div class="result-header notfound">❌ "${id}" not found in waiting queue.</div></div>`;
    confirmDiv.style.display = 'none';
    removeTargetId = null;
    return;
  }

  removeTargetId = id;
  container.innerHTML = resultCard(p, 'waiting', `⚠️ Patient to Remove: ${p.name}`);
  confirmDiv.style.display = 'block';
}

function confirmRemove() {
  if (!removeTargetId) return;
  const p = heap.searchById(removeTargetId);
  const name = p ? p.name : removeTargetId;

  heap.removeById(removeTargetId);   // O(n + log n)
  saveData();
  updateTopbar();

  toast(`🗑️ ${name} (${removeTargetId}) removed. Heap reorganized!`, 'success');
  cancelRemove();
}

function cancelRemove() {
  removeTargetId = null;
  document.getElementById('remove-current').innerHTML = '';
  document.getElementById('remove-confirm').style.display = 'none';
  document.getElementById('remove-id').value = '';
}

// ═══════════════ 7. STATISTICS ════════════════════════════════
function renderStats() {
  const total   = heap.size + treated.length;
  const waiting = heap.size;
  const nTreated= treated.length;

  // Count by urgency in waiting queue
  const counts = { 5:0, 4:0, 3:0, 2:0, 1:0 };
  for (const p of heap.toArray()) counts[p.urgencyLevel]++;

  document.getElementById('stats-grid').innerHTML = `
    <div class="stat-card s-total">
      <div class="s-val">${total}</div>
      <div class="s-lbl">Total Registered</div>
    </div>
    <div class="stat-card">
      <div class="s-val" style="color:#0f172a">${waiting}</div>
      <div class="s-lbl">Currently Waiting</div>
    </div>
    <div class="stat-card s-treated">
      <div class="s-val">${nTreated}</div>
      <div class="s-lbl">Treated</div>
    </div>
    <div class="stat-card s-critical">
      <div class="s-val">${counts[5]}</div>
      <div class="s-lbl">Critical Waiting</div>
    </div>
    <div class="stat-card" style="border-color:#ea580c">
      <div class="s-val" style="color:#ea580c">${counts[4]}</div>
      <div class="s-lbl">High Waiting</div>
    </div>
    <div class="stat-card" style="border-color:#d97706">
      <div class="s-val" style="color:#d97706">${counts[3]}</div>
      <div class="s-lbl">Moderate Waiting</div>
    </div>`;

  const maxCount = Math.max(1, waiting);
  document.getElementById('stats-bars').innerHTML = `
    <h3>Waiting Queue by Urgency Level</h3>
    ${[5,4,3,2,1].map(lv => `
      <div class="bar-row">
        <div class="bar-label">${urgencyLabel(lv)}</div>
        <div class="bar-track">
          <div class="bar-fill bar-${lv}" style="width:${(counts[lv]/maxCount*100).toFixed(1)}%"></div>
        </div>
        <div class="bar-count">${counts[lv]}</div>
      </div>`).join('')}`;
}

// ═══════════════ 8. TREATED HISTORY ════════════════════════════
function renderHistory() {
  const tbody = document.getElementById('history-tbody');
  const wrap  = document.getElementById('history-table-wrap');
  const empty = document.getElementById('history-empty');

  if (treated.length === 0) {
    wrap.style.display  = 'none';
    empty.style.display = 'block';
    return;
  }

  wrap.style.display  = 'block';
  empty.style.display = 'none';

  tbody.innerHTML = treated.map((p, i) => `
    <tr>
      <td><span class="rank-num">${i + 1}</span></td>
      <td><strong>${p.id}</strong></td>
      <td>${esc(p.name)}</td>
      <td>${p.age}</td>
      <td>${urgencyBadge(p.urgencyLevel)}</td>
      <td>${esc(p.complaint)}</td>
      <td>${p.treatedAt || '—'}</td>
    </tr>`).join('');
}

function clearHistory() {
  if (treated.length === 0) { toast('ℹ️ History is already empty.', 'info'); return; }
  if (!confirm(`Clear all ${treated.length} treated patient records?`)) return;
  treated = [];
  saveData();
  renderHistory();
  updateTopbar();
  toast('🗑️ Treatment history cleared.', 'info');
}

// ═══════════════ UTILITY ═══════════════════════════════════════
// Escape HTML to prevent XSS
function esc(str) {
  return String(str)
    .replace(/&/g,'&amp;')
    .replace(/</g,'&lt;')
    .replace(/>/g,'&gt;')
    .replace(/"/g,'&quot;');
}

// ═══════════════ KEYBOARD SHORTCUTS ════════════════════════════
document.addEventListener('keydown', e => {
  if (e.key === 'Enter') {
    const active = document.querySelector('.panel.active');
    if (active && active.id === 'panel-search')  searchPatient();
    if (active && active.id === 'panel-update')  loadPatientForUpdate();
    if (active && active.id === 'panel-remove')  loadPatientForRemove();
  }
});

// ═══════════════ INIT ══════════════════════════════════════════
(function init() {
  loadData();       // Restore from localStorage
  prefillId();      // Set ID field
  updateTopbar();   // Set sidebar counts

  console.log(
    `%c🏥 Hospital Patient Priority Queue\n%cBinary Max-Heap loaded. ${heap.size} waiting, ${treated.length} treated.`,
    'color:#2563eb;font-size:14px;font-weight:bold',
    'color:#64748b;font-size:12px'
  );
})();
