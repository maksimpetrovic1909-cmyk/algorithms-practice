#include <iostream>
#include <string>
using namespace std;

// Варіант 19: вузол "Тварина" з полями: назва, клас, вага.

struct Node {
    char name[32] = {0};
    char cls[24] = {0};
    double weight = 0.0;
    Node *next = nullptr;          // початкове значення зв'язку - nullptr, а не випадкова адреса
};

struct List {
    Node *head = nullptr;          // початок
    Node *tail = nullptr;          // кінець
    int count = 0;                 // кількість вузлів
};

// ---------- Лічильники ----------
long long nodesCreated = 0, nodesFreed = 0;     // у межах поточного сценарію
long long totalCreated = 0, totalFreed = 0;     // за всю програму
long long ptrAssign = 0;                        // присвоєння покажчиків при створенні
long long walkSteps = 0;                        // кроки переходу по next
long long active() { return nodesCreated - nodesFreed; }
void resetCounters() { nodesCreated = nodesFreed = 0; ptrAssign = walkSteps = 0; }
void flushCounters() { totalCreated += nodesCreated; totalFreed += nodesFreed; }

// ---------- Допоміжні функції ----------
void strCopy(char *dst, int cap, const char *src) {
    int i = 0;
    while (src[i] && i < cap - 1) { dst[i] = src[i]; i++; }
    dst[i] = 0;
}
bool strEq(const char *a, const char *b) {
    int i = 0;
    while (a[i] && b[i]) { if (a[i] != b[i]) return false; i++; }
    return a[i] == b[i];
}
int visLen(const string &s) { int l = 0; for (unsigned char c : s) if ((c & 0xC0) != 0x80) l++; return l; }
string pad(const string &s, int w) { int l = visLen(s); return s + string(w > l ? w - l : 0, ' '); }
string padL(const string &s, int w) { int l = visLen(s); return string(w > l ? w - l : 0, ' ') + s; }
string fmt1(double v) {
    long long x = (long long)(v * 10 + (v >= 0 ? 0.5 : -0.5));
    return (x < 0 && x / 10 == 0 ? "-" : "") + to_string(x / 10) + "." + to_string((x < 0 ? -x : x) % 10);
}
string addr(const void *p) {
    if (!p) return "nullptr";
    unsigned long long v = (unsigned long long)(size_t)p;
    string s = "0x";
    bool started = false;
    for (int sh = 60; sh >= 0; sh -= 4) {
        int d = (v >> sh) & 0xF;
        if (d || started || sh == 0) { s += "0123456789abcdef"[d]; started = true; }
    }
    return s;
}

// ---------- Вузол ----------
Node *makeNode(const char *n, const char *c, double w) {
    Node *p = new Node();           // () - усі поля, зокрема next, ініціалізовано
    nodesCreated++;
    strCopy(p->name, 32, n);
    strCopy(p->cls, 24, c);
    p->weight = w;
    return p;
}

// ---------- Створення у кінець (порядок введення) ----------
void appendBack(List &l, const char *n, const char *c, double w) {
    Node *p = makeNode(n, c, w);
    if (l.head == nullptr) {        // перший елемент окремо
        l.head = p; l.tail = p; ptrAssign += 2;
    } else {
        l.tail->next = p;           // приєднуємо після поточного останнього
        l.tail = p;
        ptrAssign += 2;
    }
    l.count++;
}
// ---------- Створення на початок (зворотний порядок) ----------
void pushFront(List &l, const char *n, const char *c, double w) {
    Node *p = makeNode(n, c, w);
    p->next = l.head;
    l.head = p;
    ptrAssign += 2;
    if (l.tail == nullptr) { l.tail = p; ptrAssign++; }   // перший вузол є й кінцем
    l.count++;
}
// Для порівняння в експерименті: приєднання в кінець без покажчика tail (пошук кінця обходом)
void appendNoTail(List &l, const char *n, const char *c, double w) {
    Node *p = makeNode(n, c, w);
    if (l.head == nullptr) { l.head = p; ptrAssign++; }
    else {
        Node *q = l.head;
        while (q->next != nullptr) { q = q->next; walkSteps++; }
        q->next = p; ptrAssign++;
    }
    l.count++;
}

// ---------- Перегляд (окремий робочий покажчик) ----------
int viewList(const List &l, bool print) {
    int viewed = 0;
    if (print) cout << pad("№", 4) << pad("Назва", 12) << pad("Клас", 10) << padL("Вага, кг", 10) << "\n";
    for (const Node *p = l.head; p != nullptr; p = p->next) {    // l.head не змінюється
        viewed++;
        if (print) cout << pad(to_string(viewed), 4) << pad(p->name, 12) << pad(p->cls, 10)
                        << padL(fmt1(p->weight), 10) << "\n";
    }
    if (print) cout << (viewed == 0 ? "(список порожній)\n" : "") << "Переглянуто вузлів: " << viewed << "\n";
    return viewed;
}
// Текстове подання (для порівняння двох переглядів)
string listToString(const List &l) {
    string s;
    for (const Node *p = l.head; p != nullptr; p = p->next)
        s += string(p->name) + "/" + p->cls + "/" + fmt1(p->weight) + ";";
    return s;
}

// ---------- Діагностичний перегляд адрес і зв'язків ----------
void viewAddresses(const List &l) {
    cout << pad("№", 4) << pad("Адреса вузла", 20) << pad("Адреса наступного", 24) << "\n";
    int i = 0;
    for (const Node *p = l.head; p != nullptr; p = p->next) {
        i++;
        cout << pad(to_string(i), 4) << pad(addr(p), 20)
             << (p->next ? addr(p->next) : string("відсутній (кінець списку)")) << "\n";
    }
    if (i == 0) cout << "(список порожній)\n";
}
// Автоматична перевірка діагностики: адреса "наступного" кожного вузла = адреса наступного в обході
bool addressesConsistent(const List &l) {
    for (const Node *p = l.head; p != nullptr; p = p->next) {
        const Node *nx = p->next;
        if (nx == nullptr) return p == l.tail;
        if (nx == p) return false;
    }
    return l.head == nullptr;
}

// ---------- Автоматична перевірка структури ----------
enum Status { OK = 0, BAD_EMPTY, BAD_COUNT, BAD_TAIL, BROKEN, CYCLE };
const char *statusName(Status s) {
    switch (s) {
        case OK: return "структура правильна";
        case BAD_EMPTY: return "неузгоджений порожній стан";
        case BAD_COUNT: return "неправильна кількість";
        case BAD_TAIL: return "хибний кінець";
        case BROKEN: return "розрив ланцюжка";
        default: return "можливий цикл";
    }
}
Status checkStructure(const List &l) {
    if (l.count == 0) return (l.head == nullptr && l.tail == nullptr) ? OK : BAD_EMPTY;
    if (l.head == nullptr || l.tail == nullptr) return BAD_EMPTY;
    long long limit = active() + 1;               // у правильному списку не більше за кількість живих вузлів
    long long steps = 0;
    const Node *p = l.head, *last = nullptr;
    while (p != nullptr && steps < limit) { last = p; p = p->next; steps++; }
    if (p != nullptr) return CYCLE;               // обмеження кроків вичерпано
    if (steps != l.count) return last == l.tail ? BAD_COUNT : BROKEN;
    if (last != l.tail || l.tail->next != nullptr) return BAD_TAIL;
    return OK;
}

// ---------- Очищення ----------
void clearList(List &l) {
    Node *p = l.head;
    while (p != nullptr) {
        Node *nx = p->next;           // адресу наступного зберігаємо ДО звільнення поточного
        delete p;
        nodesFreed++;
        p = nx;
    }
    l.head = nullptr; l.tail = nullptr; l.count = 0;
}

// ---------- Тести ----------
int testNo = 0, testFailed = 0;

void fillValues(List &l, const double *v, int n, bool front) {
    for (int i = 0; i < n; i++) {
        string nm = "N" + to_string(i + 1);
        if (front) pushFront(l, nm.c_str(), "Тест", v[i]);
        else appendBack(l, nm.c_str(), "Тест", v[i]);
    }
}

// Перевіряє порядок значень, кількість, початок, кінець; два перегляди; структуру
bool verifyList(const List &l, const double *exp, int n) {
    if (l.count != n) return false;
    if (n == 0) return l.head == nullptr && l.tail == nullptr && checkStructure(l) == OK;
    int i = 0;
    for (const Node *p = l.head; p != nullptr; p = p->next, i++) {
        if (i >= n || p->weight != exp[i]) return false;
    }
    if (i != n) return false;
    if (l.head->weight != exp[0] || l.tail->weight != exp[n - 1] || l.tail->next != nullptr) return false;
    Node *h0 = l.head, *t0 = l.tail;
    string s1 = listToString(l);
    viewList(l, false);
    string s2 = listToString(l);
    if (s1 != s2 || l.head != h0 || l.tail != t0) return false;     // перегляд не змінює структуру
    return checkStructure(l) == OK;
}

void report(const char *what, bool ok, long long activeAfter) {
    testNo++;
    bool good = ok && activeAfter == 0;
    if (!good) testFailed++;
    cout << "Тест " << padL(to_string(testNo), 2) << " [" << what << "]: порядок "
         << (ok ? "успішно" : "помилка") << ", активних вузлів " << activeAfter << " -> "
         << (good ? "успішно" : "ПОМИЛКА") << "\n";
}

// Загальний сценарій: створити -> перевірити -> очистити -> перевірити порожній стан
void scenario(const char *what, const double *v, int n, bool front, const double *exp) {
    resetCounters();
    List l;
    fillValues(l, v, n, front);
    bool ok = verifyList(l, exp, n);
    clearList(l);
    ok = ok && l.head == nullptr && l.tail == nullptr && l.count == 0 && checkStructure(l) == OK;
    long long a = active();
    flushCounters();
    report(what, ok, a);
}

void negativeTest(const char *what, Status expected, int kind) {
    resetCounters();
    List l;
    double v[3] = {1, 2, 3};
    fillValues(l, v, 3, false);
    Node *second = l.head->next, *third = l.tail;
    Node *savedTail = l.tail;
    int savedCount = l.count;
    switch (kind) {
        case 0: l.count = 4; break;                 // хибна кількість
        case 1: l.tail = l.head; break;             // хибний кінець
        case 2: second->next = nullptr; break;      // розрив: третій вузол відокремлено
        case 3: third->next = l.head; break;        // цикл
    }
    Status s = checkStructure(l);
    // відновлення перед очищенням (інакше пам'ять буде втрачено)
    l.count = savedCount; l.tail = savedTail; second->next = third; third->next = nullptr;
    bool ok = s == expected && checkStructure(l) == OK;
    clearList(l);
    long long a = active();
    flushCounters();
    testNo++;
    bool good = ok && a == 0;
    if (!good) testFailed++;
    cout << "Тест " << padL(to_string(testNo), 2) << " [" << what << "]: виявлено \"" << statusName(s)
         << "\", активних вузлів " << a << " -> " << (good ? "успішно" : "ПОМИЛКА") << "\n";
}

bool readLine(string &s) {
    return (bool)getline(cin, s);
}

int main() {
    // ===== Вузол і порожній список =====
    cout << "=== Вузол і порожній список ===\n";
    resetCounters();
    {
        Node *p = new Node(); nodesCreated++;
        cout << "Новий вузол: next = " << addr(p->next) << ", weight = " << fmt1(p->weight)
             << ", name = \"" << p->name << "\"\n";
        delete p; nodesFreed++;
        List e;
        cout << "Порожній список: head = " << addr(e.head) << ", tail = " << addr(e.tail) << ", count = " << e.count
             << ", перевірка: " << statusName(checkStructure(e)) << "\n";
        viewList(e, true);
    }
    flushCounters();

    // ===== Дані варіанта (з введення) =====
    int n = 5;
    string nm[16], cl[16]; double wt[16];
    bool got = false;
    {
        cout << "\nВведіть кількість тварин (1..15): ";
        if (cin >> n && n >= 1 && n <= 15) {
            got = true;
            for (int i = 0; i < n && got; i++) { cout << "\nТварина " << i + 1 << ": "; string a, b; double w; if (cin >> a >> b >> w) { nm[i] = a; cl[i] = b; wt[i] = w; } else got = false; }
        }
        if (!got) {
            n = 5;
            const char *dn[5] = {"Лев", "Орел", "Слон", "Акула", "Пінгвін"};
            const char *dc[5] = {"Ссавці", "Птахи", "Ссавці", "Риби", "Птахи"};
            double dw[5] = {190.5, 5.2, 4800.0, 600.0, 30.0};
            for (int i = 0; i < 5; i++) { nm[i] = dn[i]; cl[i] = dc[i]; wt[i] = dw[i]; }
            cout << "\n(введення недоступне, використано стандартні дані варіанта)";
        }
    }

    // ===== Створення у кінець =====
    cout << "\n\n=== Створення у кінець (порядок введення) ===\n";
    resetCounters();
    List a;
    for (int i = 0; i < n; i++) appendBack(a, nm[i].c_str(), cl[i].c_str(), wt[i]);
    cout << "count = " << a.count << ", head -> " << a.head->name << ", tail -> " << a.tail->name
         << ", tail->next = " << addr(a.tail->next) << "\n";
    viewList(a, true);

    // ===== Створення на початок =====
    cout << "\n=== Створення на початок (зворотний порядок) ===\n";
    List b;
    for (int i = 0; i < n; i++) pushFront(b, nm[i].c_str(), cl[i].c_str(), wt[i]);
    cout << "count = " << b.count << ", head -> " << b.head->name << ", tail -> " << b.tail->name
         << ", tail->next = " << addr(b.tail->next) << "\n";
    viewList(b, true);

    // ===== Повторний перегляд =====
    cout << "\n=== Повторний перегляд ===\n";
    {
        Node *h0 = a.head, *t0 = a.tail;
        string s1 = listToString(a);
        viewList(a, false);
        string s2 = listToString(a);
        cout << "Два перегляди однакові: " << (s1 == s2 ? "так" : "ні") << "; початок і кінець не змінились: "
             << (a.head == h0 && a.tail == t0 ? "так" : "ні") << "\n";
    }

    // ===== Діагностичний перегляд =====
    cout << "\n=== Діагностичний перегляд адрес і зв'язків (список створено у кінець) ===\n";
    viewAddresses(a);
    cout << "Автоматична перевірка ланцюжка: " << (addressesConsistent(a) ? "один ланцюжок без розривів і циклів" : "ПОМИЛКА") << "\n";

    // ===== Перевірка структури =====
    cout << "\n=== Автоматична перевірка структури ===\n";
    cout << "Список у кінець: " << statusName(checkStructure(a)) << "\n";
    cout << "Список на початок: " << statusName(checkStructure(b)) << "\n";

    // ===== Очищення =====
    cout << "\n=== Очищення ===\n";
    clearList(a); clearList(b);
    cout << "Після очищення: head = " << addr(a.head) << ", tail = " << addr(a.tail) << ", count = " << a.count
         << ", перевірка: " << statusName(checkStructure(a)) << "\n";
    cout << "Створено вузлів: " << nodesCreated << ", звільнено: " << nodesFreed << ", активних: " << active() << "\n";
    for (int i = 0; i < 3; i++) appendBack(a, nm[i].c_str(), cl[i].c_str(), wt[i]);       // повторне створення
    cout << "Повторне створення після очищення: count = " << a.count << ", перевірка: " << statusName(checkStructure(a)) << "\n";
    clearList(a);
    cout << "Після повторного очищення: створено " << nodesCreated << ", звільнено " << nodesFreed << ", активних " << active() << "\n";
    flushCounters();

    // ===== Тести =====
    cout << "\n=== Автоматизовані перевірки ===\n";
    {
        double none[1] = {0};
        double s7[] = {7};
        double s123[] = {1, 2, 3}, r321[] = {3, 2, 1};
        double s555[] = {5, 5, 5};
        double sn[] = {-2, 0, 4}, rn[] = {4, 0, -2};
        double t10[10], t10r[10], t100[100];
        for (int i = 0; i < 10; i++) { t10[i] = i + 1; t10r[i] = 10 - i; }
        for (int i = 0; i < 100; i++) { t100[i] = i; }
        scenario("без елементів, у кінець", none, 0, false, none);
        scenario("7, у кінець", s7, 1, false, s7);
        scenario("7, на початок", s7, 1, true, s7);
        scenario("1,2,3 у кінець", s123, 3, false, s123);
        scenario("1,2,3 на початок", s123, 3, true, r321);
        scenario("5,5,5 у кінець", s555, 3, false, s555);
        scenario("-2,0,4 у кінець", sn, 3, false, sn);
        scenario("-2,0,4 на початок", sn, 3, true, rn);
        scenario("10 елементів у кінець", t10, 10, false, t10);
        scenario("10 елементів на початок", t10, 10, true, t10r);
        {   // 11: три елементи, два перегляди
            resetCounters();
            List l; fillValues(l, s123, 3, false);
            string v1 = listToString(l), v2 = listToString(l);
            bool ok = v1 == v2 && viewList(l, false) == 3 && viewList(l, false) == 3 && verifyList(l, s123, 3);
            clearList(l); long long act = active(); flushCounters();
            report("3 елементи, два перегляди", ok, act);
        }
        scenario("100 елементів: створити й очистити", t100, 100, false, t100);
        {   // 13: повторне створення після очищення
            resetCounters();
            List l; fillValues(l, s123, 3, false);
            clearList(l);
            fillValues(l, sn, 3, true);
            bool ok = verifyList(l, rn, 3);
            clearList(l); long long act = active(); flushCounters();
            report("власний: повторне створення після очищення", ok, act);
        }
        {   // 14: два послідовні цикли
            resetCounters();
            bool ok = true;
            for (int cycle = 0; cycle < 2; cycle++) {
                List l; fillValues(l, t10, 10, cycle == 1);
                ok = ok && verifyList(l, cycle == 0 ? t10 : t10r, 10);
                clearList(l);
                ok = ok && l.count == 0 && active() == 0;
            }
            long long act = active();
            ok = ok && nodesCreated == 20 && nodesFreed == 20;
            flushCounters();
            report("власний: два цикли створення й очищення", ok, act);
        }
        {   // 15: діагностичний режим
            resetCounters();
            List l; fillValues(l, t10, 10, false);
            bool ok = addressesConsistent(l);
            int chain = 0;
            for (const Node *p = l.head; p != nullptr; p = p->next) chain++;
            ok = ok && chain == 10;
            clearList(l); long long act = active(); flushCounters();
            report("власний: діагностичний режим", ok, act);
        }
        negativeTest("негативний: хибна кількість", BAD_COUNT, 0);
        negativeTest("негативний: хибний кінець", BAD_TAIL, 1);
        negativeTest("негативний: розрив ланцюжка", BROKEN, 2);
        negativeTest("негативний: цикл", CYCLE, 3);
    }
    cout << "Тестів: " << testNo << ", невдалих: " << testFailed << "\n";
    if (testFailed) return 1;

    // ===== Експеримент =====
    cout << "\n=== Експеримент: операції при створенні, перегляді та очищенні ===\n";
    cout << padL("Вузлів", 7) << padL("Режим", 22) << padL("Створено", 10) << padL("Присвоєнь", 11)
         << padL("Кроків пошуку кінця", 21) << padL("Кроків перегляду", 18) << padL("Звільнено", 11) << "\n";
    int sizes[3] = {10, 100, 1000};
    for (int s = 0; s < 3; s++) {
        for (int mode = 0; mode < 3; mode++) {
            resetCounters();
            List l;
            for (int i = 0; i < sizes[s]; i++) {
                if (mode == 0) appendBack(l, "Т", "К", i);
                else if (mode == 1) pushFront(l, "Т", "К", i);
                else appendNoTail(l, "Т", "К", i);
            }
            long long assign = ptrAssign, search = walkSteps;
            int viewed = viewList(l, false);
            bool okStruct = mode == 2 ? true : checkStructure(l) == OK;
            clearList(l);
            const char *mn = mode == 0 ? "у кінець (tail)" : (mode == 1 ? "на початок" : "у кінець без tail");
            cout << padL(to_string(sizes[s]), 7) << padL(mn, 22) << padL(to_string(nodesCreated), 10)
                 << padL(to_string(assign), 11) << padL(to_string(search), 21) << padL(to_string(viewed), 18)
                 << padL(to_string(nodesFreed), 11) << (okStruct && active() == 0 ? "  ok" : "  ПОМИЛКА") << "\n";
            flushCounters();
        }
    }

    cout << "\n=== Підсумок діагностики ===\n";
    cout << "Створено вузлів: " << totalCreated << ", звільнено: " << totalFreed << ", активних вузлів: "
         << totalCreated - totalFreed << "\n";
    cout << (totalCreated == totalFreed ? "Витоків пам'яті немає." : "ВИТІК ПАМ'ЯТІ!") << "\n";
    return totalCreated == totalFreed ? 0 : 2;
}
