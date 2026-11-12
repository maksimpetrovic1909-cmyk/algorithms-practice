#include <iostream>
#include <string>
using namespace std;

const int MAXN = 10000;

// Статистика: порівняння значень, обміни (один обмін двох елементів = 1),
// рекурсивні виклики та максимальна глибина (перший виклик має глибину 1).
struct Stats {
    long long cmp = 0, swp = 0, calls = 0;
    int maxDepth = 0;
};
Stats S;
bool traceOn = false;
long long badRanges = 0;   // скільки разів підділянка не стала коротшою за поточну

void sw(int &x, int &y) { int t = x; x = y; y = t; S.swp++; }

void printArr(const int a[], int n) {
    cout << "[";
    for (int i = 0; i < n; i++) cout << (i ? ", " : "") << a[i];
    cout << "]";
}

// ---------- Розбиття Ломуто: опорний елемент на правій межі ----------
// Повертає остаточну позицію опорного: ліворуч <= опорного, праворуч > опорного
int lomutoPartition(int a[], int lo, int hi) {
    int pivot = a[hi];
    int i = lo - 1;
    for (int j = lo; j < hi; j++) {
        S.cmp++;
        if (a[j] <= pivot) {
            i++;
            if (i != j) sw(a[i], a[j]);
        }
    }
    if (i + 1 != hi) sw(a[i + 1], a[hi]);
    return i + 1;
}

// ---------- Розбиття Гоара: опорне значення = середній елемент ----------
// Повертає межу j: ділянки [lo, j] та [j+1, hi]; опорне значення не обов'язково на місці
int hoarePartition(int a[], int lo, int hi, int &iEnd) {
    int pivot = a[lo + (hi - lo) / 2];
    int i = lo - 1, j = hi + 1;
    while (true) {
        do { i++; S.cmp++; } while (a[i] < pivot);
        do { j--; S.cmp++; } while (a[j] > pivot);
        if (i >= j) { iEnd = i; return j; }
        sw(a[i], a[j]);
    }
}

// Медіана трьох (лише для дослідження): медіана переноситься на праву межу
void medianToEnd(int a[], int lo, int hi) {
    int mid = lo + (hi - lo) / 2;
    S.cmp++; if (a[mid] < a[lo]) sw(a[mid], a[lo]);
    S.cmp++; if (a[hi] < a[lo]) sw(a[hi], a[lo]);
    S.cmp++; if (a[hi] < a[mid]) sw(a[hi], a[mid]);
    // тепер a[lo] <= a[mid] <= a[hi]; медіана a[mid] -> позиція hi
    if (mid != hi) sw(a[mid], a[hi]);
}

// ---------- QuickSort зі схемою Ломуто ----------
void quickLomuto(int a[], int lo, int hi, int depth, int n) {
    long long id = ++S.calls;
    if (depth > S.maxDepth) S.maxDepth = depth;
    if (lo >= hi) return;
    bool tr = traceOn && n <= 30;
    if (tr) cout << "виклик " << id << ", глибина " << depth << ", межі [" << lo << "," << hi
                 << "], опорне " << a[hi] << "\n";
    int p = lomutoPartition(a, lo, hi);
    if (tr) { cout << "  після: "; printArr(a, n);
              cout << "; опорний на позиції " << p << "; далі [" << lo << "," << p - 1 << "] і ["
                   << p + 1 << "," << hi << "]\n"; }
    int len = hi - lo + 1;
    if (p - lo >= len || hi - p >= len) badRanges++;     // підділянки мають бути коротшими
    quickLomuto(a, lo, p - 1, depth + 1, n);
    quickLomuto(a, p + 1, hi, depth + 1, n);
}

// ---------- QuickSort зі схемою Гоара ----------
void quickHoare(int a[], int lo, int hi, int depth, int n) {
    long long id = ++S.calls;
    if (depth > S.maxDepth) S.maxDepth = depth;
    if (lo >= hi) return;
    bool tr = traceOn && n <= 30;
    if (tr) cout << "виклик " << id << ", глибина " << depth << ", межі [" << lo << "," << hi
                 << "], опорне " << a[lo + (hi - lo) / 2] << "\n";
    int iEnd;
    int j = hoarePartition(a, lo, hi, iEnd);
    if (tr) { cout << "  після: "; printArr(a, n);
              cout << "; межа j=" << j << " (i=" << iEnd << "); далі [" << lo << "," << j << "] і ["
                   << j + 1 << "," << hi << "]\n"; }
    int len = hi - lo + 1;
    if (j - lo + 1 >= len || hi - j >= len) badRanges++;
    quickHoare(a, lo, j, depth + 1, n);
    quickHoare(a, j + 1, hi, depth + 1, n);
}

// ---------- Ломуто + медіана трьох (дослідження) ----------
void quickLomutoM3(int a[], int lo, int hi, int depth, int n) {
    S.calls++;
    if (depth > S.maxDepth) S.maxDepth = depth;
    if (lo >= hi) return;
    if (hi - lo >= 2) medianToEnd(a, lo, hi);
    int p = lomutoPartition(a, lo, hi);
    quickLomutoM3(a, lo, p - 1, depth + 1, n);
    quickLomutoM3(a, p + 1, hi, depth + 1, n);
}

typedef void (*QFn)(int[], int, int, int, int);
const int ALG = 3;
QFn qfn[ALG] = {quickLomuto, quickHoare, quickLomutoM3};
const char *algName[ALG] = {"Ломуто", "Гоар", "Ломуто+M3"};

// ---------- Перевірки ----------
bool isSorted(const int a[], int n) {
    for (int i = 1; i < n; i++) if (a[i] < a[i - 1]) return false;
    return true;
}
bool sameComposition(const int x[], const int y[], int n) {
    long long s1 = 0, s2 = 0, q1 = 0, q2 = 0;
    for (int i = 0; i < n; i++) {
        s1 += x[i]; q1 += (long long)x[i] * x[i];
        s2 += y[i]; q2 += (long long)y[i] * y[i];
    }
    return s1 == s2 && q1 == q2;
}
void copyArr(const int s[], int d[], int n) { for (int i = 0; i < n; i++) d[i] = s[i]; }

int visLen(const string &s) { int l = 0; for (unsigned char c : s) if ((c & 0xC0) != 0x80) l++; return l; }
string pad(const string &s, int w) { int l = visLen(s); return s + string(w > l ? w - l : 0, ' '); }
string padL(const string &s, int w) { int l = visLen(s); return string(w > l ? w - l : 0, ' ') + s; }

// Один повний запуск зі скиданням лічильників
bool runSort(int scheme, const int src[], int n, int work[], Stats &out) {
    copyArr(src, work, n);
    S = Stats(); badRanges = 0; traceOn = false;
    qfn[scheme](work, 0, n - 1, 1, n);
    out = S;
    return isSorted(work, n) && sameComposition(src, work, n) && badRanges == 0;
}

int totalChecks = 0, failedChecks = 0;
void runTest(int no, const int src[], int n, const int expected[], const char *what) {
    for (int k = 0; k < 2; k++) {
        static int w[MAXN];
        Stats st;
        bool ok = runSort(k, src, n, w, st);
        for (int i = 0; i < n && ok; i++) if (w[i] != expected[i]) ok = false;
        totalChecks++;
        cout << "Тест " << padL(to_string(no), 2) << " " << pad(algName[k], 7) << " [" << what << "]: ";
        if (ok) cout << "успішно\n";
        else { failedChecks++; cout << "помилка, отримано: "; printArr(w, n); cout << "\n"; }
    }
}

// Перевірки функцій розбиття
int partChecks = 0, partFailed = 0;
bool outsideSame(const int a[], const int b[], int n, int lo, int hi) {
    for (int i = 0; i < n; i++) if ((i < lo || i > hi) && a[i] != b[i]) return false;
    return true;
}
void checkLomuto(const char *what, const int src[], int n, int lo, int hi) {
    int a[64]; copyArr(src, a, n);
    int pv = a[hi];
    S = Stats();
    int p = lomutoPartition(a, lo, hi);
    bool ok = p >= lo && p <= hi && a[p] == pv && outsideSame(a, src, n, lo, hi);
    for (int i = lo; i < p; i++) if (a[i] > pv) ok = false;
    for (int i = p + 1; i <= hi; i++) if (a[i] <= pv) ok = false;
    { int s1 = 0, s2 = 0; for (int i = lo; i <= hi; i++) { s1 += src[i]; s2 += a[i]; } if (s1 != s2) ok = false; }
    partChecks++; if (!ok) partFailed++;
    cout << "Розбиття Ломуто " << partChecks << " [" << what << "]: опорне " << pv << ", позиція " << p << ", масив ";
    printArr(a, n); cout << ": " << (ok ? "успішно" : "помилка") << "\n";
}
void checkHoare(const char *what, const int src[], int n, int lo, int hi) {
    int a[64]; copyArr(src, a, n);
    int pv = a[lo + (hi - lo) / 2];
    S = Stats();
    int iEnd;
    int j = hoarePartition(a, lo, hi, iEnd);
    bool ok = j >= lo && j < hi && outsideSame(a, src, n, lo, hi);
    for (int i = lo; i <= j; i++) if (a[i] > pv) ok = false;
    for (int i = j + 1; i <= hi; i++) if (a[i] < pv) ok = false;
    { int s1 = 0, s2 = 0; for (int i = lo; i <= hi; i++) { s1 += src[i]; s2 += a[i]; } if (s1 != s2) ok = false; }
    partChecks++; if (!ok) partFailed++;
    cout << "Розбиття Гоара " << partChecks - 5 << " [" << what << "]: опорне " << pv << ", межа j=" << j
         << ", i=" << iEnd << ", масив ";
    printArr(a, n); cout << ": " << (ok ? "успішно" : "помилка") << "\n";
}

bool readArray(int a[], int &n) {
    cout << "Введіть кількість елементів (1.." << MAXN << "): ";
    if (!(cin >> n)) return false;
    while (n < 1 || n > MAXN) {
        cout << "Недопустимий розмір, повторіть: ";
        if (!(cin >> n)) return false;
    }
    cout << "Введіть " << n << " цілих чисел: ";
    for (int i = 0; i < n; i++) if (!(cin >> a[i])) return false;
    return true;
}

void makeSet(int type, int n, int a[]) {
    for (int i = 0; i < n; i++) {
        if (type == 0) a[i] = i + 1;                        // зростання
        else if (type == 1) a[i] = n - i;                   // спадання
        else if (type == 2) a[i] = (i * 37 + 11) % (n + 1); // змішаний (повторюваний)
        else a[i] = (i * 7 + 3) % 10;                       // багато однакових значень
    }
}

int main() {
    // ===== Автотести сортування =====
    cout << "=== Автоматизовані перевірки сортування ===\n";
    {
        int e0[1] = {0};
        int t2[] = {7};
        int t3[] = {1, 2};
        int t4[] = {2, 1};
        int t5[] = {1, 2, 3, 4, 5};
        int t6[] = {5, 4, 3, 2, 1};
        int t7[] = {4, 1, 5, 2, 3};
        int t8[] = {3, 1, 3, 2, 1}, r8[] = {1, 1, 2, 3, 3};
        int t9[] = {0, -5, 8, -2, 4}, r9[] = {-5, -2, 0, 4, 8};
        int t10[] = {6, 6, 6, 6};
        int t11[] = {9, -1, 9, 0, -1, 5}, r11[] = {-1, -1, 0, 5, 9, 9};
        int t12[] = {3, 2, 1, 4, 5};
        int t13[] = {5, 3, 8, 3, 9, 1, 5, 7, 3}, r13[] = {1, 3, 3, 3, 5, 5, 7, 8, 9};
        int t14[] = {2, 2, 1, 1, 3, 3, 2, 1, 3, 2, 1, 3}, r14[] = {1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3};
        runTest(1, e0, 0, e0, "порожній масив");
        runTest(2, t2, 1, t2, "один елемент");
        runTest(3, t3, 2, t3, "два впорядковані");
        runTest(4, t4, 2, t3, "два переставлені");
        runTest(5, t5, 5, t5, "зростання");
        runTest(6, t6, 5, t5, "спадання");
        runTest(7, t7, 5, t5, "змішаний порядок");
        runTest(8, t8, 5, r8, "повторення");
        runTest(9, t9, 5, r9, "від'ємні числа");
        runTest(10, t10, 4, t10, "усі рівні");
        runTest(11, t11, 6, r11, "комбінований випадок");
        runTest(12, t12, 5, t5, "частково впорядкований");
        runTest(13, t13, 9, r13, "власний: непарна кількість, дублікати");
        runTest(14, t14, 12, r14, "власний: три значення");
    }
    cout << "Перевірок сортування: " << totalChecks << ", невдалих: " << failedChecks << "\n";

    // ===== Перевірки розбиття =====
    cout << "\n=== Перевірки функцій розбиття ===\n";
    {
        int l1[] = {5, 8, 6, 9, 2};           checkLomuto("опорний найменший", l1, 5, 0, 4);
        int l2[] = {3, 1, 4, 2, 9};           checkLomuto("опорний найбільший", l2, 5, 0, 4);
        int l3[] = {7, 2, 9, 4, 5};           checkLomuto("опорний середній", l3, 5, 0, 4);
        int l4[] = {4, 4, 1, 4, 4};           checkLomuto("опорний повторюється", l4, 5, 0, 4);
        int l5[] = {9, 9, 3, 8, 1, 7, 9, 9};  checkLomuto("підділянка [2,5]", l5, 8, 2, 5);
        int h1[] = {5, 8, 6, 9, 2};           checkHoare("змішаний порядок", h1, 5, 0, 4);
        int h2[] = {1, 2, 3, 4, 5};           checkHoare("зростання", h2, 5, 0, 4);
        int h3[] = {6, 6, 6, 6};              checkHoare("усі рівні", h3, 4, 0, 3);
        int h4[] = {2, 1};                    checkHoare("два елементи", h4, 2, 0, 1);
        int h5[] = {9, 9, 3, 8, 1, 7, 9, 9};  checkHoare("підділянка [2,5]", h5, 8, 2, 5);
    }
    cout << "Перевірок розбиття: " << partChecks << ", невдалих: " << partFailed << "\n";
    if (failedChecks || partFailed) return 1;

    // ===== Експеримент =====
    const char *typeName[4] = {"Зростання", "Спадання", "Змішаний", "Багато однакових"};
    int sizes[3] = {100, 1000, 10000};
    static Stats st[3][4][ALG];
    static bool ok[3][4];
    static int src[MAXN], work[MAXN];
    for (int z = 0; z < 3; z++)
        for (int t = 0; t < 4; t++) {
            makeSet(t, sizes[z], src);
            bool all = true;
            for (int k = 0; k < ALG; k++) all &= runSort(k, src, sizes[z], work, st[z][t][k]);
            ok[z][t] = all;
        }

    cout << "\n=== Експеримент: Ломуто та Гоар (порівняння / обміни / глибина) ===\n";
    cout << padL("n", 6) << "  " << pad("Набір", 17) << padL("Ломуто порівн.", 15) << padL("обміни", 10)
         << padL("глиб.", 7) << padL("Гоар порівн.", 14) << padL("обміни", 10) << padL("глиб.", 7) << "  Перевірка\n";
    for (int z = 0; z < 3; z++)
        for (int t = 0; t < 4; t++) {
            Stats &a = st[z][t][0], &b = st[z][t][1];
            cout << padL(to_string(sizes[z]), 6) << "  " << pad(typeName[t], 17)
                 << padL(to_string(a.cmp), 15) << padL(to_string(a.swp), 10) << padL(to_string(a.maxDepth), 7)
                 << padL(to_string(b.cmp), 14) << padL(to_string(b.swp), 10) << padL(to_string(b.maxDepth), 7)
                 << "  " << (ok[z][t] ? "успішно" : "помилка") << "\n";
        }

    cout << "\n=== Дослідження: Ломуто з медіаною трьох ===\n";
    cout << padL("n", 6) << "  " << pad("Набір", 17) << padL("порівн.", 12) << padL("обміни", 10)
         << padL("глибина", 9) << "  Перевірка\n";
    for (int z = 0; z < 3; z++)
        for (int t = 0; t < 4; t++) {
            Stats &a = st[z][t][2];
            cout << padL(to_string(sizes[z]), 6) << "  " << pad(typeName[t], 17)
                 << padL(to_string(a.cmp), 12) << padL(to_string(a.swp), 10) << padL(to_string(a.maxDepth), 9)
                 << "  " << (ok[z][t] ? "успішно" : "помилка") << "\n";
        }

    // ===== Варіант 19 =====
    cout << "\n=== Варіант 19 ===\n";
    static int var[MAXN]; int n = 15;
    if (!readArray(var, n)) {
        int def[15] = {32, 9, 39, 20, 14, 35, 23, 12, 37, 17, 8, 29, 22, 13, 26};
        n = 15; copyArr(def, var, 15);
        cout << "\n(введення недоступне, використано масив варіанта 19)";
    }
    cout << "\nПочаткова послідовність: "; printArr(var, n); cout << "\n";
    cout << pad("Схема", 8) << padL("Порівн.", 8) << padL("Обміни", 8) << padL("Виклики", 9)
         << padL("Глибина", 9) << "  Перевірка  Результат\n";
    for (int k = 0; k < 2; k++) {
        Stats s;
        bool good = runSort(k, var, n, work, s);
        cout << pad(algName[k], 8) << padL(to_string(s.cmp), 8) << padL(to_string(s.swp), 8)
             << padL(to_string(s.calls), 9) << padL(to_string(s.maxDepth), 9) << "  "
             << pad(good ? "успішно" : "помилка", 9) << "  ";
        printArr(work, n); cout << "\n";
    }
    for (int k = 0; k < 2; k++) {
        cout << "\nТрасування, схема " << algName[k] << ":\n";
        copyArr(var, work, n);
        S = Stats(); traceOn = true;
        qfn[k](work, 0, n - 1, 1, n);
        traceOn = false;
    }
    return 0;
}
