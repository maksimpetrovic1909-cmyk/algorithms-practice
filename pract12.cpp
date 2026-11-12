#include <iostream>
#include <string>
using namespace std;

// Варіант 19: вузол "Тварина" (назва, клас, вага). Числовий ключ - поле вага.
// Структура вузла, створення, перегляд і очищення - з практичної №11.

struct Node {
    char name[32] = {0};
    char cls[24] = {0};
    double weight = 0.0;
    Node *next = nullptr;
};
struct List {
    Node *head = nullptr;
    Node *tail = nullptr;
    int count = 0;
};

long long created = 0, freed = 0;                 // лічильники поточного сценарію
long long totalCreated = 0, totalFreed = 0;       // за всю програму
long long active() { return created - freed; }
void resetCounters() { created = freed = 0; }
void flushCounters() { totalCreated += created; totalFreed += freed; }

// ---------- Допоміжні ----------
void strCopy(char *dst, int cap, const char *src) {
    int i = 0;
    while (src[i] && i < cap - 1) { dst[i] = src[i]; i++; }
    dst[i] = 0;
}
int visLen(const string &s) { int l = 0; for (unsigned char c : s) if ((c & 0xC0) != 0x80) l++; return l; }
string pad(const string &s, int w) { int l = visLen(s); return s + string(w > l ? w - l : 0, ' '); }
string padL(const string &s, int w) { int l = visLen(s); return string(w > l ? w - l : 0, ' ') + s; }
string num(double v) { return to_string((long long)v); }

Node *makeNode(double key) {
    Node *p = new Node();
    created++;
    strCopy(p->name, 32, "Тварина");
    strCopy(p->cls, 24, "Клас");
    p->weight = key;
    return p;
}
void freeNode(Node *&p) {
    delete p;
    p = nullptr;
    freed++;
}

// Короткий опис останньої зміни (журнал змін)
bool headChanged = false, tailChanged = false;
string where;
void resetInfo() { headChanged = tailChanged = false; where.clear(); }

// ---------- Подання списку ----------
string listStr(const List &l) {
    string s = "[";
    for (const Node *p = l.head; p != nullptr; p = p->next) s += (p == l.head ? "" : ", ") + num(p->weight);
    return s + "]";
}
unsigned long long hashList(const List &l) {        // контрольна сума послідовності (беззнакова: переповнення визначене)
    unsigned long long h = (unsigned long long)l.count;
    for (const Node *p = l.head; p != nullptr; p = p->next) h = h * 1000003ULL + (unsigned long long)(long long)(p->weight * 10) + 7ULL;
    return h;
}

// ---------- Пошук ----------
// Перший збіг: повертає вузол або nullptr; pos - позиція, viewed - переглянуті вузли
Node *searchFirst(const List &l, double key, int &pos, int &viewed) {
    pos = -1; viewed = 0;
    int i = 0;
    for (Node *p = l.head; p != nullptr; p = p->next, i++) {
        viewed++;
        if (p->weight == key) { pos = i; return p; }
    }
    return nullptr;
}
// Усі збіги: позиції в res, повертає кількість
int searchAll(const List &l, double key, int res[], int &viewed) {
    int cnt = 0, i = 0; viewed = 0;
    for (const Node *p = l.head; p != nullptr; p = p->next, i++) {
        viewed++;
        if (p->weight == key) res[cnt++] = i;
    }
    return cnt;
}

// ---------- Вставлення ----------
bool insertAt(List &l, int pos, double key) {
    resetInfo();
    if (pos < 0 || pos > l.count) return false;               // допустимі позиції 0..count
    Node *n = makeNode(key);
    if (pos == 0) {                                           // початок (або порожній список)
        n->next = l.head;
        l.head = n; headChanged = true;
        if (l.tail == nullptr) { l.tail = n; tailChanged = true; where = "у порожній список (head і tail)"; }
        else where = "на початок (head змінено)";
    } else if (pos == l.count) {                              // кінець
        l.tail->next = n;
        l.tail = n; tailChanged = true;
        where = "в кінець (tail змінено)";
    } else {                                                  // середина: знаходимо попередній вузол
        Node *prev = l.head;
        for (int i = 0; i < pos - 1; i++) prev = prev->next;
        n->next = prev->next;                                 // спочатку зв'язок нового вузла,
        prev->next = n;                                       // потім зв'язок попереднього
        where = "у середину (після позиції " + to_string(pos - 1) + ")";
    }
    l.count++;
    return true;
}
bool insertAfterKey(List &l, double key, double nk) {
    resetInfo();
    int pos, v;
    Node *f = searchFirst(l, key, pos, v);
    if (!f) return false;                                     // ключа немає: вузол не створюється
    Node *n = makeNode(nk);
    n->next = f->next;
    f->next = n;
    if (f == l.tail) { l.tail = n; tailChanged = true; where = "після останнього (tail змінено)"; }
    else where = "після першого збігу (позиція " + to_string(pos) + ")";
    l.count++;
    return true;
}
bool insertBeforeKey(List &l, double key, double nk) {
    resetInfo();
    Node *prev = nullptr, *cur = l.head;
    while (cur && cur->weight != key) { prev = cur; cur = cur->next; }
    if (!cur) return false;
    Node *n = makeNode(nk);
    n->next = cur;
    if (prev == nullptr) { l.head = n; headChanged = true; where = "перед першим вузлом (head змінено)"; }
    else { prev->next = n; where = "перед першим збігом"; }
    l.count++;
    return true;
}

// ---------- Видалення ----------
// Відокремлює і звільняє cur (prev - попередній вузол або nullptr)
void removeNode(List &l, Node *prev, Node *cur) {
    Node *nx = cur->next;                                      // адреса наступного зберігається до звільнення
    if (prev == nullptr) {
        l.head = nx; headChanged = true;
        if (cur == l.tail) { l.tail = nullptr; tailChanged = true; }   // єдиний вузол
    } else {
        prev->next = nx;
        if (cur == l.tail) { l.tail = prev; tailChanged = true; }
    }
    freeNode(cur);
    l.count--;
}
bool removeAt(List &l, int pos) {
    resetInfo();
    if (pos < 0 || pos >= l.count) return false;
    Node *prev = nullptr, *cur = l.head;
    for (int i = 0; i < pos; i++) { prev = cur; cur = cur->next; }
    bool wasSingle = l.count == 1;
    bool last = cur == l.tail;
    removeNode(l, prev, cur);
    where = wasSingle ? "єдиний вузол (head і tail скинуто)" : (pos == 0 ? "перший (head змінено)" : (last ? "останній (tail змінено)" : "із середини"));
    return true;
}
bool removeFirstKey(List &l, double key) {
    resetInfo();
    Node *prev = nullptr, *cur = l.head;
    while (cur && cur->weight != key) { prev = cur; cur = cur->next; }
    if (!cur) return false;
    removeNode(l, prev, cur);
    where = "перший збіг";
    return true;
}
int removeAllKey(List &l, double key) {
    resetInfo();
    int removed = 0;
    Node *prev = nullptr, *cur = l.head;
    while (cur != nullptr) {
        Node *nx = cur->next;                                  // зберігаємо наступний до можливого звільнення
        if (cur->weight == key) { removeNode(l, prev, cur); removed++; }
        else prev = cur;                                       // prev оновлюється лише для вузла, що лишається
        cur = nx;
    }
    where = "усі збіги";
    return removed;
}
void clearList(List &l) {
    Node *p = l.head;
    while (p != nullptr) { Node *nx = p->next; freeNode(p); p = nx; }
    l.head = l.tail = nullptr; l.count = 0;
}

// ---------- Перевірка інваріантів ----------
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
Status checkInvariants(const List &l) {
    if (l.count == 0) return (l.head == nullptr && l.tail == nullptr) ? OK : BAD_EMPTY;
    if (l.head == nullptr || l.tail == nullptr) return BAD_EMPTY;
    long long limit = active() + 1;                  // обмеження кроків: не більше за число живих вузлів
    long long steps = 0;
    const Node *p = l.head, *last = nullptr;
    while (p != nullptr && steps < limit) { last = p; p = p->next; steps++; }
    if (p != nullptr) return CYCLE;
    if (steps != l.count) return last == l.tail ? BAD_COUNT : BROKEN;
    if (last != l.tail || l.tail->next != nullptr) return BAD_TAIL;
    return OK;
}
bool memoryOk(const List &l) { return active() == l.count; }   // кожен живий вузол належить списку

// ---------- Незалежна модель (звичайний масив) ----------
const int MAXM = 4000;
struct Model {
    double a[MAXM];
    int n = 0;
    bool insertAt(int pos, double k) {
        if (pos < 0 || pos > n || n >= MAXM) return false;
        for (int i = n; i > pos; i--) a[i] = a[i - 1];
        a[pos] = k; n++; return true;
    }
    bool removeAt(int pos) {
        if (pos < 0 || pos >= n) return false;
        for (int i = pos; i + 1 < n; i++) a[i] = a[i + 1];
        n--; return true;
    }
    int find(double k) const { for (int i = 0; i < n; i++) if (a[i] == k) return i; return -1; }
    bool insertAfter(double k, double nk) { int i = find(k); return i >= 0 && insertAt(i + 1, nk); }
    bool insertBefore(double k, double nk) { int i = find(k); return i >= 0 && insertAt(i, nk); }
    bool removeFirst(double k) { int i = find(k); return i >= 0 && removeAt(i); }
    int removeAll(double k) { int c = 0; for (int i = 0; i < n;) { if (a[i] == k) { removeAt(i); c++; } else i++; } return c; }
};
bool sameAsModel(const List &l, const Model &m) {
    if (l.count != m.n) return false;
    int i = 0;
    for (const Node *p = l.head; p != nullptr; p = p->next, i++) if (i >= m.n || p->weight != m.a[i]) return false;
    return i == m.n;
}

// ---------- Виконання операцій ----------
enum Op { SEARCH_FIRST, SEARCH_ALL, INSERT_AT, REMOVE_AT, REMOVE_FIRST, REMOVE_ALL, INSERT_AFTER, INSERT_BEFORE };
struct Cmd { Op op; double key; int pos; double key2; };

string cmdName(const Cmd &c) {
    switch (c.op) {
        case SEARCH_FIRST: return "пошук " + num(c.key);
        case SEARCH_ALL: return "пошук усіх " + num(c.key);
        case INSERT_AT: return "вставити " + num(c.key) + " на " + to_string(c.pos);
        case REMOVE_AT: return "видалити позицію " + to_string(c.pos);
        case REMOVE_FIRST: return "видалити перше " + num(c.key);
        case REMOVE_ALL: return "видалити всі " + num(c.key);
        case INSERT_AFTER: return "вставити " + num(c.key2) + " після " + num(c.key);
        default: return "вставити " + num(c.key2) + " перед " + num(c.key);
    }
}

// Виконує команду над списком і моделлю; повертає успіх операції, result - текст
bool apply(List &l, Model &m, const Cmd &c, string &result, bool &modelAgree) {
    bool ok = false, mok = false;
    switch (c.op) {
        case SEARCH_FIRST: {
            int pos, v; Node *f = searchFirst(l, c.key, pos, v);
            ok = f != nullptr; mok = m.find(c.key) >= 0;
            result = ok ? "знайдено, позиція " + to_string(pos) + ", переглянуто " + to_string(v)
                        : "не знайдено, переглянуто " + to_string(v);
            break;
        }
        case SEARCH_ALL: {
            int res[MAXM], v; int cnt = searchAll(l, c.key, res, v);
            ok = cnt > 0; mok = m.find(c.key) >= 0;
            result = to_string(cnt) + " збігів";
            if (cnt) { result += ", позиції "; for (int i = 0; i < cnt; i++) result += (i ? "," : "") + to_string(res[i]); }
            result += ", переглянуто " + to_string(v);
            break;
        }
        case INSERT_AT:
            ok = insertAt(l, c.pos, c.key); mok = m.insertAt(c.pos, c.key);
            result = ok ? "вставлено " + where : "відмова: недопустима позиція";
            break;
        case REMOVE_AT:
            ok = removeAt(l, c.pos); mok = m.removeAt(c.pos);
            result = ok ? "видалено " + where : "відмова: недопустима позиція";
            break;
        case REMOVE_FIRST:
            ok = removeFirstKey(l, c.key); mok = m.removeFirst(c.key);
            result = ok ? "видалено " + where : "відмова: ключа немає";
            break;
        case REMOVE_ALL: {
            int r = removeAllKey(l, c.key); int mr = m.removeAll(c.key);
            ok = r > 0; mok = mr > 0;
            if (r != mr) mok = !ok;                           // розбіжність кількості видалених
            result = ok ? "видалено " + to_string(r) + " вузлів (" + where + ")" : "відмова: ключа немає";
            break;
        }
        case INSERT_AFTER:
            ok = insertAfterKey(l, c.key, c.key2); mok = m.insertAfter(c.key, c.key2);
            result = ok ? "вставлено " + where : "відмова: ключа немає";
            break;
        default:
            ok = insertBeforeKey(l, c.key, c.key2); mok = m.insertBefore(c.key, c.key2);
            result = ok ? "вставлено " + where : "відмова: ключа немає";
            break;
    }
    modelAgree = (ok == mok) && sameAsModel(l, m);
    return ok;
}

// ---------- Сценарії тестування ----------
int testNo = 0, testFailed = 0;

void buildList(List &l, Model &m, const double *v, int n) {
    for (int i = 0; i < n; i++) { insertAt(l, l.count, v[i]); m.insertAt(m.n, v[i]); }
}
bool matches(const List &l, const double *exp, int n) {
    if (l.count != n) return false;
    int i = 0;
    for (const Node *p = l.head; p != nullptr; p = p->next, i++) if (i >= n || p->weight != exp[i]) return false;
    return i == n && (n == 0 ? (l.head == nullptr && l.tail == nullptr) : (l.tail != nullptr && l.tail->weight == exp[n - 1] && l.tail->next == nullptr && l.head->weight == exp[0]));
}
string arrStr(const double *a, int n) {
    string s = "[";
    for (int i = 0; i < n; i++) s += (i ? ", " : "") + num(a[i]);
    return s + "]";
}
void finishTest(const string &label, const List &l, bool dataOk, const string &actual, const string &expected) {
    testNo++;
    bool inv = checkInvariants(l) == OK, mem = memoryOk(l);
    bool good = dataOk && inv && mem;
    if (!good) testFailed++;
    cout << "Сценарій " << padL(to_string(testNo), 2) << " [" << label << "]: " << actual
         << (dataOk ? " = " : " != ") << expected << ", структура " << (inv ? "ok" : "ПОМИЛКА")
         << ", пам'ять " << (mem ? "ok" : "ПОМИЛКА") << " -> " << (good ? "успішно" : "ПОМИЛКА") << "\n";
}

// Один сценарій із таблиці
void scenario(const char *label, const double *init, int n, Op op, double key, int pos, double key2,
              const double *exp, int m, bool expectFail) {
    resetCounters();
    List l; Model mod;
    buildList(l, mod, init, n);
    unsigned long long before = hashList(l);
    Cmd c{op, key, pos, key2};
    string res; bool agree;
    bool ok = apply(l, mod, c, res, agree);
    bool dataOk = matches(l, exp, m) && agree && (ok == !expectFail);
    if (expectFail) dataOk = dataOk && hashList(l) == before;      // невдала операція не змінює список
    string actual = listStr(l);
    finishTest(label, l, dataOk, actual, arrStr(exp, m));
    clearList(l);
    if (active() != 0) { testFailed++; cout << "  ПОМИЛКА: після очищення активних вузлів " << active() << "\n"; }
    flushCounters();
}

void ownTests() {
    // 16: послідовні однакові ключі
    {
        resetCounters();
        List l; Model m; double v[] = {2, 2, 2, 2, 2};
        buildList(l, m, v, 5);
        string r; bool ag;
        apply(l, m, Cmd{REMOVE_FIRST, 2, 0, 0}, r, ag);
        bool ok = l.count == 4 && ag;
        apply(l, m, Cmd{REMOVE_ALL, 2, 0, 0}, r, ag);
        ok = ok && l.count == 0 && l.head == nullptr && l.tail == nullptr && ag;
        double none[1] = {0};
        finishTest("власний: п'ять однакових ключів, перше і всі", l, ok && matches(l, none, 0), listStr(l), "[]");
        clearList(l); if (active() != 0) testFailed++; flushCounters();
    }
    // 17: повторне заповнення після повного видалення
    {
        resetCounters();
        List l; Model m; double v[] = {1, 2, 3};
        buildList(l, m, v, 3);
        string r; bool ag, ok = true;
        for (int i = 0; i < 3; i++) { ok = ok && apply(l, m, Cmd{REMOVE_AT, 0, 0, 0}, r, ag) && ag && checkInvariants(l) == OK; }
        ok = ok && l.head == nullptr && l.tail == nullptr && active() == 0;
        ok = ok && apply(l, m, Cmd{INSERT_AT, 7, 0, 0}, r, ag) && apply(l, m, Cmd{INSERT_AT, 8, 1, 0}, r, ag) && ag;
        double exp[] = {7, 8};
        finishTest("власний: заповнення після повного видалення", l, ok && matches(l, exp, 2), listStr(l), "[7, 8]");
        clearList(l); if (active() != 0) testFailed++; flushCounters();
    }
    // 18: серія з десяти змішаних операцій
    {
        resetCounters();
        List l; Model m;
        Cmd cs[10] = {{INSERT_AT, 5, 0, 0}, {INSERT_AT, 3, 0, 0}, {INSERT_AT, 9, 2, 0}, {INSERT_AT, 7, 1, 0},
                      {REMOVE_AT, 0, 0, 0}, {INSERT_AT, 1, 3, 0}, {REMOVE_FIRST, 9, 0, 0}, {REMOVE_ALL, 7, 0, 0},
                      {INSERT_AFTER, 1, 0, 4}, {INSERT_BEFORE, 5, 0, 0}};
        bool ok = true; string r; bool ag;
        for (int i = 0; i < 10; i++) {
            apply(l, m, cs[i], r, ag);
            ok = ok && ag && checkInvariants(l) == OK && memoryOk(l);
        }
        double exp[] = {0, 5, 1, 4};
        finishTest("власний: десять змішаних операцій", l, ok && matches(l, exp, 4), listStr(l), "[0, 5, 1, 4]");
        clearList(l); if (active() != 0) testFailed++; flushCounters();
    }
    // 19: послідовні збіги на початку, в середині та наприкінці
    {
        resetCounters();
        List l; Model m; double v[] = {2, 2, 1, 2, 2, 3, 2, 2};
        buildList(l, m, v, 8);
        int removed = removeAllKey(l, 2); int mr = m.removeAll(2);
        double exp[] = {1, 3};
        bool ok = removed == 5 + 1 && mr == removed && sameAsModel(l, m);
        finishTest("власний: збіги на початку, в середині й в кінці", l, ok && matches(l, exp, 2), listStr(l), "[1, 3]");
        clearList(l); if (active() != 0) testFailed++; flushCounters();
    }
    // 20: операції з відсутнім ключем не змінюють список і не створюють вузлів
    {
        resetCounters();
        List l; Model m; double v[] = {1, 2, 3};
        buildList(l, m, v, 3);
        long long cr = created; unsigned long long h = hashList(l);
        bool ok = !insertAfterKey(l, 99, 5) && !insertBeforeKey(l, 99, 5) && !removeFirstKey(l, 99) && removeAllKey(l, 99) == 0;
        ok = ok && created == cr && hashList(l) == h && checkInvariants(l) == OK;
        finishTest("власний: відсутній ключ, чотири операції", l, ok && matches(l, v, 3), listStr(l), "[1, 2, 3]");
        clearList(l); if (active() != 0) testFailed++; flushCounters();
    }
}

// ---------- Негативні перевірки інваріантів ----------
void negative(const char *what, Status expected, int kind) {
    resetCounters();
    List l; Model m; double v[] = {1, 2, 3};
    buildList(l, m, v, 3);
    Node *second = l.head->next, *third = l.tail, *savedTail = l.tail;
    int savedCount = l.count;
    switch (kind) {
        case 0: l.count = 4; break;
        case 1: l.tail = l.head; break;
        case 2: second->next = nullptr; break;
        case 3: third->next = l.head; break;
    }
    Status s = checkInvariants(l);
    l.count = savedCount; l.tail = savedTail; second->next = third; third->next = nullptr;   // відновлення
    bool ok = s == expected && checkInvariants(l) == OK;
    clearList(l);
    bool good = ok && active() == 0;
    cout << "Порушення [" << what << "]: виявлено \"" << statusName(s) << "\", після відновлення активних вузлів "
         << active() << " -> " << (good ? "успішно" : "ПОМИЛКА") << "\n";
    if (!good) testFailed++;
    flushCounters();
}

int main() {
    // ===== Перевірки пошуку =====
    cout << "=== Перевірки пошуку ===\n";
    int searchFailed = 0;
    {
        resetCounters();
        List l; Model m; double v[] = {4, 7, 9};
        buildList(l, m, v, 3);
        int pos, viewed;
        unsigned long long h = hashList(l);
        Node *f = searchFirst(l, 4, pos, viewed);
        bool ok1 = f && pos == 0 && viewed == 1;
        cout << "Пошук першого елемента (4) у [4, 7, 9]: позиція " << pos << ", переглянуто " << viewed << " -> " << (ok1 ? "успішно" : "ПОМИЛКА") << "\n";
        f = searchFirst(l, 5, pos, viewed);
        bool ok2 = !f && viewed == 3;
        cout << "Пошук відсутнього (5) у [4, 7, 9]: " << (f ? "знайдено" : "не знайдено") << ", переглянуто " << viewed << " (весь список) -> " << (ok2 ? "успішно" : "ПОМИЛКА") << "\n";
        clearList(l);
        List l2; Model m2; double v2[] = {2, 5, 2, 2};
        buildList(l2, m2, v2, 4);
        int res[16]; int cnt = searchAll(l2, 2, res, viewed);
        bool ok3 = cnt == 3 && res[0] == 0 && res[1] == 2 && res[2] == 3 && viewed == 4 && listStr(l2) == "[2, 5, 2, 2]";
        cout << "Пошук усіх 2 у [2, 5, 2, 2]: " << cnt << " збіги, позиції " << res[0] << "," << res[1] << "," << res[2]
             << ", переглянуто " << viewed << ", список не змінено -> " << (ok3 ? "успішно" : "ПОМИЛКА") << "\n";
        int none = searchAll(l2, 9, res, viewed);
        bool ok4 = none == 0 && viewed == 4;
        cout << "Невдалий пошук усіх (9): " << none << " збігів, переглянуто " << viewed << " -> " << (ok4 ? "успішно" : "ПОМИЛКА") << "\n";
        clearList(l2);
        searchFailed = !ok1 + !ok2 + !ok3 + !ok4;
        (void)h;
        flushCounters();
    }

    // ===== Сценарії =====
    cout << "\n=== Автоматизовані сценарії ===\n";
    {
        double none[1] = {0};
        double a5[] = {5}, a13[] = {1, 3}, a12[] = {1, 2}, a123[] = {1, 2, 3}, a1223[] = {1, 2, 2, 3}, a2212[] = {2, 2, 1, 2};
        double e013[] = {1, 2, 3}, e012[] = {0, 1, 2}, e23[] = {2, 3}, e13[] = {1, 3}, e12[] = {1, 2}, e1[] = {1};
        double e1239[] = {1, 2, 3, 9}, e0123[] = {0, 1, 2, 3};
        scenario("порожньо, пошук 5", none, 0, SEARCH_FIRST, 5, 0, 0, none, 0, true);
        scenario("порожньо, вставити 5 на 0", none, 0, INSERT_AT, 5, 0, 0, a5, 1, false);
        scenario("[5], видалити позицію 0", a5, 1, REMOVE_AT, 0, 0, 0, none, 0, false);
        scenario("[1, 3], вставити 2 на 1", a13, 2, INSERT_AT, 2, 1, 0, e013, 3, false);
        scenario("[1, 2], вставити 0 на 0", a12, 2, INSERT_AT, 0, 0, 0, e012, 3, false);
        scenario("[1, 2], вставити 3 на 2", a12, 2, INSERT_AT, 3, 2, 0, e013, 3, false);
        scenario("[1, 2, 3], видалити позицію 0", a123, 3, REMOVE_AT, 0, 0, 0, e23, 2, false);
        scenario("[1, 2, 3], видалити позицію 1", a123, 3, REMOVE_AT, 0, 1, 0, e13, 2, false);
        scenario("[1, 2, 3], видалити позицію 2", a123, 3, REMOVE_AT, 0, 2, 0, e12, 2, false);
        scenario("[1, 2, 2, 3], видалити перше 2", a1223, 4, REMOVE_FIRST, 2, 0, 0, e013, 3, false);
        scenario("[2, 2, 1, 2], видалити всі 2", a2212, 4, REMOVE_ALL, 2, 0, 0, e1, 1, false);
        scenario("[1, 2, 3], вставити на позицію 5", a123, 3, INSERT_AT, 8, 5, 0, a123, 3, true);
        scenario("[1, 2, 3], видалити позицію -1", a123, 3, REMOVE_AT, 0, -1, 0, a123, 3, true);
        scenario("[1, 2, 3], вставити 9 після 3", a123, 3, INSERT_AFTER, 3, 0, 9, e1239, 4, false);
        scenario("[1, 2, 3], вставити 0 перед 1", a123, 3, INSERT_BEFORE, 1, 0, 0, e0123, 4, false);
        ownTests();
    }
    cout << "Сценаріїв: " << testNo << ", невдалих: " << testFailed + searchFailed << "\n";

    cout << "\n=== Виявлення порушень інваріантів ===\n";
    negative("count збільшено на 1", BAD_COUNT, 0);
    negative("tail переставлено на head", BAD_TAIL, 1);
    negative("зв'язок другого вузла обнулено", BROKEN, 2);
    negative("останній вузол вказує на head", CYCLE, 3);
    if (testFailed || searchFailed) return 1;

    // ===== Варіант 19: сценарій команд =====
    cout << "\n=== Варіант 19: сценарій команд ===\n";
    resetCounters();
    List l; Model mod;
    double init[] = {6, 12, 3, 12, 9};
    buildList(l, mod, init, 5);
    cout << "Початковий список " << listStr(l) << ", кількість " << l.count << "\n";
    Cmd cmds[26] = {
        {SEARCH_FIRST, 4, 0, 0},   {SEARCH_FIRST, 12, 0, 0}, {SEARCH_ALL, 12, 0, 0},    {INSERT_AT, 1, 2, 0},
        {SEARCH_FIRST, 1, 0, 0},   {REMOVE_ALL, 12, 0, 0},   {INSERT_AT, 0, 0, 0},      {INSERT_AT, 5, 5, 0},
        {INSERT_AT, 7, 9, 0},      {INSERT_BEFORE, 3, 0, 2}, {INSERT_AFTER, 5, 0, 8},   {REMOVE_AT, 0, 0, 0},
        {REMOVE_AT, 0, 6, 0},      {REMOVE_AT, 0, -1, 0},    {REMOVE_FIRST, 9, 0, 0},   {REMOVE_FIRST, 4, 0, 0},
        {INSERT_AT, 6, 2, 0},      {SEARCH_ALL, 6, 0, 0},    {REMOVE_ALL, 6, 0, 0},     {REMOVE_AT, 0, 0, 0},
        {REMOVE_AT, 0, 1, 0},      {REMOVE_AT, 0, 1, 0},     {REMOVE_AT, 0, 0, 0},      {INSERT_AT, 4, 0, 0},
        {SEARCH_FIRST, 4, 0, 0},   {INSERT_AT, 3, 1, 0}};
    int bad = 0;
    for (int i = 0; i < 26; i++) {
        string res; bool agree;
        unsigned long long hb = hashList(l);
        bool ok = apply(l, mod, cmds[i], res, agree);
        bool unchanged = hashList(l) == hb;
        bool read = cmds[i].op == SEARCH_FIRST || cmds[i].op == SEARCH_ALL;
        bool dataOk = agree && ((!ok || read) ? unchanged : true);
        bool inv = checkInvariants(l) == OK, mem = memoryOk(l);
        if (!dataOk || !inv || !mem) bad++;
        cout << "Команда " << padL(to_string(i + 1), 2) << ": " << cmdName(cmds[i]) << " -> " << res << "\n";
        cout << "   список " << listStr(l) << ", кількість " << l.count << "; дані " << (dataOk ? "ok" : "ПОМИЛКА")
             << ", структура " << (inv ? "ok" : "ПОМИЛКА") << ", пам'ять " << (mem ? "ok" : "ПОМИЛКА") << "\n";
    }
    clearList(l);
    cout << "Після очищення: head і tail порожні, count = " << l.count << ", активних вузлів " << active()
         << ", порушень за сценарієм: " << bad << "\n";
    flushCounters();
    if (bad || active() != 0) return 1;

    // ===== Довга серія змішаних операцій =====
    cout << "\n=== Довга серія змішаних операцій (10 000) ===\n";
    {
        resetCounters();
        List ls; Model ms;
        unsigned long long x = 2024;
        auto rnd = [&](int mod) { x = (x * 6364136223846793005ULL + 1442695040888963407ULL); return (int)((x >> 33) % mod); };
        long long cnt[8] = {0}, okc[8] = {0}, viol = 0, diff = 0, changedOnFail = 0;
        int maxLen = 0;
        for (int i = 0; i < 10000; i++) {
            int kind = rnd(8);
            if (ls.count > 150 && kind <= 2) kind = 3 + rnd(3);                 // обмежуємо довжину
            Cmd c{(Op)kind, (double)rnd(20), rnd(ls.count + 3) - 1, (double)rnd(20)};
            unsigned long long hb = hashList(ls);
            string res; bool agree;
            bool ok = apply(ls, ms, c, res, agree);
            cnt[kind]++; if (ok) okc[kind]++;
            if (!agree) diff++;
            bool read = kind == SEARCH_FIRST || kind == SEARCH_ALL;
            if ((!ok || read) && hashList(ls) != hb) changedOnFail++;
            if (checkInvariants(ls) != OK || !memoryOk(ls)) viol++;
            if (ls.count > maxLen) maxLen = ls.count;
        }
        const char *nm[8] = {"пошук першого", "пошук усіх", "вставлення за позицією", "видалення за позицією",
                             "видалення першого за ключем", "видалення всіх за ключем", "вставлення після ключа", "вставлення перед ключем"};
        cout << pad("Операція", 30) << padL("Виконано", 10) << padL("Успішних", 10) << padL("Відмов", 8) << "\n";
        for (int k = 0; k < 8; k++)
            cout << pad(nm[k], 30) << padL(to_string(cnt[k]), 10) << padL(to_string(okc[k]), 10) << padL(to_string(cnt[k] - okc[k]), 8) << "\n";
        cout << "Максимальна довжина списку: " << maxLen << ", кінцева довжина: " << ls.count << "\n";
        cout << "Порушень інваріантів: " << viol << ", розбіжностей із моделлю: " << diff
             << ", невдалих операцій, що змінили список: " << changedOnFail << "\n";
        clearList(ls);
        cout << "Після очищення: створено " << created << ", звільнено " << freed << ", активних " << active() << "\n";
        if (viol || diff || changedOnFail || active() != 0) return 1;
        flushCounters();
    }

    cout << "\n=== Підсумок діагностики ===\n";
    cout << "Створено вузлів: " << totalCreated << ", звільнено: " << totalFreed << ", активних вузлів: "
         << totalCreated - totalFreed << "\n";
    cout << (totalCreated == totalFreed ? "Витоків пам'яті немає." : "ВИТІК ПАМ'ЯТІ!") << "\n";
    return totalCreated == totalFreed ? 0 : 2;
}
