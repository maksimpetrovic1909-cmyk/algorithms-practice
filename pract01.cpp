#include <iostream>
#include <iomanip>
using namespace std;

// ---------- Алгоритми пошуку ----------

// Прямий пошук: повертає індекс або -1, cmp = кількість порівнянь з ключем
int linearSearch(const int a[], int size, int key, long long &cmp) {
    cmp = 0;
    for (int i = 0; i < size; i++) {
        cmp++;                      // порівняння елемента з ключем
        if (a[i] == key) return i;
    }
    return -1;
}

// Ітеративний бінарний пошук (масив має бути впорядкованим за зростанням)
int binarySearch(const int a[], int size, int key, long long &cmp) {
    cmp = 0;
    int left = 0, right = size - 1;
    while (left <= right) {         // перевірка умови циклу не рахується
        int mid = left + (right - left) / 2;
        cmp++;                      // порівняння середнього елемента з ключем
        if (a[mid] == key) return mid;
        if (a[mid] < key) left = mid + 1;
        else right = mid - 1;
    }
    return -1;
}

// ---------- Автоматизовані перевірки ----------
int testNo = 0, failed = 0;

void runTest(const int a[], int size, int key, int expected, const char *what) {
    long long c1, c2;
    int r1 = linearSearch(a, size, key, c1);
    int r2 = binarySearch(a, size, key, c2);
    testNo++;
    bool ok = (r1 == expected) && (r2 == expected);
    cout << "Тест " << setw(2) << testNo << " [" << what << "]: ";
    if (ok) cout << "успішно\n";
    else {
        failed++;
        cout << "помилка (очікувано " << expected << ", прямий " << r1
             << ", бінарний " << r2 << ")\n";
    }
}

int main() {
    // ----- Дані варіанта 1 -----
    const int varN = 7;
    int varArr[varN]    = {28, 16, 41, 11, 23, 46, 14};
    int varSorted[varN] = {11, 14, 16, 23, 28, 41, 46};
    int varKeys[3]      = {16, 46, 32};

    // ----- Крок 3: тести -----
    cout << "=== Автоматизовані перевірки ===\n";
    int one[1] = {8};
    int s7[7] = {2, 5, 8, 11, 14, 17, 20};
    int empty[1] = {0};  // фактичний розмір 0
    runTest(one, 1, 8, 0, "єдиний елемент знайдено");
    runTest(one, 1, 3, -1, "єдиного елемента немає");
    runTest(s7, 7, 2, 0, "перший елемент");
    runTest(s7, 7, 11, 3, "середній елемент");
    runTest(s7, 7, 20, 6, "останній елемент");
    runTest(s7, 7, 9, -1, "ключ між елементами");
    runTest(s7, 7, -4, -1, "ключ менший за мінімум");
    runTest(s7, 7, 100, -1, "ключ більший за максимум");
    runTest(empty, 0, 5, -1, "порожній масив");
    // власні тести
    int s2[2] = {1, 9};
    int s8[8] = {-7, -2, 0, 3, 10, 15, 22, 40};
    runTest(s2, 2, 9, 1, "власний: два елементи, останній");
    runTest(s2, 2, 1, 0, "власний: два елементи, перший");
    runTest(s8, 8, -2, 1, "власний: парний розмір, від'ємне");
    runTest(s8, 8, 40, 7, "власний: парний розмір, останній");
    cout << "Невдалих тестів: " << failed << "\n";
    if (failed) return 1;

    // ----- Крок 4: порівняльний експеримент -----
    cout << "\n=== Експеримент (впорядкований масив 0..n-1) ===\n";
    static int big[100000];
    for (int i = 0; i < 100000; i++) big[i] = i;
    int sizes[4] = {10, 100, 1000, 100000};
    const char *names[3] = {"A", "B", "C"};
    cout << "       n  сценарій    прямий  бінарний\n";
    for (int s = 0; s < 4; s++) {
        int n = sizes[s];
        int keys[3] = {0, n - 1, n};
        for (int k = 0; k < 3; k++) {
            long long c1, c2;
            linearSearch(big, n, keys[k], c1);
            binarySearch(big, n, keys[k], c2);
            cout << setw(8) << n << setw(10) << names[k]
                 << setw(10) << c1 << setw(10) << c2 << "\n";
        }
    }

    // ----- Крок 5: середні значення -----
    cout << "\n=== Середнє за 1000 пошуків, ключ = (i*37) mod n, i = 1..1000 ===\n";
    cout << "       n      прямий    бінарний\n";
    cout << fixed << setprecision(3);
    for (int s = 0; s < 4; s++) {
        int n = sizes[s];
        long long sum1 = 0, sum2 = 0;
        for (int i = 1; i <= 1000; i++) {
            int key = (int)(((long long)i * 37) % n);
            long long c1, c2;
            linearSearch(big, n, key, c1);
            binarySearch(big, n, key, c2);
            sum1 += c1; sum2 += c2;
        }
        cout << setw(8) << n << setw(12) << sum1 / 1000.0
             << setw(12) << sum2 / 1000.0 << "\n";
    }

    // ----- Крок 6: невпорядкований масив -----
    cout << "\n=== Невпорядкований масив 12 3 19 5 8 1 (бінарний пошук) ===\n";
    int uns[6] = {12, 3, 19, 5, 8, 1};
    int srt[6] = {1, 3, 5, 8, 12, 19};
    for (int pass = 0; pass < 2; pass++) {
        const int *arr = pass == 0 ? uns : srt;
        cout << (pass == 0 ? "Невпорядкований:\n" : "Впорядкований:\n");
        int errors = 0;
        for (int i = 0; i < 6; i++) {
            long long c;
            int r = binarySearch(arr, 6, arr[i], c);
            bool ok = (r == i);
            if (!ok) errors++;
            cout << "  ключ " << setw(2) << arr[i] << ": очікувано " << i
                 << ", отримано " << setw(2) << r << (ok ? "  ok" : "  ПОМИЛКА") << "\n";
        }
        cout << "  помилок: " << errors << "\n";
    }

    // ----- Індивідуальний варіант 1 -----
    cout << "\n=== Варіант 19 ===\n";
    cout << " ключ  прямий(інд.)  порівн.  бінарний(інд.)  порівн.\n";
    for (int k = 0; k < 3; k++) {
        long long c1, c2;
        int r1 = linearSearch(varArr, varN, varKeys[k], c1);
        int r2 = binarySearch(varSorted, varN, varKeys[k], c2);
        cout << setw(6) << varKeys[k] << setw(14) << r1 << setw(10) << c1
             << setw(14) << r2 << setw(10) << c2 << "\n";
    }
    return 0;
}
