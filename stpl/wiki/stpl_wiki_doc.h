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
				typedef EntityT										                entity_type;
				typedef StringT										                string_type;
				typedef IteratorT 									                iterator;
				typedef typename Document<EntityT>::entity_iterator                 entity_iterator;
				typedef typename Document<EntityT>::container_type                  container_type;

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

				std::string to_html() {
					std::stringstream ss;
					ss << "<html>" << std::endl;
					ss << "<body>" << std::endl;
					auto nodes = this->children();
					for (auto it = nodes.begin(); it != nodes.end(); ++it) {
						ss << '(' << (*it)->get_id() << ") ";
						ss << (*it)->to_html();
						ss << std::endl;
					}
					ss << "</body>" << std::endl;
					ss << "</html>";
					return ss.str();
				}

				std::string to_json() {
					std::stringstream ss;
					auto nodes = this->children();
					for (auto it = nodes.begin(); it != nodes.end(); ++it) {
						ss << (*it)->to_html();
						ss << std::endl;
					}
					return ss.str();
				}				

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

				/**
				 * There is a char not recognised in the parser
				 */
				virtual EntityT* handle_char(IteratorT begin, IteratorT end) {
					return new Text<StringT, IteratorT>(begin, end);
				}				

			protected:

				virtual void on_new_child_entity(EntityT* entity_ptr, EntityT* child_entity) {
					// nothing yet, you may build up the relationship here
					child_entity->set_parent(entity_ptr);
					entity_ptr->add_child(child_entity);
				}

				virtual void on_child_entity_done(EntityT* entity_ptr, EntityT* child_entity) {
					// delete child_entity;
				}

				virtual EntityT* state_check(IteratorT& begin, EntityT* parent_ptr) {
					bool start_from_newline = false;

					IteratorT end = this->end();

					while (begin < end) {
						if (*begin == ' ')
							++begin;
						else if (*begin == '\n') {
							// ++begin; no we are not skipping new line as entity may need newline for boundary
							start_from_newline = true;
							break;
						}	   
						else
							break;
					}

					IteratorT it = begin, next;

					EntityT* entity_ptr = NULL;
					int previous_state = Scanner<EntityT>::state_;
					IteratorT pre_it;
					int new_entity_check_passed = -1;

					/**
					 * We only need the openings, and let the entity finish itself
					 * but for the text node, we won't be able to do so, so anything that is between those
					 * special nodes are text nodes
					 */
					while (it < end) {
						// skip white spaces
						while (*it == ' ' && it != end) {
							++it;
						}

						pre_it = it;

						switch (*it)
						{
						case ' ':
							break;
						case '\n':
							{
								if (parent_ptr && parent_ptr->isopen()) {
									// some nodes need new line to end
									return parent_ptr;
								}
								start_from_newline = true;
								// it has been handled
								// some entities need newline to end
								// for example list, there may be others add them here later
								// if (parent_ptr && parent_ptr->get_type() == LAYOUT_UL) {
								// 	++it;
								// 	parent_ptr->end(it);
								// 	parent_ptr->set_open(false);
								// 	return parent_ptr;
								// }

							}
							break;
						case '-':
							{
								next = it + 1;
								// confirmation
								if (*next == '{') {
									new_entity_check_passed = 1;
								}
							}
							break;								
						case WikiEntityConstants::WIKI_KEY_STYLE:
							{
								if (parent_ptr && parent_ptr->get_group() == STYLE && parent_ptr->isopen()) {
									// the style node needs to be closed now
									return parent_ptr;
								}
								else {
									next = it + 1;
									// confirmation
									if (*next == WikiEntityConstants::WIKI_KEY_STYLE) {
										new_entity_check_passed = 1;
									}
								}
							}
							break;
						case WikiEntityConstants::WIKI_KEY_STYLE_INDENT:
							{
								if (parent_ptr && parent_ptr->get_group() == LINK) {
									// there shouldn't be indent for a link type
									entity_ptr = parent_ptr;
								}
								else {
									pre_it = it > begin ? it - 1 : it;
									if (start_from_newline || *pre_it == '\n')
										new_entity_check_passed = 1;
									else
										entity_ptr = parent_ptr;
								}
							}
							break;													
						case WikiEntityConstants::WIKI_KEY_OPEN_TAG:
							start_from_newline = false;
							break;
						case WikiEntityConstants::WIKI_KEY_HEADING:
							// there won't be any text in front of a heading, WHO said that???????
							// This is most rediculous statement ever
							// 1) there could be text from the last section
							// 2) there could be text before the first section
							// new_entity_check_passed = true;
							// break;
							// {	
							// 	if (it > begin) {
							// 		entity_ptr = new Text<StringT, IteratorT>(begin, it);
							// 		entity_ptr->set_open(false);
							// 		entity_ptr->set_group(TEXT);
							// 		begin = it;
							// 	}
							// 	break;					
							// }
						case WikiEntityConstants::WIKI_KEY_OPEN_TEMPLATE:
						case WikiEntityConstants::WIKI_KEY_OPEN_LINK:
						// list might be inside an entity
						case WikiEntityConstants::WIKI_KEY_LIST:
						case WikiEntityConstants::WIKI_KEY_LIST_ORDERED:
							{	
								new_entity_check_passed = 1;
								start_from_newline = false;	
							}
							break;
						case WikiEntityConstants::WIKI_KEY_PROPERTY_DELIMITER:
							new_entity_check_passed = 1;
							start_from_newline = false;
							break;							
						case WikiEntityConstants::WIKI_KEY_CLOSE_LINK:
							new_entity_check_passed = 1;
							start_from_newline = false;
							if (parent_ptr && parent_ptr->get_group() == LINK) {
								next = it + 1;
								if (*next == WikiEntityConstants::WIKI_KEY_CLOSE_LINK) {
									parent_ptr->end(++next);
									parent_ptr->set_open(false);
									begin = next;
									return parent_ptr;
								}
							}
							break;
						case WikiEntityConstants::WIKI_KEY_CLOSE_TEMPLATE:
							new_entity_check_passed = 1;
							start_from_newline = false;
							next = it + 1;
							if (*next == WikiEntityConstants::WIKI_KEY_CLOSE_TEMPLATE) {
								if (parent_ptr && parent_ptr->get_group() == PROPERTY) {
									parent_ptr->end(it);
									parent_ptr->set_open(false);
									begin = it;
									return parent_ptr;
								}
								else if (parent_ptr && parent_ptr->get_group() == TBASE) {
									parent_ptr->end(++next);
									parent_ptr->set_open(false);
									begin = next;
									return parent_ptr;
								}
							}
							break;
						default:
							start_from_newline = false;
							break;
						}

						/**
						 * Before we go any further we need to handle the text node first
						 */
						if (!entity_ptr && new_entity_check_passed == 1 && it > begin) {
							entity_ptr = new Text<StringT, IteratorT>(begin, it);
							entity_ptr->set_open(false);
							entity_ptr->set_group(TEXT);
							begin = it;
						}

						if (!entity_ptr) {
							switch (*it)
							{
							case '-':
								{
									next = it + 1;
									// confirmation
									if (*next == '{') {
										entity_ptr = new LangVariant<StringT, IteratorT>(it, end);
										entity_ptr->set_group(LANG);
										entity_ptr->set_type(LANG_VARIANT);					
									}
								}
								break;								
							case WikiEntityConstants::WIKI_KEY_STYLE:
								{
									next = it + 1;
									// confirmation
									if (*next == WikiEntityConstants::WIKI_KEY_STYLE) {
										entity_ptr = new Style<StringT, IteratorT>(it, end);
										entity_ptr->set_group(STYLE);
										entity_ptr->set_type(STYLE_ITALIC);					
									}
								}
								break;
							case WikiEntityConstants::WIKI_KEY_STYLE_INDENT:
								{
									next = it + 1;
									// confirmation
									if (*next == WikiEntityConstants::WIKI_KEY_STYLE_INDENT) {
										entity_ptr = new StyleIndent<StringT, IteratorT>(it, end);
										entity_ptr = new Style<StringT, IteratorT>(it);
										entity_ptr->set_group(STYLE);
										entity_ptr->set_type(STYLE_INDENT);											
									}
								}
								break;																					
							case WikiEntityConstants::WIKI_KEY_OPEN_TAG:
								// at current stage, all tags will be ignored, it will be just part of TEXT node
		//						Scanner<EntityT>::state_ = TAG;
		//						entity_ptr = new ElemTag<StringT, IteratorT>(it, end);
								break;
							case WikiEntityConstants::WIKI_KEY_OPEN_TEMPLATE:
								next = it + 1;
								if (*next == '{') {
									Scanner<EntityT>::state_ = TBASE;
									entity_ptr = new Template<StringT, IteratorT>(it, end);

									entity_ptr->set_group(TBASE);
									entity_ptr->set_type(TEMPLATE);
								}
								else if (*next == '|') {
									Scanner<EntityT>::state_ = TBASE;
									entity_ptr = new Table<StringT, IteratorT>(it, end);

									entity_ptr->set_group(TBASE);
									entity_ptr->set_type(TABLE);
								}
								break;
							case WikiEntityConstants::WIKI_KEY_LIST:
								if (parent_ptr && parent_ptr->get_group() == LAYOUT_ITEM && parent_ptr->isopen()) {
									parent_ptr->end(it);
									parent_ptr->set_open(false);
									entity_ptr = parent_ptr;
								}							
								else if (parent_ptr && parent_ptr->get_type() == LAYOUT_UL) {
									entity_ptr = new ListItemUnordered<StringT, IteratorT>(it, end);
									// entity_ptr->set_group(LAYOUT);
									// entity_ptr->set_type(LAYOUT_UL);
									// parent_ptr->set_open(false);
									// return parent_ptr;
								}
								else {
									Scanner<EntityT>::state_ = LAYOUT;
									entity_ptr = new WikiEntityContainer<StringT, IteratorT>(it, end);
									entity_ptr->set_group(LAYOUT);
									entity_ptr->set_type(LAYOUT_UL);
								}
								break;
							case WikiEntityConstants::WIKI_KEY_LIST_ORDERED:
								{
									// could be redirect node
									if (*(it + 1) == 'R' && *(it + 2) == 'E' && *(it + 3) == 'D' && *(it + 4) == 'I' && *(it + 5) == 'R' && *(it + 6) == 'E' && *(it + 7) == 'C' && *(it + 8) == 'T') {
										entity_ptr = new Redirect<StringT, IteratorT>(it, end);
									}
									else if (*(it + 1) == 'D' && *(it + 2) == 'E' && *(it + 3) == 'B' && *(it + 4) == 'U' && *(it + 5) == 'G') {
										entity_ptr = new DebugNode<StringT, IteratorT>(it, end);
									}
									else {
										IteratorT next = it + 1;
										if (*next == ' '/* parent_ptr && parent_ptr->get_group() != PROPERTY */) {
											if (parent_ptr && parent_ptr->get_group() == LAYOUT_ITEM && parent_ptr->isopen()) {
												parent_ptr->end(it);
												parent_ptr->set_open(false);
												entity_ptr = parent_ptr;
											}
											else if (parent_ptr && parent_ptr->get_type() == LAYOUT_LI) {
												entity_ptr = new ListItemOrdered<StringT, IteratorT>(it, end);
											}
											else {
												Scanner<EntityT>::state_ = LAYOUT;
												entity_ptr = new WikiEntityOrdered<StringT, IteratorT>(it, end);
												entity_ptr->set_group(LAYOUT);
												entity_ptr->set_type(LAYOUT_LI);
											}
										}
									}
								}
															
								break;
							case WikiEntityConstants::WIKI_KEY_OPEN_LINK:
								//++it;
								//if (*it == WikiEntityConstants::WIKI_KEY_OPEN_LINK) {
									Scanner<EntityT>::state_ = LINK;

									// the specific type can be updated during the pasing
									entity_ptr = new Link<StringT, IteratorT>(it, end);
								//}
								break;
							case WikiEntityConstants::WIKI_KEY_HEADING:
								// It is not a heading if it has a parent
								// but it is not necessary to start with a newline as last section could go until
								// it sees a new HEADNING
								{
									IteratorT prev_it = this->begin_;
									if (it > prev_it)
										prev_it = it - 1;
									if (!start_from_newline && (prev_it == this->begin_ || *prev_it == '\n'))
										start_from_newline = true;

									if (start_from_newline && !parent_ptr) {
										Scanner<EntityT>::state_ = LAYOUT;
										entity_ptr = new LayoutLeveled<StringT, IteratorT>(it, end);
										entity_ptr->set_group(LAYOUT);
										entity_ptr->set_type(LAYOUT_HEADING);
										start_from_newline = false;
									}
								}
								break;
							case WikiEntityConstants::WIKI_KEY_PROPERTY_DELIMITER:
								if (parent_ptr) {
									if (parent_ptr->get_group() == PROPERTY) {
										// a property can't not be the paranet of another property
										parent_ptr->set_open(false);
										parent_ptr->end(it);
										// entity_ptr = new WikiProperty<StringT, IteratorT>(++it, end);
										// entity_ptr->set_parent(parent_ptr->get_parent());
										// previous_state = PROPERTY;
										entity_ptr = parent_ptr;
									}
									else if (parent_ptr->get_group() == TBASE) {
										// because | is for separator it can't be part of next entity
										begin = ++it;
										entity_ptr = new WikiProperty<StringT, IteratorT>(begin, end);
										previous_state = PROPERTY;
									}
								}
								break;								
							default:
								break;
							}

							if (new_entity_check_passed == 1 && !entity_ptr) {
								// need to uncheck the indicator
								new_entity_check_passed = 0;
							}
						}

						if (entity_ptr) {
							// found a new entity
							return entity_ptr;
						}

						// also as we couldn't find a new entity based on current character, and parent entity is not closed yet
						// so we return it
						else if (new_entity_check_passed == 0 &&  parent_ptr && parent_ptr->isopen()) {
							// otherwise, if something bad happened, it will get stuck
							begin = it;
							return parent_ptr;
						}
						
						// as we couldn't find a new entity based on current character, 
						// forward one character, otherwise we will be stuck here
						++it;						
					}

					// Anything that we cannot parse it is a text node
					if (it >= end && !entity_ptr && it > begin) {
						entity_ptr = new Text<StringT, IteratorT>(begin, end);
						entity_ptr->set_open(false);
						entity_ptr->set_group(TEXT);						
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

				typedef NRule<
								EntityT,
								DocumentT, Scanner<EntityT>
								>	ManyEntitiessRule;

			protected:
				void add_rules() {
					RuleT* rule_ptr = new RuleT(this->document_ptr_);
					rule_ptr->set_continue(true);
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
