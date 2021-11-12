/*
	STRINGS.H
	-----
*/
#ifndef STRINGS_H_
#define STRINGS_H_

#include <string>
#include <sstream>
#include <algorithm>

#include <string.h>
#include <stddef.h>

#ifdef __APPLE__
	#include <stdlib.h>
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

    bool iccompare(const std::string& str1, const std::string& str2) {
            if (str1.size() != str2.size()) {
                return false;
            }
            for (string::const_iterator c1 = str1.begin(), c2 = str2.begin(); c1 != str1.end(); ++c1, ++c2) {
                if (tolower(static_cast<unsigned char>(*c1)) != tolower(static_cast<unsigned char>(*c2))) {
                    return false;
                }
            }
            return true;
    }    
}
#endif  /* STRINGS_H_ */
