/*
 * ============================================================
 *   HOSPITAL PATIENT PRIORITY QUEUE MANAGEMENT SYSTEM
 *   ADSA Mini Project - B.Tech CSE Semester 3
 * ============================================================
 *
 *  Data Structure : Binary Max-Heap (manual array implementation)
 *  Concept        : Priority Queue
 *  Language       : C++17
 *  Standard       : C++17
 *
 *  Priority Rule  :
 *    - Higher urgency level (1-5) → higher priority
 *    - Tie-break  : Earlier registration (smaller arrival order) goes first
 *
 *  Heap invariant (Max-Heap by composite key):
 *    comparePriority(parent, child) == true  →  parent has HIGHER priority
 * ============================================================
 */

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <iomanip>
#include <limits>
#include <algorithm>
#include <stdexcept>

// ─────────────────────────── CONSTANTS ──────────────────────────────────────
const std::string DATA_DIR           = "data/";
const std::string PATIENTS_FILE      = DATA_DIR + "patients.csv";
const std::string TREATED_FILE       = DATA_DIR + "treated.csv";
const std::string ID_COUNTER_FILE    = DATA_DIR + "id_counter.txt";

// Width constants for the display table
const int W_ID   = 8;
const int W_NAME = 22;
const int W_AGE  = 5;
const int W_UL   = 10;
const int W_COMP = 28;
const int W_STATUS = 10;

// ─────────────────────────── PATIENT STRUCT ─────────────────────────────────
/*
 * Patient holds all data for one patient.
 * 'arrivalOrder' is a monotonically increasing counter used as tie-breaker:
 *   lower arrivalOrder → arrived earlier → gets higher priority when urgency is equal.
 */
struct Patient {
    std::string id;           // Unique ID, e.g. "P001"
    std::string name;
    int         age;
    std::string complaint;
    int         urgencyLevel; // 1 (Minor) .. 5 (Critical)
    int         arrivalOrder; // tie-breaker: smaller = arrived earlier
    bool        treated;      // true once removed via "Treat Next Patient"

    Patient()
        : age(0), urgencyLevel(0), arrivalOrder(0), treated(false) {}

    Patient(const std::string& id_, const std::string& name_,
            int age_, const std::string& complaint_,
            int urgency_, int arrival_)
        : id(id_), name(name_), age(age_), complaint(complaint_),
          urgencyLevel(urgency_), arrivalOrder(arrival_), treated(false) {}

    // Convert urgency integer to label string
    std::string urgencyLabel() const {
        switch (urgencyLevel) {
            case 5: return "Critical";
            case 4: return "High";
            case 3: return "Moderate";
            case 2: return "Low";
            case 1: return "Minor";
            default: return "Unknown";
        }
    }
};

// ─────────────────────── BINARY MAX-HEAP (Priority Queue) ───────────────────
/*
 *  BINARY MAX-HEAP IMPLEMENTATION
 *  --------------------------------
 *  The heap is stored in a 0-indexed vector (heap[]).
 *
 *  For node at index i:
 *    Parent      : (i - 1) / 2
 *    Left child  : 2*i + 1
 *    Right child : 2*i + 2
 *
 *  Max-Heap property (custom comparator):
 *    A patient A has HIGHER priority than B when:
 *      1. A.urgencyLevel > B.urgencyLevel   (primary key)
 *      2. urgency equal AND A.arrivalOrder < B.arrivalOrder  (secondary key / FIFO)
 *
 *  Therefore heap[0] is always the HIGHEST-priority patient (the one to treat next).
 */
class PatientHeap {
private:
    std::vector<Patient> heap; // internal storage of the max-heap

    // ── Helper: return true if patient A has STRICTLY higher priority than B ──
    bool higherPriority(const Patient& a, const Patient& b) const {
        if (a.urgencyLevel != b.urgencyLevel)
            return a.urgencyLevel > b.urgencyLevel;        // higher urgency wins
        return a.arrivalOrder < b.arrivalOrder;            // earlier arrival wins (FIFO)
    }

    // ── HEAPIFY UP ────────────────────────────────────────────────────────────
    /*
     * After inserting a new element at the LAST position, bubble it UP
     * until the heap property is satisfied.
     * Time Complexity: O(log n)
     */
    void heapifyUp(int index) {
        while (index > 0) {
            int parent = (index - 1) / 2;
            if (higherPriority(heap[index], heap[parent])) {
                std::swap(heap[index], heap[parent]);
                index = parent;
            } else {
                break; // Heap property satisfied
            }
        }
    }

    // ── HEAPIFY DOWN ─────────────────────────────────────────────────────────
    /*
     * After removing the root (index 0) or decreasing a key, sink the
     * element at 'index' DOWN until the heap property is satisfied.
     * Time Complexity: O(log n)
     */
    void heapifyDown(int index) {
        int size = static_cast<int>(heap.size());
        while (true) {
            int highest = index;
            int left    = 2 * index + 1;
            int right   = 2 * index + 2;

            if (left < size && higherPriority(heap[left], heap[highest]))
                highest = left;

            if (right < size && higherPriority(heap[right], heap[highest]))
                highest = right;

            if (highest != index) {
                std::swap(heap[index], heap[highest]);
                index = highest;
            } else {
                break; // Heap property satisfied
            }
        }
    }

public:
    // ── INSERT (Enqueue) ──────────────────────────────────────────────────────
    /*
     * Add a new patient to the priority queue.
     * 1. Append to the end of the array.
     * 2. Heapify Up to restore the heap property.
     * Time Complexity: O(log n)
     */
    void insert(const Patient& p) {
        heap.push_back(p);
        heapifyUp(static_cast<int>(heap.size()) - 1);
    }

    // ── PEEK (Get max without removing) ──────────────────────────────────────
    /*
     * Return the highest-priority patient (root of the heap).
     * Time Complexity: O(1)
     */
    const Patient& peek() const {
        if (heap.empty())
            throw std::runtime_error("Priority queue is empty.");
        return heap[0];
    }

    // ── EXTRACT MAX (Dequeue) ─────────────────────────────────────────────────
    /*
     * Remove and return the highest-priority patient.
     * 1. Swap root with the last element.
     * 2. Remove the last element (which was the root).
     * 3. Heapify Down from the root to restore the heap property.
     * Time Complexity: O(log n)
     */
    Patient extractMax() {
        if (heap.empty())
            throw std::runtime_error("Priority queue is empty.");

        Patient top = heap[0];           // save the root (highest priority)
        heap[0] = heap.back();           // move last element to root
        heap.pop_back();                 // remove the last slot

        if (!heap.empty())
            heapifyDown(0);              // restore heap property

        return top;
    }

    // ── REMOVE BY ID ─────────────────────────────────────────────────────────
    /*
     * Remove a patient with a specific ID from anywhere in the heap.
     * 1. Find the patient using linear search  O(n).
     * 2. Swap it with the last element.
     * 3. Remove the last element.
     * 4. Restore heap property (heapify up OR down depending on new key).
     * Time Complexity: O(n)
     */
    bool removeById(const std::string& id) {
        int idx = -1;
        for (int i = 0; i < static_cast<int>(heap.size()); ++i) {
            if (heap[i].id == id) { idx = i; break; }
        }
        if (idx == -1) return false;

        heap[idx] = heap.back();
        heap.pop_back();

        if (idx < static_cast<int>(heap.size())) {
            heapifyUp(idx);       // might need to go up
            heapifyDown(idx);     // might need to go down
        }
        return true;
    }

    // ── UPDATE PRIORITY ───────────────────────────────────────────────────────
    /*
     * Change the urgency level of a patient identified by ID.
     * 1. Find the patient  O(n).
     * 2. Update urgencyLevel.
     * 3. Restore heap property  O(log n).
     * Total Time Complexity: O(n)
     */
    bool updatePriority(const std::string& id, int newUrgency) {
        for (int i = 0; i < static_cast<int>(heap.size()); ++i) {
            if (heap[i].id == id) {
                heap[i].urgencyLevel = newUrgency;
                heapifyUp(i);
                heapifyDown(i);
                return true;
            }
        }
        return false;
    }

    // ── SEARCH BY ID ─────────────────────────────────────────────────────────
    /*
     * Linear search through the heap array.
     * Time Complexity: O(n)
     */
    const Patient* searchById(const std::string& id) const {
        for (const auto& p : heap) {
            if (p.id == id) return &p;
        }
        return nullptr;
    }

    // ── SORTED VIEW (for display) ─────────────────────────────────────────────
    /*
     * Return a copy of the heap sorted by priority (highest first).
     * We perform a simulation: repeatedly extract the max from a temporary copy.
     * This gives us the true treatment order without disturbing the actual heap.
     * Time Complexity: O(n log n)
     */
    std::vector<Patient> getSortedPatients() const {
        // Make a temporary copy of the heap and sort it in priority order
        PatientHeap tempHeap;
        for (const auto& p : heap)
            tempHeap.insert(p);

        std::vector<Patient> sorted;
        while (!tempHeap.isEmpty())
            sorted.push_back(tempHeap.extractMax());

        return sorted;
    }

    bool isEmpty()  const { return heap.empty(); }
    int  size()     const { return static_cast<int>(heap.size()); }

    // Raw access (used only for file saving)
    const std::vector<Patient>& rawHeap() const { return heap; }
};

// ─────────────────────── GLOBAL STATE ───────────────────────────────────────
PatientHeap       gHeap;              // Live priority queue
std::vector<Patient> gTreated;       // Treated patients history
int               gNextArrival = 1;  // Monotonic counter for arrival order
int               gNextIdNum   = 1;  // Numeric part of patient IDs

// ─────────────────────── UTILITY FUNCTIONS ──────────────────────────────────

// Generate a formatted patient ID like "P001"
std::string generateId(int num) {
    std::ostringstream oss;
    oss << "P" << std::setw(3) << std::setfill('0') << num;
    return oss.str();
}

// Clear the console screen (cross-platform friendly)
void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

// Print a horizontal line of given character and width
void printLine(char c = '=', int width = 60) {
    std::cout << std::string(width, c) << "\n";
}

// Print a centered title inside a box
void printHeader(const std::string& title) {
    printLine('=', 60);
    int padding = (60 - static_cast<int>(title.size())) / 2;
    std::cout << std::string(std::max(0, padding), ' ') << title << "\n";
    printLine('=', 60);
}

// Flush the input buffer after a bad read
void clearInputBuffer() {
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

// Read a validated integer in [lo, hi]
int readInt(const std::string& prompt, int lo, int hi) {
    int val;
    while (true) {
        std::cout << prompt;
        if (std::cin >> val && val >= lo && val <= hi) {
            clearInputBuffer();
            return val;
        }
        clearInputBuffer();
        std::cout << "  [!] Invalid input. Enter a number between "
                  << lo << " and " << hi << ".\n";
    }
}

// Read a non-empty trimmed string
std::string readNonEmptyString(const std::string& prompt) {
    std::string s;
    while (true) {
        std::cout << prompt;
        std::getline(std::cin, s);
        // Trim leading/trailing spaces
        size_t start = s.find_first_not_of(" \t\r\n");
        size_t end   = s.find_last_not_of(" \t\r\n");
        if (start != std::string::npos) {
            s = s.substr(start, end - start + 1);
            if (!s.empty()) return s;
        }
        std::cout << "  [!] Input cannot be empty. Please try again.\n";
    }
}

// Escape commas/newlines in a CSV field
std::string csvEscape(const std::string& s) {
    // If the field contains a comma or quote, wrap in double-quotes and escape inner quotes
    if (s.find(',') != std::string::npos || s.find('"') != std::string::npos
        || s.find('\n') != std::string::npos) {
        std::string out = "\"";
        for (char c : s) {
            if (c == '"') out += "\"\"";
            else out += c;
        }
        out += "\"";
        return out;
    }
    return s;
}

// Parse one CSV field (handles quoted fields)
std::string csvParseField(std::istringstream& ss) {
    std::string field;
    if (ss.peek() == '"') {
        ss.get(); // consume opening quote
        while (ss.good()) {
            char c = static_cast<char>(ss.get());
            if (c == '"') {
                if (ss.peek() == '"') { ss.get(); field += '"'; }
                else break; // closing quote
            } else {
                field += c;
            }
        }
        if (ss.peek() == ',') ss.get(); // consume comma after closing quote
    } else {
        std::getline(ss, field, ',');
    }
    return field;
}

// ─────────────────────── FILE I/O ───────────────────────────────────────────

void saveIdCounter() {
    std::ofstream f(ID_COUNTER_FILE);
    if (f.is_open()) {
        f << gNextIdNum << "\n" << gNextArrival << "\n";
    }
}

void loadIdCounter() {
    std::ifstream f(ID_COUNTER_FILE);
    if (f.is_open()) {
        f >> gNextIdNum >> gNextArrival;
        if (gNextIdNum < 1)   gNextIdNum   = 1;
        if (gNextArrival < 1) gNextArrival = 1;
    }
}

// Convert Patient to a CSV line
std::string patientToCsv(const Patient& p) {
    std::ostringstream oss;
    oss << csvEscape(p.id) << ","
        << csvEscape(p.name) << ","
        << p.age << ","
        << csvEscape(p.complaint) << ","
        << p.urgencyLevel << ","
        << p.arrivalOrder << ","
        << (p.treated ? "1" : "0");
    return oss.str();
}

// Parse a CSV line into a Patient struct; returns false on parse error
bool patientFromCsv(const std::string& line, Patient& p) {
    if (line.empty() || line[0] == '#') return false;
    std::istringstream ss(line);
    try {
        p.id          = csvParseField(ss);
        p.name        = csvParseField(ss);
        std::string ageStr     = csvParseField(ss);
        std::string complStr   = csvParseField(ss);
        std::string urgStr     = csvParseField(ss);
        std::string arrStr     = csvParseField(ss);
        std::string treatStr   = csvParseField(ss);

        p.age          = std::stoi(ageStr);
        p.complaint    = complStr;
        p.urgencyLevel = std::stoi(urgStr);
        p.arrivalOrder = std::stoi(arrStr);
        p.treated      = (treatStr == "1");
        return !p.id.empty() && !p.name.empty();
    } catch (...) {
        return false;
    }
}

// Save the entire waiting queue and treated list to their CSV files
void saveData() {
    // ── Save waiting patients ──
    {
        std::ofstream f(PATIENTS_FILE);
        if (!f.is_open()) {
            std::cerr << "  [!] Warning: Could not open " << PATIENTS_FILE << " for writing.\n";
        } else {
            f << "# id,name,age,complaint,urgencyLevel,arrivalOrder,treated\n";
            for (const auto& p : gHeap.rawHeap())
                f << patientToCsv(p) << "\n";
        }
    }
    // ── Save treated patients ──
    {
        std::ofstream f(TREATED_FILE);
        if (!f.is_open()) {
            std::cerr << "  [!] Warning: Could not open " << TREATED_FILE << " for writing.\n";
        } else {
            f << "# id,name,age,complaint,urgencyLevel,arrivalOrder,treated\n";
            for (const auto& p : gTreated)
                f << patientToCsv(p) << "\n";
        }
    }
    saveIdCounter();
}

// Load waiting + treated patients from CSV files at startup
void loadData() {
    loadIdCounter();

    // ── Load waiting patients ──
    {
        std::ifstream f(PATIENTS_FILE);
        if (f.is_open()) {
            std::string line;
            while (std::getline(f, line)) {
                Patient p;
                if (patientFromCsv(line, p) && !p.treated)
                    gHeap.insert(p);
            }
        }
    }
    // ── Load treated patients ──
    {
        std::ifstream f(TREATED_FILE);
        if (f.is_open()) {
            std::string line;
            while (std::getline(f, line)) {
                Patient p;
                if (patientFromCsv(line, p) && p.treated)
                    gTreated.push_back(p);
            }
        }
    }
}

// ─────────────────────── DISPLAY HELPERS ────────────────────────────────────

// Print a colored urgency badge (console text only — no ANSI codes needed for wide compat)
std::string urgencyBadge(int level) {
    switch (level) {
        case 5: return "[CRITICAL]";
        case 4: return "[HIGH    ]";
        case 3: return "[MODERATE]";
        case 2: return "[LOW     ]";
        case 1: return "[MINOR   ]";
        default:return "[UNKNOWN ]";
    }
}

// Print the queue table header
void printTableHeader() {
    printLine('-', 90);
    std::cout << std::left
              << std::setw(4)   << "#"
              << std::setw(W_ID)   << "  ID"
              << std::setw(W_NAME) << "Name"
              << std::setw(W_AGE+2)<< "Age"
              << std::setw(W_UL+2) << "Urgency"
              << std::setw(12)     << "Level"
              << "Complaint\n";
    printLine('-', 90);
}

// Print one patient row in the table
void printPatientRow(int rank, const Patient& p) {
    std::cout << std::left
              << std::setw(4)      << rank
              << std::setw(W_ID)   << ("  " + p.id)
              << std::setw(W_NAME) << p.name.substr(0, W_NAME - 1)
              << std::setw(W_AGE+2)<< p.age
              << std::setw(W_UL+2) << urgencyBadge(p.urgencyLevel)
              << std::setw(12)     << p.urgencyLabel()
              << p.complaint.substr(0, 30)
              << "\n";
}

// Print full details for a single patient
void printPatientDetail(const Patient& p, bool showStatus = false) {
    printLine('-', 60);
    std::cout << "  Patient ID    : " << p.id << "\n";
    std::cout << "  Name          : " << p.name << "\n";
    std::cout << "  Age           : " << p.age << "\n";
    std::cout << "  Complaint     : " << p.complaint << "\n";
    std::cout << "  Urgency Level : " << p.urgencyLevel
              << " - " << p.urgencyLabel()
              << " " << urgencyBadge(p.urgencyLevel) << "\n";
    std::cout << "  Arrival Order : " << p.arrivalOrder << "\n";
    if (showStatus)
        std::cout << "  Status        : " << (p.treated ? "Treated" : "Waiting") << "\n";
    printLine('-', 60);
}

// ─────────────────────── MENU ACTIONS ───────────────────────────────────────

// ── 1. Register New Patient ───────────────────────────────────────────────────
void registerPatient() {
    clearScreen();
    printHeader("  REGISTER NEW PATIENT  ");
    std::cout << "\n";

    Patient p;
    p.id           = generateId(gNextIdNum);
    p.arrivalOrder = gNextArrival;

    std::cout << "  Auto-generated Patient ID : " << p.id << "\n\n";

    p.name = readNonEmptyString("  Enter Patient Name      : ");

    p.age  = readInt("  Enter Patient Age       : ", 0, 150);

    // Flush getline buffer after readInt
    p.complaint = readNonEmptyString("  Enter Medical Complaint : ");

    std::cout << "\n  Urgency Levels:\n";
    std::cout << "    5 - Critical  (Life-threatening emergency)\n";
    std::cout << "    4 - High      (Severe pain / serious condition)\n";
    std::cout << "    3 - Moderate  (Needs attention, stable)\n";
    std::cout << "    2 - Low       (Non-urgent, can wait)\n";
    std::cout << "    1 - Minor     (Very minor, e.g. small cut)\n\n";

    p.urgencyLevel = readInt("  Assign Urgency Level (1-5): ", 1, 5);
    p.treated = false;

    gHeap.insert(p);
    ++gNextIdNum;
    ++gNextArrival;

    std::cout << "\n";
    printLine('-', 60);
    std::cout << "  [+] Patient registered successfully!\n";
    std::cout << "  ID: " << p.id << " | Name: " << p.name
              << " | Priority: " << p.urgencyLabel() << "\n";
    printLine('-', 60);
    std::cout << "\n  Press Enter to return to menu...";
    std::cin.get();
}

// ── 2. View Patient Queue ─────────────────────────────────────────────────────
void viewQueue() {
    clearScreen();
    printHeader("  PATIENT PRIORITY QUEUE  ");
    std::cout << "\n";

    if (gHeap.isEmpty()) {
        std::cout << "  [i] The priority queue is currently empty.\n\n";
        std::cout << "  Press Enter to return to menu...";
        std::cin.get();
        return;
    }

    std::cout << "  Total patients waiting: " << gHeap.size() << "\n\n";
    printTableHeader();

    std::vector<Patient> sorted = gHeap.getSortedPatients();
    int rank = 1;
    for (const auto& p : sorted) {
        printPatientRow(rank++, p);
    }

    printLine('-', 90);
    std::cout << "\n  [i] Patients are listed in TREATMENT ORDER (highest priority first).\n";
    std::cout << "  [i] Equal urgency patients are ordered by arrival time (FIFO).\n\n";
    std::cout << "  Press Enter to return to menu...";
    std::cin.get();
}

// ── 3. Treat Next Patient ─────────────────────────────────────────────────────
void treatNextPatient() {
    clearScreen();
    printHeader("  TREAT NEXT PATIENT  ");
    std::cout << "\n";

    if (gHeap.isEmpty()) {
        std::cout << "  [i] No patients in the queue. All patients have been treated!\n\n";
        std::cout << "  Press Enter to return to menu...";
        std::cin.get();
        return;
    }

    Patient p = gHeap.extractMax();  // O(log n) extraction from heap
    p.treated = true;
    gTreated.push_back(p);

    std::cout << "  Treating the highest-priority patient now...\n";
    std::cout << "\n";
    printPatientDetail(p);
    std::cout << "\n  [+] Patient " << p.id << " (" << p.name << ") is now being treated.\n";
    std::cout << "  [-] Removed from the priority queue.\n";
    std::cout << "  Remaining patients in queue: " << gHeap.size() << "\n\n";

    if (!gHeap.isEmpty()) {
        const Patient& next = gHeap.peek();  // O(1) peek
        std::cout << "  [>] Next in line: " << next.name
                  << " (" << next.urgencyLabel() << ")\n";
    }

    std::cout << "\n  Press Enter to return to menu...";
    std::cin.get();
}

// ── 4. Search Patient ─────────────────────────────────────────────────────────
void searchPatient() {
    clearScreen();
    printHeader("  SEARCH PATIENT  ");
    std::cout << "\n";

    std::cout << "  Enter Patient ID to search (e.g. P001): ";
    std::string searchId;
    std::getline(std::cin, searchId);
    // Trim
    size_t s = searchId.find_first_not_of(" \t\r\n");
    size_t e = searchId.find_last_not_of(" \t\r\n");
    if (s != std::string::npos) searchId = searchId.substr(s, e - s + 1);
    // Uppercase P prefix
    if (!searchId.empty() && searchId[0] == 'p') searchId[0] = 'P';

    if (searchId.empty()) {
        std::cout << "  [!] No ID entered.\n\n";
        std::cout << "  Press Enter to return to menu...";
        std::cin.get();
        return;
    }

    // Search in waiting queue  O(n)
    const Patient* found = gHeap.searchById(searchId);
    if (found) {
        std::cout << "\n  [+] Patient found in WAITING QUEUE:\n";
        printPatientDetail(*found, true);
    } else {
        // Search in treated history
        bool treatedFound = false;
        for (const auto& p : gTreated) {
            if (p.id == searchId) {
                std::cout << "\n  [+] Patient found in TREATED HISTORY:\n";
                printPatientDetail(p, true);
                treatedFound = true;
                break;
            }
        }
        if (!treatedFound) {
            std::cout << "\n  [-] No patient found with ID: " << searchId << "\n";
            std::cout << "  Please verify the patient ID and try again.\n";
        }
    }

    std::cout << "\n  Press Enter to return to menu...";
    std::cin.get();
}

// ── 5. Update Patient Priority ────────────────────────────────────────────────
void updatePriority() {
    clearScreen();
    printHeader("  UPDATE PATIENT PRIORITY  ");
    std::cout << "\n";

    if (gHeap.isEmpty()) {
        std::cout << "  [i] The priority queue is empty. No patients to update.\n\n";
        std::cout << "  Press Enter to return to menu...";
        std::cin.get();
        return;
    }

    std::cout << "  Enter Patient ID to update (e.g. P001): ";
    std::string uid;
    std::getline(std::cin, uid);
    size_t s = uid.find_first_not_of(" \t\r\n");
    size_t e = uid.find_last_not_of(" \t\r\n");
    if (s != std::string::npos) uid = uid.substr(s, e - s + 1);
    if (!uid.empty() && uid[0] == 'p') uid[0] = 'P';

    if (uid.empty()) {
        std::cout << "  [!] No ID entered.\n\n";
        std::cout << "  Press Enter to return to menu...";
        std::cin.get();
        return;
    }

    const Patient* found = gHeap.searchById(uid);
    if (!found) {
        std::cout << "\n  [-] Patient ID " << uid << " not found in waiting queue.\n";
        std::cout << "  (Note: Cannot update priority of already treated patients.)\n\n";
        std::cout << "  Press Enter to return to menu...";
        std::cin.get();
        return;
    }

    std::cout << "\n  Current details:\n";
    printPatientDetail(*found);

    std::cout << "\n  New Urgency Levels:\n";
    std::cout << "    5 - Critical  |  4 - High  |  3 - Moderate  |  2 - Low  |  1 - Minor\n\n";

    int newLevel = readInt("  Enter new urgency level (1-5): ", 1, 5);

    if (newLevel == found->urgencyLevel) {
        std::cout << "\n  [i] Urgency level is already " << newLevel
                  << " (" << found->urgencyLabel() << "). No change made.\n";
    } else {
        std::string oldLabel = found->urgencyLabel();
        gHeap.updatePriority(uid, newLevel);  // O(log n) after O(n) search
        // Re-fetch to get updated label
        const Patient* updated = gHeap.searchById(uid);
        std::cout << "\n  [+] Priority updated successfully!\n";
        std::cout << "  " << uid << ": " << oldLabel << "  -->  "
                  << (updated ? updated->urgencyLabel() : "Updated") << "\n";
        std::cout << "  Heap has been reorganized.\n";
    }

    std::cout << "\n  Press Enter to return to menu...";
    std::cin.get();
}

// ── 6. Remove Patient ─────────────────────────────────────────────────────────
void removePatient() {
    clearScreen();
    printHeader("  REMOVE PATIENT  ");
    std::cout << "\n";

    if (gHeap.isEmpty()) {
        std::cout << "  [i] The priority queue is empty. No patients to remove.\n\n";
        std::cout << "  Press Enter to return to menu...";
        std::cin.get();
        return;
    }

    std::cout << "  Enter Patient ID to remove (e.g. P001): ";
    std::string rid;
    std::getline(std::cin, rid);
    size_t s = rid.find_first_not_of(" \t\r\n");
    size_t e = rid.find_last_not_of(" \t\r\n");
    if (s != std::string::npos) rid = rid.substr(s, e - s + 1);
    if (!rid.empty() && rid[0] == 'p') rid[0] = 'P';

    if (rid.empty()) {
        std::cout << "  [!] No ID entered.\n\n";
        std::cout << "  Press Enter to return to menu...";
        std::cin.get();
        return;
    }

    const Patient* found = gHeap.searchById(rid);
    if (!found) {
        std::cout << "\n  [-] Patient ID " << rid << " not found in waiting queue.\n\n";
        std::cout << "  Press Enter to return to menu...";
        std::cin.get();
        return;
    }

    std::cout << "\n  Patient to be removed:\n";
    std::string pname = found->name;
    printPatientDetail(*found);

    std::cout << "\n  Are you sure you want to remove " << rid << " (" << pname << ")? (y/n): ";
    char confirm;
    std::cin >> confirm;
    clearInputBuffer();

    if (confirm == 'y' || confirm == 'Y') {
        gHeap.removeById(rid);  // O(n) for search + O(log n) for heap fix
        std::cout << "\n  [+] Patient " << rid << " (" << pname
                  << ") has been removed from the queue.\n";
        std::cout << "  Heap has been reorganized. Remaining: " << gHeap.size() << " patients.\n";
    } else {
        std::cout << "\n  [i] Operation cancelled. Patient remains in the queue.\n";
    }

    std::cout << "\n  Press Enter to return to menu...";
    std::cin.get();
}

// ── 7. View Hospital Statistics ────────────────────────────────────────────────
void viewStatistics() {
    clearScreen();
    printHeader("  HOSPITAL STATISTICS  ");
    std::cout << "\n";

    int totalRegistered = gHeap.size() + static_cast<int>(gTreated.size());
    int waiting         = gHeap.size();
    int treated         = static_cast<int>(gTreated.size());

    // Count by urgency in waiting queue
    int criticalWaiting = 0, highWaiting = 0, moderateWaiting = 0,
        lowWaiting = 0, minorWaiting = 0;
    for (const auto& p : gHeap.rawHeap()) {
        switch (p.urgencyLevel) {
            case 5: ++criticalWaiting; break;
            case 4: ++highWaiting;    break;
            case 3: ++moderateWaiting;break;
            case 2: ++lowWaiting;     break;
            case 1: ++minorWaiting;   break;
        }
    }

    // Count by urgency in treated list
    int criticalTreated = 0, highTreated = 0;
    for (const auto& p : gTreated) {
        if (p.urgencyLevel == 5) ++criticalTreated;
        if (p.urgencyLevel == 4) ++highTreated;
    }

    printLine('-', 60);
    std::cout << "  Total Registered Patients  : " << totalRegistered  << "\n";
    std::cout << "  Patients Currently Waiting : " << waiting          << "\n";
    std::cout << "  Patients Treated           : " << treated          << "\n";
    printLine('-', 60);
    std::cout << "  WAITING QUEUE BREAKDOWN:\n";
    std::cout << "    Critical  (Level 5) : " << criticalWaiting  << "\n";
    std::cout << "    High      (Level 4) : " << highWaiting      << "\n";
    std::cout << "    Moderate  (Level 3) : " << moderateWaiting  << "\n";
    std::cout << "    Low       (Level 2) : " << lowWaiting       << "\n";
    std::cout << "    Minor     (Level 1) : " << minorWaiting     << "\n";
    printLine('-', 60);
    std::cout << "  TREATED HISTORY BREAKDOWN:\n";
    std::cout << "    Critical  (Level 5) : " << criticalTreated  << "\n";
    std::cout << "    High      (Level 4) : " << highTreated      << "\n";
    printLine('-', 60);

    if (waiting > 0) {
        const Patient& top = gHeap.peek();
        std::cout << "\n  NEXT PATIENT TO TREAT:\n";
        std::cout << "    " << top.id << " | " << top.name
                  << " | " << top.urgencyLabel() << "\n";
    }

    std::cout << "\n  Press Enter to return to menu...";
    std::cin.get();
}

// ── 8. View Treated Patients History ──────────────────────────────────────────
void viewTreatedPatients() {
    clearScreen();
    printHeader("  TREATED PATIENTS HISTORY  ");
    std::cout << "\n";

    if (gTreated.empty()) {
        std::cout << "  [i] No patients have been treated yet.\n\n";
        std::cout << "  Press Enter to return to menu...";
        std::cin.get();
        return;
    }

    std::cout << "  Total patients treated: " << gTreated.size() << "\n\n";
    printTableHeader();

    int rank = 1;
    for (const auto& p : gTreated) {
        printPatientRow(rank++, p);
    }
    printLine('-', 90);
    std::cout << "\n  [i] Listed in treatment order (most recently treated last).\n\n";
    std::cout << "  Press Enter to return to menu...";
    std::cin.get();
}

// ─────────────────────── MAIN MENU ──────────────────────────────────────────
void showMainMenu() {
    clearScreen();
    printLine('=', 60);
    std::cout << "         HOSPITAL PATIENT MANAGEMENT SYSTEM\n";
    std::cout << "             Priority Queue - Binary Heap\n";
    printLine('=', 60);
    std::cout << "\n";
    std::cout << "   1.  Register New Patient\n";
    std::cout << "   2.  View Patient Priority Queue\n";
    std::cout << "   3.  Treat Next Patient\n";
    std::cout << "   4.  Search Patient\n";
    std::cout << "   5.  Update Patient Priority\n";
    std::cout << "   6.  Remove Patient\n";
    std::cout << "   7.  View Hospital Statistics\n";
    std::cout << "   8.  View Treated Patients History\n";
    std::cout << "   9.  Exit\n";
    std::cout << "\n";
    printLine('-', 60);
    if (!gHeap.isEmpty()) {
        const Patient& top = gHeap.peek();
        std::cout << "   Next in queue : " << top.name
                  << " [" << top.urgencyLabel() << "]  |  "
                  << "Waiting: " << gHeap.size() << "\n";
    } else {
        std::cout << "   Queue is empty.  Waiting: 0\n";
    }
    printLine('=', 60);
    std::cout << "\n   Enter your choice (1-9): ";
}

// ─────────────────────── MAIN FUNCTION ──────────────────────────────────────
int main() {
    // ── Load persisted data from CSV files ──
    loadData();

    std::cout << "\n";
    printHeader("  HOSPITAL MANAGEMENT SYSTEM  ");
    std::cout << "  Loading patient records...\n";
    std::cout << "  Loaded " << gHeap.size() << " waiting patient(s) and "
              << gTreated.size() << " treated record(s).\n";
    std::cout << "\n  Press Enter to continue...";
    std::cin.get();

    // ── Main application loop ──
    while (true) {
        showMainMenu();
        int choice;
        // Read menu choice safely
        if (!(std::cin >> choice)) {
            clearInputBuffer();
            std::cout << "\n  [!] Invalid input. Please enter a number between 1 and 9.\n";
            std::cout << "  Press Enter to continue...";
            std::cin.get();
            continue;
        }
        clearInputBuffer();

        switch (choice) {
            case 1: registerPatient();    break;
            case 2: viewQueue();          break;
            case 3: treatNextPatient();   break;
            case 4: searchPatient();      break;
            case 5: updatePriority();     break;
            case 6: removePatient();      break;
            case 7: viewStatistics();     break;
            case 8: viewTreatedPatients();break;
            case 9:
                clearScreen();
                printHeader("  SAVING & EXITING  ");
                std::cout << "\n  Saving patient records to file...\n";
                saveData();
                std::cout << "  [+] Data saved successfully.\n";
                std::cout << "  Goodbye! Stay healthy.\n\n";
                printLine('=', 60);
                return 0;
            default:
                std::cout << "\n  [!] Invalid choice. Please enter a number between 1 and 9.\n";
                std::cout << "  Press Enter to continue...";
                std::cin.get();
                break;
        }

        // Auto-save after every action to minimize data loss
        saveData();
    }

    return 0;
}
