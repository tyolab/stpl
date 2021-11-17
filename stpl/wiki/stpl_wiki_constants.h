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
 * @twitter				https://twitter.com/_e_tang
 *
 *******************************************************************************/

#ifndef STPL_WIKI_CONSTANTS_H_
#define STPL_WIKI_CONSTANTS_H_

#include <map>
#include <iostream>

namespace stpl {
	namespace WIKI {
		
		// TEMPLATE node includes wiki declarations, text declarations,
		// If not any of others, then just TEXT
		enum WikiNodeGroup {
			GRUOP_NONE,
			LAYOUT, 
			LAYOUT_ITEM,
			TAG,
			LINK,
			TBASE,
			COMMENT,
			TEXT,
			REDIRECT,
			PROPERTY,
			CELL,
			STYLE,
			LANG,
			SECTION,
			DEBUG_NODE
		};

		// NONE for un-initialized node, or just TEXT
		// MISC node type includes COMMENT, PI or DOCTYPE (COMMENT)
		enum WikiNodeType {
			NONE,
			// Sub node of LANG
			LANG_VARIANT,

			// Sub node types of STYLE
			STYLE_ITALIC,
			STYLE_BOLD,
			STYLE_BOTH,
			STYLE_INDENT,

			// Sub node types of LAYOUT
			LAYOUT_SECTION, 
			LAYOUT_HEADING, 
			LAYOUT_HZ_RULE, 
			LAYOUT_T_LB,
			LAYOUT_T_LI,
			LAYOUT_T_UL,
			LAYOUT_LI,
			LAYOUT_UL,
			LAYOUT_DESC,

			// Sub node types of TABLE
			TABLE,

			// Sub node types of TEMPLATE
			TEMPLATE,
			TEMPLATE_PA,                         // pronuciation aids
			TEMPLATE_COLBEGIN, 	                // column begin
			TEMPLATE_COLEND,   					// column end
			TEMPLATE_DEFN, 						// definition / description lists

			// Sub node types of TAG
			TAG_REF,
			// Cite book, web, needed
			// attributes:
			// |isbn
			// |url
			// |title
			// |author
			// |first
			// |last
			// |location
			// |publisher
			// |date
			// |year
			// |accessdate

			// Templates
			TEMPLATE_COLOR,
			TEMPLATE_FONT_COLOR,
			TEMPLATE_FONT_STRIKE,

			TAG_NOWIKI,
			TAG_PRE,
			TAG_POEM,

			// Property Types
			P_LINK,
			P_PROPERTY,
			P_CELL,
			P_HEADER,
			P_ROW_HEADER,
			
			// Separator
			SEPARATOR,

			// Sub node types of LINK
			LINK_EXTERNAL = 100,
			LINK_P,
			LINK_REDIRECT,
			LINK_LANG,
			LINK_INTERWIKI,
			LINK_CATEGORY,
			LINK_T_ASOF,
			LINK_MEDIA,
			LINK_IMAGE			
		}; 

		class WikiEntityConstants {
			public:
				static const char WIKI_KEY_NEWLINE = '\n';
				static const char WIKI_KEY_OPEN_TAG = '<';
				static const char WIKI_KEY_STYLE = '\'';
				static const char WIKI_KEY_STYLE_INDENT = ':';
				static const char WIKI_KEY_CLOSE_TAG = '>';
				static const char WIKI_KEY_HEADING = '=';
				static const char WIKI_KEY_LIST = '*';
				static const char WIKI_KEY_LIST_ORDERED = '#';
				static const char WIKI_KEY_OPEN_TEMPLATE = '{';
				static const char WIKI_KEY_CLOSE_TEMPLATE = '}';
				static const char WIKI_KEY_OPEN_TABLE = '{';
				static const char WIKI_KEY_CLOSE_TABLE = '}';				
				static const char WIKI_KEY_OPEN_LINK = '[';
				static const char WIKI_KEY_CLOSE_LINK = ']';
				static const char WIKI_KEY_PROPERTY_DELIMITER = '|';
				static const char WIKI_KEY_SLASH = '/';
				static const char WIKI_KEY_OPEN_LANGVARIANT = '-';
				static const char WIKI_KEY_CLOSE_LANGVARIANT = '-';

				static const char *WIKI_KEY_CHARS_STYLE_INDENT;
				static const char *WIKI_KEY_CHARS_NEWLINE;
				static const char *WIKI_KEY_CHARS_STYLE;
				static const char *WIKI_KEY_CHARS_HEADING;
				static const char *WIKI_KEY_CHARS_OPEN_TEMPLATE;
				static const char *WIKI_KEY_CHARS_CLOSE_TEMPLATE;
				static const char *WIKI_KEY_CHARS_OPEN_TABLE;
				static const char *WIKI_KEY_CHARS_CLOSE_TABLE;
				static const char *WIKI_KEY_CHARS_OPEN_LINK;
				static const char *WIKI_KEY_CHARS_CLOSE_LINK;
				static const char *WIKI_KEY_CHARS_OPEN_LANGVARIANT;
				static const char *WIKI_KEY_CHARS_CLOSE_LANGVARIANT;
		};

		class WikiEntityVariables {
			public:
				static std::string protocol;
				static std::string path;
				static std::string host;

				static std::string html_head;

				static std::string link_category;
				static std::string link_file;
		};

		std::string WikiEntityVariables::host = "localhost";
		std::string WikiEntityVariables::protocol = "http";
		std::string WikiEntityVariables::path = "/";		

		std::string WikiEntityVariables::link_category = "Category";
		std::string WikiEntityVariables::link_file = "File";

		std::string WikiEntityVariables::html_head = "";

		const char *WikiEntityConstants::WIKI_KEY_CHARS_STYLE_INDENT = ":";
		const char *WikiEntityConstants::WIKI_KEY_CHARS_NEWLINE = "\n";
		const char *WikiEntityConstants::WIKI_KEY_CHARS_STYLE = "\'";
		const char *WikiEntityConstants::WIKI_KEY_CHARS_HEADING = "=";
		const char *WikiEntityConstants::WIKI_KEY_CHARS_OPEN_TEMPLATE = "{";
		const char *WikiEntityConstants::WIKI_KEY_CHARS_CLOSE_TEMPLATE = "}";
		const char *WikiEntityConstants::WIKI_KEY_CHARS_OPEN_TABLE = "{|";
		const char *WikiEntityConstants::WIKI_KEY_CHARS_CLOSE_TABLE = "|}";				
		const char *WikiEntityConstants::WIKI_KEY_CHARS_OPEN_LINK = "[";
		const char *WikiEntityConstants::WIKI_KEY_CHARS_CLOSE_LINK = "]";
		const char *WikiEntityConstants::WIKI_KEY_CHARS_OPEN_LANGVARIANT = "-{";
		const char *WikiEntityConstants::WIKI_KEY_CHARS_CLOSE_LANGVARIANT = "}-";		
	}

}
#endif /*STPL_WIKI_CONSTANTS_H_*/
