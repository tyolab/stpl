/******************************************************************************
 * This file is part of the Simple Text Processing Library(STPL).
 * (c) Copyright 2021 TYONLINE TECHNOLOGY PTY. LTD.
 *
 * This file may be distributed and/or modified under the terms of the
 * GNU LESSER GENERAL PUBLIC LICENSE, Version 3 as published by the Free Software
 * Foundation and appearing in the file LICENSE.LGPL included in the
 * packaging of this file.
 *
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 *
 *******************************************************************************
 *
 * @author				Ling-Xiang(Eric) Tang
 *
 *******************************************************************************/

#ifndef STPL_UTILS_XML_H_
#define STPL_UTILS_XML_H_

#include <string>
#include <string.h>

namespace utils {

    inline void unescape_xml(std::string& source)
    {
        static const char *entities[] = {"&quot;", "&lt;", "&gt;", "&apos;", "&amp;" };
        static const char *to_chars[] = {"\"", "<", ">", "'", "&" };
        static const std::size_t num = sizeof(entities)/sizeof(entities[0]);

        std::size_t pos = 0;
        for (std::size_t i = 0; i < num; ++i) 
            {
            for (;(pos = source.find(entities[i]), pos) != std::string::npos;)
                {
                source.replace(pos, strlen(entities[i]), to_chars[i]);
                }
            pos = 0;
            }
    }

}

#endif /* STPL_UTILS_XML_H_ */
