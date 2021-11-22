/******************************************************************************
 * This file is part of the Simple Text Processing Library(STPL).
 * (c) Copyright 2021 TYONLINE TECHNOLOGY PTY. LTD.
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

#ifndef STPL_WIKI_ENTITY_H_
#define STPL_WIKI_ENTITY_H_

#include <sstream>
#include <ostream>
#include <cassert>
#include <map>
#include <list>
#include <regex>

#include "stpl_wiki_basic.h"

#include "../stpl_property.h"

#include "../../utils/icstring.h"
#include "../../utils/strings.h"
#include "../../utils/xml.h"

#include "../lang/stpl_character.h"


namespace stpl {
	namespace WIKI {

		template <typename StringT = std::string
					, typename IteratorT = typename StringT::iterator
				  >
		class CommonChildEntity: public WikiEntity<StringT, IteratorT>
		{
			public:
				typedef StringT													string_type;
				typedef IteratorT												iterator;

			public:
				CommonChildEntity() : WikiEntity<StringT, IteratorT>::WikiEntity()
							 { init(); }
				CommonChildEntity(IteratorT it) :
					WikiEntity<StringT, IteratorT>::WikiEntity(it)/*, start_k_(it, it)*/
					 { init(); }
				CommonChildEntity(IteratorT begin, IteratorT end) :
					WikiEntity<StringT, IteratorT>::WikiEntity(begin, end)/*, start_k_(begin, begin)*/
					 { init(); }

				virtual ~CommonChildEntity() {
				}

				virtual std::string to_text() {
					return WikiEntity<StringT, IteratorT>::to_text();
				}

				virtual std::string to_html() {
					if (this->children().size() == 0) {
						return "";
					}
					WikiNodeGroup group = this->parent_ptr_->get_group();
					if (group == LINK) {
						std::stringstream ss;
						auto first = this->children().begin();
						auto second = first + 1;
						ss << "<a href=\"";
						ss << (*first)->to_std_string();
						ss << "\">";

						// now the anchor text
						// the anchor text could be a compound entity
						if (second != this->children().end())
							ss << (*second)->to_html();
						// we are using the url as the anchor text
						else
							ss << (*first)->to_std_string();
						ss << "</a> ";
						return ss.str();
					}
					
					std::stringstream ss;

					auto it = this->children_.begin();
					for (; it != this->children_.end(); ++it)
						ss << (*it)->to_html();

					return ss.str();

					return this->to_std_string();
				}

				virtual bool is_child_end(WikiNodeGroup group, WikiNodeType type, IteratorT& it) {
					return this->parent_ptr_->is_child_end(group, type, it);
				}

				virtual bool is_pause(IteratorT& it) {
					if (*it == '|' && this->get_type() == P_HEADING) {
						// this is not a property for either Link or Others
						++it;
						return false;
					}
					return WikiEntity<StringT, IteratorT>::is_pause(it);
				}				

			private:
				void init() {
					this->set_group(PROPERTY);
					this->set_type(P_NONE);
				}
		};

		template <typename StringT = std::string, typename IteratorT = typename StringT::iterator>
		class WikiProperty : public Property<StringT, IteratorT>, public WikiEntity<StringT, IteratorT> {
			public:
				typedef std::map<StringT, WikiProperty*>	        attributes_type;
				typedef StringBound<StringT, IteratorT>             StringB;

			private:
				int													property_id_;
				int                                                 name_count_;       // for the size of the name

			public:
				WikiProperty() :
					WikiEntity<StringT, IteratorT>::WikiEntity() { init(); }
				WikiProperty(IteratorT begin) :
					WikiEntity<StringT, IteratorT>::WikiEntity(begin) { init(); }
				WikiProperty(IteratorT begin, IteratorT end) :
					WikiEntity<StringT, IteratorT>::WikiEntity(begin, end) { init(); }
				WikiProperty(StringT content) :
					WikiEntity<StringT, IteratorT>::WikiEntity(content) {
					init();
				}
				virtual ~WikiProperty() {}

				// virtual BasicWikiEntity<StringT, IteratorT> *create_child(IteratorT& begin, IteratorT& end) {
				// 	return new CommonChildEntity<StringT, IteratorT>(begin, end);
				// }

				int get_property_id() const {
					return property_id_;
				}

				void set_property_id(int propertyId) {
					property_id_ = propertyId;
				}

				virtual void end(IteratorT it) { 
					// need to close things up
					if (this->value_.begin() > this->name_.end()) {
						this->value_.end(it);
					}
					WikiEntity<StringT, IteratorT>::end(it); 
				}

				virtual void create_text_child_pre(IteratorT it) {
					if (it > this->value_.begin()) {
						this->last_child_ = new Text<StringT, IteratorT>(this->value_.begin(), it);
						this->add(this->last_child_);
					}
				}

				virtual std::string to_std_string() {
					if (this->children_.size() > 0) {
						stringstream ss;
						auto it = this->children_.begin();
						while (it != this->children_.end()) {
							ss << (*it++)->to_html();
						}
						return ss.str();
					}
					return BasicWikiEntity<StringT, IteratorT>::to_std_string();
				}

				virtual std::string to_json() {
					stringstream ss;
					ss << "{" << std::endl;
					auto it = this->children_.begin();
					ss << "\"name\": ";
					if (this->has_delimiter_) {
						ss << "\"" << std::string(this->name_.begin(), this->name_.end()) << "\",";
						ss << "\"value\": ";
						ss <<  "\"";
						it = it + name_count_;
						// ss << name << "=";
						// if (this->has_quote())
						// 	ss << "\"";
						while (it != this->children_.end()) {
							std::string content = (*it++)->to_html();
							ss << utils::escape_quote(content);
						}

						ss << "\"" << std::endl;
					}
					else {
						std::string content = this->children_to_html();
						ss <<  "\"" << utils::escape_quote(content) << "\"" << std::endl;	
					}			
					ss << "}" << std::endl;	
					return ss.str();
				}

				virtual std::string to_text() {
					return "";
				};

				/**
				 * For all properties, it could be a property name or property value, which is decided by 
				 * the appearing order
				 */
				virtual std::string to_html() {
					stringstream ss;
					auto it = this->children_.begin();

					ss << "<property";
					if (this->has_delimiter_) {
						ss << " name=\"";
						std::string name = std::string(this->name_.begin(), this->name_.end());
						name = utils::escape_quote(name);

						ss << name << "\">";
						if (this->children_.size() > 0) {
							// name is a child too
							it = it + name_count_;
							// ss << name << "=";
							// if (this->has_quote())
							// 	ss << "\"";
							while (it != this->children_.end()) {
								ss << (*it++)->to_html();
							}

							// if (this->has_quote())
							// 	ss << "\"";
						}
						else {
							std::string value;
							//if (this->has_delimiter()) {
							value = this->value_.to_std_string();
							value = std::regex_replace(value, std::regex("'"), "&apos;");
							value = std::regex_replace(value, std::regex("\""), "&quot;");
							ss << value;
								//return name + "=\"" + value + "\"";
							//}
						}
					}
					else {
						ss << ">";
						ss << this->children_to_html();
					}
					ss << "</property>";
	
					//return Property<StringT, IteratorT>::to_std_string();
					return ss.str();
				}

				virtual bool is_start(IteratorT& it) {
					this->name_.begin(it);
					this->name_.end(it);
					this->value_.begin(it);
					this->value_.end(it);
					Property<StringT, IteratorT>::is_start(it);
					return true;
				}

				virtual bool is_delimiter(IteratorT& it) {
					if ( *it == '|' || *it == '}') {
						if (this->value_.begin() > this->name_.end())
							this->value_.end(it);
						return true;
					}
					return Property<StringT, IteratorT>::is_delimiter(it);
				}

				/**
				 * A property can be inside a template or a link
				 */
				virtual bool is_end(IteratorT& it, bool advance=true) {
					// Shouldn't use property's end for WikiProperty, as an normal property, e.g. a property inside a html/xml tag, 
					// space is used for an end
					// it better not to do it, as WikiProperty could be child of Template or Table
					// if (*it == '}') {
					// 	IteratorT next = it + 1;
					// 	return (*next == '}');
					// }
					// else 
					if (*it == '|') {
						return true;
					}
					else if (*it == '}') {
						IteratorT next = it + 1;
						return (*next == '}');
					}					
					// else 					
					// if (WikiEntity<StringT, IteratorT>::is_end(it)) {/*  || Property<StringT, IteratorT>::is_end(it) */
					// 	// need to close things up
					// 	// if (this->value_.begin() > this->name_.end()) {
					// 	// 	this->value_.end(it);
					// 	// }
					// 	return true;
					// }
					return false;
				}

				// we collect text node and others
				virtual bool is_pause(IteratorT& it) {
					// when it come to '=', it may be the start of a new entity
					// so we pause and see
					// if (WikiEntity<StringT, IteratorT>::is_pause(it)) {
					// 	return true;
					// }
					// else 
					if (*it == '=') {
					    if (!this->has_delimiter_) {
							this->has_delimiter_ = true; // WikiProperty doesn't use quote for value boundary
							// backward for removing space
							// deside where is the end of name
							IteratorT pre = it;
							WikiEntity<StringT, IteratorT>::skip_whitespace_backward(--pre);
							++pre;
							if (pre > this->name_.begin()) {
								this->name_.end(pre);
								name_count_ = this->children_.size();

								++it;

								WikiEntity<StringT, IteratorT>::skip_whitespace(it);

								this->value_.begin(it);
								this->value_.end(it);
							}
							else {
								// name supposed to be not empty
								++it;
								return false;
							}
						}
						else
							return false;
					}
					// else if (*it == '\n') {
					// 	// do we need to advance here?
					// 	// please check and find out, and put some reasons here
					// 	// ++it;
					// 	return true;
					// }
					// else if (*it == '|')
					// 	return true;
					return true;
				}

				virtual bool is_separated(IteratorT& it) {
					return *it == WikiEntityConstants::WIKI_KEY_PROPERTY_DELIMITER;
				}	

				virtual bool is_child_end(WikiNodeGroup group, WikiNodeType type, IteratorT& it) {
					if (TEXT == group && *it == '=') {
						if (this->has_delimiter_) 
							return false;
						return true;
					} 
					return false;
				}							

			private:
				void init() {
					this->has_delimiter_ = false;
					this->group_ = PROPERTY;
					this->set_type(P_PROPERTY);
					this->name_count_ = 0;
				}
		};

		template <typename StringT = std::string,
					typename IteratorT = typename StringT::iterator
				  >
		class WikiSection : public WikiEntity<StringT, IteratorT>
		{
			public:
				typedef	StringT                                      string_type;
				typedef IteratorT	                                 iterator;
				typedef StringBound<StringT, IteratorT>              StringB;

			protected:
				int                                                  id_;
				std::string											 line_;

			public:
				WikiSection() : WikiEntity<StringT, IteratorT>::WikiEntity() {}
				WikiSection(IteratorT it) :
					WikiEntity<StringT, IteratorT>::WikiEntity(it) {
					init();
				}
				WikiSection(IteratorT begin, IteratorT end) :
					WikiEntity<StringT, IteratorT>::WikiEntity(begin, end) { init(); }
				WikiSection(StringT content) :
					WikiEntity<StringT, IteratorT>::WikiEntity(content) {
					init();
				}
				virtual ~WikiSection() {};

				virtual std::string to_json() {
					stringstream ss;
					ss << "{" << std::endl;
					ss << "\"id\": "  << "\"" << this->get_id() << "\"," << std::endl;
					if (this->get_level() > 0) {
						ss << "\"level\": " << "\"" << this->get_level() << "\"," << std::endl;
						ss << "\"line\": "  << "\"" << this->get_line() << "\"," << std::endl;
					}
					std::string html = this->children_to_html();
					html.erase(std::remove(html.begin(), html.end(), '\n'), html.end());
					ss <<  "\"text\": "  << "\"" << utils::escape_quote(html) << "\"" << std::endl;
					ss <<  "}" << std::endl;

					return ss.str();
				}				

				int get_id() const {
					return id_;
				}

				void set_id(int id) {
					id_ = id;
				}

				const std::string& get_line() const {
					return line_;
				}

				void set_line(const std::string &line) {
					line_ = line;
				}

			protected:


			private:
				void init() { 
					this->set_group(SECTION);
					this->set_own_children(false);
				}	
		};

		template <typename StringT = std::string,
							typename IteratorT = typename StringT::iterator
						  >
		class TableCell : public WikiEntity<StringT, IteratorT>
		{
			public:
				typedef	StringT                                      string_type;
				typedef IteratorT	                                 iterator;
				typedef StringBound<StringT, IteratorT>              StringB;

			protected:
				bool                                                 ordered_;
				char                                                 start_with_;

			private:
				int                                                  cell_id_;
				int                                                  row_id_;
				int                                                  col_id_;

			public:
				TableCell() : WikiEntity<StringT, IteratorT>::WikiEntity() {}
				TableCell(IteratorT it) :
					WikiEntity<StringT, IteratorT>::WikiEntity(it) {
					init();
				}
				TableCell(IteratorT begin, IteratorT end) :
					WikiEntity<StringT, IteratorT>::WikiEntity(begin, end) { init(); }
				TableCell(StringT content) :
					WikiEntity<StringT, IteratorT>::WikiEntity(content) {
					init();
				}
				virtual ~TableCell() {};

				virtual std::string to_text() {
					return "";
				}			

				int get_cell_id() const {
					return cell_id_;
				}

				void set_cell_id(int cellId) {
					cell_id_ = cellId;
				}

				virtual bool is_child_end(WikiNodeGroup group, WikiNodeType type, IteratorT& it) {
					if (TEXT == group && *it == '\n') {
						return true;
					} 
					return false;
				}					

				virtual bool is_end(IteratorT& it, bool advance=true) {
					// no, newline is not end yet
				    if (*it == '\n') {
						// do we need to advance here?
						// please check and find out, and put some reasons here
						// IteratorT next = it + 1;
						// if (*next == '|' || *next == '!') {
						// 	// we are not gonna advance, because table need new line for other properties
						// 	// if (advance)
						// 	// 	it = next + 1;
						// 	return true;
						// }
						// if (advance)
						// 	++it;
						// // if it not start with a pipe, it ends
						// return *it;
						return true;
					}
				    else 
					if (*it == '|')
						return true;
					else if (*it == '!' && row_id_ == 0) {
						// transform it to make it recogiize as a cell
						*it = '|';
						return true;
					}

					// The cell is only ended when a pipe is seen
					return false; // WikiEntity<StringT, IteratorT>::is_end(it);
				}

				// we collect text node and others
				// anything is a pause including new line				

				int get_col_id() const {
					return col_id_;
				}

				void set_col_id(int colId) {
					col_id_ = colId;
				}

				int get_row_id() const {
					return row_id_;
				}

				void set_row_id(int rowId) {
					row_id_ = rowId;
				}

			private:
				void init() { 
					this->set_group(CELL);
					this->set_type(P_CELL);					
				}	
		};		

		/**
		 * 
		 * The unit inside a container
		 * for those without a clear boundary
		 */
		template <typename StringT = std::string,
							typename IteratorT = typename StringT::iterator
						  >
		class ListItem : public WikiEntity<StringT, IteratorT>
		{
			public:
				typedef	StringT                                      string_type;
				typedef IteratorT	                                 iterator;
				typedef StringBound<StringT, IteratorT>              StringB;

			protected:
				bool                                                 ordered_;
				char                                                 start_with_;
				char	                                             key_char_;

			public:
				ListItem() : WikiEntity<StringT, IteratorT>::WikiEntity() {}
				ListItem(IteratorT it) :
					WikiEntity<StringT, IteratorT>::WikiEntity(it) {
					init();
				}
				ListItem(IteratorT begin, IteratorT end) :
					WikiEntity<StringT, IteratorT>::WikiEntity(begin, end) { init(); }
				ListItem(StringT content) :
					WikiEntity<StringT, IteratorT>::WikiEntity(content) {
					init();
				}
				virtual ~ListItem() {};

				virtual std::string to_html() {
					return "<li>" + WikiEntity<StringT, IteratorT>::to_html() + "</li>";
				}

				virtual void set_key_char() = 0;					

				virtual bool is_end(IteratorT& it, bool advance=true) {
					if (*it == '\n') {
						// if (this->last_child_ && it > this->last_child_->end()) {
						// 	// we need to record the last text node
						// 	this->create_text_child_after(it);
						// }
						
						IteratorT next = it + 1;
						// if (advance)
						// 	it = it + 1;

						// this->skip_whitespace(next);
						if (*next == key_char_) {
							// we are not gonna advance, because table need new line for other properties
							// if (advance)
							// 	it = next + 1;
							int levels = 1;
							++next;
							while (*next == key_char_) {
								++next;
								++levels;
							}

							if (levels <= this->get_level()) {								
								return true;
							}
							// have to skip the new line
							if (advance)
								++it;
							return false;
						}

						return true;
					}
					// for List Item, newline is the only character to end it
					// so we dont need to refer to parent
					return false;
				}

				virtual bool is_start(IteratorT& it) {
					if (*it == key_char_) {
						this->level_ = 1;
						++it;
						while (*it == key_char_) {
							++it;
							++this->level_;
						}
						return true;
					}
					return false;
				}				

			private:
				void init() { 
					this->level_ = 0;
					this->set_group(LIST_ITEM);					
				}	
		};

		template <typename StringT = std::string,
							typename IteratorT = typename StringT::iterator
						  >
		class ListItemUnordered : public ListItem<StringT, IteratorT>
		{
			public:
				typedef	StringT                                      string_type;
				typedef IteratorT	                                 iterator;
				typedef StringBound<StringT, IteratorT>              StringB;

			public:
				ListItemUnordered() : ListItem<StringT, IteratorT>::ListItem() {}
				ListItemUnordered(IteratorT it) :
					ListItem<StringT, IteratorT>::ListItem(it) {
					init();
				}
				ListItemUnordered(IteratorT begin, IteratorT end) :
					ListItem<StringT, IteratorT>::ListItem(begin, end) { init(); }
				ListItemUnordered(StringT content) :
					ListItem<StringT, IteratorT>::ListItem(content) {
					init();
				}
				virtual ~ListItemUnordered() {};

			protected:

				virtual void set_key_char() {
					this->key_char_ = WikiEntityConstants::WIKI_KEY_LIST;
				}	

				// we collect text node and others
				virtual bool is_pause(IteratorT& it) {
					return true;
				}				

			private:
				void init() { 
					this->set_type(LAYOUT_UL);	
					this->set_key_char();	
				}			
		};	

		template <typename StringT = std::string,
							typename IteratorT = typename StringT::iterator
						  >
		class ListItemOrdered : public ListItem<StringT, IteratorT>
		{
			public:
				typedef	StringT                                      string_type;
				typedef IteratorT	                                 iterator;
				typedef StringBound<StringT, IteratorT>              StringB;

			public:
				ListItemOrdered() : ListItem<StringT, IteratorT>::ListItem() {}
				ListItemOrdered(IteratorT it) :
					ListItem<StringT, IteratorT>::ListItem(it) {
					init();
				}
				ListItemOrdered(IteratorT begin, IteratorT end) :
					ListItem<StringT, IteratorT>::ListItem(begin, end) { init(); }
				ListItemOrdered(StringT content) :
					ListItem<StringT, IteratorT>::ListItem(content) {
					init();
				}
				virtual ~ListItemOrdered() {};

			protected:

				virtual void set_key_char() {
					this->key_char_ = WikiEntityConstants::WIKI_KEY_LIST_ORDERED;
				}				

			private:
				void init() {
					this->set_type(LAYOUT_OL);
					this->set_key_char();
				 }			
		};

		template <typename StringT = std::string, typename IteratorT = typename StringT::iterator>
		class LayoutList: public WikiEntity<StringT, IteratorT>
		{
			public:
				typedef	StringT						string_type;
				typedef IteratorT					iterator;

			public:
				LayoutList() : WikiEntity<StringT, IteratorT>::WikiEntity() { init(); }
				LayoutList(IteratorT it)
					 : WikiEntity<StringT, IteratorT>::WikiEntity(it) { init(); }
				LayoutList(IteratorT begin, IteratorT end)
					 : WikiEntity<StringT, IteratorT>::WikiEntity(begin, end) { init(); }
				LayoutList(StringT content) :
					WikiEntity<StringT, IteratorT>::WikiEntity() {
					init();
					this->create(content);
				}
				virtual ~LayoutList() {}

				virtual BasicWikiEntity<StringT, IteratorT> *create_child(IteratorT& begin, IteratorT& end) {
					return this;
				}

			private:
				void init() { 
					this->set_group(LAYOUT_LIST);
				}
		};						

		template <typename StringT = std::string
							, typename IteratorT = typename StringT::iterator
						  >
		class LayoutUnorderedList: public LayoutList<StringT, IteratorT>
		{
			public:
				typedef StringT													string_type;
				typedef IteratorT												iterator;

			public:
				LayoutUnorderedList() : LayoutList<StringT, IteratorT>::LayoutList()
							 { init(); }
				LayoutUnorderedList(IteratorT it) :
					LayoutList<StringT, IteratorT>::LayoutList(it)/*, start_k_(it, it)*/
					 { init(); }
				LayoutUnorderedList(IteratorT begin, IteratorT end) :
					LayoutList<StringT, IteratorT>::LayoutList(begin, end)/*, start_k_(begin, begin)*/
					 { init(); }

				virtual ~LayoutUnorderedList() {
				}

				virtual std::string to_html() {
					return "<ul>" + LayoutList<StringT, IteratorT>::to_html() + "</ul>";
				}

				virtual bool is_start(IteratorT& it) {
					return *it == WikiEntityConstants::WIKI_KEY_LIST;
				}

				virtual bool is_pause(IteratorT& it) {
					if (*it == WikiEntityConstants::WIKI_KEY_LIST) {
						return true;
					}
					return LayoutList<StringT, IteratorT>::is_pause(it);
				}

				virtual bool is_end(IteratorT& it, bool advance=true) {
					if (*it == '\n') {
						IteratorT next = it + 1;
						if (*next != '*') {
							// no we cannot eat the new line char
							// if (advance)
							// 	it = next;
							return true;
						}
						// otherwise we need to move forward a char 
						// more children in the list
						if (advance)
							++it;
						return false;
					}
					return *it != '*';
				}

			private:
				void init() {
					this->set_type(LAYOUT_UL);					
				}
		};

		template <typename StringT = std::string, typename IteratorT = typename StringT::iterator>
		class LayoutOrderedList: public LayoutList<StringT, IteratorT>
		{
			public:
				typedef	StringT	string_type;
				typedef IteratorT	iterator;

			public:
				LayoutOrderedList() : LayoutList<StringT, IteratorT>::LayoutList() { init(); }
				LayoutOrderedList(IteratorT it)
					 : LayoutList<StringT, IteratorT>::LayoutList(it) { init(); }
				LayoutOrderedList(IteratorT begin, IteratorT end)
					 : LayoutList<StringT, IteratorT>::LayoutList(begin, end) { init(); }
				LayoutOrderedList(StringT content) :
					LayoutList<StringT, IteratorT>::LayoutList() {
					init();
					this->create(content);
				}
				virtual ~LayoutOrderedList() {}

				virtual std::string to_html() {
					return "<ol>" + LayoutList<StringT, IteratorT>::to_html() + "</ol>";
				}

				virtual bool is_end(IteratorT& it, bool advance=true) {
					if (*it == '\n') {
						IteratorT next = it + 1;
						// this->skip_whitespace(next);
						if (*next != '#') {
							// if (advance)
							// 	it = next;
							return true;
						}
						if (advance)
							++it;
						return false;
					}
					return *it != '#';
				}

			private:
				void init() { 
					this->set_type(LAYOUT_OL);
				}
		};

		/**
		 * @TODO
		 *
		 * make it become matching sequence(s)
		 * like the template {{}}
		 *          table    {||}
		 */
		template <typename StringT = std::string, typename IteratorT = typename StringT::iterator>
		class WikiEntityLeveled: public WikiEntity<StringT, IteratorT>				
		{
			public:
				typedef	StringT	                              string_type;
				typedef IteratorT	                          iterator;

			protected:
				char                                          wiki_key_char_start_;
				char                                          wiki_key_char_end_;
				bool										  is_reversed_sequence_;

				int											  start_sequence_size_;
				int											  end_sequence_size_;

				int											  max_level_;
				int											  min_level_;

				int                                           matched_levels_;

				bool                                          end_in_same_line_;
				bool                                          strict_;

			public:
				WikiEntityLeveled() : WikiEntity<StringT, IteratorT>::WikiEntity() { init(); }
				WikiEntityLeveled(IteratorT it)
					 : WikiEntity<StringT, IteratorT>::WikiEntity(it) { init(); }
				WikiEntityLeveled(IteratorT begin, IteratorT end)
					 : WikiEntity<StringT, IteratorT>::WikiEntity(begin, end) { init(); }
				WikiEntityLeveled(StringT content) :
					WikiEntity<StringT, IteratorT>::WikiEntity() {
					init();
					this->create(content);
				}
				virtual ~WikiEntityLeveled() {
				}

				virtual StringT to_string() {
					if (this->begin() == this->end())
						return StringT("");
					return StringT(this->begin() + this->level_, this->end() - this->level_);
				}

				/**
				 * As if the StringT is char*, it won't work
				 * so we have to make it std::string
				 */
				virtual std::string to_std_string() {
					if (this->begin() == this->end())
						return std::string("");
					auto begin = StringBound<StringT, IteratorT>::begin() + this->level_;
					auto end = StringBound<StringT, IteratorT>::end() - this->level_;
					if (begin >= end)
						return std::string("");
					return std::string(begin, end);
				}				

				virtual bool is_start(IteratorT& it) {
					if (!this->wiki_key_char_start_)
						throw std::invalid_argument("No start sequence defined");

					if (start_sequence_size_ == 1) {
						while (*it == this->wiki_key_char_start_) {
							++this->level_;
							++it;
						}

						if (this->max_level_ > 0 && this->level_ > this->max_level_) {
							// we are back to minimum level
							while (this->level_ > this->min_level_) {
								--this->level_;
								--it;
							}
						}
					}
					else {
						this->level_ = 1;
						int count = 0;
						while (count < start_sequence_size_) {
							// char matched_char = this->wiki_key_char_start_[count];
							if (*it != this->wiki_key_char_start_)
								return false;
							++count;
							++it;
						}
					}
					this->matched_levels_ = -this->level_;
					return true;
				}

				virtual bool is_end(IteratorT& it, bool advance=true) {
					if (!this->wiki_key_char_end_)
						throw std::invalid_argument("No end sequence defined");

					if (this->end_in_same_line_ && *it == '\n') {
						if (this->matched_levels_ != 0) {
							// OK, this is not a valid entity
#ifdef DEBUG
							int a = 1;
#endif // DEBUG				
						}

						return true;
					}
					else if (this->eow(it))
						return true;
					else {
						if (start_sequence_size_ == 1) {
							if (*it == this->wiki_key_char_end_) {
								// BUG before if adavance is false, it wasn't dealt with
								// if advance
								IteratorT next = it + 1;
								if (advance) {
									++this->matched_levels_;
									while (this->matched_levels_ < 0) {
										if (*next != this->wiki_key_char_end_)
											return false;

										++this->matched_levels_;
										++next;
									}

									// if (strict_ && this->matched_levels_ != 0) {
									// 	this->begin(it);
									// 	this->end(it);
									// }
									// now there is a delimma, should we go strict or auto correct?
									it = next;
								}
								else {
									int count = this->level_ - 1;
									while (count > 0) {
										if (*next != this->wiki_key_char_end_) {
											return false;
										}

										--count;
										++next;
									}
								}							
								return true;
							}
						}
						else {
							int count = 0;
							if (*it == wiki_key_char_end_) {
								++count;
								IteratorT next = it + 1;
								while (count < end_sequence_size_) {
									char matched_char = wiki_key_char_end_;
									if (*next != matched_char)
										return false;
									++next;
									++count;
								}
								if (advance)
									it = next;
								return true;
							}
						}
					}
#ifdef DEBUG
#endif // DEBUG					
					return WikiEntity<StringT, IteratorT>::is_end(it); // this->eow(it) || text_stop(it);
				}

				virtual void add_content(StringT& text) {
					this->ref().append(text);
				}

			protected:
				virtual void set_wiki_key_char() = 0;

			private:
				void init() {
					end_in_same_line_ = false;
					strict_ = false;
					is_reversed_sequence_ = false;
					start_sequence_size_ = 1;
					end_sequence_size_ = 1;

					this->max_level_ = -1;
					this->min_level_ = 1;					
				}
		};

		template <typename StringT = std::string,
							typename IteratorT = typename StringT::iterator
						  >
		class LangVariantProperty : public WikiProperty<StringT, IteratorT>
		{
			public:
				typedef	StringT                                      string_type;
				typedef IteratorT	                                 iterator;
				typedef StringBound<StringT, IteratorT>              StringB;

			protected:
				bool                                                 ordered_;
				char                                                 start_with_;

			public:
				LangVariantProperty() : WikiProperty<StringT, IteratorT>::WikiProperty() {}
				LangVariantProperty(IteratorT it) :
					WikiProperty<StringT, IteratorT>::WikiProperty(it) {
					init();
				}
				LangVariantProperty(IteratorT begin, IteratorT end) :
					WikiProperty<StringT, IteratorT>::WikiProperty(begin, end) { init(); }
				LangVariantProperty(StringT content) :
					WikiProperty<StringT, IteratorT>::WikiProperty(content) {
					init();
				}
				virtual ~LangVariantProperty() {};

				virtual std::string to_html() {
					return this->to_std_string();
				}

				virtual std::string to_json() {
					return this->to_std_string();
				}

				virtual std::string to_text() {
					return this->to_std_string();
				}

				virtual std::string to_std_string() {
					if (this->children().size() == 0)
						return "";
					auto it = this->children().begin();
					std:;stringstream ss;
					int count = 0;
					ss << " (";
					while (it != this->children().end()) {
						if (count > 0)
							ss << ", ";
						ss << (*it)->to_std_string();
						++it;
					}
					ss << ") ";
				}

				virtual bool is_delimiter(IteratorT& it) {
					if ( *it == ';' || *it == '}') {
						if (this->value_.begin() > this->name_.end())
							this->value_.end(it);
						return true;
					}
					return Property<StringT, IteratorT>::is_delimiter(it);
				}

				virtual bool is_end(IteratorT& it, bool advance=true) {
					if (*it == ';') {
						// if (this->last_child_ && it > this->last_child_->end()) {
						// 	// we need to record the last text node
						// 	this->create_text_child_after(it);
						// }
						if (advance)
							it = it + 1;
						return true;
					}
					return WikiEntity<StringT, IteratorT>::is_end(it);
				}

			private:
				void init() {
					this->set_group(LIST_ITEM);
				}
		};

		template <
			typename StringT = std::string,
			typename IteratorT = typename StringT::iterator
			>
		class LangVariant: public WikiEntityLeveled<StringT, IteratorT>
		{
			public:
				typedef	StringT	string_type;
				typedef IteratorT	iterator;

			public:
				LangVariant() : WikiEntityLeveled<StringT, IteratorT>::WikiEntityLeveled() { init(); }
				LangVariant(IteratorT it)
					 : WikiEntityLeveled<StringT, IteratorT>::WikiEntityLeveled(it) { init(); }
				LangVariant(IteratorT begin, IteratorT end)
					 : WikiEntityLeveled<StringT, IteratorT>::WikiEntityLeveled(begin, end) { init(); }
				LangVariant(StringT content) :
					WikiEntityLeveled<StringT, IteratorT>::WikiEntityLeveled() {
					init();
					this->create(content);
				}
				virtual ~LangVariant() {}

				virtual bool is_pause(IteratorT& it) {
					return *it == ';';
				}

				virtual bool is_start(IteratorT& it) {
					if (WikiEntityLeveled<StringT, IteratorT>::is_start(it)) {
						IteratorT next = it + 1;
						if (*next == '{') {
							it = next + 1;
							return true;
						}
						return false;
					}
					return false;
				}

				virtual bool is_end(IteratorT& it, bool advance = true) {
					if (WikiEntityLeveled<StringT, IteratorT>::is_end(it, advance)) {
						IteratorT pre = it - 1;
						if (*pre == '}') {
							if (advance)
								it = it + 1;
							return true;
						}
						return false;
					}
					return false;
				}

			protected:
				virtual void set_wiki_key_char() override {
					this->wiki_key_char_start_ = WikiEntityConstants::WIKI_KEY_OPEN_LANGVARIANT;
					this->wiki_key_char_end_ = WikiEntityConstants::WIKI_KEY_CLOSE_LANGVARIANT;

					this->start_sequence_size_ = 2;
					this->end_sequence_size_ = 2;
				}

			private:
				void init() {
					this->set_wiki_key_char();
				 }
		};

		template <typename StringT = std::string, typename IteratorT = typename StringT::iterator>
		class LayoutLeveled: public WikiEntityLeveled<StringT, IteratorT>
		{
			public:
				typedef	StringT	string_type;
				typedef IteratorT	iterator;

			public:
				LayoutLeveled() : WikiEntityLeveled<StringT, IteratorT>::WikiEntityLeveled() { init(); }
				LayoutLeveled(IteratorT it)
					 : WikiEntityLeveled<StringT, IteratorT>::WikiEntityLeveled(it) { init(); }
				LayoutLeveled(IteratorT begin, IteratorT end)
					 : WikiEntityLeveled<StringT, IteratorT>::WikiEntityLeveled(begin, end) { init(); }
				LayoutLeveled(StringT content) :
					WikiEntityLeveled<StringT, IteratorT>::WikiEntityLeveled() {
					init();
					this->create(content);
				}
				virtual ~LayoutLeveled() {}

				virtual void set_wiki_key_char() override {
					this->wiki_key_char_start_ = WikiEntityConstants::WIKI_KEY_HEADING;
					this->wiki_key_char_end_ = WikiEntityConstants::WIKI_KEY_HEADING;				
				}

				virtual bool is_child_end(WikiNodeGroup group, WikiNodeType type, IteratorT& it) {
					if (*it == '\n') {
						return true;
					}					
					else if (*it == WikiEntityConstants::WIKI_KEY_HEADING)
						return true;
					return false;
				}

				virtual bool is_end(IteratorT& it, bool advance=true) {
					// when the line ends it ends
					if (*it == '\n') {
						if (advance && this->matched_levels_ != 0) {
							// this is not a HEADING
							this->set_group(GROUP_NONE);
							this->set_type(NONE);
						}
						return true;
					}
					else if (WikiEntityLeveled<StringT, IteratorT>::is_end(it, advance))
						return true;
					return false;
				}

				virtual BasicWikiEntity<StringT, IteratorT> *create_child(IteratorT& begin, IteratorT& end) {
					BasicWikiEntity<StringT, IteratorT> *entity = new CommonChildEntity<StringT, IteratorT>(begin, end);
					entity->set_type(P_HEADING);
					return entity;
				}
				
				virtual std::string to_std_string() {
					return WikiEntityLeveled<StringT, IteratorT>::to_std_string();
				}

				virtual std::string to_html() {
					return WikiEntityLeveled<StringT, IteratorT>::to_html();
				}

				virtual std::string to_json() {
					return WikiEntityLeveled<StringT, IteratorT>::to_json();
				}

			private:
				void init() {
					this->end_in_same_line_ = true;
					this->set_group(LAYOUT);
					this->set_type(LAYOUT_HEADING);
					this->set_wiki_key_char();
				}

		};

		template <typename StringT = std::string, typename IteratorT = typename StringT::iterator>
		class Style: public WikiEntityLeveled<StringT, IteratorT>
		{
			public:
				typedef	StringT	string_type;
				typedef IteratorT	iterator;

			public:
				Style() : WikiEntityLeveled<StringT, IteratorT>::WikiEntityLeveled() { init(); }
				Style(IteratorT it)
					 : WikiEntityLeveled<StringT, IteratorT>::WikiEntityLeveled(it) { init(); }
				Style(IteratorT begin, IteratorT end)
					 : WikiEntityLeveled<StringT, IteratorT>::WikiEntityLeveled(begin, end) { init(); }
				Style(StringT content) :
					WikiEntityLeveled<StringT, IteratorT>::WikiEntityLeveled() {
					init();
					this->create(content);
				}
				virtual ~Style() {}

				virtual std::string to_html() {
					if (this->level_ == 2)
						// return "<span style=\"font-style: italic;\">" + WikiEntityLeveled<StringT, IteratorT>::to_html() + "</span>";
						return "<i>" + WikiEntityLeveled<StringT, IteratorT>::to_html() + "</i>";
					else if (this->level_ == 3)
						return "<b>" + WikiEntityLeveled<StringT, IteratorT>::to_html() + "</b>";
					else if (this->level_ == 5)
						return "<b><i>" + WikiEntityLeveled<StringT, IteratorT>::to_html() + "</i></b>";
					else
						return WikiEntityLeveled<StringT, IteratorT>::to_html();
				}

				virtual void set_wiki_key_char() override {
					this->wiki_key_char_start_ = WikiEntityConstants::WIKI_KEY_STYLE;
					this->wiki_key_char_end_ = WikiEntityConstants::WIKI_KEY_STYLE;					
				}

				virtual bool is_end(IteratorT& it, bool advance=true) {
					if (WikiEntityLeveled<StringT, IteratorT>::is_end(it, advance)) {
						if (advance) {
							if (this->level_ != 2 && this->level_ != 3 && this->level_ != 5) {
								this->begin(it);
								this->end(it);
							}
							else {
								if (this->level_ == 3)
									this->set_type(STYLE_BOLD);
								else if (this->level_ == 5)
									this->set_type(STYLE_BOTH);
							}
						}
						return true;
					}
					return false;
				}

				// we collect text node and others			

			private:
				void init() {
					this->set_wiki_key_char();
					this->end_in_same_line_ = true;
					this->strict_ = true;
				}

		};		

		template <typename StringT = std::string, typename IteratorT = typename StringT::iterator>
		class StyleIndent: public Style<StringT, IteratorT>
		{
			public:
				typedef	StringT	string_type;
				typedef IteratorT	iterator;

			public:
				StyleIndent() : Style<StringT, IteratorT>::Style() { init(); }
				StyleIndent(IteratorT it)
					 : Style<StringT, IteratorT>::Style(it) { init(); }
				StyleIndent(IteratorT begin, IteratorT end)
					 : Style<StringT, IteratorT>::Style(begin, end) { init(); }
				StyleIndent(StringT content) :
					Style<StringT, IteratorT>::Style() {
					init();
					this->create(content);
				}
				virtual ~StyleIndent() {}

			public:
				virtual bool is_end(IteratorT& it, bool advance=true) {
					if (*it == '\n') {
						return true;
					}
					return false;
				}

				virtual bool is_start(IteratorT& it) {
					while (*it == WikiEntityConstants::WIKI_KEY_STYLE_INDENT) {
						this->level_++;
						++it;
					}
					return true;
				}
				
				virtual std::string to_html() {
					std::stringstream ss;
					
					ss << "<div class=\"textindent" << this->level_ << "\"" << ">" << std::endl;
					ss << this->children_to_html();
					ss << "</div>" << std::endl;
					
					return ss.str();
				}							

			protected:
				virtual void set_wiki_key_char() override {
					this->wiki_key_char_start_ = WikiEntityConstants::WIKI_KEY_STYLE_INDENT;
					this->wiki_key_char_end_ = WikiEntityConstants::WIKI_KEY_NEWLINE;						
				}

			private:
				void init() {
					this->set_wiki_key_char();
					this->end_in_same_line_ = true;
					this->strict_ = true;
				}

		};			

		template <typename StringT = std::string,
				  typename IteratorT = typename StringT::iterator
				  >
		class Link : public WikiEntityLeveled<StringT, IteratorT>
		{
			public:
				typedef	StringT	                                    string_type;
				typedef IteratorT	                                iterator;
				typedef StringBound<StringT, IteratorT>             StringB;

			private:
				StringB                                             url_;
				StringB                                             anchor_;

				bool                                                external_;

			public:
				Link() : WikiEntityLeveled<StringT, IteratorT>::WikiEntityLeveled() {}
				Link(IteratorT it) :
					WikiEntityLeveled<StringT, IteratorT>::WikiEntityLeveled(it) {
					init();
				}
				Link(IteratorT begin, IteratorT end) :
					WikiEntityLeveled<StringT, IteratorT>::WikiEntityLeveled(begin, end) { init(); }
				Link(StringT content) :
					WikiEntityLeveled<StringT, IteratorT>::WikiEntityLeveled(content) {
					init();
				}
				virtual ~Link() {};

				virtual BasicWikiEntity<StringT, IteratorT> *create_child(IteratorT& begin, IteratorT& end) {
					BasicWikiEntity<StringT, IteratorT> *entity = new CommonChildEntity<StringT, IteratorT>(begin, end);
					entity->set_type(P_LINK);
					return entity;
				}

				bool is_external() {
					return external_;
				}

				virtual bool is_start(IteratorT& it) {
					bool ret = WikiEntityLeveled<StringT, IteratorT>::is_start(it);
					if (ret) {
						this->external_ = this->level_ == 1;
						if (this->external_) 
							this->set_type(LINK_EXTERNAL);
						url_.begin(it);
						url_.end(it);
						anchor_.begin(it);
						anchor_.end(it);						
					}
					return ret;
				}

				virtual bool is_end(IteratorT& it, bool advance=true) {
					if (WikiEntityLeveled<StringT, IteratorT>::is_end(it, advance)) {
						return true;
					}
					return false;
				}

				virtual bool is_child_end(WikiNodeGroup group, WikiNodeType type, IteratorT& it) {
					// from a text child to enqurie if parent is ended
					if (TEXT == group) {
						if (LINK_EXTERNAL == this->get_type()) {
							if (*it == ' ')
								return true;
							else if (*it == ':' || *it == '#' || *it == '-') {
								// we need to move forward a char otherwise we will miss the next char
								++it;
								return false;	
							}							
						}
						else {
							// this is for the link
							if (this->size() == 1) {
								if (*it == ':') {
								// ok, now we backward to see what kind of link it is
									IteratorT pre = this->begin() + this->level_;
									std::string link_kind = std::string(pre, it);
									if (utils::iccompare(link_kind, WikiEntityVariables::link_category)) {
										this->set_type(LINK_CATEGORY);
									}
									else if (utils::iccompare(link_kind, WikiEntityVariables::link_file)) {
										this->set_type(LINK_IMAGE);
									}
									// we need to move forward otherwise it will be stopped
									++it;
									return false;
								}
								else if (*it == '#' || *it == '-') {
									++it;
									return false;	
								}								
							}
							if (*it == '|')
								return true;
							// specifial character for FILE: TEMPLATE: CATEGORY, LINK TO CATEGORY
						}
					} 
					return false;
				}					

				const StringB& get_anchor() const {
					return anchor_;
				}

				void set_anchor(const StringB &anchor) {
					anchor_ = anchor;
				}

				const StringB& get_url() const {
					return url_;
				}

				void set_url(const StringB &url) {
					url_ = url;
				}

				virtual std::string to_text() {
					std::stringstream ss;
					if (this->get_type() == LINK_IMAGE)
						return "";
					else if (this->get_type() == LINK_EXTERNAL)
						return "";
					auto first = this->children_.begin();
					std::string url = (*first)->to_text();

					ss << (this->get_type() == LINK_CATEGORY ? url.substr(9) : url);
					auto second = this->children_.end() - 1;
					if (second > first)
						ss << "(" << (*second)->to_text() <<  ") ";
					return ss.str();
				}

				/**
				 * For the link things might get a bit interesting
				 * 
				 */
				virtual std::string to_html() {
					if (this->children_.size() == 0)
						return "";
					auto first = this->children_.begin();
					auto second = this->children_.end() - 1;
					stringstream ss;
					if (this->get_type() == LINK_IMAGE) {
						ss << "<div class=\"placeholder\" style=\"display:none;\"";
						// must be a file link
						if (this->size() > 2) {
							auto it = first + 1;
							while (it != second) {
								ss << (*it)->to_std_string() << " ";
								++it;
							}
						}

						ss << ">" << std::endl;

						ss << "<div class='innerlink'>" << std::endl;
						ss << "<a href=\"";
						std::string img_url;
						// if (this->external_) {
						// 	img_url =  (*first)->to_std_string();
						// }
						// else {
							// ss /* << WikiEntityVariables::protocol << "://" + WikiEntityVariables::host */ << WikiEntityVariables::path <<  (*first)->to_std_string();
						img_url =  WikiEntityVariables::path + (*first)->to_std_string();
						// }
						ss << img_url;
						ss << "\">";
						ss << "<img src=\"" << "\">" << std::endl;
						ss << "</img>" << std::endl;
						ss << "</a>" << std::endl;
						ss << "<div class='linkcaption'>" << std::endl;
						ss << (*second)->to_html();
						ss << "</div>" << std::endl;
						ss << "</div>" << std::endl;

						ss << "</div>" << std::endl;
					}
					else {
						if (this->external_) {
							// as external link use space to break
							// the child will be CommonChildProperty type
							ss << (*first)->to_html();
						}
						else {
							ss << "<a href=\"";
								ss << WikiEntityVariables::protocol << "://" + WikiEntityVariables::host << WikiEntityVariables::path <<  (*first)->to_std_string();
							
							ss << "\">";

							// now the anchor text
							// the anchor text could be a compound entity
							if (second > first)
								ss << (*second)->to_html();
							// we are using the url as the anchor text
							else
								ss << (*first)->to_std_string();
							ss << "</a> ";
						}
					}

					return ss.str();
				}

				virtual std::string to_json() {			
					if (this->children_.size() == 0)
						return "{url: \"\"}";	
					auto first = this->children_.begin();
					auto second = this->children_.end() - 1;
					stringstream ss;
					ss << "{" << std::endl;
					if (second > first) {
						std::string html = (*second)->to_html();
						html = utils::escape_quote(html);
						if (this->children().size() > 2) {
							ss << "\"caption\":\"";
							ss << html;
							ss  << "\"," << std::endl;
							ss << "\"properties\": [";
							auto it = first + 1;
							int count = 0;
							while (it < second) {
								if (count > 0)
									ss << ",";
								std::string content =  (*it)->to_html();
								ss << "\"" << utils::escape_quote(content) << "\"" ;
								++it;
								++count;
							}
							ss  << "]," << std::endl;
						}
						else {
							ss << "\"anchor\":\"";
							ss << html;
							ss  << "\"," << std::endl;
						}
					}
					ss << "\"url\":\"";
					if (this->external_) {
						ss << (*first)->to_std_string();
					}
					else {
						ss /* << WikiEntityVariables::path */ <<  (*first)->to_std_string();
					}
					ss << "\"" << std::endl;
					ss << "}" << std::endl;
					return ss.str();
				}

				virtual std::string to_std_string() {
					if (LINK_REDIRECT == this->get_type()) {
						auto first = this->children_.begin();
						if (first != this->children_.end()) {
							return (*first)->to_std_string();
						}
						return "";
					}
					return WikiEntityLeveled<StringT, IteratorT>::to_std_string();
				}

			protected:
				virtual void set_wiki_key_char() override {
					this->wiki_key_char_start_ = WikiEntityConstants::WIKI_KEY_OPEN_LINK;
					this->wiki_key_char_end_ = WikiEntityConstants::WIKI_KEY_CLOSE_LINK;				
				}

			private:
				void init() {
					this->group_ = LINK;
					this->set_type(LINK_P);
					this->set_wiki_key_char();
				}
		};

		/**
		 * Can be either Template or Table
		 */
		template <typename StringT = std::string,
							typename IteratorT = typename StringT::iterator
						  >
		class TBase : public WikiEntityLeveled<StringT, IteratorT>
		{
			public:
				typedef	StringT                                      string_type;
				typedef IteratorT	                                 iterator;
				typedef StringBound<StringT, IteratorT>              StringB;

			protected:
				StringBound<StringT, IteratorT>                      name_;
				int	                                                 level_marks_;

			public:
				TBase() :  WikiEntityLeveled<StringT, IteratorT>:: WikiEntityLeveled() {}
				TBase(IteratorT it) :
					 WikiEntityLeveled<StringT, IteratorT>:: WikiEntityLeveled(it) {
					init();
				}
				TBase(IteratorT begin, IteratorT end) :
					 WikiEntityLeveled<StringT, IteratorT>:: WikiEntityLeveled(begin, end) { init(); }
				TBase(StringT content) :
					 WikiEntityLeveled<StringT, IteratorT>:: WikiEntityLeveled(content) {
					init();
				}
				virtual ~TBase() {};

			protected:
				virtual bool is_delimiter(IteratorT& it) {
					return *it == '|';
				}

				virtual bool is_start(IteratorT& it) {
					if (WikiEntityLeveled<StringT, IteratorT>::is_start(it)) {
						IteratorT from = it;
						while (!this->eow(it) && ( *it != '|' && *it != '}' && *it != '\n' && *it != '\r'))
							++it;
						name_.begin(from);
						name_.end(it);
						return true;
					}

					return false;
				}

				virtual bool is_pause(IteratorT& it) {
					if (*it == '|') {
						IteratorT next = it + 1;
						return !(this->type_ == TABLE && *next == '}');
					}
					return false;
				}


			private:
				void init() {
				
				}
		};


		template <
			typename StringT = std::string,
			typename IteratorT = typename StringT::iterator
			>
		class Table: public TBase<StringT, IteratorT>
		{
			public:
				typedef	StringT	string_type;
				typedef IteratorT	iterator;

			private:
				StringBound<StringT, IteratorT>           caption_;
				StringBound<StringT, IteratorT>           style_;

				int                                       cell_id_;
				int                                       col_id_;
				int                                       row_id_;
				int                                       rows_;           // including headers
				int                                       row_prev_;
				int                                       col_prev_;       // previously added column id

				bool                                      has_row_header_;
				bool                                      has_header_;

				std::string                               rows_header_ind_;

				WikiProperty<StringT, IteratorT>*         current_parent_;

				TableCell<StringT, IteratorT>*            last_cell_ptr_;

			public:
				Table() : TBase<StringT, IteratorT>::TBase() { init(); }
				Table(IteratorT it)
					 : TBase<StringT, IteratorT>::TBase(it) { init(); }
				Table(IteratorT begin, IteratorT end)
					 : TBase<StringT, IteratorT>::TBase(begin, end) { init(); }
				Table(StringT content) :
					TBase<StringT, IteratorT>::TBase() {
					init();
					this->create(content);
				}
				virtual ~Table() {}

				virtual BasicWikiEntity<StringT, IteratorT> *create_child(IteratorT& begin, IteratorT& end) {
					// ++this->cell_id_;
					return new TableCell<StringT, IteratorT>(begin, end);
				}

				virtual void process_child(BasicWikiEntity<StringT, IteratorT>* child) {
					TableCell<StringT, IteratorT>* cell_ptr = dynamic_cast<TableCell<StringT, IteratorT> *>(child);

                    if (cell_ptr) {
						cell_ptr->set_cell_id(this->cell_id_);
						cell_ptr->set_row_id(this->row_id_);
						cell_ptr->set_col_id(this->col_id_);						
						WikiEntityLeveled<StringT, IteratorT>::process_child(cell_ptr);
						last_cell_ptr_ = cell_ptr;
					}
                    else {	
						// std::cerr << "Table::process_child: child is not a TableCell" << std::endl;
						if (!last_cell_ptr_) {
							last_cell_ptr_ = new TableCell<StringT, IteratorT>(child->begin(), child->end());
							last_cell_ptr_->add(child);
							last_cell_ptr_->set_cell_id(++this->cell_id_);
							last_cell_ptr_->set_row_id(this->row_id_);
							last_cell_ptr_->set_col_id(this->col_id_);
							WikiEntityLeveled<StringT, IteratorT>::process_child(last_cell_ptr_);
						}
						else {
							last_cell_ptr_->add(child);
							last_cell_ptr_->end(child->end());
						}
					}

					// if (this->cell_id_ == -1)
					// 	std::cerr << "Table::process_child: cell_id_ is -1" << std::endl;
					// else
					#ifdef DEBUG
					//std::cerr << "Table::process_child: child, cell id: " << last_cell_ptr_->get_cell_id() << std::endl;
					#endif // DEBUG
				}

				void print_last_cell(std::ostream &ss, TableCell<StringT, IteratorT> *last_format, std::vector<TableCell<StringT, IteratorT>* >& last_cells, int rows, bool row_header, bool& first_col) {
					if (!last_format)
						return;

					if (rows == 0 && row_header) {
						ss << "<th";
					}
					else {
						ss << "<td";
					}
					if (rows > 0 && first_col && row_header) {
						ss << " " << "row-header=\"true\" ";
						first_col = false;
					}

					/**
					 *
					 * handle the cell style
					 */
					if (last_cells.size() > 0) {
						std::string format = last_format->to_std_string();
						utils::unescape_xml(format);
						if (format.size() > 0) {
							if (format[0] != '<' && last_format && last_cells.size() > 0)
								ss << " " << format;
						}
					}

					ss << ">";

					if (last_cells.size() > 0) {
						for (int i = 0; i < last_cells.size(); ++i) {
							auto cell = last_cells[i];
							if (i == 0)
								ss << cell->to_html();
							else {
								ss << "|";
								ss << cell->to_html();
							}
						}
					}
					else if (last_format)
						ss << last_format->to_html();

					if (rows == 0 && row_header) {
						ss << "</th>" << std::endl;
					}
					else {
						ss << "</td>" << std::endl;
					}
					last_format = NULL;
					last_cells.clear();
				}

				virtual std::string to_html() {
					std::stringstream ss;
					ss << "<table";
					if (style_.length() > 0) {
						std::string table_style = style_.to_std_string();
						utils::unescape_xml(table_style);
						ss << " " << table_style;
					}
					ss << ">" << std::endl;;
					int rows = -1;
					int cols = -1;
					auto it = this->children_.begin();
					bool first_col = true;
					int last_cell_id = -1; // because cell id starts from 0
					TableCell<StringT, IteratorT> *last_format = NULL, *last_cell_ptr = NULL, *cell_ptr;
					std::vector<TableCell<StringT, IteratorT>* > last_cells;
					bool row_header = false;
					while (it != this->children_.end()) {
						if ((*it)->get_type() == SEPARATOR || (rows == -1 && cols == -1)) {
							// ss << (*it)->to_html() << std::endl;
							if ((*it)->get_type() == SEPARATOR) {
								print_last_cell(ss, last_format, last_cells, rows, row_header, first_col);
							}

							if (rows > 0) {
								ss << "</tr>" << std::endl;
							}
							ss << "<tr>" << std::endl;
							cols = 0;
							++rows;
							last_cells.clear();
							last_format = NULL;
							first_col = true;

							if ((*it)->get_type() == SEPARATOR) {
								++it;
								continue;
							}
						}

						cell_ptr = static_cast<TableCell<StringT, IteratorT> *>(*it);
						if (cell_ptr) {
							row_header = rows_header_ind_[rows] == '1';
							int skip_cells = cell_ptr->get_cell_id() - last_cell_id;
							/**
							 * Can't remember why I was doing this
							 * when it is not a cell???
							 */
							// bool should_be_cell = false;
							// if (cell_ptr->children().size() > 0) {
							// 	auto child = cell_ptr->children().begin();
							// 	while (child != cell_ptr->children().end()) {
							// 		if ((*child)->get_group() != TEXT) {
							// 			should_be_cell = true;
							// 			break;
							// 		}
							// 		++child;
							// 	}
							// }

							if (skip_cells > 1) {
								// fill up the missing cells
								for (int i = 1; i < skip_cells; ++i) {
									if (rows == 0 && row_header) {
										ss << "<th ></th>" << std::endl;
									}
									else
										ss << "<td ></td>" << std::endl;
								}
								last_cell_id += skip_cells - 1;
							}
							// else {
							// 	if (should_be_cell) {
							// 		skip_cells = 1;
							////      this line of cause the same cell gets printed twice
							// 		last_format = cell_ptr;
							// 	}
							// }

							// we print it only we have the last cell
							if (last_format && skip_cells > 0) {
								print_last_cell(ss, last_format, last_cells, rows, row_header, first_col);
								last_format = cell_ptr;
							}
							else {
								if (last_format) {
									last_cells.push_back(cell_ptr);
								}
								else
									last_format = cell_ptr;
							}

							last_cell_ptr = cell_ptr;
							last_cell_id = cell_ptr->get_cell_id();
						}
						++it;
					}

					print_last_cell(ss, last_format, last_cells, rows, row_header, first_col);
					if (rows > 0)
						ss << "</tr>" << std::endl;
					ss << "</table>";
					return ss.str();
				}

				virtual bool is_start(IteratorT& it) {
					if (*it == '{' ) {
						IteratorT next = it + 1;
						if (*next == '|') {
							// | will be used as
							// ++it;
							it = next + 1;
							while (*it == ' ')
								++it;
							this->style_.begin(it);

							while (*it != '\n')
								++it;
							this->style_.end(it);

							return true; // TBase<StringT, IteratorT>::is_start(it);
						}
					}
					return false;
				}

				virtual bool is_pause(IteratorT& it) {
					if (*it == '\n') {
						IteratorT next = it + 1;
						while (*next == ' ')
							++next;
						// if (*next == '}') {
						// 	return true;
						// }
						// else 
						if (*next == '|') {
							++next;
							switch (*next) {
								case '+':
									{
										++next;
										caption_.begin(next);
										while (*next != '\n')
											++next;

										caption_.end(next);

										it = next;
									}
									return true;
								// new row
								case '-':
									{
										// need to set a row separator
										auto separator = new Text<StringT, IteratorT>(it, it);
										separator->set_type(SEPARATOR);
										this->add(separator);

										col_id_ = -1;
										row_id_ = rows_;
										++rows_;

										rows_header_ind_.push_back('0');

										while (*next != '\n')
											++next;
										// table will come back here again if it can see a pipe
										it = next + 1;
									}
									return true;
								default:
									break;
							}
							++cell_id_;
							++col_id_;
							it = next;
							// that will be a table cell
							return true;
						}
						else if (*next == '!') {
							if (row_id_ < 0){
								row_id_ = rows_;
								++rows_;
							}							
							++cell_id_;
							++col_id_;
							// if (row_id_ > 0) {
							// 	// row header
							// 	rows_header_ind_[row_id_] = '1';
							// }
							// else {
								// column header
								has_header_ = true;
								rows_header_ind_[row_id_] = '1';
								// IteratorT next = it + 1;
								// if (*next == '!')
								// 	it = next;
							// }
							it = next;
							*it = '|';
							return true;
						}
					}
					else if (*it == '|') {
						// this is in the middle of a cell
						// last child needs to become a parent of many things
						IteratorT next = it + 1;
						if (*next == '|' || (*next == '!' && row_id_ == 0)) {
							++cell_id_;
							++col_id_;
							*next = '|';
						}
						it = next;
						return true;
						// auto temp = new Text<StringT, IteratorT>(last_child_->begin(), last_child_->end());
						// last_child_->process_child(temp);
					}
					else if (*it == '!') {
						// this is in the middle of a cell of header row
						IteratorT next = it + 1;
						if (row_id_ == 0) {
							if (*next == '!') {
								++cell_id_;
								++col_id_;
							}
						}
						it = next;
						*it = '|';
						return true;
					}

					return false;
				}

				/**
				 * It has clear end boundary, it end only when a boundary is seen
				 */
				virtual bool is_end(IteratorT& it, bool advance=true) {
					if (*it == '\n') {
						IteratorT next = it + 1;
						// while (*next == ' ')
						// 	++next;
						if (*next == '!') {
							return false;
						}
						else if (*next == '|') {
							++next;
							if (*next == '}') {
								if (advance)
									it = next + 1;
								return true;
							}
							return false;
						}
						// table should be ended as there shouldn't be andy table without '|'
						return true;
					}
					else if (*it == '|') {
						IteratorT next = it + 1;
						if (*next == '}') {
							if (advance)
								it = next + 1;
							return true;
						}
					}
					else if (*it == '}') {
						IteratorT prev = it - 1;
						if (*prev == '|') {
							if (advance)
								++it;
							return true;
						}
					}
					return false;
				}

			protected:
				virtual void set_wiki_key_char() override {
					this->wiki_key_char_start_ = WikiEntityConstants::WIKI_KEY_OPEN_TABLE;
					this->wiki_key_char_end_ = WikiEntityConstants::WIKI_KEY_CLOSE_TABLE;

					this->start_sequence_size_ = 2;
					this->end_sequence_size_ = 2;
				}

			private:
				void init() {
					row_prev_ = -1;
					rows_ = 0;
					row_id_ = -1;
					rows_header_ind_ = "0";
					cell_id_ = -1;

					last_cell_ptr_ = NULL;

					this->set_group(TBASE);
					this->set_type(TABLE);

					this->set_wiki_key_char();						
				}
		};

		template <
			typename StringT = std::string,
			typename IteratorT = typename StringT::iterator
			>
		class Template: public TBase<StringT, IteratorT>
		{
			public:
				typedef	StringT	string_type;
				typedef IteratorT	iterator;

			public:
				Template() : TBase<StringT, IteratorT>::TBase() { init(); }
				Template(IteratorT it)
					 : TBase<StringT, IteratorT>::TBase(it) { init(); }
				Template(IteratorT begin, IteratorT end)
					 : TBase<StringT, IteratorT>::TBase(begin, end) { init(); }
				Template(StringT content) :
					TBase<StringT, IteratorT>::TBase() {
					init();
					this->create(content);
				}
				virtual ~Template() {}

				virtual BasicWikiEntity<StringT, IteratorT> *create_child(IteratorT& begin, IteratorT& end) {
					return new WikiProperty<StringT, IteratorT>(begin, end);
				}

				virtual std::string to_json() {
					std::stringstream ss;
					ss << "{" << std::endl;
					ss << "\"name\": \"" << this->name_.to_std_string() << "\"," << std::endl;
					ss << "\"properties\": [" << std::endl;
					auto it = this->children_.begin();
					while (it != this->children_.end()) {
						if (it > this->children_.begin())
							ss << ",";
						ss << (*it)->to_json();
						++it;
					}
					ss << "]" << std::endl;
					ss << "}" << std::endl;
					return ss.str();
				}

				virtual std::string to_html() {
					std::stringstream ss;
					int count = 0;
					std::string name = this->name_.to_std_string();
					if (("lang") == name.substr(0, 4)) {
						ss << "<span type=\"template\" lang=";
						auto it = this->children_.begin();
						if (name.size() > 2) {
							while (it != this->children_.end()) {
								if (count == 0)
									ss << "\"" << (*it)->to_std_string() << "\"";
								else if (count == 1)
									break;

								++count;
								++it;
							}
							ss << ">";
							if (it != this->children_.end())
								ss << (*it)->to_std_string();
						}
						else {
							ss << "\"" << name << "\"";
							ss << ">";
							ss << (*it)->to_std_string();
						}
						ss << "</span>";

					}
					else {
						auto it = this->children_.begin();
						ss << "<template ";
						ss << " name=\"" << name << "\">";
						while (it != this->children_.end()) {
							ss  << (*it)->to_html();

							++count;
							++it;
						}
						ss << "</template>";
					}
					return ss.str();
				}

				virtual bool is_pause(IteratorT& it) {
					return (*it == '|');
				}

				virtual bool is_end(IteratorT& it, bool advance = true) {
					if (*it == '}') {
						IteratorT next = it + 1;
						if (*next == '}') {
							if (advance)
								it = next + 1;
							return true;
						}
						return false;
					}
					return false;
				}				

			protected:
				virtual void set_wiki_key_char() override {
					this->wiki_key_char_start_ = WikiEntityConstants::WIKI_KEY_OPEN_TEMPLATE;
					this->wiki_key_char_end_ = WikiEntityConstants::WIKI_KEY_CLOSE_TEMPLATE;			

					this->start_sequence_size_ = 1;
					this->end_sequence_size_ = 1;

					this->max_level_ = 3;
					this->min_level_ = 2;
				}

			private:
				void init() {
					this->set_group(TBASE);
					this->set_type(TEMPLATE);

					this->set_wiki_key_char();	
				}
		};

	}
}

#endif /*STPL_WIKI_ENTITY_H_*/
