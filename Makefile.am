
#AUTOMAKE_OPTIONS = foreign

stpl_basic_headers=\
				stpl/stpl_doc.h \
				stpl/stpl_entity.h \
				stpl/stpl_exception.h \
				stpl/stpl_grammar.h \
				stpl/stpl_keyword.h \
				stpl/stpl_log.h \
				stpl/stpl_othertraits.h \
				stpl/stpl_parser.h \
				stpl/stpl_property.h \
				stpl/stpl_rule.h \
				stpl/stpl_scanner.h \
				stpl/stpl_simple.h \
				stpl/stpl_stream.h \
				stpl/stpl_typetraits.h \
				stpl/stpl_unichar.h \
				stpl/stpl_unigrammar.h \
				stpl/stpl_uniword.h \
				stpl/stpl_word.h
stpl_basic_sources=

stpl_xml_headers= \
				stpl/xml/stpl_xml_basic.h \
				stpl/xml/stpl_xml_entity.h \
				stpl/xml/stpl_xml.h
stpl_xml_sources=			
				
stpl_html_headers = \
				stpl/html/stpl_html_entity.h \
				stpl/html/stpl_html.h
stpl_html_sources =
				
stpl_misc_headers = \
				stpl/misc/stpl_trec.h
stpl_misc_sources =	
			
stpl_characters_headers = \
				stpl/lang/stpl_character.h \
				stpl/lang/stpl_chinese.h \
				stpl/lang/stpl_unicode.h \
				stpl/lang/stpl_uscanner.h				
stpl_characters_sources =		

stpl_stpl_sources = \
				$(stpl_basic_headers) \
				$(stpl_basic_sources) 
#				$(stpl_xml_headers) \
#				$(stpl_xml_sources)			
#				$(stpl_html_headers) \
#				$(stpl_html_sources) \
#				$(stpl_misc_headers) \
#				$(stpl_misc_sources) \
#				$(stpl_characters_headers) \
#				$(stpl_characters_sources)
				
stpl_spec_headers = \
			spec/default.h \
			spec/property.h \
			spec/text.h
stpl_spec_sources = $(stpl_spec_headers)

stpl_utils_headers = \
			utils/charset_converter.h \
			utils/fs.h \
			utils/icstring.h \
			utils/archive.h
stpl_utils_sources = $(stpl_utils_headers)
			
#includedir = $(prefix)/include/stpl/utils
#includedir = $(prefix)/include/stpl/spec
#include_HEADERS = $(stpl_spec_sources)						
#includedir = $(prefix)/include/stpl/stpl
				
xmlincludedir = $(prefix)/include/stpl/stpl/xml
xmlinclude_HEADERS = \
				$(stpl_xml_headers) \
				$(stpl_xml_sources)
				
htmlincludedir = $(prefix)/include/stpl/stpl/html
htmlinclude_HEADERS = \
				$(stpl_html_headers) \
				$(stpl_html_sources)				

miscincludedir = $(prefix)/include/stpl/stpl/misc
miscinclude_HEADERS = \
				$(stpl_misc_headers) \
				$(stpl_misc_sources)
				
charactersincludedir = $(prefix)/include/stpl/stpl/characters
charactersinclude_HEADERS = \
				$(stpl_characters_headers) \
				$(stpl_characters_sources)			
				
includedir = $(prefix)/include/stpl
include_HEADERS = $(stpl_stpl_sources) \
							$(stpl_spec_sources) \
							$(stpl_utils_sources)

SUBDIRS = tools test

EXTRA_DIST = LICENSE.LGPL