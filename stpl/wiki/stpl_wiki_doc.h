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
				typedef typename std::vector<EntityT* >             container_type;

			private:
				container_type                                  	nodes_;

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
				virtual ~WikiDoc() {
				}

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

		typedef WikiDoc<>	                    WikiDocument;
		typedef WikiDoc<std::string, char *>	WikiFile;

		template< typename EntityT = WikiDocument::entity_type >
		class WikiScanner: public Scanner< EntityT > {	

			private:										
				typedef	typename EntityT::string_type                                StringT;
				typedef typename EntityT::iterator	                                 IteratorT;		

			public:
				WikiScanner() : Scanner<EntityT>::Scanner() {
					Scanner<EntityT>::state_ = TEXT;
				}
				WikiScanner(IteratorT begin, IteratorT end) : Scanner<EntityT>::Scanner(begin, end) {
					Scanner<EntityT>::state_ = TEXT;
				}
				~WikiScanner() {

				}

				virtual void reset_state(EntityT* entity_ptr) {
					// set the new state
					Scanner<EntityT>::state_ = TEXT;
				}

			protected:

				virtual void on_new_child_entity(EntityT* entity_ptr, EntityT* child_entity) {
					// nothing yet, you may build up the relationship here
					child_entity->set_parent(entity_ptr);
				}

				virtual void on_child_entity_done(EntityT* entity_ptr, EntityT* child_entity) {
					// delete child_entity;
				}

				virtual EntityT* state_check(IteratorT begin, EntityT* parent_ptr) {
					// IteratorT begin = this->current();
					IteratorT it = begin;
					IteratorT end = this->end();
					EntityT* entity_ptr = NULL;
					int previous_state = Scanner<EntityT>::state_;

					/**
					 * We only need the openings, and let the entity finish itself
					 * but for the text node, we won't be able to do so, so anything that is between those
					 * special nodes are text nodes
					 */
					while (it != end) {
						switch (*it)
						{
						case BasicWikiEntity<StringT, IteratorT>::WIKI_KEY_OPEN_TAG:
							break;
						case BasicWikiEntity<StringT, IteratorT>::WIKI_KEY_OPEN_TEMPLATE:
						case BasicWikiEntity<StringT, IteratorT>::WIKI_KEY_LIST:
						case BasicWikiEntity<StringT, IteratorT>::WIKI_KEY_LIST_ORDERED:
						case BasicWikiEntity<StringT, IteratorT>::WIKI_KEY_OPEN_LINK:
						case BasicWikiEntity<StringT, IteratorT>::WIKI_KEY_HEADING: 
							{	
								if (previous_state == TEXT) {
									while (begin != end && isspace(*begin))
										++begin;
									if (it > begin) {
										entity_ptr = new Text<StringT, IteratorT>(begin, it);
										entity_ptr->set_group(TEXT);
									}
								}
								// if (parent_ptr && 
								// 	parent_ptr->get_group() == PROPERTY 
								// 	// && 
								// 	// *it != BasicWikiEntity<StringT, IteratorT>::WIKI_KEY_OPEN_TAG
								// 	) 
								else if (parent_ptr && parent_ptr->isopen()) {
									// ok, now we are encountering embeded nodes for the property
									// 
									// ok, we need to create a text child for the text before this sub node
									WikiEntity<StringT, IteratorT> *node_ptr = reinterpret_cast<WikiEntity<StringT, IteratorT> *>(parent_ptr);
									if (!node_ptr->get_last_child())
										node_ptr->create_text_child(it);
								}
	
								break;
							}
						case BasicWikiEntity<StringT, IteratorT>::WIKI_KEY_PROPERTY_DELIMITER:
							if (parent_ptr && parent_ptr->get_group() == TBASE) {
								entity_ptr = new WikiProperty<StringT, IteratorT>(++it, end);
								previous_state = PROPERTY;
							}
							break;							
						case BasicWikiEntity<StringT, IteratorT>::WIKI_KEY_CLOSE_LINK:
							if (parent_ptr && parent_ptr->get_group() == LINK) {
								++it;
								if (*it == BasicWikiEntity<StringT, IteratorT>::WIKI_KEY_CLOSE_LINK) {
									parent_ptr->end(++it);
									parent_ptr->set_open(false);
									return parent_ptr;
								}
							}
							break;
						case BasicWikiEntity<StringT, IteratorT>::WIKI_KEY_CLOSE_TEMPLATE:
							if (parent_ptr && parent_ptr->get_group() == TBASE) {
								++it;
								if (*it == BasicWikiEntity<StringT, IteratorT>::WIKI_KEY_CLOSE_TEMPLATE) {
									parent_ptr->end(++it);
									parent_ptr->set_open(false);
									return parent_ptr;
								}
							}
							break;
						default:
							break;
						}

						if (!entity_ptr) {

							switch (*it)
							{
							case BasicWikiEntity<StringT, IteratorT>::WIKI_KEY_OPEN_TAG:
								// at current stage, all tags will be ignored, it will be just part of TEXT node
		//						Scanner<EntityT>::state_ = TAG;
		//						entity_ptr = new ElemTag<StringT, IteratorT>(it, end);
								break;
							case BasicWikiEntity<StringT, IteratorT>::WIKI_KEY_OPEN_TEMPLATE:
								++it;
								if (*it == '{') {
									Scanner<EntityT>::state_ = TBASE;
									entity_ptr = new Template<StringT, IteratorT>(it - 1, end);

									entity_ptr->set_group(TBASE);
									entity_ptr->set_type(TEMPLATE);
								}
								else if (*it == '|') {
									Scanner<EntityT>::state_ = TBASE;
									entity_ptr = new Table<StringT, IteratorT>(it - 1, end);

									entity_ptr->set_group(TBASE);
									entity_ptr->set_type(TABLE);
								}
								break;
							case BasicWikiEntity<StringT, IteratorT>::WIKI_KEY_LIST:
								Scanner<EntityT>::state_ = LAYOUT;
								entity_ptr = new Layout<StringT, IteratorT>(it, end);
								entity_ptr->set_group(LAYOUT);
								entity_ptr->set_type(LAYOUT_UL);
								break;
							case BasicWikiEntity<StringT, IteratorT>::WIKI_KEY_LIST_ORDERED:
								Scanner<EntityT>::state_ = LAYOUT;
								entity_ptr = new LayoutOrdered<StringT, IteratorT>(it, end);
								entity_ptr->set_group(LAYOUT);
								entity_ptr->set_type(LAYOUT_LI);
								break;
							case BasicWikiEntity<StringT, IteratorT>::WIKI_KEY_OPEN_LINK:
								++it;
								if (*it == BasicWikiEntity<StringT, IteratorT>::WIKI_KEY_OPEN_LINK) {
									Scanner<EntityT>::state_ = LINK;

									// the specific type can be updated during the pasing
									entity_ptr = new Link<StringT, IteratorT>(it - 1, end);
								}
								break;
							case BasicWikiEntity<StringT, IteratorT>::WIKI_KEY_HEADING:
								// It is not a heading if it has a parent
								// or it is not a property
								if (!parent_ptr || parent_ptr->get_group() != PROPERTY) {
									Scanner<EntityT>::state_ = LAYOUT;
									entity_ptr = new LayoutLeveled<StringT, IteratorT>(it, end);
									entity_ptr->set_group(LAYOUT);
									entity_ptr->set_type(LAYOUT_HEADING);
								}
								// else {
								// 	// it could be part of a property entity

								// }
								break;
							default:
								break;
							}
						}

						if (entity_ptr) {
							// found a new entity
							return entity_ptr;
						}
						else {
							// as we couldn't find a new entity, and parent entity is not closed yet
							// so we return it
							if (parent_ptr && parent_ptr->isopen())
								return parent_ptr;							
						}
						++it;
					}
					return entity_ptr;
				}
		};

		template <
			 	typename DocumentT = WikiDocument,
		 		typename EntityT = typename DocumentT::entity_type,
			 	typename ScannerT = WikiScanner<EntityT>,
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

	}
}


#endif /* STPL_WIKI_DOC_H_ */
