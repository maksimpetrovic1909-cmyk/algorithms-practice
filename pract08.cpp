#include <iostream>
#include <string>
using namespace std;

const int MAXTEXT = 10001;
const int MAXRES = 10001;

// Єдині правила для текстових і числових даних:
//  - допустимі початки: 0 .. n - m (зразок має поміститися до кінця);
//  - порожній зразок (m = 0) вважається входженням на позиції 0, порівнянь немає;
//  - зразок довший за текст (m > n): результат -1 / 0 входжень, порівнянь немає;
//  - порівняння = кожне зіставлення пари елементів; позиції = кожен початок, з якого розпочато перевірку;
//  - після збігу наступна перевірка починається з наступної позиції (накладені входження не пропускаються).
struct Stats {
    long long cmp = 0, pos = 0;
};

// ---------- Порівняння елементів ----------
char lowerCh(char c) { return (c >= 'A' && c <= 'Z') ? char(c - 'A' + 'a') : c; }
bool eqCase(char a, char b) { return a == b; }
bool eqNoCase(char a, char b) { return lowerCh(a) == lowerCh(b); }
bool eqInt(int a, int b) { return a == b; }

// ---------- Пошук першого входження (шаблон: тексти та числа) ----------
template <class T>
int findFirstT(const T *a, int n, const T *p, int m, Stats &s, bool (*eq)(T, T)) {
    s = Stats();
    if (m == 0) return 0;
    if (m > n) return -1;
    for (int i = 0; i <= n - m; i++) {
        s.pos++;
        int j = 0;
        while (j < m) {
            s.cmp++;
            if (!eq(a[i + j], p[j])) break;
            j++;
        }
        if (j == m) return i;
    }
    return -1;
}

// ---------- Пошук усіх входжень (включно з накладеними) ----------
template <class T>
int findAllT(const T *a, int n, const T *p, int m, int res[], Stats &s, bool (*eq)(T, T)) {
    s = Stats();
    int cnt = 0;
    if (m == 0) { res[cnt++] = 0; return cnt; }
    if (m > n) return 0;
    for (int i = 0; i <= n - m; i++) {
        s.pos++;
        int j = 0;
        while (j < m) {
            s.cmp++;
            if (!eq(a[i + j], p[j])) break;
            j++;
        }
        if (j == m) res[cnt++] = i;     // наступний початок i + 1, а не i + m
    }
    return cnt;
}

// Обгортки для текстів
int strLen(const char *s) { int n = 0; while (s[n]) n++; return n; }
int firstText(const char *t, const char *p, Stats &s, bool ic = false) {
    return findFirstT<char>(t, strLen(t), p, strLen(p), s, ic ? eqNoCase : eqCase);
}
int allText(const char *t, const char *p, int res[], Stats &s, bool ic = false) {
    return findAllT<char>(t, strLen(t), p, strLen(p), res, s, ic ? eqNoCase : eqCase);
}

// ---------- Допоміжні ----------
int visLen(const string &s) { int l = 0; for (unsigned char c : s) if ((c & 0xC0) != 0x80) l++; return l; }
string pad(const string &s, int w) { int l = visLen(s); return s + string(w > l ? w - l : 0, ' '); }
string padL(const string &s, int w) { int l = visLen(s); return string(w > l ? w - l : 0, ' ') + s; }

string listStr(const int r[], int c) {
    if (c == 0) return "немає";
    string s;
    for (int i = 0; i < c; i++) s += (i ? ", " : "") + to_string(r[i]);
    return s;
}

int totalFirst = 0, failFirst = 0, totalAll = 0, failAll = 0;

// Перевірка: фактичний індекс першого входження і список усіх позицій
void check(int no, const string &label, int firstGot, int firstExp, const int allGot[], int cntGot,
           const int allExp[], int cntExp, const Stats &s) {
    bool okF = firstGot == firstExp;
    bool okA = cntGot == cntExp;
    for (int i = 0; i < cntExp && okA; i++) if (allGot[i] != allExp[i]) okA = false;
    totalFirst++; totalAll++;
    if (!okF) failFirst++;
    if (!okA) failAll++;
    cout << "Тест " << padL(to_string(no), 2) << " [" << label << "]: перше " << firstGot
         << ", усі " << listStr(allGot, cntGot) << ", порівнянь " << s.cmp << ", позицій " << s.pos
         << " -> перше: " << (okF ? "успішно" : "помилка (очікувано " + to_string(firstExp) + ")")
         << ", усі: " << (okA ? "успішно" : "помилка (очікувано " + listStr(allExp, cntExp) + ")") << "\n";
}

void textTest(int no, const char *t, const char *p, int firstExp, const int allExp[], int cntExp) {
    int res[MAXRES]; Stats s1, s2;
    int f = firstText(t, p, s1);
    int c = allText(t, p, res, s2);
    check(no, string("\"") + t + "\" / \"" + p + "\"", f, firstExp, res, c, allExp, cntExp, s2);
}
void numTest(int no, const int *t, int n, const int *p, int m, int firstExp, const int allExp[], int cntExp) {
    int res[MAXRES]; Stats s1, s2;
    int f = findFirstT<int>(t, n, p, m, s1, eqInt);
    int c = findAllT<int>(t, n, p, m, res, s2, eqInt);
    string a, b;
    for (int i = 0; i < n; i++) a += (i ? "," : "") + to_string(t[i]);
    for (int i = 0; i < m; i++) b += (i ? "," : "") + to_string(p[i]);
    check(no, "{" + a + "} / {" + b + "}", f, firstExp, res, c, allExp, cntExp, s2);
}

bool readLine(char *buf, int maxLen) {
    cin.getline(buf, maxLen);
    return !cin.fail();
}

int main() {
    // ===== Автотести =====
    cout << "=== Автоматизовані перевірки ===\n";
    {
        int none[1] = {0};
        int a1[] = {0}, a2[] = {4}, a3[] = {6}, a5[] = {0, 1, 2}, a6[] = {0, 2, 4}, a8[] = {5}, a9[] = {4, 8}, a10[] = {0, 2};
        textTest(1, "algorithm", "algo", 0, a1, 1);
        textTest(2, "algorithm", "rith", 4, a2, 1);
        textTest(3, "algorithm", "thm", 6, a3, 1);
        textTest(4, "algorithm", "sort", -1, none, 0);
        textTest(5, "aaaa", "aa", 0, a5, 3);
        textTest(6, "abababa", "aba", 0, a6, 3);
        textTest(7, "abc", "abcd", -1, none, 0);
        textTest(8, "Data data DATA", "data", 5, a8, 1);
        textTest(9, "one two two", "two", 4, a9, 2);
        { int t[] = {1, 2, 1, 2, 1}, p[] = {1, 2, 1}; numTest(10, t, 5, p, 3, 0, a10, 2); }
        { int t[] = {4, 5, 6}, p[] = {5, 7}; numTest(11, t, 3, p, 2, -1, none, 0); }
        textTest(12, "", "x", -1, none, 0);
        // власні текстові
        int b13[] = {3}, b14[] = {1, 3};
        textTest(13, "aaaaab", "aab", 3, b13, 1);
        textTest(14, "ababab", "bab", 1, b14, 2);
        // власні числові
        int b15[] = {0, 1, 2}, b16[] = {3};
        { int t[] = {7, 7, 7, 7}, p[] = {7, 7}; numTest(15, t, 4, p, 2, 0, b15, 3); }
        { int t[] = {3, 1, 4, 1, 5, 9, 2, 6}, p[] = {1, 5, 9}; numTest(16, t, 8, p, 3, 3, b16, 1); }
        // порожній зразок: входження на позиції 0
        int e0[] = {0};
        textTest(17, "abc", "", 0, e0, 1);
        { int t[] = {1, 2, 3}; int p[1] = {0}; numTest(18, t, 3, p, 0, 0, e0, 1); }
    }
    cout << "Тестів першого входження: " << totalFirst << ", невдалих: " << failFirst
         << "; тестів усіх входжень: " << totalAll << ", невдалих: " << failAll << "\n";
    if (failFirst || failAll) return 1;

    // ===== Приклади регістру =====
    cout << "\n=== Режими з урахуванням і без урахування регістру ===\n";
    {
        const char *ex[2][2] = {{"Data data DATA", "data"}, {"Case sensitive CASE case", "case"}};
        for (int e = 0; e < 2; e++)
            for (int ic = 0; ic < 2; ic++) {
                int res[MAXRES]; Stats s1, s2;
                int f = firstText(ex[e][0], ex[e][1], s1, ic == 1);
                int c = allText(ex[e][0], ex[e][1], res, s2, ic == 1);
                cout << "\"" << ex[e][0] << "\" / \"" << ex[e][1] << "\", "
                     << (ic ? "без урахування регістру" : "з урахуванням регістру") << ": перше " << f
                     << ", усі " << listStr(res, c) << ", порівнянь " << s2.cmp << ", позицій " << s2.pos << "\n";
            }
    }

    // ===== Експеримент =====
    cout << "\n=== Експеримент: перше входження ===\n";
    cout << padL("n", 6) << "  " << pad("Сценарій", 34) << padL("Індекс", 8) << padL("Порівняння", 12)
         << padL("Позиції", 9) << "\n";
    static char text[MAXTEXT];
    int sizes[3] = {100, 1000, 10000};
    const char *needle = "NEEDLE";
    const char *dPat = "aaaaaaab";
    const char *scName[5] = {"A: на початку", "B: у середині", "C: у кінці",
                             "D: відсутній, довгі часткові збіги", "D0: відсутній, без часткових збігів"};
    for (int z = 0; z < 3; z++) {
        int n = sizes[z];
        int m = strLen(needle);
        // фон з літер a..d, повторюваний генератор
        unsigned long long x = 12345;
        auto background = [&](char *t) {
            x = 12345;
            for (int i = 0; i < n; i++) { x = (x * 1103515245ULL + 12345ULL) & 0x7fffffffULL; t[i] = char('a' + ((x >> 16) % 4)); }
            t[n] = 0;
        };
        for (int sc = 0; sc < 5; sc++) {
            const char *pat = needle;
            if (sc < 3) {
                background(text);
                int at = sc == 0 ? 0 : (sc == 1 ? n / 2 : n - m);
                for (int i = 0; i < m; i++) text[at + i] = needle[i];
            } else if (sc == 3) {
                for (int i = 0; i < n; i++) text[i] = 'a';
                text[n] = 0;
                pat = dPat;
            } else {
                background(text);              // зразок із першою літерою, якої немає в тексті
            }
            Stats s;
            int f = firstText(text, pat, s);
            cout << padL(to_string(n), 6) << "  " << pad(scName[sc], 34) << padL(to_string(f), 8)
                 << padL(to_string(s.cmp), 12) << padL(to_string(s.pos), 9) << "\n";
        }
    }

    // ===== Варіант 19 =====
    cout << "\n=== Варіант 19 ===\n";
    static char vText[MAXTEXT], vPat[MAXTEXT];
    cout << "Введіть основний текст: ";
    bool got = readLine(vText, MAXTEXT);
    if (got) { cout << "\nВведіть зразок: "; got = readLine(vPat, MAXTEXT); }
    if (!got) {
        const char *dt = "Case sensitive CASE case", *dp = "case";
        int i = 0; while ((vText[i] = dt[i])) i++;
        i = 0; while ((vPat[i] = dp[i])) i++;
        cout << "\n(введення недоступне, використано дані варіанта 19)";
    }
    cout << "\nТекст: \"" << vText << "\" (довжина " << strLen(vText) << "), зразок: \"" << vPat
         << "\" (довжина " << strLen(vPat) << ")\n";
    cout << pad("Режим", 26) << padL("Перше", 7) << padL("Усі входження", 18) << padL("Збіги", 7)
         << padL("Порівн.", 9) << padL("Позиції", 9) << "\n";
    for (int ic = 0; ic < 2; ic++) {
        int res[MAXRES]; Stats s1, s2;
        int f = firstText(vText, vPat, s1, ic == 1);
        int c = allText(vText, vPat, res, s2, ic == 1);
        cout << pad(ic ? "без урахування регістру" : "з урахуванням регістру", 26) << padL(to_string(f), 7)
             << padL(listStr(res, c), 18) << padL(to_string(c), 7) << padL(to_string(s2.cmp), 9)
             << padL(to_string(s2.pos), 9) << "   (пошук першого: порівнянь " << s1.cmp << ", позицій " << s1.pos << ")\n";
    }
    return 0;
}
