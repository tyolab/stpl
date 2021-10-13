/*
	STRINGS.H
	-----
*/
#ifndef STRINGS_H_
#define STRINGS_H_

#include <string>
#include <sstream>

#include <string.h>
#include <stddef.h>

#ifdef __APPLE__
	#include <stdlib.h>
#endif

#ifdef _MSC_VER
	inline char *strlower(char *a) { return _strlwr(a); }
	inline char *strlower(const char *a) { return strlower((char *)a); }
	#define strnicmp _strnicmp
#else
	inline int strnicmp (const char *s1, const char *s2, size_t n) { return ::strncasecmp (s1, s2, n); }
#endif

namespace utils {

    std::string escape_quote(std::string& source) {
        std::stringstream ss;
        std::string::iterator pre, it = source.begin();
        int count = 0;
        while (it != source.end()) {
            if (*it == '"') {
                if (count == 0 || *pre != '\\')
                    ss << "\\\"";
            }
            else 
                ss << *it;
            ++count;
            pre = it++;
        }
        return ss.str();
    }

    static inline char *trimr(char *text) {
        char *to = text + strlen(text);
        if (isspace(*(to - 1)))
            to--;
        *to = '\0';
        return text;
    }
}
#endif  /* STRINGS_H_ */
