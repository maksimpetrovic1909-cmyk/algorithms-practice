#include <iostream>
#include <string>
using namespace std;

const int MAXN = 10000;
const int MAXSTEPS = 40;

// Порівняння = зіставлення двох значень; переміщення = запис значення в елемент
// масиву або тимчасову змінну. Службові перевірки циклів не враховуються.
struct Stats {
    long long cmp = 0, mov = 0;
    int steps = 0;
};

// ---------- Послідовності кроків ----------
// 0 - ділення навпіл; 1 - 1,4,13,40,... (h = 3h+1) у зворотному порядку;
// 2 - власна додаткова: 1,5,19,41,109,... (Седжвіка) у зворотному порядку
const int SEQ = 3;
const char *seqName[SEQ] = {"Шелл: ділення навпіл", "Шелл: 3h+1", "Шелл: Седжвік"};

int genSteps(int type, int n, int st[]) {
    int cnt = 0;
    if (n < 2) return 0;
    if (type == 0) {
        for (int h = n / 2; h >= 1; h /= 2) st[cnt++] = h;
    } else {
        int asc[MAXSTEPS], m = 0;
        if (type == 1) {
            for (int h = 1; h < n; h = 3 * h + 1) asc[m++] = h;
        } else {
            const int sed[] = {1, 5, 19, 41, 109, 209, 505, 929, 2161, 3905, 8929, 16001};
            for (int i = 0; i < 12 && sed[i] < n; i++) asc[m++] = sed[i];
        }
        for (int i = m - 1; i >= 0; i--) st[cnt++] = asc[i];   // від найбільшого до 1
    }
    return cnt;
}

void printArr(const int a[], int n) {
    cout << "[";
    for (int i = 0; i < n; i++) cout << (i ? ", " : "") << a[i];
    cout << "]";
}

// ---------- Сортування Шелла ----------
// demo = true: після кожного завершеного кроку друкується весь масив
void shellSort(int a[], int n, int seq, Stats &s, bool demo = false, int *stepsOut = nullptr) {
    s = Stats();
    int st[MAXSTEPS];
    int cnt = genSteps(seq, n, st);
    s.steps = cnt;
    if (stepsOut) for (int i = 0; i < cnt; i++) stepsOut[i] = st[i];
    for (int k = 0; k < cnt; k++) {
        int h = st[k];
        for (int i = h; i < n; i++) {            // вставки в групах з відстанню h
            int key = a[i]; s.mov++;
            int j = i;
            while (j >= h) {
                s.cmp++;
                if (a[j - h] > key) { a[j] = a[j - h]; s.mov++; j -= h; }
                else break;
            }
            a[j] = key; s.mov++;
        }
        if (demo) { cout << "  після кроку " << h << ": "; printArr(a, n); cout << "\n"; }
    }
}

// Звичайні вставки (практична робота 2)
void insertionSort(int a[], int n, Stats &s) {
    s = Stats(); s.steps = 1;
    for (int i = 1; i < n; i++) {
        int key = a[i]; s.mov++;
        int j = i - 1;
        while (j >= 0) {
            s.cmp++;
            if (a[j] > key) { a[j + 1] = a[j]; s.mov++; j--; }
            else break;
        }
        a[j + 1] = key; s.mov++;
    }
}

// ---------- Допоміжні функції ----------
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

// Перевірка послідовності кроків: спадна, остання = 1, найбільша < n
bool stepsValid(int seq, int n) {
    int st[MAXSTEPS];
    int c = genSteps(seq, n, st);
    if (n < 2) return c == 0;
    if (c == 0 || st[c - 1] != 1 || st[0] >= n) return false;
    for (int i = 1; i < c; i++) if (st[i] >= st[i - 1]) return false;
    return true;
}

int visLen(const string &s) { int l = 0; for (unsigned char c : s) if ((c & 0xC0) != 0x80) l++; return l; }
string pad(const string &s, int w) { int l = visLen(s); return s + string(w > l ? w - l : 0, ' '); }
string padL(const string &s, int w) { int l = visLen(s); return string(w > l ? w - l : 0, ' ') + s; }

int totalChecks = 0, failedChecks = 0;
void runTest(int no, const int src[], int n, const int expected[], const char *what) {
    for (int k = 0; k < SEQ; k++) {
        static int a[MAXN];
        copyArr(src, a, n);
        Stats s;
        shellSort(a, n, k, s);
        bool ok = isSorted(a, n) && sameComposition(src, a, n) && stepsValid(k, n);
        for (int i = 0; i < n && ok; i++) if (a[i] != expected[i]) ok = false;
        totalChecks++;
        cout << "Тест " << padL(to_string(no), 2) << " " << pad(seqName[k], 22) << " [" << what << "]: ";
        if (ok) cout << "успішно\n";
        else { failedChecks++; cout << "помилка, отримано: "; printArr(a, n); cout << "\n"; }
    }
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
        if (type == 0 || type == 2) a[i] = i + 1;
        else if (type == 1) a[i] = n - i;
        else a[i] = (i * 37 + 11) % (n + 1);        // повторюваний змішаний набір
    }
    if (type == 2) {                                // кілька сусідніх пар міняємо місцями
        int step = n / 5 > 3 ? n / 5 : 3;
        for (int i = 1; i + 1 < n; i += step) { int t = a[i]; a[i] = a[i + 1]; a[i + 1] = t; }
    }
}

int main() {
    // ===== Автотести =====
    cout << "=== Автоматизовані перевірки ===\n";
    {
        int e0[1] = {0};
        int t2[] = {4};
        int t3[] = {1, 2, 3, 4, 5};
        int t4[] = {5, 4, 3, 2, 1};
        int t5[] = {7, 2, 9, 1, 5}, r5[] = {1, 2, 5, 7, 9};
        int t6[] = {3, 1, 3, 2, 1}, r6[] = {1, 1, 2, 3, 3};
        int t7[] = {0, -6, 8, -2, 4}, r7[] = {-6, -2, 0, 4, 8};
        int t8[] = {6, 6, 6, 6};
        int t9[] = {2, 1}, r9[] = {1, 2};
        int t10[] = {1, 2, 4, 3, 5, 6}, r10[] = {1, 2, 3, 4, 5, 6};
        int t11[] = {9, 3, 7, 1, 8, 2, 5}, r11[] = {1, 2, 3, 5, 7, 8, 9};
        int t12[22], r12[22];
        for (int i = 0; i < 22; i++) { t12[i] = (i * 7 + 3) % 23 - 8; }
        copyArr(t12, r12, 22);
        for (int i = 0; i < 22; i++) for (int j = i + 1; j < 22; j++) if (r12[j] < r12[i]) { int t = r12[i]; r12[i] = r12[j]; r12[j] = t; }
        runTest(1, e0, 0, e0, "порожній масив");
        runTest(2, t2, 1, t2, "один елемент");
        runTest(3, t3, 5, t3, "уже впорядкований");
        runTest(4, t4, 5, t3, "зворотний порядок");
        runTest(5, t5, 5, r5, "змішаний порядок");
        runTest(6, t6, 5, r6, "повторення");
        runTest(7, t7, 5, r7, "від'ємні значення");
        runTest(8, t8, 4, t8, "однакові елементи");
        runTest(9, t9, 2, r9, "мінімальна перестановка");
        runTest(10, t10, 6, r10, "майже впорядкований");
        runTest(11, t11, 7, r11, "власний: непарна кількість");
        runTest(12, t12, 22, r12, "власний: 22 елементи");
    }
    cout << "Усього перевірок: " << totalChecks << ", невдалих: " << failedChecks << "\n";
    if (failedChecks) return 1;

    // ===== Експеримент =====
    const char *typeName[4] = {"Зростання", "Спадання", "Майже впоряд.", "Змішаний"};
    int sizes[3] = {100, 1000, 10000};
    static Stats st[3][4][4];            // [розмір][набір][0 вставки, 1..3 Шелл]
    static bool ok[3][4];
    static int src[MAXN], tmp[MAXN];
    for (int z = 0; z < 3; z++)
        for (int t = 0; t < 4; t++) {
            int n = sizes[z];
            makeSet(t, n, src);
            bool all = true;
            copyArr(src, tmp, n); insertionSort(tmp, n, st[z][t][0]);
            all &= isSorted(tmp, n) && sameComposition(src, tmp, n);
            for (int k = 0; k < SEQ; k++) {
                copyArr(src, tmp, n);
                shellSort(tmp, n, k, st[z][t][k + 1]);
                all &= isSorted(tmp, n) && sameComposition(src, tmp, n);
            }
            ok[z][t] = all;
        }

    for (int what = 0; what < 2; what++) {
        cout << "\n=== Експеримент: " << (what == 0 ? "порівняння" : "переміщення") << " ===\n";
        cout << padL("n", 6) << "  " << pad("Набір", 14) << padL("Вставки", 13) << padL("Шелл n/2", 13)
             << padL("Шелл 3h+1", 13) << padL("Седжвік", 13) << "  Перевірка\n";
        for (int z = 0; z < 3; z++)
            for (int t = 0; t < 4; t++) {
                long long v[4]; long long mn = -1;
                for (int k = 0; k < 4; k++) {
                    v[k] = what == 0 ? st[z][t][k].cmp : st[z][t][k].mov;
                    if (mn < 0 || v[k] < mn) mn = v[k];
                }
                cout << padL(to_string(sizes[z]), 6) << "  " << pad(typeName[t], 14);
                for (int k = 0; k < 4; k++) cout << padL(to_string(v[k]) + (v[k] == mn ? "*" : ""), 13);
                cout << "  " << (ok[z][t] ? "успішно" : "помилка") << "\n";
            }
        cout << "(* - найменше значення в рядку)\n";
    }
    cout << "\nКількість кроків і самі кроки (n = 100, 1000, 10000):\n";
    for (int z = 0; z < 3; z++)
        for (int k = 0; k < SEQ; k++) {
            int s[MAXSTEPS]; int c = genSteps(k, sizes[z], s);
            cout << "  n=" << padL(to_string(sizes[z]), 5) << " " << pad(seqName[k], 22) << padL(to_string(c), 2) << ": ";
            for (int i = 0; i < c; i++) cout << (i ? " " : "") << s[i];
            cout << "\n";
        }

    // ===== Варіант 19 =====
    cout << "\n=== Варіант 19 ===\n";
    static int var[MAXN]; int n = 15;
    if (!readArray(var, n)) {
        int def[15] = {31, 9, 38, 19, 13, 34, 23, 11, 36, 16, 8, 28, 21, 12, 25};
        n = 15; copyArr(def, var, 15);
        cout << "\n(введення недоступне, використано масив варіанта 19)";
    }
    cout << "\nПочаткова послідовність: "; printArr(var, n); cout << "\n";
    cout << pad("Алгоритм", 24) << padL("Порівн.", 8) << padL("Переміщ.", 10) << padL("Кроки", 7)
         << "  Перевірка  Використані кроки\n";
    {
        int a[MAXN]; Stats s;
        copyArr(var, a, n); insertionSort(a, n, s);
        cout << pad("Звичайні вставки", 24) << padL(to_string(s.cmp), 8) << padL(to_string(s.mov), 10)
             << padL("1", 7) << "  " << pad(isSorted(a, n) && sameComposition(var, a, n) ? "успішно" : "помилка", 9) << "  1\n";
        for (int k = 0; k < SEQ; k++) {
            int used[MAXSTEPS];
            copyArr(var, a, n);                  // відновлення початкових даних
            shellSort(a, n, k, s, false, used);
            bool good = isSorted(a, n) && sameComposition(var, a, n) && stepsValid(k, n);
            cout << pad(seqName[k], 24) << padL(to_string(s.cmp), 8) << padL(to_string(s.mov), 10)
                 << padL(to_string(s.steps), 7) << "  " << pad(good ? "успішно" : "помилка", 9) << "  ";
            for (int i = 0; i < s.steps; i++) cout << (i ? " → " : "") << used[i];
            cout << "\n";
        }
    }
    cout << "\nПроміжні стани (режим демонстрації):\n";
    for (int k = 0; k < SEQ; k++) {
        int a[MAXN]; Stats s;
        copyArr(var, a, n);
        cout << seqName[k] << ":\n  початок:      "; printArr(a, n); cout << "\n";
        shellSort(a, n, k, s, true);
    }
    return 0;
}
