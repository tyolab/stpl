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

#include "../../utils/strings.h"

#include "stpl_wiki_basic.h"

#include "../stpl_property.h"

#include "../lang/stpl_character.h"

namespace stpl {
	namespace WIKI {

		template <typename StringT = std::string, typename IteratorT = typename StringT::iterator>
		class WikiProperty : public Property<StringT, IteratorT>, public WikiEntity<StringT, IteratorT> {
			public:
				typedef std::map<StringT, WikiProperty*>	        attributes_type;
				typedef StringBound<StringT, IteratorT>             StringB;

			private:
				int													property_id_;

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
							++it;
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
					else
						ss << this->children_to_html();
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
					else 					
					if (WikiEntity<StringT, IteratorT>::is_end(it)) {/*  || Property<StringT, IteratorT>::is_end(it) */
						// need to close things up
						// if (this->value_.begin() > this->name_.end()) {
						// 	this->value_.end(it);
						// }
						return true;
					}
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
						this->has_delimiter_ = true; // WikiProperty doesn't use quote for value boundary
						// backward for removing space
						// deside where is the end of name
						IteratorT pre = it;
						WikiEntity<StringT, IteratorT>::skip_whitespace_backward(--pre);
						this->name_.end(++pre);

						++it;

						//WikiEntity<StringT, IteratorT>::skip_whitespace(it);

						// this->value_.begin(it);
						// this->value_.end(it);
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

			private:
				void init() {
					this->has_delimiter_ = false;
					this->group_ = PROPERTY;
					this->set_type(P_PROPERTY);
				}
		};

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

				// virtual bool is_start(IteratorT& it) {
				// 	return true;
				// }

				virtual bool is_pause(IteratorT& it) {
					return true;
				}

				virtual bool is_end(IteratorT& it, bool advance=true) {
					return this->parent_ptr_ && this->parent_ptr_->is_end(it, false);
				}

			private:
				void init() {}
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
				int                                                  level_;
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

				virtual std::string to_html() {
					// stringstream ss;
					// ss << "<section>" << std::endl;
					// ss <<  this->children_to_html();
					// ss <<  "</section>" << std::endl;

					// return ss.str();
					return this->children_to_html();
				}

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

				int get_level() const {
					return level_;
				}

				void set_level(int level) {
					level_ = level;
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

				virtual std::string to_html() {
					// stringstream ss;
					// if (this->get_type() == P_HEADER) {
					// 	ss << "<th>" << this->children_to_html() <<  "</th>";
					// }
					// else if (this->get_type() == P_ROW_HEADER || this->get_type() == P_CELL) {
					// 	ss << "<td" << (this->get_type() == P_ROW_HEADER ? " class=\"row-header\"" : "") << ">" << this->children_to_html() <<  "</td>";
					// }
					// else
					// 	throw new runtime_error("Invalid table cell type");
					// return ss.str();
					return this->children_to_html();
				}		

				int get_cell_id() const {
					return cell_id_;
				}

				void set_cell_id(int cellId) {
					cell_id_ = cellId;
				}

				virtual bool is_end(IteratorT& it, bool advance=true) {
					// no, newline is not end yet
				    if (*it == '\n') {
						// do we need to advance here?
						// please check and find out, and put some reasons here
						IteratorT next = it + 1;
						if (*next == '|') {
							// we are not gonna advance, because table need new line for other properties
							// if (advance)
							// 	it = next + 1;
							return true;
						}
						return false;
					}
				    else 
					if (*it == '|')
						return true;

					return WikiEntity<StringT, IteratorT>::is_end(it);
				}

				// we collect text node and others
				// anything is a pause including new line
				virtual bool is_pause(IteratorT& it) {
					return true;
				}					

			private:
				void init() { 
					this->set_group(CELL);					
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

				virtual bool is_end(IteratorT& it, bool advance=true) {
					if (*it == '\n') {
						if (this->last_child_ && it > this->last_child_->end()) {
							// we need to record the last text node
							this->create_text_child_after(it);
						}
						return true;
					}
					return false;
				}

			private:
				void init() { 
					this->set_group(LAYOUT_ITEM);					
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

				virtual bool is_start(IteratorT& it) {
					if (*it == WikiEntityConstants::WIKI_KEY_LIST) {
						++it;
						this->begin(it);
						return true;
					}
					return false;
				}

				// we collect text node and others
				virtual bool is_pause(IteratorT& it) {
					return true;
				}				

			private:
				void init() { 
					this->set_type(LAYOUT_UL);		
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

				virtual bool is_start(IteratorT& it) {
					if (*it == WikiEntityConstants::WIKI_KEY_LIST_ORDERED) {
						++it;
						this->begin(it);
						return true;
					}
					return false;
				}

			private:
				void init() {
					this->set_type(LAYOUT_LI);
				 }			
		};				

		/**
		 * Can be either Template or Table
		 */
		template <typename StringT = std::string,
							typename IteratorT = typename StringT::iterator
						  >
		class TBase : public WikiEntity<StringT, IteratorT>
		{
			public:
				typedef	StringT                                      string_type;
				typedef IteratorT	                                 iterator;
				typedef StringBound<StringT, IteratorT>              StringB;

			protected:
				StringBound<StringT, IteratorT>                      name_;
				int	                                                 level_marks_;

			public:
				TBase() : WikiEntity<StringT, IteratorT>::WikiEntity() {}
				TBase(IteratorT it) :
					WikiEntity<StringT, IteratorT>::WikiEntity(it) {
					init();
				}
				TBase(IteratorT begin, IteratorT end) :
					WikiEntity<StringT, IteratorT>::WikiEntity(begin, end) { init(); }
				TBase(StringT content) :
					WikiEntity<StringT, IteratorT>::WikiEntity(content) {
					init();
				}
				virtual ~TBase() {};

			protected:
				virtual bool is_delimiter(IteratorT& it) {
					return *it == '|';
				}

				virtual bool is_start(IteratorT& it) {
					IteratorT from = it;
					while (!this->eow(it) && ( *it != '|' && *it != '}' && *it != '\n' && *it != '\r'))
						++it;
					name_.begin(from);
					name_.end(it);

					return name_.length() > 0;
				}

				virtual bool is_pause(IteratorT& it) {
					if (*it == '|') {
						IteratorT next = it + 1;
						return !(this->type_ == TABLE && *next == '}');
					}
					return false;
				}


			private:
				void init() { }			
		};

		template <typename StringT = std::string
							, typename IteratorT = typename StringT::iterator
						  >
		class LayoutUnorderedList: public WikiEntity<StringT, IteratorT>
		{
			public:
				typedef StringT													string_type;
				typedef IteratorT												iterator;

			public:
				LayoutUnorderedList() : WikiEntity<StringT, IteratorT>::WikiEntity()
							 { init(); }
				LayoutUnorderedList(IteratorT it) :
					WikiEntity<StringT, IteratorT>::WikiEntity(it)/*, start_k_(it, it)*/
					 { init(); }
				LayoutUnorderedList(IteratorT begin, IteratorT end) :
					WikiEntity<StringT, IteratorT>::WikiEntity(begin, end)/*, start_k_(begin, begin)*/
					 { init(); }

				virtual ~LayoutUnorderedList() {
				}

				virtual std::string to_html() {
					return "<ul>" + WikiEntity<StringT, IteratorT>::to_html() + "</ul>";
				}

				virtual bool is_start(IteratorT& it) {
					return *it == WikiEntityConstants::WIKI_KEY_LIST;
				}

				virtual bool is_pause(IteratorT& it) {
					if (*it == WikiEntityConstants::WIKI_KEY_LIST) {
						return true;
					}
					return BasicWikiEntity<StringT, IteratorT>::is_pause(it);
				}

				virtual bool is_end(IteratorT& it, bool advance=true) {
					if (*it == '\n') {
						IteratorT next = it + 1;
						this->skip_whitespace(next);
						return *next != '*';
					}
					return WikiEntity<StringT, IteratorT>::is_end(it);
				}

			private:
				void init() {}
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

				virtual void process_child(BasicWikiEntity<StringT, IteratorT>* child) {
					TableCell<StringT, IteratorT>* cell_ptr = reinterpret_cast<TableCell<StringT, IteratorT> *>(child);

					cell_ptr->set_cell_id(this->cell_id_);
					WikiEntity<StringT, IteratorT>::process_child(cell_ptr);
				}

				void print_last_cell(std::ostream &ss, TableCell<StringT, IteratorT> *last_format, std::vector<TableCell<StringT, IteratorT>* >& last_cells, int rows, bool row_header, bool& first_col) {
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

					if (last_format && last_cells.size() > 0) 
						ss << " " << last_format->to_std_string();
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
				}

				virtual std::string to_html() {
					std::stringstream ss;
					ss << "<table";
					if (style_.length() > 0) {
						ss << " " << style_.to_std_string();
					}
					ss << ">" << std::endl;;
					int rows = 0;
					int cols = 0;
					auto it = this->children_.begin();
					bool first_col = true;
					int last_cell_id = 0;
					TableCell<StringT, IteratorT> *last_format = NULL, *last_cell_ptr = NULL, *cell_ptr;
					std::vector<TableCell<StringT, IteratorT>* > last_cells;
					bool row_header = false;
					while (it != this->children_.end()) {
						if ((*it)->get_type() == SEPARATOR || (rows == 0 && cols == 0)) {
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

						cell_ptr = reinterpret_cast<TableCell<StringT, IteratorT> *>(*it);
						row_header = rows_header_ind_[rows] == '1';
						int skip_cells = cell_ptr->get_cell_id() - last_cell_id;
						if (skip_cells > 1) {
							// fill up the missing cells
							for (int i = 1; i < skip_cells; ++i) {
								if (rows == 0 && row_header) {
									ss << "<th ></th>" << std::endl;
								}
								else 
									ss << "<td ></td>" << std::endl;
							}
						}

						// we print it only we have the last cell
						if (last_format && skip_cells > 0) {
							print_last_cell(ss, last_format, last_cells, rows, row_header, first_col);
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
						if (*next == '|') {
							// IteratorT pre = it - 1;
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
									return true;;
								// new row
								case '-': 
									{
										//if (rows_ > 0) {
											// need to set a row separator
											auto separator = new Text<StringT, IteratorT>(it, it);
											separator->set_type(SEPARATOR);
											this->add(separator);
										//}

										col_id_ = -1;
										++rows_;
										row_id_ = rows_;
										// if (row_prev_ == -1)
										// 	row_prev_ = 0;
										// else
										// 	row_prev_ = row_id_;

										rows_header_ind_.push_back('0');

										while (*next != '\n')
											++next;			

										it = next;						
									}
									return true;
								// case '|':
								// 	this is not a possible case;
								//  if that happens it means the previous is an empty cell
								// 	don't need to do anything
								// 	just create a new WikiProperty
								// 	++cell_id_;
								// 	++col_id_;
								// 	++it;
								// 	return true;
								// case '!':
								// 	{
								// 		row_id_ = 0;
								// 	}
								// 	break;		
								default:
									break;					
							}
							++cell_id_;
							++col_id_;
							++it;
							// that will be a table cell
							return true;
						}
						else if (*next == '!') {
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
						if (*next == '|') {
							++cell_id_;
							++col_id_;
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
					if (*it == '|') {
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

			private:
				void init() { 
					row_prev_ = -1;
					rows_ = 0;
					row_id_ -1;
					rows_header_ind_ = "0";
					cell_id_ = -1;
				}
		};

		template <
			typename StringT = std::string, 
			typename IteratorT = typename StringT::iterator
			>
		class LangVariant: public TBase<StringT, IteratorT>
		{
			public:
				typedef	StringT	string_type;
				typedef IteratorT	iterator;

			public:
				LangVariant() : TBase<StringT, IteratorT>::TBase() { init(); }
				LangVariant(IteratorT it)
					 : TBase<StringT, IteratorT>::TBase(it) { init(); }
				LangVariant(IteratorT begin, IteratorT end)
					 : TBase<StringT, IteratorT>::TBase(begin, end) { init(); }
				LangVariant(StringT content) :
					TBase<StringT, IteratorT>::TBase() {
					init();
					this->create(content);
				}
				virtual ~LangVariant() {}

				virtual std::string to_html() {
					return "";
				}

				virtual bool is_start(IteratorT& it) {
					if (*it == '-' && (*++it) == '{') {
						++it;
						return true; // TBase<StringT, IteratorT>::is_start(it);
					}
					return false;
				}

				virtual bool is_pause(IteratorT& it) {
					return *it == ';';
				}

				/**
				 * It has clear end boundary, it end only when a boundary is seen
				 */
				virtual bool is_end(IteratorT& it, bool advance=true) {
					if (*it == '}') {
						IteratorT next = it + 1;
						if (*next == '-') {
							if (advance)
								it = next + 1;
							return true;
						}
					}
					return false;
				}

			private:
				void init() { }
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

				virtual std::string to_html() {
					std::stringstream ss;
					int count = 0;
					std::string name = this->name_.to_std_string();
					if (name == "lang") {
						ss << "<span type=\"template\" lang=";
						auto it = this->children_.begin();
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
						ss << "</span>";

					}
					else {
						
						
						auto it = this->children_.begin();

						// if (this->children_.size() == 1) {
						// 	ss << "<template ";
						// 	ss << " name=\"" << name << "\"";
						// 	ss << " value=\"" << (*it)->to_string() << "\"";
						// 	ss << "></template>";
						// }
						// else if (this->children_.size() == 2) {
						// 	ss << "<span type=\"template\"";
						// 	ss << " " << name <<  "=\"" << (*it++)->to_string() << "\"";
						// 	ss << ">" << (*it)->to_string() << "</span>";
						// }
						// else {
							ss << "<template ";
							ss << " name=\"" << name << "\">";
							while (it != this->children_.end()) {
								// if (count >= 0) 
								// 	ss << " ";
								ss  << (*it)->to_html();

								++count;
								++it;
							}
							ss << "</template>";
						// }
						
					}
					return ss.str();
				}

				virtual bool is_start(IteratorT& it) {
					if (*it == '{' && (*++it) == '{')
						return TBase<StringT, IteratorT>::is_start(++it);
					return false;
				}

				virtual bool is_pause(IteratorT& it) {
					return (*it == '|');
				}

				/**
				 * It has clear end boundary, it end only when a boundary is seen
				 */
				virtual bool is_end(IteratorT& it, bool advance=true) {
					if (*it == '}') {
						IteratorT next = it + 1;
						if (*next == '}') {
							if (advance)
								it = next + 1;
							return true;
						}
					}
					return false;
				}

			private:
				void init() { }
		};

		template <typename StringT = std::string, typename IteratorT = typename StringT::iterator>
		class LayoutOrderedList: public WikiEntity<StringT, IteratorT>
		{
			public:
				typedef	StringT	string_type;
				typedef IteratorT	iterator;

			public:
				LayoutOrderedList() : WikiEntity<StringT, IteratorT>::WikiEntity() { init(); }
				LayoutOrderedList(IteratorT it)
					 : WikiEntity<StringT, IteratorT>::WikiEntity(it) { init(); }
				LayoutOrderedList(IteratorT begin, IteratorT end)
					 : WikiEntity<StringT, IteratorT>::WikiEntity(begin, end) { init(); }
				LayoutOrderedList(StringT content) :
					WikiEntity<StringT, IteratorT>::WikiEntity() {
					init();
					this->create(content);
				}
				virtual ~LayoutOrderedList() {}

				virtual std::string to_html() {
					return "<ol>" + WikiEntity<StringT, IteratorT>::to_html() + "</ol>";
				}

				virtual bool is_end(IteratorT& it, bool advance=true) {
					if (*it == '\n') {
						IteratorT next = it + 1;
						this->skip_whitespace(next);
						return *next != '#';
					}
					return WikiEntity<StringT, IteratorT>::is_end(it);
				}

			private:
				void init() {  }
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

				int			                                  level_;
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
				virtual ~WikiEntityLeveled() {}

				int get_level() const {
					return level_;
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
					return std::string(begin, end);
				}				

				virtual bool is_start(IteratorT& it) {
					while (*it == this->wiki_key_char_start_) {
						++level_;
						++it;
					}
					this->matched_levels_ = -level_;
					// this->skip_whitespace(it);
					// this->begin(it);
					return true;
				}

				virtual bool is_end(IteratorT& it, bool advance=true) {
					if (this->end_in_same_line_ && *it == '\n') {
						if (this->matched_levels_ != 0) {
							// OK, this is not a valid entity
							this->begin(it);
							this->end(it);
						}

						return true;
					}
					else if (this->eow(it))
						return true;
					else if (*it == this->wiki_key_char_end_) {
						IteratorT next = it + 1;
						++this->matched_levels_;
						while (*next == this->wiki_key_char_end_ && !this->eow(it)) {
							++this->matched_levels_;
							++next;
							if (this->matched_levels_ > 0)
								break;
							else if (this->end_in_same_line_ && *it == '\n')
								break;
						}
						
						level_ += this->matched_levels_;

						if (strict_ && this->matched_levels_ != 0) {
							this->begin(it);
							this->end(it);
						}
						// now there is a delimma, should we go strict or auto correct?
						
						if (advance)
							it = next;
#ifdef DEBUG
						else
							int a = 1;
#endif // DEBUG							s
						return true;
					}
					return WikiEntity<StringT, IteratorT>::is_end(it); // this->eow(it) || text_stop(it);
				}

				virtual void add_content(StringT& text) {
					this->ref().append(text);
				}

				virtual void set_wiki_key_char() = 0;

			private:
				void init() {
					level_ = 0;
					end_in_same_line_ = false;
					strict_ = false;
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

				// virtual std::string to_html() {
				// 	return "<section>" + this->to_std_string() + "</section>";
				// }

				virtual void set_wiki_key_char() override {
					this->wiki_key_char_start_ = WikiEntityConstants::WIKI_KEY_HEADING;
					this->wiki_key_char_end_ = WikiEntityConstants::WIKI_KEY_HEADING;
				}

				virtual bool is_end(IteratorT& it, bool advance=true) {
					// when the line ends it ends
					if (*it == '\n') {
						return true;
					}
					return WikiEntityLeveled<StringT, IteratorT>::is_end(it);
				}

			private:
				void init() {
					this->set_wiki_key_char();
					this->end_in_same_line_ = true;
					this->set_group(LAYOUT);
					this->set_type(LAYOUT_HEADING);
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
					this->wiki_key_char_start_ = '\'';
					this->wiki_key_char_end_ = '\'';
				}

				virtual bool is_end(IteratorT& it, bool advance=true) {
					if (WikiEntityLeveled<StringT, IteratorT>::is_end(it, advance)) {
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
						return true;
					}
					return WikiEntityLeveled<StringT, IteratorT>::is_end(it);
				}


				// we collect text node and others
				virtual bool is_pause(IteratorT& it) {
					return true;
				}				

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

			protected:
				virtual void set_wiki_key_char() override {
					this->wiki_key_char_start_ = ':';
					this->wiki_key_char_end_ = '\n';
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

				bool is_external() {
					return external_;
				}

				virtual bool is_start(IteratorT& it) {
					bool ret = WikiEntityLeveled<StringT, IteratorT>::is_start(it);
					if (ret) {
						this->external_ = this->matched_levels_ == 1;
						this->set_type(LINK_EXTERNAL);
						url_.begin(it);
						url_.end(it);
						anchor_.begin(it);
						anchor_.end(it);						
					}
					return ret;
				}

				// we collect text node and others
				// anything is a pause including new line
				virtual bool is_pause(IteratorT& it) {
					return true;
				}

				// virtual bool is_end(IteratorT& it, bool advance=true) {
				// 	// when the line ends it ends
				// 	// if (url_.end() == url_.begin() && (external_ && *it == ' ') || *it == '|') {
				// 	// 	url_.end(it++);
				// 	// 	anchor_.begin(it);
				// 	// 	anchor_.end(it);
				// 	// 	return false;
				// 	// }
				// 	bool ret = WikiEntityLeveled<StringT, IteratorT>::is_end(it, advance);
				// 	if (ret) {
				// 		anchor_.end(it - this->level_);
				// 		if (url_.begin() == anchor_.begin())
				// 			url_.end(anchor_.end());
				// 		// anchor and url are the same if there is only one property
				// 	}
				// 	return ret;
				// }

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
				
				/**
				 * For the link things might get a bit interesting
				 * 
				 */
				virtual std::string to_html() {
					auto first = this->children_.begin();
					auto second = this->children_.end() - 1;
					stringstream ss;
					if (this->size() > 2) {
						ss << "<div ";
						auto it = first + 1;
						while (it != second) {
							ss << (*it)->to_std_string();
							++it;
						}
						ss << ">" << std::endl;
					}
//					else {
						ss << "<a href=\"";
						if (this->external_) {
							ss << (*first)->to_std_string();
						}
						else {
							ss << WikiEntityVariables::protocol << "://" + WikiEntityVariables::host << WikiEntityVariables::path <<  (*first)->to_std_string();
						}
						ss << "\">";

						if (second > first)
							ss << (*second)->to_std_string();
						else
							ss << (*first)->to_std_string();
						ss << "</a>";
//					}
					if (this->size() > 2) {
						ss << "</div>" << std::endl;
					}
					return ss.str();
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

	}
}

#endif /*STPL_WIKI_ENTITY_H_*/
