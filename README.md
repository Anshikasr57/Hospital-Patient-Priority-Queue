# Hospital Patient Priority Queue Management System

> **ADSA Mini Project | B.Tech CSE Semester 3**  
> Advanced Data Structures and Algorithms

---

## 🏥 Project Description

A fully functional **Hospital Emergency Patient Management System** built in **C++17** that uses a manually implemented **Binary Max-Heap** to manage and prioritize patients based on their medical urgency. Patients with critical conditions are always treated before less urgent ones, with FIFO order maintained for patients at the same urgency level.

---

## 📋 Problem Statement

In a hospital emergency ward, patients arrive at different times with varying degrees of medical urgency. A simple queue (FIFO) cannot work here — a patient with a minor complaint who arrived first should not block a critical patient from receiving immediate care. This project solves that problem by using a **Priority Queue backed by a Binary Max-Heap** to always serve the most critical patient next, regardless of arrival order.

---

## 🎯 Objectives

- Demonstrate the implementation and working of a **Binary Max-Heap**.
- Apply the **Priority Queue** concept to a real-world hospital scenario.
- Handle **tie-breaking** using arrival order (FIFO for equal-priority patients).
- Provide **persistent data storage** using CSV file handling.
- Build a clean, professional **console-based user interface**.

---

## ✨ Features

| # | Feature | Description |
|---|---------|-------------|
| 1 | **Register Patient** | Auto-generate patient ID, accept name/age/complaint/urgency |
| 2 | **View Priority Queue** | Display patients in correct treatment order |
| 3 | **Treat Next Patient** | Extract and treat the highest-priority patient |
| 4 | **Search Patient** | Find any patient by ID (waiting or treated) |
| 5 | **Update Priority** | Change urgency level and re-heapify |
| 6 | **Remove Patient** | Delete a patient from the queue with heap fix |
| 7 | **Statistics** | Show counts by urgency level and treatment status |
| 8 | **Treated History** | View all previously treated patients |
| 9 | **Persistence** | Auto-save to CSV; reload on restart |

---

## 🛠️ Technologies Used

- **Language**: C++17
- **Core Data Structure**: Binary Max-Heap (manual implementation using `std::vector`)
- **File I/O**: CSV-based persistent storage (`fstream`)
- **Interface**: Console-based with structured formatting
- **Compiler**: g++ (MinGW / GCC)

---

## 🗂️ Data Structures Implemented

### Binary Max-Heap
The heap is stored as a **0-indexed array** (vector). For node at index `i`:
- **Parent**: `(i - 1) / 2`
- **Left child**: `2*i + 1`
- **Right child**: `2*i + 2`

The **heap invariant** (Max-Heap property) is maintained using a **custom comparator**:
- **Primary key**: Higher `urgencyLevel` → higher priority
- **Secondary key (tie-break)**: Lower `arrivalOrder` → higher priority (FIFO)

This ensures `heap[0]` is always the patient who should be treated next.

---

## 📖 Explanation of Binary Max-Heap

A **Binary Max-Heap** is a complete binary tree stored in an array where every parent node has a value ≥ its children. This property guarantees:
- The **maximum element** (highest priority patient) is always at **index 0**.
- Insertion and deletion both run in **O(log n)** time.

### How insertion works (Heapify Up):
1. Add new patient at the **end of the array**.
2. Compare with its parent; if it has higher priority, **swap**.
3. Repeat until the heap property is satisfied.

### How extraction works (Heapify Down):
1. Save the **root** (index 0 — the highest priority patient).
2. Move the **last element** to the root.
3. Compare with children; swap with the **highest-priority child** if needed.
4. Repeat until the heap property is satisfied.

---

## ⏱️ Time Complexity Table

| Operation | Time Complexity | Description |
|-----------|----------------|-------------|
| **Insertion** (Register) | O(log n) | Heapify Up from last element |
| **Extract Max** (Treat Next) | O(log n) | Heapify Down from root |
| **Peek** (View Next Patient) | O(1) | Direct access to `heap[0]` |
| **Search** (by ID) | O(n) | Linear scan of heap array |
| **Update Priority** | O(n + log n) | O(n) search + O(log n) heapify |
| **Remove by ID** | O(n + log n) | O(n) search + O(log n) heapify |
| **View Sorted Queue** | O(n log n) | Simulate extractions on a copy |
| **Space Complexity** | O(n) | n = number of patients in queue |

---

## 🏷️ Urgency Levels

| Level | Label | Description |
|-------|-------|-------------|
| 5 | **Critical** | Life-threatening emergency |
| 4 | **High** | Severe pain / serious condition |
| 3 | **Moderate** | Needs attention, stable |
| 2 | **Low** | Non-urgent, can wait |
| 1 | **Minor** | Very minor (e.g., small cut) |

---

## 🗄️ Algorithms Used

1. **Binary Max-Heap Insert** — Heapify Up
2. **Binary Max-Heap Extract Max** — Heapify Down
3. **Remove by ID** — Linear search + Heapify Up + Heapify Down
4. **Update Priority** — Linear search + Heapify Up + Heapify Down
5. **Sorted View** — Simulate full sort using temporary heap copy
6. **FIFO Tie-breaking** — Using monotonic `arrivalOrder` counter
7. **CSV File I/O** — With proper quoting for fields containing commas

---

## 🚀 How to Compile and Run

### Prerequisites
- **g++ compiler** (MinGW on Windows, or GCC on Linux/macOS)
- C++17 support

### On Windows (MinGW)
```bash
# Navigate to the project folder
cd Hospital-Patient-Priority-Queue

# Compile
g++ -std=c++17 -Wall -O2 -o hospital_pms.exe src/main.cpp

# Run
.\hospital_pms.exe
```

### On Linux / macOS
```bash
# Navigate to the project folder
cd Hospital-Patient-Priority-Queue

# Compile
g++ -std=c++17 -Wall -O2 -o hospital_pms src/main.cpp

# Run
./hospital_pms
```

> **Note**: The program creates the `data/` files automatically on the first run. Make sure the `data/` directory exists or the program will show a warning and create it.

---

## 🖥️ Sample Output

```
============================================================
         HOSPITAL PATIENT MANAGEMENT SYSTEM
             Priority Queue - Binary Heap
============================================================

   1.  Register New Patient
   2.  View Patient Priority Queue
   3.  Treat Next Patient
   4.  Search Patient
   5.  Update Patient Priority
   6.  Remove Patient
   7.  View Hospital Statistics
   8.  View Treated Patients History
   9.  Exit

------------------------------------------------------------
   Next in queue : Alice Smith [Critical]  |  Waiting: 5
============================================================

   Enter your choice (1-9):
```

**View Priority Queue Output:**
```
============================================================
              PATIENT PRIORITY QUEUE
============================================================

  Total patients waiting: 5

------------------------------------------------------------------------------------------
#     ID    Name                  Age    Urgency     Level       Complaint
------------------------------------------------------------------------------------------
1     P001  Alice Smith           35     [CRITICAL]  Critical    Chest Pain
2     P005  Eva Brown             60     [CRITICAL]  Critical    Chest Pain
3     P003  Carla Reyes           50     [HIGH    ]  High        Severe Headache
4     P004  David Lee             45     [MODERATE]  Moderate    Fracture
5     P002  Bob Jones             22     [MINOR   ]  Minor       Minor Cut
------------------------------------------------------------------------------------------

  [i] Patients are listed in TREATMENT ORDER (highest priority first).
  [i] Equal urgency patients are ordered by arrival time (FIFO).
```

---

## 📁 Project Structure

```
Hospital-Patient-Priority-Queue/
│
├── src/
│   └── main.cpp          ← Complete C++ source (single file)
│
├── data/
│   ├── patients.csv      ← Waiting patients (auto-created)
│   ├── treated.csv       ← Treatment history (auto-created)
│   └── id_counter.txt    ← ID/arrival counter (auto-created)
│
├── screenshots/          ← Add your screenshots here
│
├── README.md             ← This file
├── .gitignore
└── LICENSE
```

---

## 🔮 Future Improvements

1. **Doctor Assignment** — Assign specific doctors to patients based on specialization.
2. **Waiting Time Estimation** — Display estimated wait time based on queue position.
3. **Multi-ward Support** — Separate queues for ICU, General, Pediatrics, etc.
4. **Graphical UI** — Migrate to a GUI using Qt or a web frontend.
5. **Database Integration** — Replace CSV files with SQLite for better scalability.
6. **Patient Aging** — Gradually increase priority of long-waiting patients.
7. **Report Generation** — Daily/weekly statistics exported to PDF/Excel.
8. **D-ary Heap** — Experiment with 4-ary or 8-ary heaps for better cache performance.

---

## 👨‍💻 Author

**B.Tech CSE — Semester 3**  
Subject: Advanced Data Structures and Algorithms (ADSA)  
Mini Project | Academic Year 2024–25

---

## 📄 License

This project is licensed under the MIT License — see the [LICENSE](LICENSE) file for details.
