/*
 * wiki_doc.h
 *
 *  Created on: May 20, 2021
 *      Author: dev
 */

#ifndef STPL_WIKI_DOC_H_
#define STPL_WIKI_DOC_H_

#include <stpl_doc.h>
#include <stpl_scanner.h>
#include <stpl_grammar.h>
#include <stpl_parser.h>

#include "wiki_entity.h"

namespace stpl {
	namespace wiki {

		template<
				typename StringT = std::string,
				typename IteratorT = typename StringT::iterator,
				typename EntityT = BasicWikiEntity<StringT, IteratorT>,
				typename RootElemT = Element<
								typename EntityT::string_type,
								typename EntityT::iterator
								>
				>
		class WikiDoc :  public Document<EntityT> {
			public:
				typedef EntityT										entity_type;
				typedef StringT										string_type;
				typedef IteratorT 									iterator;
				typedef typename RootElemT::tree_type				tree_type;
				typedef RootElemT 									element_type;
				typedef typename Document<EntityT>::entity_iterator entity_iterator;

			private:
				//typedef	typename EntityT::string_type StringT;
				//typedef typename EntityT::iterator	IteratorT;

				RootElemT* 	root_;

			public:
				WikiDoc() : Document<EntityT>::Document() { init(); }
				WikiDoc(IteratorT it) : Document<EntityT>::Document(it) {
					 init();
				}
				WikiDoc(IteratorT begin, IteratorT end) :
					Document<EntityT>::Document(begin, end) {
					 init();
				}
				WikiDoc(StringT content) : Document<EntityT>::Document(content) {
					 init();
				}
				WikiDoc(StringT& content) : Document<EntityT>::Document(content) {
					 init();
				}
				virtual ~WikiDoc() {}

				RootElemT* root() { return root_; }
				void root(RootElemT* root) {
					root_ = root;
				}

				void write(std::string filename) {
					ofstream outfile (filename.c_str(),ofstream::binary);
					outfile << this->ref();
					outfile.close();
				}

				virtual void flush(int level=0) {
					//this->ref().erase();
					entity_iterator it;
					for (it = this->iter_begin(); it != this->iter_end(); ++it) {
						(*it)->flush(level);
						this->ref().append((*it)->ref());
					}
				}

				virtual StringT to_string() { return 0; }

			private:
				void init() {
					//debug
					// this->ref().append("<?xml version=\"1.0\" encoding=\"utf-8\"?>");
				}
		};

		typedef WikiDoc<>	WikiDocument;
		typedef WikiDoc<std::string, char *>	WikiFile;

		template <
			 	typename DocumentT = WikiDocument,
		 		typename EntityT = typename DocumentT::entity_type,
			 	typename ScannerT = Scanner<EntityT>,
			 	typename BaseRuleT = BaseRule<EntityT, DocumentT, ScannerT>
		 	 >
		class BasicWikiGrammar : public Grammar<DocumentT, EntityT, ScannerT, BaseRuleT> {
			public:
				typedef typename EntityT::string_type	 string_type;
				typedef typename EntityT::iterator		 iterator;

			private:
				typedef Rule<EntityT, DocumentT, ScannerT> RuleT;

				typedef Element<
								string_type,
								iterator,
								WikiNodeTypes<string_type, iterator>/*,
										0*/
								> TempElement;

				typedef InfoNode<
								string_type,
								iterator
								> TempInfoNode;

				typedef NRule<
								TempElement,
								DocumentT, Scanner<TempElement>,
								1
								>	OneRootElementRule;

				typedef NRule<
								TempInfoNode,
								DocumentT, Scanner<TempInfoNode>
								>	NInfoNodeRule;

			protected:
				void add_rules() {
					RuleT* rule_ptr = new RuleT(this->document_ptr_);
					rule_ptr->set_continue(true);
					rule_ptr->add_rule(reinterpret_cast<BaseRuleT*>(new NInfoNodeRule(this->document_ptr_)));
					rule_ptr->add_rule(reinterpret_cast<BaseRuleT*>(new OneRootElementRule(this->document_ptr_)));
					this->add(reinterpret_cast<BaseRuleT*>(rule_ptr));
				}

				void init() {
					add_rules();
				}

			public:
				BasicWikiGrammar(DocumentT*	document_ptr)  :
						Grammar<DocumentT, EntityT, ScannerT, BaseRuleT>::Grammar(document_ptr) { init(); }
				virtual ~BasicWikiGrammar() {}

		};

	}
}


#endif /* STPL_WIKI_DOC_H_ */
