/**
 * ================================================================
 *  BINARY MAX-HEAP — JavaScript Implementation
 *  ADSA Mini Project: Hospital Patient Priority Queue
 * ================================================================
 *
 *  Heap invariant (Max-Heap):
 *    higherPriority(parent, child) == true
 *
 *  Priority Rule:
 *    1. Higher urgencyLevel → higher priority   (primary key)
 *    2. Equal urgency → smaller arrivalOrder → higher priority (FIFO)
 *
 *  Storage: 0-indexed array
 *    Parent of i     : Math.floor((i - 1) / 2)
 *    Left child of i : 2*i + 1
 *    Right child of i: 2*i + 2
 * ================================================================
 */

class PatientHeap {
  constructor() {
    this._heap = []; // internal array storage
  }

  // ── Comparator ───────────────────────────────────────────────
  // Returns true if patient A has STRICTLY higher priority than B
  _higherPriority(a, b) {
    if (a.urgencyLevel !== b.urgencyLevel)
      return a.urgencyLevel > b.urgencyLevel;      // higher urgency wins
    return a.arrivalOrder < b.arrivalOrder;         // earlier arrival wins (FIFO)
  }

  // ── HEAPIFY UP ────────────────────────────────────────────────
  // After insert at the last index, bubble it UP until heap is valid.
  // Time Complexity: O(log n)
  _heapifyUp(index) {
    while (index > 0) {
      const parent = Math.floor((index - 1) / 2);
      if (this._higherPriority(this._heap[index], this._heap[parent])) {
        // Swap child with parent
        [this._heap[index], this._heap[parent]] =
          [this._heap[parent], this._heap[index]];
        index = parent;
      } else {
        break; // Heap property satisfied
      }
    }
  }

  // ── HEAPIFY DOWN ─────────────────────────────────────────────
  // After removing root or decreasing key, sink element DOWN.
  // Time Complexity: O(log n)
  _heapifyDown(index) {
    const n = this._heap.length;
    while (true) {
      let highest = index;
      const left  = 2 * index + 1;
      const right = 2 * index + 2;

      if (left  < n && this._higherPriority(this._heap[left],  this._heap[highest]))
        highest = left;
      if (right < n && this._higherPriority(this._heap[right], this._heap[highest]))
        highest = right;

      if (highest !== index) {
        [this._heap[index], this._heap[highest]] =
          [this._heap[highest], this._heap[index]];
        index = highest;
      } else {
        break; // Heap property satisfied
      }
    }
  }

  // ── INSERT (Enqueue) ──────────────────────────────────────────
  // 1. Append to the end   → O(1)
  // 2. Heapify Up          → O(log n)
  // Total: O(log n)
  insert(patient) {
    this._heap.push({ ...patient });
    this._heapifyUp(this._heap.length - 1);
  }

  // ── PEEK ──────────────────────────────────────────────────────
  // Return highest-priority patient without removing.
  // Time Complexity: O(1)
  peek() {
    if (this._heap.length === 0) return null;
    return this._heap[0];
  }

  // ── EXTRACT MAX (Dequeue) ─────────────────────────────────────
  // 1. Save root (max)
  // 2. Move last element to root
  // 3. Pop last
  // 4. Heapify Down
  // Time Complexity: O(log n)
  extractMax() {
    if (this._heap.length === 0) return null;
    const top = this._heap[0];
    const last = this._heap.pop();
    if (this._heap.length > 0) {
      this._heap[0] = last;
      this._heapifyDown(0);
    }
    return top;
  }

  // ── REMOVE BY ID ─────────────────────────────────────────────
  // Find → swap with last → pop → restore heap
  // Time Complexity: O(n + log n)
  removeById(id) {
    const idx = this._heap.findIndex(p => p.id === id);
    if (idx === -1) return false;

    this._heap[idx] = this._heap[this._heap.length - 1];
    this._heap.pop();

    if (idx < this._heap.length) {
      this._heapifyUp(idx);
      this._heapifyDown(idx);
    }
    return true;
  }

  // ── UPDATE PRIORITY ───────────────────────────────────────────
  // Find → change key → restore heap
  // Time Complexity: O(n + log n)
  updatePriority(id, newUrgency) {
    const idx = this._heap.findIndex(p => p.id === id);
    if (idx === -1) return false;
    this._heap[idx].urgencyLevel = newUrgency;
    this._heapifyUp(idx);
    this._heapifyDown(idx);
    return true;
  }

  // ── SEARCH BY ID ─────────────────────────────────────────────
  // Linear scan through the array.
  // Time Complexity: O(n)
  searchById(id) {
    return this._heap.find(p => p.id === id) || null;
  }

  // ── SORTED VIEW ───────────────────────────────────────────────
  // Returns all patients sorted by priority (highest first),
  // WITHOUT disturbing the actual heap.
  // Simulates extractMax repeatedly on a temporary copy.
  // Time Complexity: O(n log n)
  getSortedPatients() {
    // Deep copy the heap array and run a simulation
    const tempHeap = new PatientHeap();
    for (const p of this._heap) tempHeap.insert(p);
    const sorted = [];
    while (!tempHeap.isEmpty()) sorted.push(tempHeap.extractMax());
    return sorted;
  }

  // Reload entire heap from an array (used for localStorage restore)
  loadFromArray(arr) {
    this._heap = [];
    for (const p of arr) this.insert(p);
  }

  get size()    { return this._heap.length; }
  isEmpty()     { return this._heap.length === 0; }
  toArray()     { return [...this._heap]; } // raw (unordered) copy for storage
}
