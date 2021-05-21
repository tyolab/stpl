/*
 * wiki_doc.h
 *
 *  Created on: May 20, 2021
 *      Author: dev
 */

#ifndef STPL_WIKI_DOC_H_
#define STPL_WIKI_DOC_H_

#include "../stpl_doc.h"
#include "../stpl_scanner.h"
#include "../stpl_grammar.h"
#include "../stpl_parser.h"

#include "stpl_wiki_entity.h"

#include <vector>
namespace stpl {
	namespace WIKI {

		template<
				typename StringT = std::string,
				typename IteratorT = typename StringT::iterator,
				typename EntityT = BasicWikiEntity<StringT, IteratorT>
				>
		class WikiDoc :  public Document<EntityT> {
			public:
				typedef EntityT										entity_type;
				typedef StringT										string_type;
				typedef IteratorT 									iterator;
				typedef typename Document<EntityT>::entity_iterator entity_iterator;

			private:
				std::vector<EntityT> 	nodes_;

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

				void write(std::string filename) {
					ofstream outfile (filename.c_str(),ofstream::binary);
					outfile << this->ref();
					outfile.close();
				}

				virtual void flush(int level=0) {
					entity_iterator it;
					for (it = this->iter_begin(); it != this->iter_end(); ++it) {
						(*it)->flush(level);
						this->ref().append((*it)->ref());
					}
				}

				virtual StringT to_string() { return 0; }

			private:
				void init() {
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

				// typedef Layout<
				// 				string_type,
				// 				iterator,
				// 				WikiNodeTypes<string_type, iterator>
				// 				> TempLayout;

				// typedef Text<
				// 				string_type,
				// 				iterator
				// 				> TempTextNode;

				typedef NRule<
								EntityT,
								DocumentT, Scanner<EntityT>
								>	ManyEntitiessRule;

				// typedef NRule<
				// 				TempTextNode,
				// 				DocumentT, Scanner<TempTextNode>
				// 				>	NInfoNodeRule;

			protected:
				void add_rules() {
					RuleT* rule_ptr = new RuleT(this->document_ptr_);
					rule_ptr->set_continue(true);
					//rule_ptr->add_rule(reinterpret_cast<BaseRuleT*>(new NInfoNodeRule(this->document_ptr_)));
					rule_ptr->add_rule(reinterpret_cast<BaseRuleT*>(new ManyEntitiessRule(this->document_ptr_)));
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

		template< typename EntityT >
		class WikiScanner: public Scanner<EntityT> {	
		private:										
			typedef	typename EntityT::string_type StringT;
			typedef typename EntityT::iterator	IteratorT;									
			public:
				WikiScanner() {
					Scanner<EntityT>::Scanner();
					Scanner<EntityT>::state_ = TEXT;
				}
				WikiScanner(IteratorT begin, IteratorT end) {
					Scanner<EntityT>::Scanner(begin, end);
					Scanner<EntityT>::state_ = TEXT;
				}
				~WikiScanner() {

				}

			protected:
				virtual EntityT* state_check() {
					IteratorT it = this->current();
					IteratorT end = this->end();
					EntityT* entity_ptr = NULL;

					/**
					 * We only need the openings, and let the entity finish itself
					 */
					switch (*it)
					{
					case BasicWikiEntity<StringT, IteratorT>::WIKI_KEY_OPEN_TAG:
						Scanner<EntityT>::state_ = TAG;
						entity_ptr = new ElemTag<StringT, IteratorT>(it, end);;
						break;
					case BasicWikiEntity<StringT, IteratorT>::WIKI_KEY_OPEN_TEMPLATE:
						Scanner<EntityT>::state_ = TEMPLATE;
						entity_ptr = new TBase<StringT, IteratorT>(it, end);

						// entity_ptr->node_ = T_T;
						break;
					case BasicWikiEntity<StringT, IteratorT>::WIKI_KEY_OPEN_LIST:
						Scanner<EntityT>::state_ = LAYOUT;
						entity_ptr = new Layout<StringT, IteratorT>(it, end);
						entity_ptr->node_ = LAYOUT_UL;
						break;
					case BasicWikiEntity<StringT, IteratorT>::WIKI_KEY_OPEN_LIST_ORDERED:
						Scanner<EntityT>::state_ = LAYOUT;
						entity_ptr = new LayoutOrdered<StringT, IteratorT>(it, end);
						entity_ptr->node_ = LAYOUT_LI;
						break;
					case BasicWikiEntity<StringT, IteratorT>::WIKI_KEY_OPEN_LINK:
						Scanner<EntityT>::state_ = new Link<StringT, IteratorT>(it, end);

						// the specific type can be updated during the pasing
						entity_ptr = LINK_FREE;
						break;
					case BasicWikiEntity<StringT, IteratorT>::WIKI_KEY_OPEN_HEADING:
						Scanner<EntityT>::state_ = LAYOUT;
						entity_ptr = new Layout<StringT, IteratorT>(it, end);
						entity_ptr->node_ = LAYOUT_HEADING;
						break;
					default:
						break;
					}
					return entity_ptr;
				}
		};

	}
}


#endif /* STPL_WIKI_DOC_H_ */
