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

			private:
				std::vector<EntityT *> 												templates_;
				std::vector<EntityT *> 												templates2_;  // templates at the end, no need to show
				std::vector<EntityT *> 												images_;      // image file in the beginning, no need to show
				std::vector<EntityT *> 												categories_;
				std::vector<EntityT *>                                              sections_;
				EntityT                                                             *redirect_;		
				bool                                                                organized_;		

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
					auto it = sections_.begin();
					if (it != sections_.end()) {
						delete *it;
						++it;
					}
					sections_.clear();
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
					organize();
					std::stringstream ss;
					
					if (redirect_) {
						ss << "<meta http-equiv='refresh' content='0;url=\"/" << redirect_->to_std_string() << "\'" << ">" << std::endl;
						return ss.str();
					}

					ss << "<html>" << std::endl;
					ss << "<head>" << std::endl;
					ss << WikiEntityVariables::html_head << std::endl;
					ss << "</head>" << std::endl;
					ss << "<body>" << std::endl;

					for (auto it = templates_.begin(); it != templates_.end(); ++it) {
						ss << (*it)->to_html() << std::endl;
					}

					int count = 0;
					WikiSection<string_type, iterator> *last_section = NULL;
					auto it = sections_.begin();
					if (it != sections_.end()) 
					do {
						WikiSection<string_type, iterator> *section = (WikiSection<string_type, iterator> *)(*it);
						// WikiSection<string_type, iterator> *next_section = NULL;
						
						// auto next = it + 1;
						// if (next != sections_.end()) {
						// 	next_section = (WikiSection<string_type, iterator> *)(*next);
						// }

						if (count > 0 && section->get_level() <= 2)
							ss << "</section>" << std::endl;

						if (section->get_level() <= 2) {
							ss << "<section id=\"" << count << "\">" << std::endl;
							if (section->get_level() >= 2)
								ss << "<div class=\"row\"><h2 class=\"section-title\">" << section->get_line() << "</h2></div>" << std::endl;
							if (section->size() > 0) {
								ss << "<div class=\"row\">" << std::endl;
								ss << section->to_html() << std::endl;
								ss << "</div>" << std::endl;
							}
							++count;
						}
						else {	
							int level = section->get_level();
							ss << "<div class=\"row\"><h" << level << ">" << section->get_line() << "</h" << level << ">" << std::endl;
							ss << section->to_html() << std::endl;
							ss << "</div>" << std::endl;
						}
						ss << std::endl;
						last_section = section;
						++it;
					} while (it != sections_.end());

					if (count > 0)
						ss << "</section>" << std::endl;	
					ss << "</body>" << std::endl;
					ss << "</html>";
					return ss.str();
				}

				/**
				 * @brief for indexing
				 * 
				 * @return std::string 
				 */
				std::string to_trec(int id) {
					organize();
					std::stringstream ss;
					ss << "<DOC>" << std::endl;
					ss << "<DOCNO>" << id << "</DOCNO>" << std::endl;
					// categories
					ss << "<CATEGORIES>" << std::endl;
					for (auto it = categories_.begin(); it != categories_.end(); ++it) {			
						ss << "<CATEGORY>" << std::endl;
						ss << (*it)->to_text();
						ss << "<CATEGORY>" << std::endl;
					}
					ss << "</CATEGORIES>" << std::endl;

					ss << "<TEXT>" << std::endl;
					for (auto it = sections_.begin(); it != sections_.end(); ++it) {		
						ss << (*it)->to_text();
						ss << std::endl;
					}
					ss << "</TEXT>" << std::endl;

					ss << "<DOC>" << std::endl;
					return ss.str();
				}

				std::string to_json(std::string extras = "") {
					organize();

					std::stringstream ss;

					ss << "{" << std::endl;

					if (redirect_) {
						ss << "\"redirect\": \"" << redirect_->to_std_string() << "\"," << std::endl;
					}
					else {
						ss << "\"article\": {" << std::endl;

						int count = 0;
						// template
						ss << "\"templates\": [" << std::endl;
						count = 0;
						for (auto it = templates_.begin(); it != templates_.end(); ++it) {
							std::string link = (*it)->to_json();
							if (count > 0) 
								ss << "," << std::endl;	

							if (link.length() > 0) {
								ss << link;
								ss << std::endl;
								++count;
							}
						}
						ss << "]," << std::endl;

						// article section
						ss << "\"sections\": [" << std::endl;
						count = 0;
						for (auto it = sections_.begin(); it != sections_.end(); ++it) {
							if (count > 0) 
								ss << "," << std::endl;				
							ss << (*it)->to_json();
							ss << std::endl;
							++count;
						}
						ss << "]," << std::endl;

						// data
						ss << "\"templates2\": [" << std::endl;
						count = 0;
						for (auto it = templates2_.begin(); it != templates2_.end(); ++it) {
							if (count > 0) 
								ss << "," << std::endl;				
							ss << (*it)->to_json();
							ss << std::endl;
							++count;
						}
						ss << "]," << std::endl;

						// categories
						ss << "\"categories\": [" << std::endl;
						count = 0;
						for (auto it = categories_.begin(); it != categories_.end(); ++it) {
							if (count > 0) 
								ss << "," << std::endl;				
							ss << (*it)->to_json();
							++count;
						}
						ss << "]" << std::endl;

						// images
						if (images_.size() > 0) {
							count = 0;
							ss << "," << std::endl;
							ss << "\"image\": [" << std::endl;
							for (auto it = images_.begin(); it != images_.end(); ++it) {
								if (count > 0) 
									ss << "," << std::endl;				
								ss << (*it)->to_json();
								ss << std::endl;
								++count;
							}
							ss << "]" << std::endl;
						}												
					}
					ss << "}" << std::endl;

					if (extras.size() > 0) {
						ss << "," << std::endl;
						ss << "extras: " << extras << std::endl;
						ss << extras;
					}
					ss << "}" << std::endl;

					return ss.str();
				}

				bool isorganized() const {
					return organized_;
				}

				void set_organized(bool organized) {
					organized_ = organized;
				}

			private:
				void clear_sections() {
					auto it = sections_.begin();
					if (it != sections_.end()) {
						delete *it;
						++it;
					}
					sections_.clear();
				}

				void clear_templates() {
					auto it = templates_.begin();
					if (it != templates_.end()) {
						delete *it;
						++it;
					}
					templates_.clear();
				}				

				void organize() {
					if (organized_)
						return;

					auto nodes = this->children();
					bool ind = true;
					int count = 0;
					auto it = nodes.begin();

					if (it != nodes.end() && (*it)->get_group() == REDIRECT) {
						++it;
						if (it != nodes.end()) 
							redirect_ = (EntityT *)(*it);
					    // should be a link
					    redirect_->set_type(LINK_REDIRECT);
						return;
					}

					WikiSection<StringT, IteratorT> *section = NULL;
					for (; it != nodes.end(); ++it) {
						EntityT *entity_ptr = *it;
						if (!section) {
							if (entity_ptr->get_group() == TBASE && entity_ptr->get_type() == TEMPLATE) {
								templates_.push_back(entity_ptr);
								continue;
							}
							else if (entity_ptr->get_group() == LINK) {
								if (entity_ptr->get_type() == LINK_IMAGE)
									images_.push_back(entity_ptr);
								else {
									static const std::vector<std::string> suffix_array = {
										".jpg", ".jpeg", ".png", ".svg", ".jiff"
									};
									Link<StringT, IteratorT> *link = (Link<StringT, IteratorT> *)entity_ptr;
									if (link->children().size() > 0) {
										auto child = link->children().begin();
										std::string link_text = (*child)->to_std_string();
										if (utils::ends_with(link_text, suffix_array)) {
											templates_.push_back(*child);
										}
									}
								}
								continue;
							}
							
						}

						if (!section || (LAYOUT == entity_ptr->get_group() && LAYOUT_HEADING == entity_ptr->get_type())) {
							if (section) 
								section->end(entity_ptr->begin());
							section = new WikiSection<StringT, IteratorT>(entity_ptr->begin(), this->end());
							sections_.push_back(section);

							section->set_id(count++);

							// the first child if it is not a layout
							if (LAYOUT != entity_ptr->get_group() && LAYOUT_HEADING != entity_ptr->get_type()) {
								section->set_level(0);
								section->add(entity_ptr);
							}
							else {
								LayoutLeveled<StringT, IteratorT> *layout_ptr = reinterpret_cast<LayoutLeveled<StringT, IteratorT> *>(entity_ptr);
								section->set_level(layout_ptr->get_level());

								// need to convert to html
								section->set_line(layout_ptr->to_html());
							}
						}
						else {
							section->add(entity_ptr);
						}
					}

					// last section
#ifdef DEBUG
					if (!section) {
						std::cout << "last section is null!!! " << std::endl;
					}
#endif // DEBUG					
					if (section && section->children().size() > 0) {
						it = section->children().end() - 1;
						int last_count = 0;
						while (it != section->children().begin()) {
							int group = (*it)->get_group();
							if (group == TEXT && (*it)->is_empty()) {
								--it;
								continue;
							}
							else if (group == LINK || group == TBASE) {
								int type = (*it)->get_type();
								if (type == LINK_CATEGORY)
									categories_.push_back(*it);
								else if (type == TEMPLATE)
									templates2_.push_back(*it);
							}
							else
								break;
							++last_count;
							--it;
						}
						++it;
						if (it < section->children().end()) {
							section->children().erase(it, section->children().end());
						}
					}

					organized_ = true;
				}

				void init() {
					organized_ = false;
					redirect_ = NULL;
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
					entity_ptr->process_child(child_entity);
				}

				virtual void on_child_entity_done(EntityT* entity_ptr, EntityT* child_entity) {
					// delete child_entity;
				}

				virtual EntityT* state_check(IteratorT& begin, EntityT* parent_ptr) {
					bool start_from_newline = false;

					IteratorT end = this->end();

					/* leave the space as text node
					   and let the node deal with newline */

					IteratorT it = begin, next;

					EntityT* entity_ptr = NULL;
					int previous_state = Scanner<EntityT>::state_;
					IteratorT pre_it;
					int new_entity_check_passed = 0;
					int new_entity_start = -1;

					/**
					 * @brief special treatment for 
					 * 
					 */
					if (!isspace(*it) && parent_ptr && (parent_ptr->get_group() == LAYOUT_LIST || parent_ptr->get_group() == LIST_ITEM)) {
						if (parent_ptr->is_end(it, false))
							return parent_ptr;
					}

					/**
					 * We only need the openings, and let the entity finish itself
					 * but for the text node, we won't be able to do so, so anything that is between those
					 * special nodes are text nodes
					 */
					while (it < end) {
						// skip white spaces
						// actually we cannot let the node deal with spaces
						// space will confuse nodes
						while (*it == ' ' && it != end) {
							++it;
						}
						begin = it;

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
									new_entity_start = 1;
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
										new_entity_start = 1;
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
									pre_it = it;
									while (pre_it > this->begin_) {
										--pre_it;
										if (*pre_it != ' ') {
											break;
										}
									}
									if (start_from_newline || *pre_it == '\n') {
										start_from_newline = true;
										new_entity_check_passed = 1;
										new_entity_start = 1;
									}
									// so it is a text node, let the parent decide what child it should be
									// else
									//	entity_ptr = parent_ptr;
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
							{
								IteratorT pre = it - 1;
								while (pre > this->begin_ && *pre == ' ')
								{
									/* code */
									--pre;
								}
								if (*pre == '\n') {
									start_from_newline = true;
									new_entity_check_passed = 1;
									new_entity_start = 1;
								}
							}
							
							break;
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
								new_entity_start = 1;
								start_from_newline = false;	
							}
							break;
						case WikiEntityConstants::WIKI_KEY_PROPERTY_DELIMITER:
							new_entity_check_passed = 1;
							new_entity_start = 1;
							start_from_newline = false;
							break;							
						case WikiEntityConstants::WIKI_KEY_CLOSE_LINK:
							{
								new_entity_check_passed = 1;
								new_entity_start = 0;
								start_from_newline = false;
								if (parent_ptr) {
									if (parent_ptr->get_type() == P_LINK) {
										parent_ptr->end(it);
										parent_ptr->set_open(false);
										begin = it;
										return parent_ptr;
									}
									else if (parent_ptr->get_group() == LINK) {
										// next = it + 1;
										// if (*next == WikiEntityConstants::WIKI_KEY_CLOSE_LINK) {
										// 	parent_ptr->end(++next);
										// 	parent_ptr->set_open(false);
										// 	begin = next;
										// 	return parent_ptr;
										// }
										return parent_ptr;
									}
								}
							}
							break;
						case WikiEntityConstants::WIKI_KEY_CLOSE_TEMPLATE:
							if (parent_ptr && parent_ptr->is_end(it, false)) {
								return parent_ptr;
							}
							else {
								new_entity_check_passed = 1;
								new_entity_start = 0;
								start_from_newline = false;
								next = it + 1;
								if (*next == WikiEntityConstants::WIKI_KEY_CLOSE_TEMPLATE) {
									if (parent_ptr && parent_ptr->get_group() == PROPERTY && parent_ptr->get_type() == P_PROPERTY) {
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
							}
							break;
						default:
							start_from_newline = false;
							break;
						}

						/**
						 * Before we go any further we need to handle the text node first
						 */
						if (new_entity_check_passed == 1 && new_entity_start == 1) {
							if (parent_ptr && !parent_ptr->should_have_children()) {
								parent_ptr->end(it);
								parent_ptr->set_open(false);
								return parent_ptr;
							}
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
									// else
									// 	new_entity_check_passed = 0;
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
									//next = it + 1;
									// confirmation
									if (start_from_newline) {
										entity_ptr = new StyleIndent<StringT, IteratorT>(it, end);
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
								}
								else if (*next == '|') {
									Scanner<EntityT>::state_ = TBASE;
									entity_ptr = new Table<StringT, IteratorT>(it, end);
								}
								break;
							case WikiEntityConstants::WIKI_KEY_LIST: 
								{
									int levels = 1;
									IteratorT next = it + 1;

									while (*next == WikiEntityConstants::WIKI_KEY_LIST)
										++next, ++levels;
									if (parent_ptr && parent_ptr->get_group() == LIST_ITEM) {
										// either same level or higher
										if (levels <= parent_ptr->get_level()) {
											parent_ptr->end(it);
											parent_ptr->set_open(false);

											// parent of parent is a list
											// this is a list item
											// parent of parent not necessary closes yet
											if (levels < parent_ptr->get_level() || parent_ptr->get_type() != LAYOUT_UL) {
												EntityT *new_parent_ptr = parent_ptr->get_parent();
												if (new_parent_ptr) {
													new_parent_ptr->end(it);
													new_parent_ptr->set_open(false);
												}
											}
											return parent_ptr;
										}
										else {
											// nested list
											entity_ptr = new LayoutUnorderedList<StringT, IteratorT>(it, end);
											entity_ptr->set_level(levels);
										}									
									}							
									else if (parent_ptr && parent_ptr->get_group() == LAYOUT_LIST && parent_ptr->get_type() == LAYOUT_UL) {
										if (parent_ptr->get_level() == levels) {
											entity_ptr = new ListItemUnordered<StringT, IteratorT>(it, end);
											// entity_ptr->set_level(levels);
										}
										else if (parent_ptr->get_level() < levels) {
											// nested list
											entity_ptr = new LayoutUnorderedList<StringT, IteratorT>(it, end);
											entity_ptr->set_level(levels);
										}
										else {
											parent_ptr->end(it);
											parent_ptr->set_open(false);
											return parent_ptr;
										}
									}
									else {
										Scanner<EntityT>::state_ = LAYOUT;
										entity_ptr = new LayoutUnorderedList<StringT, IteratorT>(it, end);
										entity_ptr->set_level(levels);
									}
								}
								break;
							case WikiEntityConstants::WIKI_KEY_LIST_ORDERED:
								{
									// could be redirect node
									if (*(it + 1) == 'R' && *(it + 2) == 'E' && *(it + 3) == 'D' && *(it + 4) == 'I' && *(it + 5) == 'R' && *(it + 6) == 'E' && *(it + 7) == 'C' && *(it + 8) == 'T') {
										entity_ptr = new Redirect<StringT, IteratorT>(it, end);
									}
									else {
										IteratorT next = it + 1;
										int levels = 1;

										while (*next == WikiEntityConstants::WIKI_KEY_LIST_ORDERED)
											++next, ++levels;
										//if (*next == ' '/* parent_ptr && parent_ptr->get_group() != PROPERTY */) {
											if (parent_ptr && parent_ptr->get_group() == LIST_ITEM/*  && parent_ptr->isopen() */) {

												// either same level or higher
												if (levels <= parent_ptr->get_level()) {
													parent_ptr->end(it);
													parent_ptr->set_open(false);

													// parent of parent is a list
													// this is a list item
													// parent of parent not necessary closes yet
													if (levels < parent_ptr->get_level() || parent_ptr->get_type() != LAYOUT_OL) {
														EntityT *new_parent_ptr = parent_ptr->get_parent();
														if (new_parent_ptr) {
															new_parent_ptr->end(it);
															new_parent_ptr->set_open(false);
														}
													}
													return parent_ptr;
												}
												else {
													// nested list
													entity_ptr = new LayoutOrderedList<StringT, IteratorT>(it, end);
												}													
											}
											else if (parent_ptr && parent_ptr->get_group() == LAYOUT_LIST && parent_ptr->get_type() == LAYOUT_OL) {
												if (parent_ptr->get_level() == levels) {
													entity_ptr = new ListItemOrdered<StringT, IteratorT>(it, end);
													entity_ptr->set_level(levels);
												}
												else if (parent_ptr->get_level() < levels) {
													// nested list
													entity_ptr = new LayoutOrderedList<StringT, IteratorT>(it, end);
													entity_ptr->set_level(levels);
												}
												else {
													parent_ptr->end(it);
													parent_ptr->set_open(false);
													return parent_ptr;
												}												
											}
											else {
												Scanner<EntityT>::state_ = LAYOUT_LIST;
												entity_ptr = new LayoutOrderedList<StringT, IteratorT>(it, end);
												entity_ptr->set_level(levels);
											}
										// }
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
									if (parent_ptr) {
										if (parent_ptr->get_group() == PROPERTY) {
											if (parent_ptr->get_type() == P_HEADING) {
												// time for the end
												// parent_ptr->end(it);
												// parent_ptr->set_open(false);
												// let it finish it
												return parent_ptr;
											}
											// if text contains = at the begining
											// created end loop
											// because we added create child function at the end
											// so probably we don't need ti
											// return parent_ptr;
										}
										else if (parent_ptr->get_type() == LAYOUT_HEADING) {
											// time for the end
											return parent_ptr;
										}										
									}
																		
									IteratorT prev_it = this->begin_;
									if (it > prev_it)
										prev_it = it - 1;
									if (!start_from_newline && (prev_it == this->begin_ || *prev_it == '\n'))
										start_from_newline = true;

									if (start_from_newline) {
										if (!parent_ptr) {
											Scanner<EntityT>::state_ = LAYOUT;
											entity_ptr = new LayoutLeveled<StringT, IteratorT>(it, end);
											start_from_newline = false;
										}
										else {
											// parent supposed to be null already already
											parent_ptr->end(it);
											parent_ptr->set_open(false);
											return parent_ptr;
										}
									}
								}
								break;
							case WikiEntityConstants::WIKI_KEY_PROPERTY_DELIMITER:
								{
									if (parent_ptr) {
										if (parent_ptr->get_group() == LINK) {
											begin = it + 1;
											entity_ptr = new CommonChildEntity<StringT, IteratorT>(it + 1, end);
										}
										else if (parent_ptr->get_group() == PROPERTY || parent_ptr->get_group() == CELL) {
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
											next = it + 1;
											if (parent_ptr->get_type() == TABLE) {
												if (*next == '}') {
													begin = it;
													return parent_ptr;
												}
												else {
													if (*next == '|' || *next == '!') {
														// let parent handle it
														// as it means that it is a cell in the same line, table needs to adjust cell id etc.
														return parent_ptr;
													}
													IteratorT pre = it - 1;
													while (*pre == ' ')
														--pre;
													// need to check the position of the cell, it is kinda of hack
													if (*pre == '\n') {
														// might be eaten by previous entity
														parent_ptr->is_pause(pre);
													}
													entity_ptr = new TableCell<StringT, IteratorT>(next, end);
													previous_state = CELL;
													begin = next;
												}
											}
											else if (parent_ptr->get_type() == TEMPLATE) {
												entity_ptr = new WikiProperty<StringT, IteratorT>(next, end);
												previous_state = PROPERTY;
												begin = next;
											}
											else
												throw new runtime_error("Invalid TBASE type");
										}
										else {
											// since we come down here, it much be a separator for either template or table, or link
											EntityT *parent_parent_ptr = parent_ptr->get_parent();
											if (parent_parent_ptr && parent_parent_ptr->is_end(it)) {
												parent_ptr->set_open(false);
												parent_ptr->end(it);
												return parent_ptr;
											}
										}
									}
								}
								break;	
							case '$':
								if (*(it + 1) == 'D' && *(it + 2) == 'E' && *(it + 3) == 'B' && *(it + 4) == 'U' && *(it + 5) == 'G') {
										entity_ptr = new DebugNode<StringT, IteratorT>(it, end);
									}
								break;															
							default:
								break;
							}
						}

						if (entity_ptr) {
							// found a new entity
							return entity_ptr;
						}

						// also as we couldn't find a new entity based on current character, and parent entity is not closed yet
						// so we return it
						else if (parent_ptr && parent_ptr->isopen()) {
							// we are not skip here here we mainly check the state
							// otherwise, if something bad happened, it will get stuck
							// no key char found, things are much simpler
							if (new_entity_check_passed == 0 || (new_entity_check_passed == 1 && new_entity_start == 1)) {
								begin = it;
								if (parent_ptr->should_have_children()) {
									// none of obvious entity is found, so let take it as a text node
									// there could be a few cases here
									// 1. parent has started with text but not in text node
									// this is handled in create text pre function
									// 2. parent has pushed child(ren) in
									// this is in the middle of it
									// 3. if parent has no child(ren) yet
									// this is the first one, so it would hurt either
									EntityT *text_ptr = parent_ptr->create_child(begin, end);
									text_ptr->set_parent(parent_ptr);							
									return text_ptr;
								}
								else
									return parent_ptr;
							}
							else {
								if (new_entity_check_passed == 1 && new_entity_start == 0) {
									// so it is a closed key char then
									// what are we gonna do? 
									// if it is a not proper written text, it could be meant for the parent
									// close the parent? let the parent eat it?
									// if create a new entity, it will come back, as it is an end
									if (parent_ptr && parent_ptr->is_end(it)) {
										parent_ptr->set_open(false);
										parent_ptr->end(it);
										return parent_ptr;
									}
									// else								
									// we are gonna let them be eaten
								}
							}
						}
						
						// as we couldn't find a new entity based on current character, 
						// forward one character, otherwise we will be stuck here						
						// ++it;
						if (*it == '\n') {
							// if it is a newline, and parent is still open, we let parent to deal with it
							// otherwise, just move forward as a new line text node by itself doesn't mean much as it 
							// merely the separator between two entities not a break between two text nodes
							if (parent_ptr && parent_ptr->isopen()) {
								return parent_ptr;
							}
							++it;
							begin = it;
						}
						else {
							begin = it;
							EntityT *text_ptr = new Text<StringT, IteratorT>(begin, end);
							if (parent_ptr && parent_ptr->should_have_children()) {
								text_ptr->set_parent(parent_ptr);
							}						
							return text_ptr;
						}
					}

					// Anything that we cannot parse it is a text node
					if (it < end && !entity_ptr && it > begin) {
						entity_ptr = new Text<StringT, IteratorT>(begin, end);
						entity_ptr->set_open(false);
						if (parent_ptr && parent_ptr->should_have_children()) {
							entity_ptr->set_parent(parent_ptr);
						}
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
