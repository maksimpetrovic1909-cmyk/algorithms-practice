#include <iostream>
#include <string>
using namespace std;

const int MAXTEXT = 10001;
const int MAXRES = 10001;

// Правила (як у практичній №8): порожній зразок = входження на позиції 0 без порівнянь;
// зразок довший за текст -> -1 (для всіх входжень порожній список) без порівнянь.
// Порівняння = кожне зіставлення елемента тексту з елементом зразка.
// Переходи за префіксом = кількість випадків j = pi[j-1] (після невідповідності або повного збігу).
struct Stats {
    long long cmp = 0;      // порівняння елементів тексту й зразка
    long long jumps = 0;    // переходи за префікс-функцією
    long long pos = 0;      // (лише прямий пошук) перевірені початкові позиції
    long long buildCmp = 0; // порівняння під час побудови префікс-функції
};

bool eqChar(char a, char b) { return a == b; }
bool eqInt(int a, int b) { return a == b; }

// ---------- Префікс-функція ----------
// pi[i] = довжина найдовшого власного префікса p[0..i], що є також його суфіксом
template <class T>
void prefixFunction(const T *p, int m, int pi[], long long &cmp, bool (*eq)(T, T)) {
    cmp = 0;
    if (m == 0) return;
    pi[0] = 0;
    int k = 0;
    for (int i = 1; i < m; i++) {
        while (k > 0) {
            cmp++;
            if (eq(p[i], p[k])) break;
            k = pi[k - 1];                 // використовуємо вже обчислене значення
        }
        if (k == 0) { cmp++; if (eq(p[i], p[0])) k = 1; }
        else k++;
        pi[i] = k;
    }
}

// Загальна реалізація; all = true збирає всі входження (res), інакше зупиняється на першому
template <class T>
int kmpSearch(const T *t, int n, const T *p, int m, const int pi[], int res[], Stats &s, bool (*eq)(T, T), bool all,
              bool trace = false) {
    long long keepBuild = s.buildCmp;
    s = Stats(); s.buildCmp = keepBuild;
    int cnt = 0;
    if (m == 0) { res[cnt++] = 0; return cnt; }
    if (m > n) return 0;
    int i = 0, j = 0, step = 0;
    while (i < n) {
        s.cmp++;
        step++;
        bool match = eq(t[i], p[j]);
        if (trace) {
            cout << "крок " << step << " | i=" << i << " | j=" << j << " | '" << t[i] << "' ? '" << p[j] << "' | ";
        }
        if (match) {
            i++; j++;
            if (trace) cout << "збіг | j'=" << j << " | -\n";
            if (j == m) {
                res[cnt++] = i - m;
                if (trace) cout << "      | повний збіг на позиції " << i - m << " | продовження з j=pi[" << m - 1 << "]=" << pi[m - 1] << "\n";
                if (!all) return cnt;
                j = pi[m - 1];             // продовження після повного збігу (накладені входження)
                s.jumps++;
            }
        } else if (j > 0) {
            int nj = pi[j - 1];
            s.jumps++;
            if (trace) cout << "невідповідність | j'=" << nj << " | перехід за префіксом: i не повертається, j=pi[" << j - 1 << "]\n";
            j = nj;
        } else {
            if (trace) cout << "невідповідність | j'=0 | i+1\n";
            i++;
        }
    }
    return cnt;
}

// ---------- Прямий пошук (з практичної №8) ----------
template <class T>
int naiveSearch(const T *a, int n, const T *p, int m, int res[], Stats &s, bool (*eq)(T, T), bool all) {
    s = Stats();
    int cnt = 0;
    if (m == 0) { res[cnt++] = 0; return cnt; }
    if (m > n) return 0;
    for (int i = 0; i <= n - m; i++) {
        s.pos++;
        int j = 0;
        while (j < m) { s.cmp++; if (!eq(a[i + j], p[j])) break; j++; }
        if (j == m) { res[cnt++] = i; if (!all) return cnt; }
    }
    return cnt;
}

int strLen(const char *s) { int n = 0; while (s[n]) n++; return n; }

int visLen(const string &s) { int l = 0; for (unsigned char c : s) if ((c & 0xC0) != 0x80) l++; return l; }
string pad(const string &s, int w) { int l = visLen(s); return s + string(w > l ? w - l : 0, ' '); }
string padL(const string &s, int w) { int l = visLen(s); return string(w > l ? w - l : 0, ' ') + s; }
string listStr(const int r[], int c) {
    if (c == 0) return "немає";
    string s;
    for (int i = 0; i < c; i++) s += (i ? ", " : "") + to_string(r[i]);
    return s;
}

// ---------- Тести префікс-функції ----------
int prefChecks = 0, prefFailed = 0;
void prefixTest(const char *p, const int expected[], int m) {
    int pi[MAXTEXT]; long long c;
    prefixFunction<char>(p, m, pi, c, eqChar);
    prefChecks++;
    int bad = -1;
    for (int i = 0; i < m; i++) if (pi[i] != expected[i]) { bad = i; break; }
    cout << "Префікс-функція " << padL(to_string(prefChecks), 2) << " \"" << p << "\": ";
    for (int i = 0; i < m; i++) cout << (i ? ", " : "") << pi[i];
    cout << " (порівнянь " << c << "): ";
    if (bad < 0) cout << "успішно\n";
    else {
        prefFailed++;
        cout << "помилка: індекс " << bad << ", символ '" << p[bad] << "', очікувано " << expected[bad]
             << ", отримано " << pi[bad] << "\n";
    }
}

// ---------- Тести пошуку ----------
int srchChecks = 0, srchFailed = 0;
template <class T>
void searchTest(const string &label, const T *t, int n, const T *p, int m, bool (*eq)(T, T), int firstExp,
                const int allExp[], int cntExp) {
    static int pi[MAXTEXT];
    int r1[MAXRES], r2[MAXRES], r3[MAXRES];
    Stats sk, sa, sn;
    long long bc;
    prefixFunction<T>(p, m, pi, bc, eq);
    sk.buildCmp = bc;
    int c1 = kmpSearch<T>(t, n, p, m, pi, r1, sk, eq, false);
    int first = c1 ? r1[0] : -1;
    int c2 = kmpSearch<T>(t, n, p, m, pi, r2, sa, eq, true);
    int c3 = naiveSearch<T>(t, n, p, m, r3, sn, eq, true);
    bool okF = first == firstExp;
    bool okA = c2 == cntExp;
    for (int i = 0; i < cntExp && okA; i++) if (r2[i] != allExp[i]) okA = false;
    bool okN = c3 == c2;                       // збіг з прямим пошуком
    for (int i = 0; i < c3 && okN; i++) if (r3[i] != r2[i]) okN = false;
    srchChecks++;
    bool ok = okF && okA && okN;
    if (!ok) srchFailed++;
    cout << "Пошук " << padL(to_string(srchChecks), 2) << " [" << label << "]: перше " << first << ", усі "
         << listStr(r2, c2) << ", КМП " << sa.cmp << " порівн. / прямий " << sn.cmp << " порівн., переходів " << sa.jumps
         << " -> " << (ok ? "успішно" : "помилка") << "\n";
}
void textSearch(const char *t, const char *p, int firstExp, const int allExp[], int cntExp) {
    searchTest<char>(string("\"") + t + "\" / \"" + p + "\"", t, strLen(t), p, strLen(p), eqChar, firstExp, allExp, cntExp);
}

bool readLine(char *buf, int maxLen) {
    cin.getline(buf, maxLen);
    return !cin.fail();
}

void printPrefix(const char *p, const int pi[], int m) {
    cout << "  символи:  "; for (int i = 0; i < m; i++) cout << padL(string(1, p[i]), 3);
    cout << "\n  індекси:  "; for (int i = 0; i < m; i++) cout << padL(to_string(i), 3);
    cout << "\n  значення: "; for (int i = 0; i < m; i++) cout << padL(to_string(pi[i]), 3);
    cout << "\n";
}

int main() {
    // ===== Префікс-функція =====
    cout << "=== Перевірки префікс-функції ===\n";
    {
        int e1[] = {0}, e2[] = {0, 1, 2, 3}, e3[] = {0, 0, 0, 0}, e4[] = {0, 0, 1, 2, 3, 0, 1},
            e5[] = {0, 1, 0, 1, 2, 2, 3}, e6[] = {0, 0, 0, 1, 2, 1, 2, 3, 4, 5};
        int o1[] = {0, 0, 0, 1, 2, 3, 4, 5, 0}, o2[] = {0, 0, 1, 2, 3, 4, 0, 1, 2}, v[] = {0, 0, 0, 0, 0, 0, 0};
        prefixTest("a", e1, 1);
        prefixTest("aaaa", e2, 4);
        prefixTest("abcd", e3, 4);
        prefixTest("ababaca", e4, 7);
        prefixTest("aabaaab", e5, 7);
        prefixTest("abcababcab", e6, 10);
        prefixTest("abcabcabd", o1, 9);        // власний
        prefixTest("abababcab", o2, 9);        // власний
        prefixTest("pattern", v, 7);           // зразок варіанта 19
    }
    cout << "Перевірок префікс-функції: " << prefChecks << ", невдалих: " << prefFailed << "\n";
    if (prefFailed) return 1;

    // ===== Пошук =====
    cout << "\n=== Перевірки пошуку КМП (результати звірено з прямим пошуком) ===\n";
    {
        int none[1] = {0};
        int a1[] = {4}, a2[] = {0, 1, 2}, a3[] = {0, 2, 4}, a5[] = {1, 4}, a6[] = {3};
        textSearch("algorithm", "rith", 4, a1, 1);
        textSearch("aaaa", "aa", 0, a2, 3);
        textSearch("abababa", "aba", 0, a3, 3);
        textSearch("abc", "abcd", -1, none, 0);
        textSearch("mississippi", "issi", 1, a5, 2);
        textSearch("abcabcabd", "abcabd", 3, a6, 1);
        textSearch("no matches here", "xyz", -1, none, 0);
        textSearch("", "a", -1, none, 0);
        // власні
        int b1[] = {6}, b2[] = {0, 3, 7}, b3[] = {2}, e0[] = {0};
        textSearch("abcabcabcabd", "abcabd", 6, b1, 1);
        textSearch("aabaabaaab", "aab", 0, b2, 3);
        { int t[] = {1, 2, 1, 2, 1, 2, 3}, p[] = {1, 2, 1, 2, 3};
          searchTest<int>("{1,2,1,2,1,2,3} / {1,2,1,2,3}", t, 7, p, 5, eqInt, 2, b3, 1); }
        textSearch("abc", "", 0, e0, 1);
    }
    cout << "Перевірок пошуку: " << srchChecks << ", невдалих: " << srchFailed << "\n";
    if (srchFailed) return 1;

    // ===== Експеримент =====
    cout << "\n=== Порівняння КМП і прямого пошуку ===\n";
    cout << padL("n", 6) << "  " << pad("Сценарій", 40) << padL("Режим", 8) << padL("Індекс/к-сть", 14)
         << padL("Прямий", 10) << padL("КМП", 9) << padL("Префікс", 9) << padL("Перех.", 8) << "\n";
    static char text[MAXTEXT], pat[64];
    static int pi[64];
    int sizes[3] = {100, 1000, 10000};
    const char *scName[7] = {"A: NEEDLE на початку", "B: NEEDLE у середині", "C: NEEDLE у кінці",
                             "D: a^n / aaaaaaab (збіг 7, відсутній)", "E: a^n / a^49 b (збіг 49, відсутній)",
                             "F: (ab)^k / (ab)^5 c (відсутній)", "G: a^n / aaaaaaaa (усі входження)"};
    for (int z = 0; z < 3; z++) {
        int n = sizes[z];
        for (int sc = 0; sc < 7; sc++) {
            bool all = false;
            const char *pp = "NEEDLE";
            if (sc <= 2) {
                unsigned long long x = 12345;
                for (int i = 0; i < n; i++) { x = (x * 1103515245ULL + 12345ULL) & 0x7fffffffULL; text[i] = char('a' + ((x >> 16) % 4)); }
                text[n] = 0;
                int at = sc == 0 ? 0 : (sc == 1 ? n / 2 : n - 6);
                for (int i = 0; i < 6; i++) text[at + i] = pp[i];
            } else if (sc == 3 || sc == 4 || sc == 6) {
                for (int i = 0; i < n; i++) text[i] = 'a';
                text[n] = 0;
                if (sc == 3) pp = "aaaaaaab";
                else if (sc == 4) { for (int i = 0; i < 49; i++) pat[i] = 'a'; pat[49] = 'b'; pat[50] = 0; pp = pat; }
                else { pp = "aaaaaaaa"; all = true; }
            } else {
                for (int i = 0; i < n; i++) text[i] = (i % 2 == 0) ? 'a' : 'b';
                text[n] = 0;
                pp = "ababababab" "c";
            }
            int m = strLen(pp);
            int r1[MAXRES], r2[MAXRES];
            Stats sk, sn; long long bc;
            prefixFunction<char>(pp, m, pi, bc, eqChar);
            sk.buildCmp = bc;
            int c1 = kmpSearch<char>(text, n, pp, m, pi, r1, sk, eqChar, all);
            int c2 = naiveSearch<char>(text, n, pp, m, r2, sn, eqChar, all);
            bool same = c1 == c2;
            for (int i = 0; i < c1 && same; i++) if (r1[i] != r2[i]) same = false;
            if (!same) { cout << "РОЗБІЖНІСТЬ результатів!\n"; return 1; }
            string res = all ? to_string(c1) + " входж." : (c1 ? to_string(r1[0]) : "-1");
            cout << padL(to_string(n), 6) << "  " << pad(scName[sc], 40) << padL(all ? "усі" : "перше", 8)
                 << padL(res, 14) << padL(to_string(sn.cmp), 10) << padL(to_string(sk.cmp), 9)
                 << padL(to_string(bc), 9) << padL(to_string(sk.jumps), 8) << "\n";
        }
    }

    // ===== Варіант 19 =====
    cout << "\n=== Варіант 19 ===\n";
    static char vText[MAXTEXT], vPat[MAXTEXT];
    cout << "Введіть текст: ";
    bool got = readLine(vText, MAXTEXT);
    if (got) { cout << "\nВведіть зразок: "; got = readLine(vPat, MAXTEXT); }
    if (!got) {
        const char *dt = "patternpatternedpattern", *dp = "pattern";
        int i = 0; while ((vText[i] = dt[i])) i++;
        i = 0; while ((vPat[i] = dp[i])) i++;
        cout << "\n(введення недоступне, використано дані варіанта 19)";
    }
    int n = strLen(vText), m = strLen(vPat);
    cout << "\nТекст: \"" << vText << "\" (довжина " << n << "), зразок: \"" << vPat << "\" (довжина " << m << ")\n";
    long long bc;
    prefixFunction<char>(vPat, m, pi, bc, eqChar);
    cout << "Префікс-функція (порівнянь при побудові: " << bc << "):\n";
    printPrefix(vPat, pi, m);
    {
        int r1[MAXRES], r2[MAXRES], r3[MAXRES];
        Stats s1, s2, s3, s4;
        s1.buildCmp = s2.buildCmp = bc;
        int c1 = kmpSearch<char>(vText, n, vPat, m, pi, r1, s1, eqChar, false);
        int c2 = kmpSearch<char>(vText, n, vPat, m, pi, r2, s2, eqChar, true);
        int d1 = naiveSearch<char>(vText, n, vPat, m, r3, s3, eqChar, false);
        int d2 = naiveSearch<char>(vText, n, vPat, m, r3, s4, eqChar, true);
        (void)c1; (void)d1; (void)d2;
        cout << pad("Алгоритм", 12) << padL("Перше", 7) << padL("Усі входження", 16) << padL("Порівн. (перше)", 17)
             << padL("Порівн. (усі)", 15) << padL("Переходи", 10) << "\n";
        cout << pad("КМП", 12) << padL(to_string(r1[0]), 7) << padL(listStr(r2, c2), 16) << padL(to_string(s1.cmp), 17)
             << padL(to_string(s2.cmp), 15) << padL(to_string(s2.jumps), 10) << "\n";
        cout << pad("Прямий", 12) << padL(to_string(r3[0]), 7) << padL(listStr(r2, c2), 16) << padL(to_string(s3.cmp), 17)
             << padL(to_string(s4.cmp), 15) << padL(to_string(s4.pos), 6) << " поз.\n";
    }
    cout << "\nТрасування КМП (весь пошук усіх входжень у тексті варіанта):\n";
    {
        int r[MAXRES]; Stats s; s.buildCmp = bc;
        kmpSearch<char>(vText, n, vPat, m, pi, r, s, eqChar, true, true);
    }
    cout << "\nТрасування КМП для abababa / aba (накладені входження і переходи за префіксом):\n";
    {
        const char *t = "abababa", *p = "aba";
        int pi2[8]; long long c; int r[16]; Stats s;
        prefixFunction<char>(p, 3, pi2, c, eqChar);
        cout << "Префікс-функція: "; for (int i = 0; i < 3; i++) cout << (i ? ", " : "") << pi2[i]; cout << "\n";
        kmpSearch<char>(t, 7, p, 3, pi2, r, s, eqChar, true, true);
    }
    cout << "\nТрасування КМП для abababd / ababd (перехід за префіксом після невідповідності):\n";
    {
        const char *t = "abababd", *p = "ababd";
        int pi2[8]; long long c; int r[16]; Stats s;
        prefixFunction<char>(p, 5, pi2, c, eqChar);
        cout << "Префікс-функція: "; for (int i = 0; i < 5; i++) cout << (i ? ", " : "") << pi2[i]; cout << "\n";
        kmpSearch<char>(t, 7, p, 5, pi2, r, s, eqChar, false, true);
        Stats sn; int r2[16];
        naiveSearch<char>(t, 7, p, 5, r2, sn, eqChar, false);
        cout << "КМП: порівнянь " << s.cmp << ", переходів " << s.jumps << "; прямий пошук: порівнянь " << sn.cmp << "\n";
    }
    return 0;
}
