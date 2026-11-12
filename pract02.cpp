#include <iostream>
#include <string>
using namespace std;

const int MAXN = 1000;

// Лічильники: порівняння = зіставлення двох значень масиву;
// переміщення = одне присвоєння елементу масиву або допоміжній змінній.
struct Stats {
    long long cmp = 0, mov = 0, passes = 0;
};

// ---------- Алгоритми ----------

void insertionSort(int a[], int n, Stats &s) {
    s = Stats();
    for (int i = 1; i < n; i++) {
        s.passes++;
        int key = a[i]; s.mov++;                 // запис у допоміжну змінну
        int j = i - 1;
        while (j >= 0) {
            s.cmp++;                             // a[j] > key
            if (a[j] > key) {
                a[j + 1] = a[j]; s.mov++;        // зсув більшого елемента
                j--;
            } else break;
        }
        a[j + 1] = key; s.mov++;                 // вставка ключа
    }
}

void selectionSort(int a[], int n, Stats &s) {
    s = Stats();
    for (int i = 0; i + 1 < n; i++) {
        s.passes++;
        int m = i;
        for (int j = i + 1; j < n; j++) {
            s.cmp++;                             // a[j] < a[m]
            if (a[j] < a[m]) m = j;
        }
        if (m != i) {                            // обмін лише за потреби
            int t = a[i]; s.mov++;
            a[i] = a[m];  s.mov++;
            a[m] = t;     s.mov++;
        }
    }
}

void bubbleSort(int a[], int n, Stats &s) {
    s = Stats();
    for (int pass = 0; pass + 1 < n; pass++) {
        s.passes++;
        bool swapped = false;
        for (int j = 0; j + 1 < n - pass; j++) {
            s.cmp++;                             // a[j] > a[j+1]
            if (a[j] > a[j + 1]) {
                int t = a[j];     s.mov++;
                a[j] = a[j + 1];  s.mov++;
                a[j + 1] = t;     s.mov++;
                swapped = true;
            }
        }
        if (!swapped) break;                     // дострокове завершення
    }
}

typedef void (*SortFn)(int[], int, Stats &);
const int ALGS = 3;
SortFn fns[ALGS] = {insertionSort, selectionSort, bubbleSort};
const char *algName[ALGS] = {"Вставки", "Вибір", "Бульбашковий"};

// ---------- Допоміжні функції ----------

bool isSorted(const int a[], int n) {
    for (int i = 1; i < n; i++)
        if (a[i] < a[i - 1]) return false;
    return true;
}

// Перевірка збереження складу: кількість, сума, сума квадратів
bool sameComposition(const int x[], const int y[], int n) {
    long long s1 = 0, s2 = 0, q1 = 0, q2 = 0;
    for (int i = 0; i < n; i++) {
        s1 += x[i]; q1 += (long long)x[i] * x[i];
        s2 += y[i]; q2 += (long long)y[i] * y[i];
    }
    return s1 == s2 && q1 == q2;
}

void copyArr(const int src[], int dst[], int n) {
    for (int i = 0; i < n; i++) dst[i] = src[i];
}

void printArr(const int a[], int n) {
    cout << "[";
    for (int i = 0; i < n; i++) cout << (i ? ", " : "") << a[i];
    cout << "]";
}

// Вирівнювання з урахуванням UTF-8 (кирилиця)
string pad(const string &s, int width) {
    int len = 0;
    for (unsigned char c : s) if ((c & 0xC0) != 0x80) len++;
    return s + string(width > len ? width - len : 0, ' ');
}
string padL(const string &s, int width) {
    int len = 0;
    for (unsigned char c : s) if ((c & 0xC0) != 0x80) len++;
    return string(width > len ? width - len : 0, ' ') + s;
}

// ---------- Автотести ----------
int totalChecks = 0, failedChecks = 0;

void runTest(int no, const int src[], int n, const int expected[], const char *what) {
    for (int k = 0; k < ALGS; k++) {
        int a[MAXN];
        copyArr(src, a, n);
        Stats s;
        fns[k](a, n, s);
        bool ok = isSorted(a, n) && sameComposition(src, a, n);
        for (int i = 0; i < n && ok; i++) if (a[i] != expected[i]) ok = false;
        totalChecks++;
        cout << pad(algName[k], 13) << " тест " << padL(to_string(no), 2) << " ["
             << what << "]: ";
        if (ok) cout << "успішно\n";
        else {
            failedChecks++;
            cout << "помилка, фактичний масив: ";
            printArr(a, n);
            cout << "\n";
        }
    }
}

// ---------- Введення з перевіркою ----------
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

int main() {
    // ----- Автоматизовані тести -----
    cout << "=== Автоматизовані перевірки ===\n";
    {
        int e0[1] = {0};
        int t2[] = {7};
        int t3[] = {1, 2, 3, 4, 5};
        int t4[] = {5, 4, 3, 2, 1};
        int t5[] = {4, 1, 5, 2, 3};
        int r5[] = {1, 2, 3, 4, 5};
        int t6[] = {3, 1, 3, 2, 1}, r6[] = {1, 1, 2, 3, 3};
        int t7[] = {0, -5, 8, -2, 4}, r7[] = {-5, -2, 0, 4, 8};
        int t8[] = {6, 6, 6, 6};
        int t9[] = {2, 1}, r9[] = {1, 2};
        int t10[] = {9, -1, 9, 0, -1, 5}, r10[] = {-1, -1, 0, 5, 9, 9};
        int t11[] = {10, -3, 10, -3, 0, 7, 7}, r11[] = {-3, -3, 0, 7, 7, 10, 10};
        int t12[] = {2, 3, 1}, r12[] = {1, 2, 3};
        runTest(1, e0, 0, e0, "порожній масив");
        runTest(2, t2, 1, t2, "один елемент");
        runTest(3, t3, 5, t3, "уже впорядкований");
        runTest(4, t4, 5, r5, "зворотний порядок");
        runTest(5, t5, 5, r5, "змішаний порядок");
        runTest(6, t6, 5, r6, "повторювані значення");
        runTest(7, t7, 5, r7, "від'ємні числа і нуль");
        runTest(8, t8, 4, t8, "усі значення однакові");
        runTest(9, t9, 2, r9, "мінімальна перестановка");
        runTest(10, t10, 6, r10, "комбінований випадок");
        runTest(11, t11, 7, r11, "власний: дублікати і від'ємні");
        runTest(12, t12, 3, r12, "власний: циклічний зсув");
    }
    cout << "Усього перевірок: " << totalChecks << ", невдалих: " << failedChecks << "\n";
    if (failedChecks) return 1;

    // ----- Порівняльний експеримент -----
    cout << "\n=== Експеримент: 100 елементів ===\n";
    int asc[100], desc[100], mixed[100];
    for (int i = 0; i < 100; i++) {
        asc[i] = i + 1;
        desc[i] = 100 - i;
        mixed[i] = (i * 37 + 11) % 101;
    }
    const char *setName[3] = {"Зростання", "Спадання", "Змішаний"};
    int *sets[3] = {asc, desc, mixed};
    cout << pad("Алгоритм", 14) << pad("Набір", 11) << padL("Порівн.", 8)
         << padL("Переміщ.", 10) << padL("Проходи", 9) << "  Перевірка\n";
    for (int k = 0; k < ALGS; k++)
        for (int d = 0; d < 3; d++) {
            int a[100]; Stats s;
            copyArr(sets[d], a, 100);
            fns[k](a, 100, s);
            bool ok = isSorted(a, 100) && sameComposition(sets[d], a, 100);
            cout << pad(algName[k], 14) << pad(setName[d], 11)
                 << padL(to_string(s.cmp), 8) << padL(to_string(s.mov), 10)
                 << padL(to_string(s.passes), 9) << "  " << (ok ? "успішно" : "помилка") << "\n";
        }

    // ----- Індивідуальний варіант 19 -----
    cout << "\n=== Варіант 19 ===\n";
    int var[MAXN] = {31, 13, 26, 17, 9, 35, 22, 11, 38, 19};
    int n = 10;
    if (!readArray(var, n)) {                    // якщо введення недоступне
        n = 10;
        int def[10] = {31, 13, 26, 17, 9, 35, 22, 11, 38, 19};
        copyArr(def, var, 10);
        cout << "\n(введення недоступне, використано масив варіанта 19)";
    }
    cout << "\nПочаткова послідовність: "; printArr(var, n); cout << "\n";
    cout << pad("Алгоритм", 14) << padL("Порівн.", 8) << padL("Переміщ.", 10)
         << padL("Проходи", 9) << "  Перевірка  Результат\n";
    for (int k = 0; k < ALGS; k++) {
        int a[MAXN]; Stats s;
        copyArr(var, a, n);                      // однакові початкові дані
        fns[k](a, n, s);
        bool ok = isSorted(a, n) && sameComposition(var, a, n);
        cout << pad(algName[k], 14) << padL(to_string(s.cmp), 8)
             << padL(to_string(s.mov), 10) << padL(to_string(s.passes), 9)
             << "  " << pad(ok ? "успішно" : "помилка", 9) << "  ";
        printArr(a, n); cout << "\n";
    }
    return 0;
}
