#include <iostream>
#include <string>
using namespace std;

// Варіант 19: вузол "Тварина" (назва, клас, вага); числовий ключ - вага.
// Три структури: двонапрямлений список (DList), циклічний однонапрямлений (CList),
// циклічний двонапрямлений (CDList).

struct SNode { char name[32] = {0}; char cls[24] = {0}; double weight = 0; SNode *next = nullptr; };
struct DNode { char name[32] = {0}; char cls[24] = {0}; double weight = 0; DNode *prev = nullptr; DNode *next = nullptr; };

struct DList  { DNode *head = nullptr, *tail = nullptr; int count = 0; };
struct CList  { SNode *head = nullptr, *tail = nullptr; int count = 0; };
struct CDList { DNode *head = nullptr, *tail = nullptr; int count = 0; };

long long created = 0, freed = 0, totalCreated = 0, totalFreed = 0;
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
string vals(const double *a, int n) {
    string s = "[";
    for (int i = 0; i < n; i++) s += (i ? ", " : "") + num(a[i]);
    return s + "]";
}
string yn(bool b) { return b ? "так" : "ні"; }

SNode *makeS(double key) { SNode *p = new SNode(); created++; strCopy(p->name, 32, "Тварина"); strCopy(p->cls, 24, "Клас"); p->weight = key; return p; }
DNode *makeD(double key) { DNode *p = new DNode(); created++; strCopy(p->name, 32, "Тварина"); strCopy(p->cls, 24, "Клас"); p->weight = key; return p; }
void freeS(SNode *&p) { delete p; p = nullptr; freed++; }
void freeD(DNode *&p) { delete p; p = nullptr; freed++; }

// ---------- Статуси перевірки ----------
enum Status { OK = 0, BAD_EMPTY, BAD_COUNT, BAD_BOUND, BROKEN, BAD_BACK, OPEN, WRONG_LEN, CYCLE };
const char *statusName(Status s) {
    switch (s) {
        case OK: return "структура правильна";
        case BAD_EMPTY: return "неузгоджений порожній стан";
        case BAD_COUNT: return "неправильна кількість";
        case BAD_BOUND: return "хибна межа";
        case BROKEN: return "розрив ланцюжка";
        case BAD_BACK: return "неправильний зворотний зв'язок";
        case OPEN: return "незамкнений цикл";
        case WRONG_LEN: return "цикл неправильної довжини";
        default: return "можливий нескінченний цикл";
    }
}

// =================== Двонапрямлений список ===================
DNode *dNodeAt(const DList &l, int pos) { DNode *p = l.head; for (int i = 0; i < pos; i++) p = p->next; return p; }

bool dInsertAt(DList &l, int pos, double key) {
    if (pos < 0 || pos > l.count) return false;
    DNode *n = makeD(key);
    if (l.count == 0) { l.head = l.tail = n; }
    else if (pos == 0) { n->next = l.head; l.head->prev = n; l.head = n; }
    else if (pos == l.count) { n->prev = l.tail; l.tail->next = n; l.tail = n; }
    else {
        DNode *cur = dNodeAt(l, pos), *p = cur->prev;           // сусіди нового вузла
        n->prev = p; n->next = cur;
        p->next = n; cur->prev = n;                             // обидва зв'язки узгоджено
    }
    l.count++;
    return true;
}
void dUnlink(DList &l, DNode *cur) {
    DNode *p = cur->prev, *nx = cur->next;                      // обидва сусіди зберігаються до звільнення
    if (p) p->next = nx; else l.head = nx;
    if (nx) nx->prev = p; else l.tail = p;
    freeD(cur);
    l.count--;
}
bool dRemoveAt(DList &l, int pos) {
    if (pos < 0 || pos >= l.count) return false;
    dUnlink(l, dNodeAt(l, pos));
    return true;
}
bool dRemoveKey(DList &l, double key) {
    for (DNode *p = l.head; p; p = p->next) if (p->weight == key) { dUnlink(l, p); return true; }
    return false;
}
int dForward(const DList &l, double out[], int cap) {
    int n = 0;
    for (const DNode *p = l.head; p && n < cap; p = p->next) out[n++] = p->weight;
    return n;
}
int dBackward(const DList &l, double out[], int cap) {
    int n = 0;
    for (const DNode *p = l.tail; p && n < cap; p = p->prev) out[n++] = p->weight;
    return n;
}
Status dCheck(const DList &l) {
    if (l.count == 0) return (!l.head && !l.tail) ? OK : BAD_EMPTY;
    if (!l.head || !l.tail) return BAD_EMPTY;
    if (l.head->prev != nullptr || l.tail->next != nullptr) return BAD_BOUND;
    long long limit = active() + 1, steps = 0;
    const DNode *p = l.head, *last = nullptr;
    while (p && steps < limit) {
        steps++; last = p;
        if (p->next && p->next->prev != p) return BAD_BACK;     // кожен прямий зв'язок підтверджується зворотним
        p = p->next;
    }
    if (p) return CYCLE;
    if (steps != l.count) return last == l.tail ? BAD_COUNT : BROKEN;
    if (last != l.tail) return BAD_BOUND;
    long long back = 0; const DNode *q = l.tail, *lastB = nullptr;
    while (q && back < limit) { back++; lastB = q; q = q->prev; }
    if (back != l.count || lastB != l.head) return BAD_BACK;
    return OK;
}
void dClear(DList &l) {
    DNode *p = l.head;
    while (p) { DNode *nx = p->next; freeD(p); p = nx; }
    l.head = l.tail = nullptr; l.count = 0;
}

// =================== Циклічний однонапрямлений ===================
SNode *cNodeAt(const CList &l, int pos) { SNode *p = l.head; for (int i = 0; i < pos; i++) p = p->next; return p; }

bool cInsertAt(CList &l, int pos, double key) {
    if (pos < 0 || pos > l.count) return false;
    SNode *n = makeS(key);
    if (l.count == 0) { l.head = l.tail = n; n->next = n; }      // один вузол посилається на себе
    else if (pos == 0) { n->next = l.head; l.head = n; l.tail->next = n; }
    else if (pos == l.count) { n->next = l.head; l.tail->next = n; l.tail = n; }
    else { SNode *prev = cNodeAt(l, pos - 1); n->next = prev->next; prev->next = n; }
    l.count++;
    return true;
}
bool cRemoveAt(CList &l, int pos) {
    if (pos < 0 || pos >= l.count) return false;
    if (l.count == 1) { freeS(l.head); l.head = l.tail = nullptr; l.count = 0; return true; }
    if (pos == 0) {
        SNode *old = l.head;
        l.head = old->next; l.tail->next = l.head;              // замикання зберігається
        freeS(old);
    } else {
        SNode *prev = cNodeAt(l, pos - 1), *cur = prev->next;
        prev->next = cur->next;
        if (cur == l.tail) l.tail = prev;
        freeS(cur);
    }
    l.count--;
    return true;
}
// k кроків від початку: out - значення відвіданих вузлів; повертає вузол, на якому зупинився робочий покажчик
SNode *cWalk(const CList &l, int k, double out[]) {
    SNode *cur = l.head;
    for (int i = 0; i < k && cur; i++) { out[i] = cur->weight; cur = cur->next; }
    return cur;
}
Status cCheck(const CList &l) {
    if (l.count == 0) return (!l.head && !l.tail) ? OK : BAD_EMPTY;
    if (!l.head || !l.tail) return BAD_EMPTY;
    long long limit = active() + 1, steps = 0;
    const SNode *p = l.head, *last = nullptr;
    while (true) {
        if (p == nullptr) return OPEN;                          // ланцюжок обривається
        steps++; last = p; p = p->next;
        if (p == l.head) break;                                 // повернення до початкового вузла
        if (steps >= limit) return OPEN;                        // обмеження кроків: до початку не повертається
    }
    if (steps != l.count) return WRONG_LEN;
    if (last != l.tail) return BAD_BOUND;
    return OK;
}
void cClear(CList &l) {                                         // кількість вузлів - точна межа
    SNode *p = l.head;
    for (int i = 0; i < l.count; i++) { SNode *nx = p->next; freeS(p); p = nx; }
    l.head = l.tail = nullptr; l.count = 0;
}

// =================== Циклічний двонапрямлений ===================
DNode *cdNodeAt(const CDList &l, int pos) { DNode *p = l.head; for (int i = 0; i < pos; i++) p = p->next; return p; }

bool cdInsertAt(CDList &l, int pos, double key) {
    if (pos < 0 || pos > l.count) return false;
    DNode *n = makeD(key);
    if (l.count == 0) { l.head = l.tail = n; n->next = n; n->prev = n; }
    else if (pos == 0) { n->next = l.head; n->prev = l.tail; l.head->prev = n; l.tail->next = n; l.head = n; }
    else if (pos == l.count) { n->prev = l.tail; n->next = l.head; l.tail->next = n; l.head->prev = n; l.tail = n; }
    else { DNode *cur = cdNodeAt(l, pos), *p = cur->prev; n->prev = p; n->next = cur; p->next = n; cur->prev = n; }
    l.count++;
    return true;
}
bool cdRemoveAt(CDList &l, int pos) {
    if (pos < 0 || pos >= l.count) return false;
    DNode *cur = cdNodeAt(l, pos);
    if (l.count == 1) { freeD(cur); l.head = l.tail = nullptr; l.count = 0; return true; }
    DNode *p = cur->prev, *nx = cur->next;
    p->next = nx; nx->prev = p;
    if (cur == l.head) l.head = nx;
    if (cur == l.tail) l.tail = p;
    freeD(cur);
    l.count--;
    return true;
}
DNode *cdWalk(const CDList &l, int k, bool forward, double out[]) {
    DNode *cur = forward ? l.head : l.tail;
    for (int i = 0; i < k && cur; i++) { out[i] = cur->weight; cur = forward ? cur->next : cur->prev; }
    return cur;
}
Status cdCheck(const CDList &l) {
    if (l.count == 0) return (!l.head && !l.tail) ? OK : BAD_EMPTY;
    if (!l.head || !l.tail) return BAD_EMPTY;
    long long limit = active() + 1, steps = 0;
    const DNode *p = l.head, *last = nullptr;
    while (true) {
        if (p == nullptr) return OPEN;
        steps++; last = p;
        if (p->next == nullptr) return OPEN;
        p = p->next;
        if (p == l.head) break;
        if (steps >= limit) return OPEN;
    }
    if (steps != l.count) return WRONG_LEN;
    if (last != l.tail) return BAD_BOUND;
    // взаємність: next->prev == вузол для кожного вузла, зворотний обхід замикається в кінці
    const DNode *q = l.head;
    for (int i = 0; i < l.count; i++, q = q->next) if (q->next->prev != q) return BAD_BACK;
    if (l.head->prev != l.tail) return BAD_BACK;
    long long back = 0; const DNode *b = l.head;
    do { back++; b = b->prev; } while (b != l.head && back < limit);
    if (back != l.count) return BAD_BACK;
    return OK;
}
void cdClear(CDList &l) {                                       // спочатку розмикаємо цикл
    if (!l.head) return;
    l.tail->next = nullptr; l.head->prev = nullptr;
    DNode *p = l.head;
    while (p) { DNode *nx = p->next; freeD(p); p = nx; }
    l.head = l.tail = nullptr; l.count = 0;
}

// ---------- Побудова зі значень ----------
void buildD(DList &l, const double *v, int n) { for (int i = 0; i < n; i++) dInsertAt(l, l.count, v[i]); }
void buildC(CList &l, const double *v, int n) { for (int i = 0; i < n; i++) cInsertAt(l, l.count, v[i]); }
void buildCD(CDList &l, const double *v, int n) { for (int i = 0; i < n; i++) cdInsertAt(l, l.count, v[i]); }

// ---------- Сценарії ----------
int testNo = 0, testFailed = 0;
void finish(const string &label, bool dataOk, const string &actual, const string &expected, Status st, bool mem, long long actAfter) {
    testNo++;
    bool good = dataOk && st == OK && mem && actAfter == 0;
    if (!good) testFailed++;
    cout << "Сценарій " << padL(to_string(testNo), 2) << " [" << label << "]: " << actual;
    if (!dataOk) cout << " != очікувано " << expected;
    if (good) cout << " -> успішно (структура ok, пам'ять ok, активних 0)\n";
    else cout << " -> ПОМИЛКА (структура " << statusName(st) << ", пам'ять " << (mem ? "ok" : "ПОМИЛКА")
              << ", активних " << actAfter << ")\n";
}
bool sameSeq(const double *a, const double *b, int n) { for (int i = 0; i < n; i++) if (a[i] != b[i]) return false; return true; }

void negative(const char *type, const char *what, Status expected, int type_id, int kind) {
    resetCounters();
    double v[3] = {1, 2, 3};
    Status s = OK; bool restoredOk = false;
    if (type_id == 0) {
        DList l; buildD(l, v, 3);
        DNode *a = l.head, *b = a->next, *c = l.tail;
        if (kind == 0) b->next = nullptr;              // розрив
        else c->prev = a;                              // неправильний зворотний зв'язок
        s = dCheck(l);
        b->next = c; c->prev = b;                      // відновлення
        restoredOk = dCheck(l) == OK;
        dClear(l);
    } else if (type_id == 1) {
        CList l; buildC(l, v, 3);
        SNode *a = l.head, *b = a->next, *c = l.tail;
        if (kind == 0) c->next = nullptr;              // незамкнений цикл
        else b->next = a;                              // повернення до початку надто рано
        s = cCheck(l);
        b->next = c; c->next = a;
        restoredOk = cCheck(l) == OK;
        cClear(l);
    } else {
        CDList l; buildCD(l, v, 3);
        DNode *a = l.head, *b = a->next, *c = l.tail;
        if (kind == 0) c->next = nullptr;              // незамкнений цикл
        else b->prev = c;                              // неправильний зворотний зв'язок
        s = cdCheck(l);
        c->next = a; b->prev = a;
        restoredOk = cdCheck(l) == OK;
        cdClear(l);
    }
    bool good = s == expected && restoredOk && active() == 0;
    if (!good) testFailed++;
    cout << "Пошкодження [" << type << ": " << what << "]: виявлено \"" << statusName(s) << "\", відновлено і очищено, активних "
         << active() << " -> " << (good ? "успішно" : "ПОМИЛКА") << "\n";
    flushCounters();
}

// Усі значення обчислюються ДО очищення (порядок обчислення аргументів функції не визначений)
#define FIN(chk, clr, label, dataOk, actual, expected) { Status _st = chk(l); bool _mem = (active() == l.count); \
    string _l = label; bool _d = (dataOk); string _a = actual; string _e = expected; clr(l); \
    finish(_l, _d, _a, _e, _st, _mem, active()); }

int main() {
    double out[256], out2[256];

    // ===== Варіант 19 =====
    cout << "=== Варіант 19: двонапрямлений список ===\n";
    double v19[4] = {6, 12, 3, 9};
    resetCounters();
    {
        DList l; buildD(l, v19, 4);
        int nf = dForward(l, out, 256), nb = dBackward(l, out2, 256);
        bool rev = nf == nb;
        for (int i = 0; i < nf && rev; i++) rev = out[i] == out2[nf - 1 - i];
        cout << "До зміни: вперед " << vals(out, nf) << ", назад " << vals(out2, nb) << ", вузлів " << l.count
             << ", назад = обернений вперед: " << yn(rev) << ", " << statusName(dCheck(l)) << "\n";
        dInsertAt(l, 2, 1);
        nf = dForward(l, out, 256); nb = dBackward(l, out2, 256);
        rev = nf == nb;
        for (int i = 0; i < nf && rev; i++) rev = out[i] == out2[nf - 1 - i];
        cout << "Після вставлення 1 на позицію 2: вперед " << vals(out, nf) << ", назад " << vals(out2, nb) << ", вузлів "
             << l.count << ", назад = обернений вперед: " << yn(rev) << ", " << statusName(dCheck(l)) << "\n";
        cout << "Межі: head -> " << num(l.head->weight) << ", tail -> " << num(l.tail->weight)
             << ", head->prev = " << (l.head->prev ? "не nullptr" : "nullptr") << ", tail->next = "
             << (l.tail->next ? "не nullptr" : "nullptr") << "\n";
        cout << "Взаємність зв'язків (значення вузла: prev / next):\n";
        for (const DNode *p = l.head; p; p = p->next)
            cout << "  " << pad(num(p->weight), 4) << ": " << pad(p->prev ? num(p->prev->weight) : "nullptr", 8)
                 << " / " << (p->next ? num(p->next->weight) : "nullptr") << "\n";
        dClear(l);
        cout << "Після очищення: head = nullptr, count = " << l.count << ", активних вузлів " << active() << "\n";
    }
    flushCounters();

    cout << "\n=== Варіант 19: циклічний однонапрямлений список (17 кроків) ===\n";
    resetCounters();
    {
        CList l; buildC(l, v19, 4); cInsertAt(l, 2, 1);
        SNode *e = cWalk(l, l.count, out);
        cout << "Список (від head): " << vals(out, l.count) << ", вузлів " << l.count << ", tail->next = head: "
             << yn(l.tail->next == l.head) << "\n";
        cout << "Після " << l.count << " кроків від початку посилання повернулось до початку: " << yn(e == l.head) << "\n";
        e = cWalk(l, 17, out);
        int idx = 0; for (SNode *p = l.head; p != e; p = p->next) idx++;
        cout << "17 кроків: " << vals(out, 17) << "\n";
        cout << "Посилання після 17 кроків вказує на позицію " << idx << " (значення " << num(e->weight) << "), 17 mod "
             << l.count << " = " << 17 % l.count << "\n";
        cout << "Перевірка замикання: " << statusName(cCheck(l)) << "\n";
        cClear(l);
        cout << "Після очищення (кількість вузлів як межа): count = " << l.count << ", активних вузлів " << active() << "\n";
    }
    flushCounters();

    cout << "\n=== Варіант 19: циклічний двонапрямлений список ===\n";
    resetCounters();
    {
        CDList l; buildCD(l, v19, 4); cdInsertAt(l, 2, 1);
        DNode *e = cdWalk(l, l.count, true, out);
        cout << "Прямий цикл: " << vals(out, l.count) << ", після " << l.count << " кроків повернення до початку: " << yn(e == l.head) << "\n";
        e = cdWalk(l, l.count, false, out);
        cout << "Зворотний цикл: " << vals(out, l.count) << ", після " << l.count << " кроків повернення до кінця: " << yn(e == l.tail) << "\n";
        e = cdWalk(l, 17, true, out);
        cout << "17 кроків вперед: " << vals(out, 17) << ", посилання вказує на значення " << num(e->weight) << "\n";
        e = cdWalk(l, 17, false, out2);
        cout << "17 кроків назад: " << vals(out2, 17) << ", посилання вказує на значення " << num(e->weight) << "\n";
        cout << "Замикання: head->prev = tail: " << yn(l.head->prev == l.tail) << ", tail->next = head: " << yn(l.tail->next == l.head)
             << "; " << statusName(cdCheck(l)) << "\n";
        cdClear(l);
        cout << "Після очищення (цикл розімкнено): count = " << l.count << ", активних вузлів " << active() << "\n";
    }
    flushCounters();

    // ===== Автоматизовані сценарії =====
    cout << "\n=== Автоматизовані сценарії ===\n";
    {   // 1: двонапрямлений, порожній, обхід
        resetCounters(); DList l;
        int a = dForward(l, out, 256), b = dBackward(l, out2, 256);
        FIN(dCheck, dClear, "двонапрямлений: порожньо, обхід", a == 0 && b == 0 && !l.head && !l.tail, "вузлів " + to_string(a) + "/" + to_string(b), "0/0");
        flushCounters();
    }
    {   // 2: один вузол, два обходи
        resetCounters(); DList l; double v[] = {5}; buildD(l, v, 1);
        int a = dForward(l, out, 256), b = dBackward(l, out2, 256);
        bool ok = a == 1 && b == 1 && l.head == l.tail && !l.head->prev && !l.head->next;
        FIN(dCheck, dClear, "двонапрямлений: 5, два обходи", ok, "вперед " + vals(out, a) + ", назад " + vals(out2, b), "[5] і [5]");
        flushCounters();
    }
    {   // 3: зворотний обхід
        resetCounters(); DList l; double v[] = {1, 2, 3}, e[] = {3, 2, 1}; buildD(l, v, 3);
        int b = dBackward(l, out2, 256);
        FIN(dCheck, dClear, "двонапрямлений: 1,2,3 зворотний обхід", b == 3 && sameSeq(out2, e, 3), vals(out2, b), "[3, 2, 1]");
        flushCounters();
    }
    {   // 4: вставлення 2
        resetCounters(); DList l; double v[] = {1, 3}, e[] = {1, 2, 3}, er[] = {3, 2, 1}; buildD(l, v, 2);
        dInsertAt(l, 1, 2);
        int a = dForward(l, out, 256), b = dBackward(l, out2, 256);
        FIN(dCheck, dClear, "двонапрямлений: 1,3 вставити 2", a == 3 && b == 3 && sameSeq(out, e, 3) && sameSeq(out2, er, 3), vals(out, a) + " / " + vals(out2, b), "[1, 2, 3] / [3, 2, 1]");
        flushCounters();
    }
    {   // 5: видалення 2
        resetCounters(); DList l; double v[] = {1, 2, 3}, e[] = {1, 3}, er[] = {3, 1}; buildD(l, v, 3);
        bool r = dRemoveKey(l, 2);
        int a = dForward(l, out, 256), b = dBackward(l, out2, 256);
        FIN(dCheck, dClear, "двонапрямлений: 1,2,3 видалити 2", r && a == 2 && b == 2 && sameSeq(out, e, 2) && sameSeq(out2, er, 2), vals(out, a) + " / " + vals(out2, b), "[1, 3] / [3, 1]");
        flushCounters();
    }
    {   // 6: видалення єдиного
        resetCounters(); DList l; double v[] = {7}; buildD(l, v, 1);
        bool r = dRemoveAt(l, 0);
        bool ok = r && l.count == 0 && !l.head && !l.tail && active() == 0;
        FIN(dCheck, dClear, "двонапрямлений: 7 видалити", ok, "[]", "[]");
        flushCounters();
    }
    {   // 7: циклічний, один вузол, 4 кроки
        resetCounters(); CList l; double v[] = {4}, e4[] = {4, 4, 4, 4}; buildC(l, v, 1);
        SNode *e = cWalk(l, 4, out);
        bool ok = l.head->next == l.head && e == l.head && sameSeq(out, e4, 4);
        FIN(cCheck, cClear, "циклічний однонапр.: 4, чотири кроки", ok, vals(out, 4) + ", повернення до початку " + yn(e == l.head), "[4, 4, 4, 4], повернення так");
        flushCounters();
    }
    {   // 8: 1,2,3, 5 кроків
        resetCounters(); CList l; double v[] = {1, 2, 3}, e[] = {1, 2, 3, 1, 2}; buildC(l, v, 3);
        cWalk(l, 5, out);
        FIN(cCheck, cClear, "циклічний однонапр.: 1,2,3, 5 кроків", sameSeq(out, e, 5), vals(out, 5), "[1, 2, 3, 1, 2]");
        flushCounters();
    }
    {   // 9: вставити 3 -> цикл довжини 3
        resetCounters(); CList l; double v[] = {1, 2}, e[] = {1, 2, 3}; buildC(l, v, 2);
        cInsertAt(l, 2, 3);
        SNode *end = cWalk(l, 3, out);
        FIN(cCheck, cClear, "циклічний однонапр.: 1,2 вставити 3", l.count == 3 && sameSeq(out, e, 3) && end == l.head, vals(out, 3) + ", довжина циклу " + to_string(l.count), "[1, 2, 3], довжина 3");
        flushCounters();
    }
    {   // 10: циклічний двонапрямлений, прямий цикл
        resetCounters(); CDList l; double v[] = {1, 2, 3}; buildCD(l, v, 3);
        DNode *e = cdWalk(l, 3, true, out);
        FIN(cdCheck, cdClear, "циклічний двонапр.: 1,2,3 прямий цикл", sameSeq(out, v, 3) && e == l.head, vals(out, 3), "[1, 2, 3]");
        flushCounters();
    }
    {   // 11: зворотний цикл
        resetCounters(); CDList l; double v[] = {1, 2, 3}, e3[] = {3, 2, 1}; buildCD(l, v, 3);
        DNode *e = cdWalk(l, 3, false, out);
        FIN(cdCheck, cdClear, "циклічний двонапр.: 1,2,3 зворотний цикл", sameSeq(out, e3, 3) && e == l.tail, vals(out, 3), "[3, 2, 1]");
        flushCounters();
    }
    {   // 12: один вузол, зв'язки на себе
        resetCounters(); CDList l; double v[] = {1}; buildCD(l, v, 1);
        bool ok = l.head == l.tail && l.head->next == l.head && l.head->prev == l.head;
        FIN(cdCheck, cdClear, "циклічний двонапр.: 1, зв'язки", ok, string("next = prev = сам вузол: ") + yn(ok), "так");
        flushCounters();
    }
    {   // 13: усі типи, 10 вузлів, очищення
        resetCounters();
        double v[10]; for (int i = 0; i < 10; i++) v[i] = i + 1;
        DList d; CList c; CDList cd; buildD(d, v, 10); buildC(c, v, 10); buildCD(cd, v, 10);
        bool st = dCheck(d) == OK && cCheck(c) == OK && cdCheck(cd) == OK && active() == 30;
        dClear(d); cClear(c); cdClear(cd);
        bool ok = st && d.count == 0 && c.count == 0 && cd.count == 0 && !d.head && !c.head && !cd.head;
        finish("усі типи: 10 вузлів, очищення", ok, "створено " + to_string(created) + ", звільнено " + to_string(freed), "30 і 30", OK, true, active());
        flushCounters();
    }
    {   // 14: повторне створення після очищення
        resetCounters();
        double v[3] = {1, 2, 3};
        DList d; CList c; CDList cd;
        buildD(d, v, 3); dClear(d); buildD(d, v, 3);
        buildC(c, v, 3); cClear(c); buildC(c, v, 3);
        buildCD(cd, v, 3); cdClear(cd); buildCD(cd, v, 3);
        bool ok = dCheck(d) == OK && cCheck(c) == OK && cdCheck(cd) == OK && d.count == 3 && c.count == 3 && cd.count == 3 && active() == 9;
        dClear(d); cClear(c); cdClear(cd);
        finish("усі типи: створення після очищення", ok, "три списки по 3 вузли", "коректні", OK, true, active());
        flushCounters();
    }
    {   // 15: циклічний, 100 кроків
        resetCounters(); CList l; double v[] = {1, 2, 3}; buildC(l, v, 3);
        SNode *e = cWalk(l, 100, out);
        bool ok = true; for (int i = 0; i < 100; i++) ok = ok && out[i] == (i % 3) + 1;
        ok = ok && e->weight == 2;                      // 100 mod 3 = 1 -> другий вузол
        FIN(cCheck, cClear, "циклічний: 3 вузли, 100 кроків", ok, "100 кроків завершено, посилання на значення " + num(e->weight), "значення 2 (100 mod 3 = 1)");
        flushCounters();
    }
    {   // 16: двонапрямлений, вставлення й видалення на обох межах
        resetCounters(); DList l; double v[] = {2, 3}, e[] = {3, 4}, er[] = {4, 3}; buildD(l, v, 2);
        bool ok = dInsertAt(l, 0, 1) && dInsertAt(l, 3, 4) && dCheck(l) == OK && l.head->weight == 1 && l.tail->weight == 4;
        ok = ok && dRemoveAt(l, 0) && dRemoveAt(l, 0) && dCheck(l) == OK;
        int a = dForward(l, out, 256), b = dBackward(l, out2, 256);
        ok = ok && a == 2 && sameSeq(out, e, 2) && sameSeq(out2, er, 2) && l.head->prev == nullptr && l.tail->next == nullptr;
        FIN(dCheck, dClear, "власний: двонапр., обидві межі", ok, vals(out, a) + " / " + vals(out2, b), "[3, 4] / [4, 3]");
        flushCounters();
    }
    {   // 17: два послідовні очищення
        resetCounters(); bool ok = true;
        for (int round = 0; round < 2; round++) {
            double v[5] = {1, 2, 3, 4, 5};
            DList d; CList c; CDList cd; buildD(d, v, 5); buildC(c, v, 5); buildCD(cd, v, 5);
            dClear(d); cClear(c); cdClear(cd);
            ok = ok && active() == 0 && d.count == 0 && c.count == 0 && cd.count == 0;
        }
        ok = ok && created == 30 && freed == 30;
        finish("власний: два послідовні очищення", ok, "створено " + to_string(created) + ", звільнено " + to_string(freed), "30 і 30", OK, true, active());
        flushCounters();
    }
    {   // 18: циклічний однонапрямлений, обидві межі
        resetCounters(); CList l; double v[] = {2, 3}, e[] = {3, 4}; buildC(l, v, 2);
        bool ok = cInsertAt(l, 0, 1) && cInsertAt(l, 3, 4) && cCheck(l) == OK && l.tail->next == l.head;
        ok = ok && cRemoveAt(l, 0) && cRemoveAt(l, 0) && cCheck(l) == OK;
        cWalk(l, 2, out);
        ok = ok && l.count == 2 && sameSeq(out, e, 2) && l.tail->next == l.head;
        FIN(cCheck, cClear, "власний: циклічний однонапр., обидві межі", ok, vals(out, 2), "[3, 4]");
        flushCounters();
    }
    {   // 19: циклічний двонапрямлений, обидві межі і єдиний вузол
        resetCounters(); CDList l; double v[] = {2, 3}, e[] = {3, 4}; buildCD(l, v, 2);
        bool ok = cdInsertAt(l, 0, 1) && cdInsertAt(l, 3, 4) && cdCheck(l) == OK && l.head->prev == l.tail && l.tail->next == l.head;
        ok = ok && cdRemoveAt(l, 0) && cdRemoveAt(l, 0) && cdCheck(l) == OK;
        cdWalk(l, 2, true, out);
        ok = ok && l.count == 2 && sameSeq(out, e, 2);
        ok = ok && cdRemoveAt(l, 1) && l.count == 1 && l.head == l.tail && l.head->next == l.head && l.head->prev == l.head && cdCheck(l) == OK;
        ok = ok && cdRemoveAt(l, 0) && l.count == 0 && !l.head && !l.tail && active() == 0;
        FIN(cdCheck, cdClear, "власний: циклічний двонапр., межі і єдиний вузол", ok, "порожньо, активних " + to_string(active()), "порожньо");
        flushCounters();
    }
    {   // 20: циклічний однонапрямлений, єдиний вузол видаляється, список можна відновити
        resetCounters(); CList l; double v[] = {9}; buildC(l, v, 1);
        bool ok = cRemoveAt(l, 0) && l.count == 0 && !l.head && !l.tail && cCheck(l) == OK && active() == 0;
        ok = ok && cInsertAt(l, 0, 5) && l.head->next == l.head && cCheck(l) == OK;
        FIN(cCheck, cClear, "власний: циклічний однонапр., єдиний вузол", ok, "після повторного створення count = " + to_string(l.count), "1");
        flushCounters();
    }
    cout << "Сценаріїв: " << testNo << ", невдалих: " << testFailed << "\n";

    cout << "\n=== Діагностичний режим: навмисні пошкодження ===\n";
    negative("двонапрямлений", "розрив next", BROKEN, 0, 0);
    negative("двонапрямлений", "хибний prev", BAD_BACK, 0, 1);
    negative("циклічний однонапр.", "tail->next = nullptr", OPEN, 1, 0);
    negative("циклічний однонапр.", "повернення до head надто рано", WRONG_LEN, 1, 1);
    negative("циклічний двонапр.", "tail->next = nullptr", OPEN, 2, 0);
    negative("циклічний двонапр.", "хибний prev", BAD_BACK, 2, 1);
    if (testFailed) return 1;

    cout << "\n=== Підсумок діагностики ===\n";
    cout << "Створено вузлів: " << totalCreated << ", звільнено: " << totalFreed << ", активних вузлів: "
         << totalCreated - totalFreed << "\n";
    cout << (totalCreated == totalFreed ? "Витоків пам'яті немає." : "ВИТІК ПАМ'ЯТІ!") << "\n";
    return totalCreated == totalFreed ? 0 : 2;
}
