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

#include <iostream>
#include <string>

#include "../stpl/stpl_stream.h"
#include "../stpl/xml/stpl_xml.h"
#include "../utils/fs.h"

#ifndef VERSION
	#define VERSION "1.0"
#endif

using namespace FILESYSTEM;

void usage(const char *program) {
	fprintf(stderr, "%s of STPL (Simple Text Processing Library) - a simple XML value extraction tool (version: %s)", program, VERSION);
	fprintf(stderr, "\n\n");
	fprintf(stderr, "usage: %s xpath[node:attr] /a/path/to/xml/file");
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
	return 0;
}
