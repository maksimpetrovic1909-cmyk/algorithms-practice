#include <iostream>
#include <string>
using namespace std;

// Варіант 19: структура "Тварина" (назва, клас, вага).
// Пошук за другим полем (клас), числовий підсумок за третім (вага), додаткова операція - середня вага.
const int MAXSIZE = 10000;     // явно визначений максимальний розмір структури

struct Animal {
    char name[32];
    char cls[24];
    double weight;
};

// ---------- Діагностичні лічильники ----------
struct Diag {
    long long allocs = 0, frees = 0;      // виділення та звільнення блоків
    long long moved = 0;                  // перенесені елементи під час зміни розміру
    long long reallocs = 0;               // замінені блоки при збільшенні (нова місткість)
    long long shrinks = 0;                // замінені блоки при скороченні
    long long active() const { return allocs - frees; }
};
Diag g;                                    // лічильники поточного сценарію
Diag total;                                // накопичені лічильники всієї програми

void resetDiag() { g = Diag(); }
void flushDiag() {                         // додає лічильники сценарію до загальних
    total.allocs += g.allocs; total.frees += g.frees; total.moved += g.moved;
    total.reallocs += g.reallocs; total.shrinks += g.shrinks;
}

// ---------- Допоміжні рядкові функції (без бібліотечних) ----------
bool strEq(const char *a, const char *b) {
    int i = 0;
    while (a[i] && b[i]) { if (a[i] != b[i]) return false; i++; }
    return a[i] == b[i];
}
void strCopy(char *dst, int cap, const char *src) {
    int i = 0;
    while (src[i] && i < cap - 1) { dst[i] = src[i]; i++; }
    dst[i] = 0;
}
Animal makeAnimal(const char *n, const char *c, double w) {
    Animal a{};
    strCopy(a.name, 32, n);
    strCopy(a.cls, 24, c);
    a.weight = w;
    return a;
}
bool sameAnimal(const Animal &a, const Animal &b) {
    return strEq(a.name, b.name) && strEq(a.cls, b.cls) && a.weight == b.weight;
}

int visLen(const string &s) { int l = 0; for (unsigned char c : s) if ((c & 0xC0) != 0x80) l++; return l; }
string pad(const string &s, int w) { int l = visLen(s); return s + string(w > l ? w - l : 0, ' '); }
string padL(const string &s, int w) { int l = visLen(s); return string(w > l ? w - l : 0, ' ') + s; }
string fmt1(double v) {
    long long x = (long long)(v * 10 + (v >= 0 ? 0.5 : -0.5));
    return to_string(x / 10) + "." + to_string((x < 0 ? -x : x) % 10);
}

// ---------- Керування блоками пам'яті ----------
// Кожне виділення має рівно одне звільнення; після звільнення покажчик стає nullptr.
Animal *allocBlock(int cap) {
    if (cap <= 0 || cap > MAXSIZE * 2) return nullptr;     // перевірка допустимості розміру
    g.allocs++;
    return new Animal[cap]();
}
void freeBlock(Animal *&p) {
    if (p == nullptr) return;                              // повторне звільнення неможливе
    delete[] p;
    p = nullptr;
    g.frees++;
}

bool validateSize(int n) { return n >= 1 && n <= MAXSIZE; }

// ---------- Динамічний масив структур ----------
struct DynArray {
    Animal *data = nullptr;
    int size = 0;
    int capacity = 0;
    int policy = 1;            // 0: збільшення на 1, 1: подвоєння місткості
};

// Створення масиву з точною місткістю n (порожній без виділення при n == 0)
bool createArray(DynArray &a, int n, int policy = 1) {
    a = DynArray();
    a.policy = policy;
    if (n == 0) return true;
    if (!validateSize(n)) return false;                    // нуль чи від'ємне - без виділення
    a.data = allocBlock(n);
    if (!a.data) return false;
    a.capacity = n;
    return true;
}
void destroyArray(DynArray &a) {
    freeBlock(a.data);
    a.size = 0; a.capacity = 0;
}

// Додавання в кінець: нова місткість -> новий блок -> перенесення -> звільнення старого -> оновлення
bool addElement(DynArray &a, const Animal &x) {
    if (a.size >= MAXSIZE) return false;
    if (a.size == a.capacity) {
        int newCap = a.policy == 0 ? a.size + 1 : (a.capacity == 0 ? 1 : a.capacity * 2);
        Animal *nb = allocBlock(newCap);
        if (!nb) return false;
        for (int i = 0; i < a.size; i++) { nb[i] = a.data[i]; g.moved++; }
        freeBlock(a.data);
        a.data = nb;
        a.capacity = newCap;
        g.reallocs++;
    }
    a.data[a.size++] = x;
    return true;
}

// Видалення за індексом. exact = true: новий блок рівно на 1 елемент менший;
// exact = false: зсув у тому ж блоці та автоматичне скорочення, коли size <= capacity/4
bool removeAt(DynArray &a, int idx, bool exact) {
    if (idx < 0 || idx >= a.size) return false;            // відмова без зміни стану
    if (a.size == 1) {                                     // єдиний елемент: порожня структура без пам'яті
        freeBlock(a.data);
        a.size = 0; a.capacity = 0;
        return true;
    }
    if (exact) {
        Animal *nb = allocBlock(a.size - 1);
        if (!nb) return false;
        int k = 0;
        for (int i = 0; i < a.size; i++) if (i != idx) { nb[k++] = a.data[i]; g.moved++; }
        freeBlock(a.data);
        a.data = nb;
        a.size--;
        a.capacity = a.size;
        return true;
    }
    for (int i = idx; i + 1 < a.size; i++) { a.data[i] = a.data[i + 1]; g.moved++; }
    a.size--;
    if (a.capacity >= 2 && a.size <= a.capacity / 4) {
        int newCap = a.capacity / 2;
        Animal *nb = allocBlock(newCap);
        if (!nb) return true;                              // скорочення необов'язкове
        for (int i = 0; i < a.size; i++) { nb[i] = a.data[i]; g.moved++; }
        freeBlock(a.data);
        a.data = nb;
        a.capacity = newCap;
        g.shrinks++;
    }
    return true;
}

// Пошук за класом: індекс першого збігу або -1; count - кількість збігів
int findByClass(const DynArray &a, const char *cls, int &count) {
    int first = -1; count = 0;
    for (int i = 0; i < a.size; i++)
        if (strEq(a.data[i].cls, cls)) { if (first < 0) first = i; count++; }
    return first;
}
double totalWeight(const DynArray &a) {
    double s = 0;
    for (int i = 0; i < a.size; i++) s += a.data[i].weight;
    return s;
}
bool averageWeight(const DynArray &a, double &avg) {      // додаткова операція варіанта
    if (a.size == 0) return false;
    avg = totalWeight(a) / a.size;
    return true;
}
void printTable(const DynArray &a) {
    cout << pad("№", 4) << pad("Назва", 12) << pad("Клас", 10) << padL("Вага, кг", 10) << "\n";
    for (int i = 0; i < a.size; i++)
        cout << pad(to_string(i + 1), 4) << pad(a.data[i].name, 12) << pad(a.data[i].cls, 10)
             << padL(fmt1(a.data[i].weight), 10) << "\n";
    if (a.size == 0) cout << "(порожньо)\n";
}

// ---------- Тести ----------
int testNo = 0, testFailed = 0;

bool sameContent(const DynArray &a, const Animal *exp, int n) {
    if (a.size != n) return false;
    for (int i = 0; i < n; i++) if (!sameAnimal(a.data[i], exp[i])) return false;
    return true;
}

typedef bool (*Scenario)(bool exact);

// Тест виконується для обох режимів видалення; після нього обов'язковий нульовий баланс
void runTest(const char *what, Scenario sc) {
    bool res[2];
    long long activeBeforeCleanup[2];
    for (int m = 0; m < 2; m++) {
        resetDiag();
        res[m] = sc(m == 0);                  // сценарій сам звільняє пам'ять наприкінці
        activeBeforeCleanup[m] = g.active();
        flushDiag();
    }
    testNo++;
    bool ok = res[0] && res[1] && activeBeforeCleanup[0] == 0 && activeBeforeCleanup[1] == 0;
    if (!ok) testFailed++;
    cout << "Тест " << padL(to_string(testNo), 2) << " [" << what << "]: блок-1 "
         << (res[0] ? "успішно" : "помилка") << ", зі зсувом " << (res[1] ? "успішно" : "помилка")
         << ", баланс " << activeBeforeCleanup[0] << "/" << activeBeforeCleanup[1] << " -> "
         << (ok ? "успішно" : "ПОМИЛКА") << "\n";
}

Animal AN[8];
void initSamples() {
    AN[0] = makeAnimal("Лев", "Ссавці", 190.0);
    AN[1] = makeAnimal("Орел", "Птахи", 5.0);
    AN[2] = makeAnimal("Акула", "Риби", 600.0);
    AN[3] = makeAnimal("Слон", "Ссавці", 4800.0);
    AN[4] = makeAnimal("Пінгвін", "Птахи", 30.0);
    AN[5] = makeAnimal("Жаба", "Земноводні", 0.3);
    AN[6] = makeAnimal("Кіт", "Ссавці", 4.5);
    AN[7] = makeAnimal("Сова", "Птахи", 1.2);
}

bool fill(DynArray &a, int n, int cap, int policy = 1) {
    if (!createArray(a, cap, policy)) return false;
    for (int i = 0; i < n; i++) if (!addElement(a, AN[i])) return false;
    return true;
}

bool t1(bool) {                               // порожня структура: перегляд
    DynArray a; createArray(a, 0);
    bool ok = a.data == nullptr && a.size == 0 && g.allocs == 0;
    destroyArray(a);
    return ok && g.active() == 0;
}
bool t2(bool ex) {                            // порожня структура: видалення
    DynArray a; createArray(a, 0);
    bool rejected = !removeAt(a, 0, ex);
    bool ok = rejected && a.size == 0 && a.data == nullptr && g.allocs == 0;
    destroyArray(a);
    return ok && g.active() == 0;
}
bool t3(bool) {                               // один елемент: пошук наявного
    DynArray a; fill(a, 1, 1);
    int cnt; int idx = findByClass(a, "Ссавці", cnt);
    bool ok = idx == 0 && cnt == 1;
    destroyArray(a);
    return ok && g.active() == 0;
}
bool t4(bool ex) {                            // один елемент: видалення
    DynArray a; fill(a, 1, 1);
    bool r = removeAt(a, 0, ex);
    bool ok = r && a.size == 0 && a.capacity == 0 && a.data == nullptr && g.active() == 0;
    destroyArray(a);
    return ok;
}
bool t5(bool) {                               // три елементи: додавання
    DynArray a; fill(a, 3, 3);
    Animal exp[4] = {AN[0], AN[1], AN[2], AN[3]};
    bool r = addElement(a, AN[3]);
    bool ok = r && sameContent(a, exp, 4) && a.capacity >= 4 && g.reallocs == 1 && g.moved == 3;
    destroyArray(a);
    return ok && g.active() == 0;
}
bool t6(bool ex) {                            // видалення першого
    DynArray a; fill(a, 3, 3);
    Animal exp[2] = {AN[1], AN[2]};
    bool ok = removeAt(a, 0, ex) && sameContent(a, exp, 2);
    destroyArray(a);
    return ok && g.active() == 0;
}
bool t7(bool ex) {                            // видалення середнього
    DynArray a; fill(a, 3, 3);
    Animal exp[2] = {AN[0], AN[2]};
    bool ok = removeAt(a, 1, ex) && sameContent(a, exp, 2);
    destroyArray(a);
    return ok && g.active() == 0;
}
bool t8(bool ex) {                            // видалення останнього
    DynArray a; fill(a, 3, 3);
    Animal exp[2] = {AN[0], AN[1]};
    bool ok = removeAt(a, 2, ex) && sameContent(a, exp, 2);
    destroyArray(a);
    return ok && g.active() == 0;
}
bool t9(bool ex) {                            // індекс -1
    DynArray a; fill(a, 3, 3);
    Animal exp[3] = {AN[0], AN[1], AN[2]};
    long long al = g.allocs;
    bool ok = !removeAt(a, -1, ex) && sameContent(a, exp, 3) && a.capacity == 3 && g.allocs == al;
    destroyArray(a);
    return ok && g.active() == 0;
}
bool t10(bool ex) {                           // індекс, рівний розміру
    DynArray a; fill(a, 3, 3);
    Animal exp[3] = {AN[0], AN[1], AN[2]};
    long long al = g.allocs;
    bool ok = !removeAt(a, 3, ex) && sameContent(a, exp, 3) && a.capacity == 3 && g.allocs == al;
    destroyArray(a);
    return ok && g.active() == 0;
}
bool t11(bool) {                              // заповнена місткість: додавання
    DynArray a; fill(a, 4, 4);
    bool ok = a.size == 4 && a.capacity == 4;
    ok = ok && addElement(a, AN[4]) && a.size == 5 && a.capacity == 8 && sameAnimal(a.data[4], AN[4]);
    destroyArray(a);
    return ok && g.active() == 0;
}
bool t12(bool ex) {                           // 100 елементів: послідовне очищення
    DynArray a; createArray(a, 0);
    bool ok = true;
    for (int i = 0; i < 100; i++) ok = ok && addElement(a, AN[i % 8]);
    ok = ok && a.size == 100;
    while (a.size > 0) ok = ok && removeAt(a, a.size - 1, ex);
    ok = ok && a.data == nullptr && a.capacity == 0 && g.active() == 0;
    destroyArray(a);
    return ok;
}
bool t13(bool ex) {                           // власний: повторне заповнення після повного очищення
    DynArray a; createArray(a, 0);
    bool ok = true;
    for (int i = 0; i < 6; i++) ok = ok && addElement(a, AN[i]);
    while (a.size > 0) ok = ok && removeAt(a, 0, ex);
    ok = ok && a.data == nullptr && g.active() == 0;
    for (int i = 0; i < 10; i++) ok = ok && addElement(a, AN[i % 8]);
    ok = ok && a.size == 10;
    for (int i = 0; i < 10 && ok; i++) ok = sameAnimal(a.data[i], AN[i % 8]);
    destroyArray(a);
    return ok && g.active() == 0;
}
bool t14(bool ex) {                           // власний: 50 чергувань додавання і видалення
    DynArray a; createArray(a, 0);
    bool ok = true;
    for (int i = 0; i < 50; i++) {
        Animal x = makeAnimal("Т", "Тест", i);
        ok = ok && addElement(a, x);
        if (i % 2 == 1) ok = ok && removeAt(a, 0, ex);       // видаляємо найстаріший
    }
    ok = ok && a.size == 25;
    for (int i = 0; i < 25 && ok; i++) ok = a.data[i].weight == 25 + i;
    destroyArray(a);
    return ok && g.active() == 0;
}
bool t15(bool) {                              // власний: невдалий пошук
    DynArray a; fill(a, 5, 5);
    double before = totalWeight(a);
    int cnt; int idx = findByClass(a, "Плазуни", cnt);
    bool ok = idx == -1 && cnt == 0 && totalWeight(a) == before && a.size == 5;
    destroyArray(a);
    return ok && g.active() == 0;
}

// ---------- Таблиця станів ----------
void stateRow(const char *op, const DynArray &a) {
    cout << pad(op, 28) << padL(to_string(a.size), 7) << padL(to_string(a.capacity), 10)
         << padL(to_string(g.allocs), 10) << padL(to_string(g.frees), 11) << padL(to_string(g.moved), 11)
         << padL(to_string(g.active()), 9) << "\n";
}

// ---------- Порівняння стратегій ----------
struct StratResult { long long reallocs, moved; };
StratResult addMany(int n, int policy) {
    resetDiag();
    DynArray a; createArray(a, 0, policy);
    for (int i = 0; i < n; i++) addElement(a, AN[i % 8]);
    StratResult r{g.reallocs + (policy == 0 ? 0 : 0), g.moved};
    // перше виділення з порожнього стану також є створенням нового блоку
    r.reallocs = g.allocs;                    // кількість створених блоків під час додавань
    destroyArray(a);
    flushDiag();
    return r;
}

int main() {
    initSamples();

    // ===== Завдання 1: динамічний об'єкт =====
    cout << "=== Динамічний об'єкт ===\n";
    resetDiag();
    {
        char n[32], c[24]; double w;
        Animal *p = new Animal();               // одне динамічне виділення
        g.allocs++;
        cout << "Введіть назву, клас і вагу тварини: ";
        if (cin >> n >> c >> w) { *p = makeAnimal(n, c, w); }
        else { *p = AN[6]; cout << "\n(введення недоступне, використано стандартні дані)"; }
        cout << "\nСтворено: " << p->name << ", " << p->cls << ", " << fmt1(p->weight) << " кг\n";
        p->weight += 0.5;                       // зміна одного поля
        cout << "Після зміни ваги: " << p->name << ", " << p->cls << ", " << fmt1(p->weight) << " кг\n";
        delete p; p = nullptr; g.frees++;       // звільнення і порожній стан
        cout << "Покажчик після звільнення: " << (p == nullptr ? "nullptr" : "НЕ порожній")
             << ", активних блоків: " << g.active() << "\n";
    }
    flushDiag();

    // ===== Завдання 2: динамічний масив =====
    cout << "\n=== Динамічний масив структур ===\n";
    resetDiag();
    {
        int tryN[4] = {0, -3, MAXSIZE + 1, 5};
        for (int i = 0; i < 4; i++) {
            DynArray t; long long before = g.allocs;
            bool ok = createArray(t, tryN[i]);
            cout << "Розмір " << tryN[i] << ": " << (ok && tryN[i] > 0 ? "прийнято" : "відхилено")
                 << ", виділень пам'яті: " << g.allocs - before << "\n";
            destroyArray(t);
        }
        int n = 5;
        cout << "Введіть кількість елементів (1.." << MAXSIZE << "): ";
        if (!(cin >> n) || !validateSize(n)) { n = 5; cout << "\n(використано 5 елементів)"; }
        DynArray a;
        createArray(a, n);
        for (int i = 0; i < n; i++) {
            char nm[32], cl[24]; double w;
            if (cin >> nm >> cl >> w) addElement(a, makeAnimal(nm, cl, w));
            else addElement(a, AN[i % 8]);
        }
        cout << "\nСтворено елементів: " << a.size << " (підтверджений розмір " << n << ")\n";
        printTable(a);

        // ===== Завдання 3: обробка =====
        cout << "\n=== Обробка даних ===\n";
        int cnt; int idx = findByClass(a, "Птахи", cnt);
        cout << "Пошук за класом \"Птахи\": перший збіг " << idx << ", усього збігів " << cnt << "\n";
        double sum = totalWeight(a), avg = 0;
        averageWeight(a, avg);
        cout << "Сумарна вага: " << fmt1(sum) << " кг; середня вага: " << fmt1(avg) << " кг\n";
        for (int i = 0; i < a.size; i++) if (strEq(a.data[i].cls, "Птахи")) a.data[i].weight += 1.0;   // усі збіги
        cout << "Після збільшення ваги всіх \"Птахи\" на 1.0 кг сумарна вага: " << fmt1(totalWeight(a)) << " кг\n";
        double before = totalWeight(a);
        idx = findByClass(a, "Плазуни", cnt);
        cout << "Пошук за класом \"Плазуни\": " << (idx < 0 ? "не знайдено" : "знайдено") << ", збігів " << cnt
             << ", масив не змінено: " << (totalWeight(a) == before ? "так" : "ні") << "\n";
        destroyArray(a);
    }
    flushDiag();

    // ===== Тести =====
    cout << "\n=== Автоматизовані перевірки ===\n";
    runTest("порожня структура: перегляд", t1);
    runTest("порожня структура: видалення", t2);
    runTest("один елемент: пошук наявного", t3);
    runTest("один елемент: видалення", t4);
    runTest("три елементи: додавання", t5);
    runTest("три елементи: видалення першого", t6);
    runTest("три елементи: видалення середнього", t7);
    runTest("три елементи: видалення останнього", t8);
    runTest("три елементи: індекс -1", t9);
    runTest("три елементи: індекс = розміру", t10);
    runTest("заповнена місткість: додавання", t11);
    runTest("100 елементів: послідовне очищення", t12);
    runTest("власний: заповнення після очищення", t13);
    runTest("власний: 50 чергувань", t14);
    runTest("власний: невдалий пошук", t15);
    cout << "Тестів: " << testNo << ", невдалих: " << testFailed << "\n";
    if (testFailed) return 1;

    // ===== Таблиця станів =====
    cout << "\n=== Таблиця станів (подвоєння місткості, скорочення при size <= capacity/4) ===\n";
    resetDiag();
    {
        DynArray a; createArray(a, 0);
        cout << pad("Операція", 28) << padL("Розмір", 7) << padL("Місткість", 10) << padL("Виділено", 10)
             << padL("Звільнено", 11) << padL("Перенесено", 11) << padL("Активні", 9) << "\n";
        stateRow("початок (порожньо)", a);
        for (int i = 0; i < 6; i++) { addElement(a, AN[i]); stateRow(("додати #" + to_string(i + 1)).c_str(), a); }
        for (int i = 0; i < 6; i++) { removeAt(a, 0, false); stateRow(("видалити 1-й, лишилось " + to_string(a.size)).c_str(), a); }
        destroyArray(a);
        stateRow("destroyArray", a);
    }
    flushDiag();

    // ===== Порівняння стратегій =====
    cout << "\n=== Порівняння стратегій збільшення (створених блоків / перенесених елементів) ===\n";
    cout << padL("Додавань", 9) << padL("+1: блоків", 12) << padL("x2: блоків", 12) << padL("Зекономлено", 13)
         << padL("+1: перенес.", 14) << padL("x2: перенес.", 14) << padL("Зекономлено", 13) << "\n";
    int ns[3] = {10, 100, 1000};
    for (int i = 0; i < 3; i++) {
        StratResult s1 = addMany(ns[i], 0), s2 = addMany(ns[i], 1);
        cout << padL(to_string(ns[i]), 9) << padL(to_string(s1.reallocs), 12) << padL(to_string(s2.reallocs), 12)
             << padL(to_string(s1.reallocs - s2.reallocs), 13) << padL(to_string(s1.moved), 14)
             << padL(to_string(s2.moved), 14) << padL(to_string(s1.moved - s2.moved), 13) << "\n";
    }
    {
        resetDiag();
        DynArray a; createArray(a, 0);
        for (int i = 0; i < 1000; i++) addElement(a, AN[i % 8]);
        int maxCap = a.capacity;
        long long grow = g.reallocs, movedGrow = g.moved;
        while (a.size > 0) removeAt(a, a.size - 1, false);
        cout << "\nДодавання 1000 елементів: місткість " << maxCap << ", блоків замінено " << grow
             << ", перенесено " << movedGrow << "\n";
        cout << "Послідовне видалення 1000 елементів: скорочень " << g.shrinks << ", перенесено "
             << g.moved - movedGrow << ", місткість " << a.capacity << ", активних блоків " << g.active() << "\n";
        flushDiag();
    }

    // ===== Підсумок ресурсів =====
    cout << "\n=== Підсумок діагностики ===\n";
    cout << "Виділень: " << total.allocs << ", звільнень: " << total.frees << ", активних блоків: "
         << total.active() << ", перенесено елементів: " << total.moved << "\n";
    cout << (total.active() == 0 ? "Витоків пам'яті немає." : "ВИТІК ПАМ'ЯТІ!") << "\n";
    return total.active() == 0 ? 0 : 2;
}
