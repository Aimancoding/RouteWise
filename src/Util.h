// =============================================================================
//  Util.h -- small string helpers shared by all modules (no STL containers)
// =============================================================================
#ifndef ROUTEWISE_UTIL_H
#define ROUTEWISE_UTIL_H

#include <string>
#include <cctype>

inline std::string trimStr(const std::string& s) {
    size_t a = 0, b = s.length();
    while (a < b && std::isspace((unsigned char)s[a])) ++a;
    while (b > a && std::isspace((unsigned char)s[b - 1])) --b;
    return s.substr(a, b - a);
}

inline std::string toLowerStr(const std::string& s) {
    std::string r = s;
    for (size_t i = 0; i < r.length(); ++i) r[i] = (char)std::tolower((unsigned char)r[i]);
    return r;
}

// Splits "a|b|c" into f[0..n-1] (trimmed). Returns number of fields (<= maxF).
inline int splitFields(const std::string& line, std::string* f, int maxF, char delim = '|') {
    int n = 0;
    size_t start = 0;
    while (n < maxF) {
        size_t p = line.find(delim, start);
        if (p == std::string::npos) { f[n++] = trimStr(line.substr(start)); break; }
        f[n++] = trimStr(line.substr(start, p - start));
        start = p + 1;
    }
    return n;
}

#endif
