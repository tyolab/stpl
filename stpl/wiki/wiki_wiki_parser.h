/*
 * wiki_doc.h
 *
 *  Created on: May 20, 2021
 *      Author: dev
 */

#ifndef STPL_WIKI_PARSER_H_
#define STPL_WIKI_PARSER_H_


#include <fstream>

#include "wiki_doc.h"

namespace stpl {
	namespace wiki {
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
				typedef typename DocumentT::tree_type 		tree_type;
				typedef typename DocumentT::element_type 	element_type;
				typedef DocumentT							document_type;
				typedef typename EntityT::string_type	 	string_type;
				typedef typename EntityT::iterator		 	iterator;

			protected:
				tree_type	tree_;


			public:
				WikiParser(IteratorT begin, IteratorT end) : Parser<GrammarT
																, DocumentT
																, EntityT
																, ScannerT
																>::Parser(begin, end) { }
				virtual ~WikiParser() {}

				tree_type& parse_tree(StringT& content) {
					this->parse_tree(content.begin(), content.end());
				}

				tree_type& parse_tree(IteratorT begin, IteratorT end) {
					init(begin, end);
					return this->parse_tree();
				}

				tree_type& parse_tree() {
					parse();
					root()->traverse(tree_);
					return tree_;
				}

				virtual DocumentT& parse() {
					Parser<GrammarT
							, DocumentT
							, EntityT
							, ScannerT
							>::parse();
					this->doc().reset();
					while (this->doc().more()) {
						typename DocumentT::entity_iterator it = this->doc().next();
						if ((*it)->type() == EntityT::element_type()) {
							this->doc().root(reinterpret_cast<element_type*>(*it));
							break;
						}
					}
					return this->doc();
				}

				element_type* root() { return this->doc().root(); }
		};

	}
}


#endif /* STPL_WIKI_PARSER_H_ */
