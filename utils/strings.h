/*
	STRINGS.H
	-----
*/
#ifndef STRINGS_H_
#define STRINGS_H_

#include <vector>
#include <string>
#include <sstream>
#include <algorithm>

#include <string.h>
#include <stddef.h>

#ifdef __APPLE__
	#include <stdlib.h>
#endif

namespace utils {

    inline bool iccompare(const std::string& str1, const std::string& str2) {
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

    inline bool ends_with(const std::string& str, const std::string& suffix) {
        if (str.size() < suffix.size())
            return false;
        std::string last_segment = str.substr(str.size() - suffix.size());
        return iccompare(last_segment, suffix);
    }    

    inline bool ends_with(const std::string& str, const std::vector<std::string>& suffix) {
        for (std::vector<std::string>::const_iterator it = suffix.begin(); it != suffix.end(); ++it) {
            if (ends_with(str, *it)) {
                return true;
            }
        }
        return false;
    }

    inline std::string escape_quote(std::string& source) {
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
