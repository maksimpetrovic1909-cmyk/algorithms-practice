#include <iostream>
#include <string>
using namespace std;

const int MAXN = 1000;

// Порівняння = зіставлення двох значень; переміщення = присвоєння елементу
// масиву або допоміжній змінній. Для шейкерного та бульбашкового рахуємо проходи.
struct Stats {
    long long cmp = 0, mov = 0, passes = 0;
};

// ---------- Базові алгоритми (з практичної роботи 2) ----------

void insertionSort(int a[], int n, Stats &s) {
    s = Stats();
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

void bubbleSort(int a[], int n, Stats &s) {
    s = Stats();
    for (int pass = 0; pass + 1 < n; pass++) {
        s.passes++;
        bool swapped = false;
        for (int j = 0; j + 1 < n - pass; j++) {
            s.cmp++;
            if (a[j] > a[j + 1]) {
                int t = a[j]; s.mov++;
                a[j] = a[j + 1]; s.mov++;
                a[j + 1] = t; s.mov++;
                swapped = true;
            }
        }
        if (!swapped) break;
    }
}

// ---------- Нові алгоритми ----------

// Позиція вставлення key у впорядкованій ділянці a[0..hi): нове значення
// ставиться ПІСЛЯ всіх рівних елементів. Кожна перевірка a[mid] <= key = 1 порівняння.
int findPos(const int a[], int hi, int key, long long &cmp) {
    int lo = 0, up = hi;                 // шукаємо в [lo, up)
    while (lo < up) {
        int mid = lo + (up - lo) / 2;
        cmp++;
        if (a[mid] <= key) lo = mid + 1;
        else up = mid;
    }
    return lo;
}

void binaryInsertionSort(int a[], int n, Stats &s) {
    s = Stats();
    for (int i = 1; i < n; i++) {
        int key = a[i]; s.mov++;                         // збереження значення
        int pos = findPos(a, i, key, s.cmp);             // бінарний пошук позиції
        for (int j = i; j > pos; j--) { a[j] = a[j - 1]; s.mov++; }  // послідовний зсув
        a[pos] = key; s.mov++;                           // вставка
    }
}

void printArr(const int a[], int n);

// Шейкерне сортування; trace=true друкує межі та масив після кожного проходу
void cocktailSort(int a[], int n, Stats &s, bool trace = false) {
    s = Stats();
    int left = 0, right = n - 1;
    while (left < right) {
        bool swapped = false;
        // прохід зліва направо
        s.passes++;
        for (int j = left; j < right; j++) {
            s.cmp++;
            if (a[j] > a[j + 1]) {
                int t = a[j]; s.mov++;
                a[j] = a[j + 1]; s.mov++;
                a[j + 1] = t; s.mov++;
                swapped = true;
            }
        }
        if (trace) { cout << "  прохід " << s.passes << " (→) межі [" << left << ", " << right << "] "; printArr(a, n); cout << "\n"; }
        if (!swapped) break;                             // дострокове завершення
        right--;
        if (left >= right) break;
        swapped = false;
        // прохід справа наліво
        s.passes++;
        for (int j = right; j > left; j--) {
            s.cmp++;
            if (a[j - 1] > a[j]) {
                int t = a[j]; s.mov++;
                a[j] = a[j - 1]; s.mov++;
                a[j - 1] = t; s.mov++;
                swapped = true;
            }
        }
        if (trace) { cout << "  прохід " << s.passes << " (←) межі [" << left << ", " << right << "] "; printArr(a, n); cout << "\n"; }
        if (!swapped) break;
        left++;
    }
}

typedef void (*SortFn)(int[], int, Stats &);
void cocktailWrap(int a[], int n, Stats &s) { cocktailSort(a, n, s, false); }

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

void copyArr(const int src[], int dst[], int n) { for (int i = 0; i < n; i++) dst[i] = src[i]; }

void printArr(const int a[], int n) {
    cout << "[";
    for (int i = 0; i < n; i++) cout << (i ? ", " : "") << a[i];
    cout << "]";
}

int visLen(const string &s) { int l = 0; for (unsigned char c : s) if ((c & 0xC0) != 0x80) l++; return l; }
string pad(const string &s, int w) { int l = visLen(s); return s + string(w > l ? w - l : 0, ' '); }
string padL(const string &s, int w) { int l = visLen(s); return string(w > l ? w - l : 0, ' ') + s; }

// Запуск алгоритму на копії; true, якщо результат правильний
bool runAlg(SortFn f, const int src[], int n, Stats &s) {
    static int a[MAXN];
    copyArr(src, a, n);
    f(a, n, s);
    return isSorted(a, n) && sameComposition(src, a, n);
}

// ---------- Автотести ----------
int totalChecks = 0, failedChecks = 0;

void runTest(int no, const int src[], int n, const int expected[], const char *what) {
    const char *names[2] = {"Бінарні вставки", "Шейкерне"};
    SortFn fns[2] = {binaryInsertionSort, cocktailWrap};
    for (int k = 0; k < 2; k++) {
        int a[MAXN]; Stats s;
        copyArr(src, a, n);
        fns[k](a, n, s);
        bool ok = isSorted(a, n) && sameComposition(src, a, n);
        for (int i = 0; i < n && ok; i++) if (a[i] != expected[i]) ok = false;
        totalChecks++;
        cout << pad(names[k], 16) << " тест " << padL(to_string(no), 2) << " [" << what << "]: ";
        if (ok) cout << "успішно\n";
        else { failedChecks++; cout << "помилка, фактичний масив: "; printArr(a, n); cout << "\n"; }
    }
}

int posChecks = 0, posFailed = 0;
void checkPos(const int a[], int hi, int key, int expected, const char *what) {
    long long c = 0;
    int r = findPos(a, hi, key, c);
    posChecks++;
    cout << "Позиція " << padL(to_string(posChecks), 2) << " [" << what << "] ключ " << key
         << ": очікувано " << expected << ", отримано " << r << " (порівнянь " << c << "): ";
    if (r == expected) cout << "успішно\n";
    else { posFailed++; cout << "помилка\n"; }
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
        if (type == 0 || type == 2) a[i] = i + 1;          // зростання / майже впорядкований
        else if (type == 1) a[i] = n - i;                  // спадання
        else a[i] = (i * 37 + 11) % (n + 1);               // змішаний
    }
    if (type == 2) {                                       // кілька сусідніх пар міняємо місцями
        int step = n / 5 > 3 ? n / 5 : 3;
        for (int i = 1; i + 1 < n; i += step) { int t = a[i]; a[i] = a[i + 1]; a[i + 1] = t; }
    }
}

int main() {
    // ===== Автоматизовані перевірки сортування =====
    cout << "=== Автоматизовані перевірки сортування ===\n";
    {
        int e0[1] = {0};
        int t2[] = {5};
        int t3[] = {1, 2, 3, 4, 5};
        int t4[] = {5, 4, 3, 2, 1};
        int t5[] = {4, 2, 5, 1, 3}, r5[] = {1, 2, 3, 4, 5};
        int t6[] = {3, 1, 3, 2, 1}, r6[] = {1, 1, 2, 3, 3};
        int t7[] = {0, -4, 7, -1, 3}, r7[] = {-4, -1, 0, 3, 7};
        int t8[] = {8, 8, 8, 8};
        int t9[] = {2, 1}, r9[] = {1, 2};
        int t10[] = {1, 2, 3, 5, 4, 6}, r10[] = {1, 2, 3, 4, 5, 6};
        int t11[] = {5, 1, 4, 2, 8, 0, -1}, r11[] = {-1, 0, 1, 2, 4, 5, 8};
        int t12[] = {3, 3, 2, 2, 1, 1}, r12[] = {1, 1, 2, 2, 3, 3};
        runTest(1, e0, 0, e0, "порожній масив");
        runTest(2, t2, 1, t2, "один елемент");
        runTest(3, t3, 5, t3, "впорядкований набір");
        runTest(4, t4, 5, r5, "зворотний порядок");
        runTest(5, t5, 5, r5, "змішаний порядок");
        runTest(6, t6, 5, r6, "повторення");
        runTest(7, t7, 5, r7, "від'ємні числа");
        runTest(8, t8, 4, t8, "однакові значення");
        runTest(9, t9, 2, r9, "два переставлені елементи");
        runTest(10, t10, 6, r10, "майже впорядкований набір");
        runTest(11, t11, 7, r11, "власний: від'ємні, нуль");
        runTest(12, t12, 6, r12, "власний: пари дублікатів");
    }
    cout << "Перевірок сортування: " << totalChecks << ", невдалих: " << failedChecks << "\n";

    cout << "\n=== Перевірки позиції бінарної вставки ===\n";
    {
        int a[] = {2, 4, 4, 4, 8, 10};
        checkPos(a, 6, 1, 0, "на початку");
        checkPos(a, 6, 4, 4, "після рівних");
        checkPos(a, 6, 5, 4, "між елементами");
        checkPos(a, 6, 9, 5, "у середині/кінці");
        checkPos(a, 6, 10, 6, "рівний останньому");
        checkPos(a, 6, 11, 6, "у кінці");
        checkPos(a, 0, 7, 0, "порожня ділянка");
    }
    cout << "Перевірок позиції: " << posChecks << ", невдалих: " << posFailed << "\n";
    if (failedChecks || posFailed) return 1;

    // ===== Експеримент =====
    const char *typeName[4] = {"Зростання", "Спадання", "Майже впоряд.", "Змішаний"};
    int sizes[3] = {10, 100, 1000};
    static Stats st[3][4][4];     // [розмір][набір][алгоритм: 0 вставки,1 бінарні,2 бульб.,3 шейкерне]
    static bool okAll[3][4];
    for (int z = 0; z < 3; z++)
        for (int t = 0; t < 4; t++) {
            int n = sizes[z];
            static int src[MAXN];
            makeSet(t, n, src);
            bool ok = true;
            ok &= runAlg(insertionSort, src, n, st[z][t][0]);
            ok &= runAlg(binaryInsertionSort, src, n, st[z][t][1]);
            ok &= runAlg(bubbleSort, src, n, st[z][t][2]);
            ok &= runAlg(cocktailWrap, src, n, st[z][t][3]);
            okAll[z][t] = ok;
        }

    cout << "\n=== Експеримент, пара 1: вставки та бінарні вставки ===\n";
    cout << padL("n", 5) << "  " << pad("Набір", 14) << padL("Вст.порівн", 11) << padL("Вст.перем", 10)
         << padL("Бін.порівн", 11) << padL("Бін.перем", 10) << "  Перевірка\n";
    for (int z = 0; z < 3; z++)
        for (int t = 0; t < 4; t++)
            cout << padL(to_string(sizes[z]), 5) << "  " << pad(typeName[t], 14)
                 << padL(to_string(st[z][t][0].cmp), 11) << padL(to_string(st[z][t][0].mov), 10)
                 << padL(to_string(st[z][t][1].cmp), 11) << padL(to_string(st[z][t][1].mov), 10)
                 << "  " << (okAll[z][t] ? "успішно" : "помилка") << "\n";

    cout << "\n=== Експеримент, пара 2: бульбашкове та шейкерне ===\n";
    cout << padL("n", 5) << "  " << pad("Набір", 14) << padL("Бул.порівн", 11) << padL("Бул.перем", 10)
         << padL("Бул.прох", 9) << padL("Шей.порівн", 11) << padL("Шей.перем", 10) << padL("Шей.прох", 9)
         << "  Перевірка\n";
    for (int z = 0; z < 3; z++)
        for (int t = 0; t < 4; t++)
            cout << padL(to_string(sizes[z]), 5) << "  " << pad(typeName[t], 14)
                 << padL(to_string(st[z][t][2].cmp), 11) << padL(to_string(st[z][t][2].mov), 10)
                 << padL(to_string(st[z][t][2].passes), 9)
                 << padL(to_string(st[z][t][3].cmp), 11) << padL(to_string(st[z][t][3].mov), 10)
                 << padL(to_string(st[z][t][3].passes), 9)
                 << "  " << (okAll[z][t] ? "успішно" : "помилка") << "\n";

    // ===== Індивідуальний варіант 19 =====
    cout << "\n=== Варіант 19 ===\n";
    int var[MAXN]; int n = 12;
    if (!readArray(var, n)) {
        int def[12] = {29, 8, 38, 17, 24, 12, 33, 20, 10, 35, 15, 26};
        n = 12; copyArr(def, var, 12);
        cout << "\n(введення недоступне, використано масив варіанта 19)";
    }
    cout << "\nПочаткова послідовність: "; printArr(var, n); cout << "\n";
    const char *names[4] = {"Вставки", "Бінарні вставки", "Бульбашкове", "Шейкерне"};
    SortFn fns[4] = {insertionSort, binaryInsertionSort, bubbleSort, cocktailWrap};
    cout << pad("Алгоритм", 17) << padL("Порівн.", 8) << padL("Переміщ.", 10) << padL("Проходи", 9)
         << "  Перевірка  Результат\n";
    for (int k = 0; k < 4; k++) {
        int a[MAXN]; Stats s;
        copyArr(var, a, n);                        // відновлення початкових даних
        fns[k](a, n, s);
        bool ok = isSorted(a, n) && sameComposition(var, a, n);
        cout << pad(names[k], 17) << padL(to_string(s.cmp), 8) << padL(to_string(s.mov), 10)
             << padL(k >= 2 ? to_string(s.passes) : string("-"), 9)
             << "  " << pad(ok ? "успішно" : "помилка", 9) << "  ";
        printArr(a, n); cout << "\n";
    }
    cout << "\nХід шейкерного сортування (межі left, right):\n";
    {
        int a[MAXN]; Stats s;
        copyArr(var, a, n);
        cout << "  початок                          "; printArr(a, n); cout << "\n";
        cocktailSort(a, n, s, true);
    }
    return 0;
}
