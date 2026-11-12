#include <iostream>
#include <string>
using namespace std;

const int MAXN = 10000;

// Статистика одного етапу: порівняння значень, обміни, запуски відновлення купи
struct Stats {
    long long cmp = 0, swp = 0, restores = 0;
};
Stats S;                    // поточний лічильник
long long recCalls = 0;     // кількість викликів рекурсивної функції відновлення
bool traceOn = false;

void sw(int &x, int &y) { int t = x; x = y; y = t; S.swp++; }

void printArr(const int a[], int n) {
    cout << "[";
    for (int i = 0; i < n; i++) cout << (i ? ", " : "") << a[i];
    cout << "]";
}
// Масив із позначкою межі активної купи: [купа | відсортована частина]
void printSplit(const int a[], int n, int active) {
    cout << "[";
    for (int i = 0; i < n; i++) {
        if (i == active) cout << (i ? " | " : "| ");
        else if (i) cout << ", ";
        cout << a[i];
    }
    cout << "]";
}

// ---------- Перевірка властивості max-купи ----------
// Повертає індекс першого батьківського вузла, де властивість порушена, або -1
int firstViolation(const int a[], int size) {
    for (int i = 0; 2 * i + 1 < size; i++) {
        int l = 2 * i + 1, r = 2 * i + 2;
        if (a[l] > a[i]) return i;
        if (r < size && a[r] > a[i]) return i;
    }
    return -1;
}

// ---------- Відновлення купи (ітеративне) ----------
void siftDown(int a[], int size, int i) {
    S.restores++;
    while (true) {
        int l = 2 * i + 1, r = 2 * i + 2, largest = i;
        if (l < size) { S.cmp++; if (a[l] > a[largest]) largest = l; }
        if (r < size) { S.cmp++; if (a[r] > a[largest]) largest = r; }
        if (largest == i) break;
        sw(a[i], a[largest]);
        i = largest;
    }
}

// ---------- Відновлення купи (рекурсивне) ----------
void siftDownRec(int a[], int size, int i, bool top = true) {
    if (top) S.restores++;
    recCalls++;
    int l = 2 * i + 1, r = 2 * i + 2, largest = i;
    if (l < size) { S.cmp++; if (a[l] > a[largest]) largest = l; }
    if (r < size) { S.cmp++; if (a[r] > a[largest]) largest = r; }
    if (largest != i) {
        sw(a[i], a[largest]);
        siftDownRec(a, size, largest, false);
    }
}

void restore(int a[], int size, int i, bool rec) {
    if (rec) siftDownRec(a, size, i); else siftDown(a, size, i);
}

// ---------- Побудова max-купи ----------
void buildHeap(int a[], int n, bool rec) {
    if (traceOn) { cout << "  початок: "; printArr(a, n); cout << "\n"; }
    for (int i = n / 2 - 1; i >= 0; i--) {       // від останнього внутрішнього вузла до кореня
        restore(a, n, i, rec);
        if (traceOn) { cout << "  вузол " << i << ", активний розмір " << n << ": "; printArr(a, n); cout << "\n"; }
    }
}

// ---------- HeapSort ----------
struct Result {
    Stats build, extract;
    long long recCalls = 0;
    int invariantErrors = 0;     // порушення перевірок після вилучень
};

// check = true: після кожного вилучення перевіряються купа і впорядкований хвіст
Result heapSort(int a[], int n, bool rec, bool check, int traceExtract = 0) {
    Result res;
    S = Stats(); recCalls = 0;
    buildHeap(a, n, rec);
    res.build = S;
    S = Stats();
    for (int end = n - 1; end >= 1; end--) {
        sw(a[0], a[end]);                         // найбільший у кінець активної частини
        restore(a, end, 0, rec);                  // активна купа тепер має розмір end
        if (check) {
            bool heapOk = firstViolation(a, end) == -1;
            bool tailOk = true;
            for (int i = end; i + 1 < n; i++) if (a[i] > a[i + 1]) tailOk = false;
            if (a[0] > a[end]) tailOk = false;    // хвіст містить найбільші елементи
            if (!heapOk || !tailOk) res.invariantErrors++;
            int step = n - end;
            if (step <= traceExtract) {
                cout << "  вилучення " << step << ": обмін a[0] і a[" << end << "], активний розмір " << end
                     << ": ";
                printSplit(a, n, end);
                cout << "; купа: " << (heapOk ? "так" : "ні") << "; хвіст впорядкований: " << (tailOk ? "так" : "ні") << "\n";
            }
        }
    }
    res.extract = S;
    res.recCalls = recCalls;
    return res;
}

// ---------- Допоміжні ----------
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

int sortChecks = 0, sortFailed = 0, buildChecks = 0, buildFailed = 0, propChecks = 0, propFailed = 0;

void runTest(int no, const int src[], int n, const int expected[], const char *what) {
    for (int rec = 0; rec < 2; rec++) {
        static int a[MAXN];
        copyArr(src, a, n);
        Result r = heapSort(a, n, rec == 1, true);
        bool ok = isSorted(a, n) && sameComposition(src, a, n) && r.invariantErrors == 0;
        for (int i = 0; i < n && ok; i++) if (a[i] != expected[i]) ok = false;
        sortChecks++; if (!ok) sortFailed++;
        cout << "Сортування " << padL(to_string(no), 2) << " " << pad(rec ? "рекурсивно" : "ітеративно", 11)
             << " [" << what << "]: ";
        if (ok) cout << "успішно\n";
        else { cout << "помилка, отримано: "; printArr(a, n); cout << "\n"; }
        if (n >= 5) {                              // результат побудови купи
            static int b[MAXN];
            copyArr(src, b, n);
            S = Stats();
            buildHeap(b, n, rec == 1);
            int mx = src[0];
            for (int i = 1; i < n; i++) if (src[i] > mx) mx = src[i];
            bool bok = firstViolation(b, n) == -1 && b[0] == mx && sameComposition(src, b, n);
            buildChecks++; if (!bok) buildFailed++;
            cout << "Побудова   " << padL(to_string(no), 2) << " " << pad(rec ? "рекурсивно" : "ітеративно", 11)
                 << " [" << what << "]: " << (bok ? "успішно" : "помилка") << "\n";
        }
    }
}

void checkProp(const char *what, const int a[], int size, int expectedViolation) {
    int v = firstViolation(a, size);
    bool ok = v == expectedViolation;
    propChecks++; if (!ok) propFailed++;
    cout << "Властивість купи " << propChecks << " [" << what << "]: ";
    printArr(a, size);
    cout << ", розмір " << size << ", перше порушення: " << (v < 0 ? string("немає") : "вузол " + to_string(v))
         << ": " << (ok ? "успішно" : "помилка") << "\n";
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
        if (type == 0) a[i] = i + 1;
        else if (type == 1) a[i] = n - i;
        else if (type == 2) a[i] = (i * 37 + 11) % (n + 1);
        else a[i] = (i * 7 + 3) % 10;
    }
}

int main() {
    cout << "=== Автоматизовані перевірки ===\n";
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
        int t12[] = {10, 7, 8, 4, 2, 1, 3}, r12[] = {1, 2, 3, 4, 7, 8, 10};
        int t13[] = {5, 3, 8, 3, 9, 1, 5, 7}, r13[] = {1, 3, 3, 5, 5, 7, 8, 9};
        int t14[] = {2, 2, 1, 1, 3, 3, 2, 1, 3, 2, 1, 3, 2, 1, 3}, r14[15];
        int t15[21], r15[21];
        for (int i = 0; i < 15; i++) r14[i] = (i < 5 ? 1 : (i < 10 ? 2 : 3));
        for (int i = 0; i < 21; i++) { t15[i] = (i * 11 + 5) % 23 - 9; r15[i] = t15[i]; }
        for (int i = 0; i < 21; i++) for (int j = i + 1; j < 21; j++) if (r15[j] < r15[i]) { int t = r15[i]; r15[i] = r15[j]; r15[j] = t; }
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
        runTest(12, t12, 7, r12, "неповний останній рівень");
        runTest(13, t13, 8, r13, "власний: 8 елементів");
        runTest(14, t14, 15, r14, "власний: три значення");
        runTest(15, t15, 21, r15, "власний: 21 елемент");
    }
    cout << "Перевірок сортування: " << sortChecks << " (невдалих " << sortFailed << "), побудови: "
         << buildChecks << " (невдалих " << buildFailed << ")\n";

    cout << "\n=== Перевірки властивості купи ===\n";
    {
        int g1[] = {9, 7, 8, 2, 3, 4, 1};
        int g2[] = {9, 7, 8, 2, 3, 4, 100};
        int g3[] = {10, 10, 10};
        int b1[] = {1, 2, 3};
        int b2[] = {9, 7, 8, 2, 10, 4, 1};
        int b3[] = {9, 7, 8, 2, 3, 9, 1};
        checkProp("правильна купа", g1, 7, -1);
        checkProp("межа активної купи", g2, 6, -1);
        checkProp("усі рівні", g3, 3, -1);
        checkProp("порушення в корені", b1, 3, 0);
        checkProp("нащадок більший за батька", b2, 7, 1);
        checkProp("порушення у правій гілці", b3, 7, 2);
    }
    cout << "Перевірок властивості: " << propChecks << ", невдалих: " << propFailed << "\n";
    if (sortFailed || buildFailed || propFailed) return 1;

    // ===== Експеримент =====
    const char *typeName[4] = {"Зростання", "Спадання", "Змішаний", "Багато однакових"};
    int sizes[3] = {100, 1000, 10000};
    static Result res[3][4], resRec[3][4];
    static bool ok[3][4], same[3][4];
    static int src[MAXN], work[MAXN];
    for (int z = 0; z < 3; z++)
        for (int t = 0; t < 4; t++) {
            int n = sizes[z];
            makeSet(t, n, src);
            copyArr(src, work, n);
            res[z][t] = heapSort(work, n, false, false);
            bool good = isSorted(work, n) && sameComposition(src, work, n);
            copyArr(src, work, n);
            resRec[z][t] = heapSort(work, n, true, false);
            good = good && isSorted(work, n) && sameComposition(src, work, n);
            ok[z][t] = good;
            same[z][t] = res[z][t].build.cmp == resRec[z][t].build.cmp && res[z][t].build.swp == resRec[z][t].build.swp &&
                         res[z][t].extract.cmp == resRec[z][t].extract.cmp && res[z][t].extract.swp == resRec[z][t].extract.swp;
        }

    cout << "\n=== Експеримент: побудова і повне сортування (порівняння / обміни) ===\n";
    cout << padL("n", 6) << "  " << pad("Набір", 17) << padL("Побуд. порівн.", 15) << padL("обміни", 9)
         << padL("Всього порівн.", 15) << padL("обміни", 10) << padL("Частка побуд.", 15) << "  Перевірка\n";
    for (int z = 0; z < 3; z++)
        for (int t = 0; t < 4; t++) {
            Result &r = res[z][t];
            long long tc = r.build.cmp + r.extract.cmp, ts = r.build.swp + r.extract.swp;
            double share = 100.0 * r.build.cmp / tc;
            char buf[16]; snprintf(buf, sizeof buf, "%.1f%%", share);
            cout << padL(to_string(sizes[z]), 6) << "  " << pad(typeName[t], 17)
                 << padL(to_string(r.build.cmp), 15) << padL(to_string(r.build.swp), 9)
                 << padL(to_string(tc), 15) << padL(to_string(ts), 10) << padL(buf, 15)
                 << "  " << (ok[z][t] ? "успішно" : "помилка") << "\n";
        }

    cout << "\n=== Рекурсивне та ітеративне відновлення ===\n";
    cout << padL("n", 6) << "  " << pad("Набір", 17) << padL("Запусків", 10) << padL("Рекурс. викликів", 18)
         << "  Порівн. та обміни збігаються\n";
    for (int z = 0; z < 3; z++)
        for (int t = 0; t < 4; t++) {
            Result &r = res[z][t];
            cout << padL(to_string(sizes[z]), 6) << "  " << pad(typeName[t], 17)
                 << padL(to_string(r.build.restores + r.extract.restores), 10)
                 << padL(to_string(resRec[z][t].recCalls), 18) << "  " << (same[z][t] ? "так" : "ні") << "\n";
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
    cout << "Побудова купи:\n";
    copyArr(var, work, n);
    traceOn = true;
    Result r;
    {
        // побудова і перші три вилучення з трасуванням
        S = Stats(); recCalls = 0;
        buildHeap(work, n, false);
        traceOn = false;
        cout << "Стан після побудови: "; printArr(work, n);
        cout << "; купа: " << (firstViolation(work, n) == -1 ? "так" : "ні") << "\n";
    }
    cout << "Перші три вилучення:\n";
    copyArr(var, work, n);
    r = heapSort(work, n, false, true, 3);
    cout << "Остаточний результат: "; printArr(work, n);
    cout << "; перевірка: " << (isSorted(work, n) && sameComposition(var, work, n) ? "успішно" : "помилка") << "\n";
    cout << "\nСтатистика:\n";
    cout << pad("Етап", 12) << padL("Порівняння", 12) << padL("Обміни", 9) << padL("Відновлення", 13) << "\n";
    cout << pad("Побудова", 12) << padL(to_string(r.build.cmp), 12) << padL(to_string(r.build.swp), 9)
         << padL(to_string(r.build.restores), 13) << "\n";
    cout << pad("Вилучення", 12) << padL(to_string(r.extract.cmp), 12) << padL(to_string(r.extract.swp), 9)
         << padL(to_string(r.extract.restores), 13) << "\n";
    cout << pad("Усього", 12) << padL(to_string(r.build.cmp + r.extract.cmp), 12)
         << padL(to_string(r.build.swp + r.extract.swp), 9)
         << padL(to_string(r.build.restores + r.extract.restores), 13) << "\n";
    return 0;
}
