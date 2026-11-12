#include <iostream>
#include <climits>
#include <string>
using namespace std;

const int MAXN = 10000;

// Єдині правила підрахунку для ВСІХ алгоритмів:
//  - порівняння: зіставлення двох значень (службові перевірки циклів не рахуються);
//  - записи: кожне присвоєння елементу основного або допоміжного масиву
//    (обмін двох елементів = 2 записи; тимчасова змінна не рахується).
struct Stats {
    long long cmp = 0, wr = 0, merges = 0;
    int depth = 0;
};
Stats S;
bool traceOn = false;

void swapW(int &x, int &y) { int t = x; x = y; y = t; S.wr += 2; }

void printRange(const int a[], int lo, int hi) {   // [lo, hi)
    cout << "[";
    for (int i = lo; i < hi; i++) cout << (i > lo ? ", " : "") << a[i];
    cout << "]";
}
void printArr(const int a[], int n) { printRange(a, 0, n); }

// ======================= MergeSort =======================
// Правило меж: ділянка [lo, hi) – lo включено, hi виключено.
static int aux[MAXN];   // єдиний допоміжний масив, використовується повторно

// Зливає сусідні впорядковані [lo, mid) і [mid, hi); при рівних першою береться ліва частина
void mergeParts(int a[], int tmp[], int lo, int mid, int hi, int depth = 0) {
    int i = lo, j = mid, k = lo;
    while (i < mid && j < hi) {
        S.cmp++;
        if (a[i] <= a[j]) { tmp[k++] = a[i++]; S.wr++; }
        else              { tmp[k++] = a[j++]; S.wr++; }
    }
    while (i < mid) { tmp[k++] = a[i++]; S.wr++; }     // залишок лівої частини
    while (j < hi)  { tmp[k++] = a[j++]; S.wr++; }     // залишок правої частини
    if (traceOn && depth > 0) {
        cout << string((depth - 1) * 2, ' ') << "злиття: глибина " << depth << ", ліва ";
        printRange(a, lo, mid); cout << " + права "; printRange(a, mid, hi);
        cout << " -> "; printRange(tmp, lo, hi); cout << "\n";
    }
    for (int t = lo; t < hi; t++) { a[t] = tmp[t]; S.wr++; }   // результат назад
    S.merges++;
}

void mergeSortRec(int a[], int tmp[], int lo, int hi, int depth) {
    if (depth > S.depth) S.depth = depth;
    if (hi - lo < 2) {
        if (traceOn && hi - lo == 1)
            cout << string((depth - 1) * 2, ' ') << "базовий випадок: глибина " << depth << ", межі [" << lo << "," << hi
                 << "), елемент " << a[lo] << "\n";
        return;
    }
    int mid = lo + (hi - lo) / 2;
    if (traceOn)
        cout << string((depth - 1) * 2, ' ') << "поділ: глибина " << depth << ", межі [" << lo << "," << hi
             << "), середина " << mid << "\n";
    mergeSortRec(a, tmp, lo, mid, depth + 1);
    mergeSortRec(a, tmp, mid, hi, depth + 1);
    mergeParts(a, tmp, lo, mid, hi, depth);
}
void mergeSort(int a[], int n) { mergeSortRec(a, aux, 0, n, 1); }

// ======================= Інші алгоритми (за єдиними правилами) =======================
void insertionSort(int a[], int n) {
    for (int i = 1; i < n; i++) {
        int key = a[i], j = i - 1;
        while (j >= 0) {
            S.cmp++;
            if (a[j] > key) { a[j + 1] = a[j]; S.wr++; j--; } else break;
        }
        a[j + 1] = key; S.wr++;
    }
}
void selectionSort(int a[], int n) {
    for (int i = 0; i + 1 < n; i++) {
        int m = i;
        for (int j = i + 1; j < n; j++) { S.cmp++; if (a[j] < a[m]) m = j; }
        if (m != i) swapW(a[i], a[m]);
    }
}
void bubbleSort(int a[], int n) {
    for (int pass = 0; pass + 1 < n; pass++) {
        bool sw = false;
        for (int j = 0; j + 1 < n - pass; j++) {
            S.cmp++;
            if (a[j] > a[j + 1]) { swapW(a[j], a[j + 1]); sw = true; }
        }
        if (!sw) break;
    }
}
void shellSort(int a[], int n) {                    // кроки 3h+1 (у зворотному порядку)
    int h = 1;
    while (3 * h + 1 < n) h = 3 * h + 1;
    if (n < 2) return;
    for (; h >= 1; h = (h - 1) / 3) {
        for (int i = h; i < n; i++) {
            int key = a[i], j = i;
            while (j >= h) {
                S.cmp++;
                if (a[j - h] > key) { a[j] = a[j - h]; S.wr++; j -= h; } else break;
            }
            a[j] = key; S.wr++;
        }
        if (h == 1) break;
    }
}
void quickRec(int a[], int lo, int hi, int depth) {   // розбиття Гоара, опорне – середній елемент
    if (depth > S.depth) S.depth = depth;
    if (lo >= hi) return;
    int pivot = a[lo + (hi - lo) / 2], i = lo - 1, j = hi + 1;
    while (true) {
        do { i++; S.cmp++; } while (a[i] < pivot);
        do { j--; S.cmp++; } while (a[j] > pivot);
        if (i >= j) break;
        swapW(a[i], a[j]);
    }
    quickRec(a, lo, j, depth + 1);
    quickRec(a, j + 1, hi, depth + 1);
}
void quickSort(int a[], int n) { quickRec(a, 0, n - 1, 1); }
void siftDown(int a[], int size, int i) {
    while (true) {
        int l = 2 * i + 1, r = 2 * i + 2, m = i;
        if (l < size) { S.cmp++; if (a[l] > a[m]) m = l; }
        if (r < size) { S.cmp++; if (a[r] > a[m]) m = r; }
        if (m == i) break;
        swapW(a[i], a[m]);
        i = m;
    }
}
void heapSort(int a[], int n) {
    for (int i = n / 2 - 1; i >= 0; i--) siftDown(a, n, i);
    for (int end = n - 1; end >= 1; end--) { swapW(a[0], a[end]); siftDown(a, end, 0); }
}

const int ALG = 7;
typedef void (*SortFn)(int[], int);
SortFn fns[ALG] = {insertionSort, selectionSort, bubbleSort, shellSort, quickSort, heapSort, mergeSort};
const char *algName[ALG] = {"Вставки", "Вибір", "Бульбашка", "Шелл", "QuickSort", "HeapSort", "MergeSort"};

// ======================= Перевірки та допоміжні =======================
bool isSorted(const int a[], int n) {
    for (int i = 1; i < n; i++) if (a[i] < a[i - 1]) return false;
    return true;
}
bool sameComposition(const int x[], const int y[], int n) {
    unsigned long long s1 = 0, s2 = 0, q1 = 0, q2 = 0;    // беззнакові: переповнення не заважає порівнянню
    for (int i = 0; i < n; i++) {
        s1 += (unsigned long long)(long long)x[i]; q1 += (unsigned long long)((long long)x[i] * x[i]);
        s2 += (unsigned long long)(long long)y[i]; q2 += (unsigned long long)((long long)y[i] * y[i]);
    }
    return s1 == s2 && q1 == q2;
}
void copyArr(const int s[], int d[], int n) { for (int i = 0; i < n; i++) d[i] = s[i]; }

int visLen(const string &s) { int l = 0; for (unsigned char c : s) if ((c & 0xC0) != 0x80) l++; return l; }
string pad(const string &s, int w) { int l = visLen(s); return s + string(w > l ? w - l : 0, ' '); }
string padL(const string &s, int w) { int l = visLen(s); return string(w > l ? w - l : 0, ' ') + s; }

// Запуск алгоритму k на копії; true, якщо результат правильний
bool runAlg(int k, const int src[], int n, int work[], Stats &out) {
    copyArr(src, work, n);
    S = Stats(); traceOn = false;
    fns[k](work, n);
    out = S;
    return isSorted(work, n) && sameComposition(src, work, n);
}

int sortChecks = 0, sortFailed = 0, mergeChecks = 0, mergeFailed = 0;

void runTest(int no, const int src[], int n, const int expected[], const char *what) {
    static int w[MAXN];
    Stats st;
    bool ok = runAlg(6, src, n, w, st);
    for (int i = 0; i < n && ok; i++) if (w[i] != expected[i]) ok = false;
    sortChecks++; if (!ok) sortFailed++;
    cout << "MergeSort тест " << padL(to_string(no), 2) << " [" << what << "]: ";
    if (ok) cout << "успішно\n";
    else { cout << "помилка, вхід "; printArr(src, n); cout << ", фактично "; printArr(w, n); cout << "\n"; }
}

// Окрема перевірка злиття: масив arr, ліва [lo, mid), права [mid, hi)
void checkMerge(const char *what, const int arr[], int n, int lo, int mid, int hi) {
    int a[64], tmp[64];
    copyArr(arr, a, n);
    S = Stats();
    mergeParts(a, tmp, lo, mid, hi);
    bool ok = isSorted(a + lo, hi - lo);
    for (int i = 0; i < n; i++) if ((i < lo || i >= hi) && a[i] != arr[i]) ok = false;   // поза ділянкою без змін
    int s1 = 0, s2 = 0;
    for (int i = lo; i < hi; i++) { s1 += arr[i]; s2 += a[i]; }
    if (s1 != s2) ok = false;
    mergeChecks++; if (!ok) mergeFailed++;
    cout << "Злиття " << mergeChecks << " [" << what << "]: ";
    printRange(arr, lo, mid); cout << " + "; printRange(arr, mid, hi); cout << " -> "; printRange(a, lo, hi);
    cout << ", порівнянь " << S.cmp << ", записів " << S.wr << ": " << (ok ? "успішно" : "помилка") << "\n";
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
        if (type == 0 || type == 2) a[i] = i + 1;            // зростання / майже впорядкований
        else if (type == 1) a[i] = n - i;                    // спадання
        else a[i] = (i * 37 + 11) % (n + 1);                 // змішаний (повторюваний)
    }
    if (type == 2) {                                         // кілька сусідніх пар міняємо місцями
        int step = n / 5 > 3 ? n / 5 : 3;
        for (int i = 1; i + 1 < n; i += step) { int t = a[i]; a[i] = a[i + 1]; a[i + 1] = t; }
    }
}

int main() {
    // ===== Тести злиття =====
    cout << "=== Перевірки функції злиття ===\n";
    {
        int m1[] = {1, 3, 5, 2, 4, 6};          checkMerge("однакова довжина", m1, 6, 0, 3, 6);
        int m2[] = {1, 2, 3, 4, 5, 6, 7};       checkMerge("різна довжина", m2, 7, 0, 2, 7);
        int m3[] = {1, 2, 3};                   checkMerge("порожня ліва частина", m3, 3, 0, 0, 3);
        int m4[] = {1, 2, 3};                   checkMerge("порожня права частина", m4, 3, 0, 3, 3);
        int m5[] = {2, 2, 4, 2, 3, 3};          checkMerge("повторення", m5, 6, 0, 3, 6);
        int m6[] = {-5, -1, 3, -4, -2, 0};      checkMerge("від'ємні числа", m6, 6, 0, 3, 6);
        int m7[] = {7, 8, 9, 1, 2, 3};          checkMerge("ліва частина більша за праву", m7, 6, 0, 3, 6);
        int m8[] = {99, 4, 6, 1, 5, 99};        checkMerge("підділянка [1,5), краї не змінюються", m8, 6, 1, 3, 5);
    }
    cout << "Перевірок злиття: " << mergeChecks << ", невдалих: " << mergeFailed << "\n";

    // ===== Тести MergeSort =====
    cout << "\n=== Автоматизовані перевірки MergeSort ===\n";
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
        int t12[] = {8, 3, 7, 2, 6, 1, 5}, r12[] = {1, 2, 3, 5, 6, 7, 8};
        int t13[15], r13[15];
        for (int i = 0; i < 15; i++) { t13[i] = (i * 7 + 4) % 15; r13[i] = i; }
        int t14[] = {INT_MAX, 0, INT_MIN, INT_MAX, -1, INT_MIN}, r14[] = {INT_MIN, INT_MIN, -1, 0, INT_MAX, INT_MAX};
        runTest(1, e0, 0, e0, "порожній масив");
        runTest(2, t2, 1, t2, "один елемент");
        runTest(3, t3, 2, t3, "два впорядковані");
        runTest(4, t4, 2, t3, "два переставлені");
        runTest(5, t5, 5, t5, "зростання");
        runTest(6, t6, 5, t5, "спадання");
        runTest(7, t7, 5, t5, "непарна довжина");
        runTest(8, t8, 5, r8, "повторення");
        runTest(9, t9, 5, r9, "від'ємні числа");
        runTest(10, t10, 4, t10, "усі рівні");
        runTest(11, t11, 6, r11, "комбінований випадок");
        runTest(12, t12, 7, r12, "кілька рівнів поділу");
        runTest(13, t13, 15, r13, "власний: 15 елементів");
        runTest(14, t14, 6, r14, "власний: INT_MIN та INT_MAX");
    }
    cout << "Перевірок MergeSort: " << sortChecks << ", невдалих: " << sortFailed << "\n";
    if (sortFailed || mergeFailed) return 1;

    // ===== Підсумковий експеримент =====
    const char *typeName[4] = {"Зростання", "Спадання", "Майже впоряд.", "Змішаний"};
    int sizes[3] = {100, 1000, 10000};
    static Stats st[3][4][ALG];
    static bool ok[3][4][ALG];
    static int src[MAXN], work[MAXN];
    bool allOk = true;
    for (int z = 0; z < 3; z++)
        for (int t = 0; t < 4; t++) {
            makeSet(t, sizes[z], src);
            for (int k = 0; k < ALG; k++) {
                ok[z][t][k] = runAlg(k, src, sizes[z], work, st[z][t][k]);
                allOk &= ok[z][t][k];
            }
        }
    cout << "\nУсі 84 запуски (3 розміри x 4 набори x 7 алгоритмів) пройшли перевірку порядку та складу: "
         << (allOk ? "так" : "НІ") << "\n";

    for (int what = 0; what < 2; what++) {
        cout << "\n=== Порівняння алгоритмів: " << (what == 0 ? "порівняння значень" : "записи до масиву") << " ===\n";
        cout << padL("n", 6) << "  " << pad("Набір", 14);
        for (int k = 0; k < ALG; k++) cout << padL(algName[k], 11);
        cout << "\n";
        for (int z = 0; z < 3; z++)
            for (int t = 0; t < 4; t++) {
                long long v[ALG], mn = -1;
                for (int k = 0; k < ALG; k++) {
                    v[k] = what == 0 ? st[z][t][k].cmp : st[z][t][k].wr;
                    if (mn < 0 || v[k] < mn) mn = v[k];
                }
                cout << padL(to_string(sizes[z]), 6) << "  " << pad(typeName[t], 14);
                for (int k = 0; k < ALG; k++) cout << padL(to_string(v[k]) + (v[k] == mn ? "*" : ""), 11);
                cout << "\n";
            }
        cout << "(* - найменше значення в рядку)\n";
    }

    cout << "\n=== MergeSort: структура витрат ===\n";
    cout << padL("n", 6) << "  " << pad("Набір", 14) << padL("Порівн.", 10) << padL("Записи", 10)
         << padL("Частка записів", 16) << padL("Злиття", 8) << padL("Глибина", 9) << "\n";
    for (int z = 0; z < 3; z++)
        for (int t = 0; t < 4; t++) {
            Stats &m = st[z][t][6];
            double share = 100.0 * m.wr / (m.cmp + m.wr);
            char buf[16]; snprintf(buf, sizeof buf, "%.1f%%", share);
            cout << padL(to_string(sizes[z]), 6) << "  " << pad(typeName[t], 14)
                 << padL(to_string(m.cmp), 10) << padL(to_string(m.wr), 10) << padL(buf, 16)
                 << padL(to_string(m.merges), 8) << padL(to_string(m.depth), 9) << "\n";
        }

    cout << "\n=== Додаткова пам'ять (n = 10000) ===\n";
    {
        int n = sizes[2];
        cout << pad("Алгоритм", 12) << padL("Додаткові елементи масиву", 28) << padL("Макс. глибина рекурсії", 24) << "\n";
        for (int k = 0; k < ALG; k++) {
            int dmax = 0;
            for (int t = 0; t < 4; t++) if (st[2][t][k].depth > dmax) dmax = st[2][t][k].depth;
            cout << pad(algName[k], 12) << padL(k == 6 ? to_string(n) : "0", 28)
                 << padL((k == 4 || k == 6) ? to_string(dmax) : "-", 24) << "\n";
        }
    }

    // ===== Варіант 19 =====
    cout << "\n=== Варіант 19 ===\n";
    static int var[MAXN]; int n = 16;
    if (!readArray(var, n)) {
        int def[16] = {32, 9, 39, 20, 14, 35, 23, 12, 37, 17, 8, 29, 22, 13, 26, 34};
        n = 16; copyArr(def, var, 16);
        cout << "\n(введення недоступне, використано масив варіанта 19)";
    }
    cout << "\nПочаткова послідовність: "; printArr(var, n); cout << "\n";
    cout << pad("Алгоритм", 12) << padL("Порівн.", 9) << padL("Записи", 8) << padL("Додатк. пам'ять", 17) << "  Перевірка\n";
    for (int k = 0; k < ALG; k++) {
        Stats s;
        bool good = runAlg(k, var, n, work, s);
        cout << pad(algName[k], 12) << padL(to_string(s.cmp), 9) << padL(to_string(s.wr), 8)
             << padL(k == 6 ? to_string(n) + " ел." : (k == 4 ? "стек " + to_string(s.depth) : string("O(1)")), 17)
             << "  " << (good ? "успішно" : "помилка") << "\n";
    }
    {
        Stats s;
        runAlg(6, var, n, work, s);
        cout << "MergeSort: порівнянь " << s.cmp << ", записів " << s.wr << ", злиттів " << s.merges
             << ", макс. глибина " << s.depth << "\n";
    }
    cout << "\nТрасування MergeSort, перші 8 елементів варіанта:\n";
    {
        int a[8]; copyArr(var, a, 8);
        S = Stats(); traceOn = true;
        mergeSort(a, 8);
        traceOn = false;
        cout << "Результат: "; printArr(a, 8); cout << "\n";
    }
    cout << "\nТрасування MergeSort для всього масиву варіанта:\n";
    {
        int a[MAXN]; copyArr(var, a, n);
        S = Stats(); traceOn = true;
        mergeSort(a, n);
        traceOn = false;
        cout << "Результат: "; printArr(a, n); cout << "\n";
    }
    return 0;
}
