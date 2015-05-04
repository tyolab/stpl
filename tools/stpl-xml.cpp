/******************************************************************************
 * This file is part of the Simple Text Processing Library(STPL).
 * (c) Copyright 2015 TYONLINE TECHNOLOGY PTY. LTD.
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

#include <stdio.h>
#include <string.h>

#include <iostream>
#include <string>

#include "../stpl/stpl_stream.h"
#include "../stpl/xml/stpl_xml.h"
#include "../utils/fs.h"

#ifndef VERSION
	#define VERSION "1.0"
#endif

using namespace FILESYSTEM;
using namespace std;
using namespace stpl;

void usage(const char *program) {
	fprintf(stderr, "stpl-xml - a simple XML value extraction tool (version: %s) from STPL (Simple Text Processing Library)\n", VERSION);
	fprintf(stderr, "\n");
	fprintf(stderr, "usage: %s xpath /a/path/to/xml/file\n", program); // [node:attr]
	fprintf(stderr, "          xpath - element[[#]][/child-element/...]:attr[=value]\n");
	exit(-1);
}

int main(int argc, char* argv[])
{
	if (argc < 3)
		usage(argv[0]);

	const char *file = argv[2];

	if (!File<>::exists(file)) {
		fprintf(stderr, "no such file: %s", file);
	}

	FileStream<string, char *> fs(file);
	string str = string(fs.begin(), fs.end());

	typedef XML::XParser<string, string::const_iterator> 		xml_parser;
	typedef xml_parser::document_type::entity_type	  			node_type;
	typedef xml_parser::document_type::entity_iterator 		entity_iterator;
	typedef xml_parser::document_type::element_type			element_type;
	xml_parser parser(str.begin(), str.end());
	parser.parse();

	xml_parser::document_type &doc = parser.doc();

	const char *xpath = argv[1];
	const char *pos = xpath;
	std::string first_tag;
	while (*pos != '\0' && *pos != '/' && *pos != '[' && *pos != ':')
		first_tag.push_back(*pos++);

	int index = -1;
	if (*pos == '[') {
		index = atoi(++pos);
		while (isdigit(*pos))
			++pos;
		if (*pos == ']')
			++pos; // ]
	}

	int count = 0;
	element_type::attribute_type *parsed_attr = NULL;
	const char *pos2 = NULL;
	const char *attr = NULL;
	string target_attr;
	string target_value;
	if (NULL != (attr = strchr(pos, ':'))) {
		++attr;
		pos2 = strchr(attr, '=');
		if (pos2) {
			parsed_attr = new element_type::attribute_type();
			string temp(attr);
			parsed_attr->match(temp.begin(), temp.end());
			target_attr = parsed_attr->name();
			target_value = parsed_attr->value();
			attr = NULL; // we are looking for the element with a particular attribute value
		}
		else {
			attr = pos;
			target_attr = attr;
		}
	}

	// allow non-stard xml file with no single document root
	entity_iterator	it;
	element_type *elem = NULL;
	for (it = doc.iter_begin(); it != doc.iter_end(); ++it) {
		node_type *node = static_cast<node_type*>((*it));

		if (node->is_element()) {
			++count;

			elem = static_cast<element_type *>(node);
			string name = elem->name();
			if (name == first_tag && (index == -1 || (count - 1) == index)) {
				if (*pos == '/') {
					element_type *d_elem = elem->get_descendent_node_by_xpath(++pos, target_attr, target_value);
					elem = d_elem;
				}
				else if (parsed_attr) {
					string value2 = elem->get_attribute(target_attr);
					if (target_value != value2)
						continue;
				}
				break;
			}
			else
				elem = NULL;
		}
	}
	if (elem) {
		if (attr) {
			if (elem->has_attribute(attr)) {
				string attr_str = elem->get_attribute(attr);
				cout << attr_str << endl;
			}
			else
				cerr << "can't find attribute \"" << attr << "\" for element \"" << elem->name() << "\"" << endl;
		}
		else
			cout << elem->text() << endl;
	}
	else
		fprintf(stderr, "could not find node for xpath: %s\n", xpath);
	return 0;
}
