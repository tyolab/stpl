/*
 * wiki_doc.h
 *
 *  Created on: May 20, 2021
 *      Author: dev
 */

#ifndef STPL_WIKI_PARSER_H_
#define STPL_WIKI_PARSER_H_


#include <fstream>

#include "stpl_wiki_doc.h"

namespace stpl {
	namespace WIKI {
		template<
					typename StringT = std::string,
					typename IteratorT = typename StringT::const_iterator,
					typename DocumentT = WikiDoc<StringT, IteratorT>,
					typename GrammarT = BasicWikiGrammar<DocumentT>,
		 			typename EntityT = typename DocumentT::entity_type,
					typename ScannerT = Scanner<EntityT>
				 >
		class WikiParser : public Parser<
									GrammarT
									, DocumentT
									, EntityT
									, ScannerT
								>{
			public:
				typedef DocumentT							document_type;
				typedef typename EntityT::string_type	 	string_type;
				typedef typename EntityT::iterator		 	iterator;

			public:
				WikiParser(IteratorT begin, IteratorT end) : Parser<GrammarT
																, DocumentT
																, EntityT
																, ScannerT
																>::Parser(begin, end) { }
				virtual ~WikiParser() {}

				virtual DocumentT& parse() {
					Parser<GrammarT
							, DocumentT
							, EntityT
							, ScannerT
							>::parse();
					return this->doc();
				}

		};

	}
}


#endif /* STPL_WIKI_PARSER_H_ */
