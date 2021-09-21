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

#include "../lang/stpl_character.h"

namespace stpl {
	namespace WIKI {

		template <typename StringT = std::string, typename IteratorT = typename StringT::iterator>
		class WikiProperty : public Property<StringT, IteratorT>, public WikiEntity<StringT, IteratorT> {
			public:
				typedef std::map<StringT, WikiProperty*>	        attributes_type;
				typedef StringBound<StringT, IteratorT>             StringB;

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

				virtual void create_text_child(IteratorT it) {
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
					//if (this->has_delimiter()) {
						ss << "<property name=\"";
						std::string name = std::string(this->name_.begin(), this->name_.end());
						ss << name << "\">";
						if (this->children_.size() > 0) {

							// ss << name << "=";
							// if (this->has_quote())
							// 	ss << "\"";
							auto it = this->children_.begin();
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
					//}
					//return Property<StringT, IteratorT>::to_std_string();
					ss << "</property>";
					return ss.str();
				}

			protected:

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
					return WikiEntity<StringT, IteratorT>::is_end(it) || Property<StringT, IteratorT>::is_end(it);
				}

				virtual bool is_pause(IteratorT& it) {
					// when it come to '=', it may be the start of a new entity
					// so we pause and see
					if (WikiEntity<StringT, IteratorT>::is_pause(it)) {
						return true;
					}
					else if (*it == '=') {
						this->has_delimiter_ = true;

						++it;

						WikiEntity<StringT, IteratorT>::skip_invalid_chars(it);

						this->value_.begin(it);
						this->value_.end(it);

						return true;
					}
					else if (*it == '\n') {
						++it;
						return true;
					}
					else if (*it == '|')
						return true;
					return false;
				}

				virtual bool is_separated(IteratorT& it) {
					return *it == WikiEntityConstants::WIKI_KEY_PROPERTY_DELIMITER;
				}				

			private:
				void init() {
					this->has_delimiter_ = false;
					this->group_ = PROPERTY;
				}
		};


		template <
			typename StringT = std::string, 
			typename IteratorT = typename StringT::iterator
			>
		class Tag: public WikiKeyword<StringT, IteratorT>
		{
			public:
				typedef	StringT	string_type;
				typedef IteratorT	iterator;

			private:
				void init() { this->type_ = TEXT; }

			public:
				Tag() : WikiKeyword<StringT, IteratorT>::WikiKeyword() { init(); }
				Tag(IteratorT it)
					 : WikiKeyword<StringT, IteratorT>::WikiKeyword(it) { init(); }
				Tag(IteratorT begin, IteratorT end)
					 : WikiKeyword<StringT, IteratorT>::WikiKeyword(begin, end) { init(); }
				Tag(StringT content) :
					WikiKeyword<StringT, IteratorT>::WikiKeyword() {
					init();
					this->create(content);
				}
				virtual ~Tag() {}

			protected:
				virtual bool is_start(IteratorT& it) {
					this->skip_whitespace(it);
					this->begin(it);
					return true;
				}

				virtual bool is_end(IteratorT& it, bool advance=true) {
					return this->eow(it) || text_stop(it);
				}

				virtual bool text_stop(IteratorT next) {
					return WikiKeyword<StringT, IteratorT>::is_start_symbol(next);
				}

				virtual void add_content(StringT& text) {
					this->ref().append(text);
				}
		};		

		template <typename StringT = std::string,
			 typename IteratorT = typename StringT::iterator,
			 typename AttributeT = WikiProperty<StringT, IteratorT> >
		class ElemTag: public Tag<StringT, IteratorT>
		{
			public:
				typedef	StringT										string_type;
				typedef IteratorT									iterator;
				typedef AttributeT 									attribute_type;
				typedef typename attribute_type::attributes_type	attributes_type;

			protected:
				attributes_type attributes_;

			private:
				StringBound<StringT, IteratorT> name_;

			public:
				ElemTag() : Tag<StringT, IteratorT>::Tag() {}
				ElemTag(IteratorT it) : Tag<StringT, IteratorT>::Tag(it), name_(it, it)  {
					init();
				}
				ElemTag(IteratorT begin, IteratorT end) :
					 Tag<StringT, IteratorT>::Tag(begin, end), name_(begin, begin) { init(); }

				ElemTag(StringT content) :
					Tag<StringT, IteratorT>::Tag(content) {
					init();
				}
				virtual ~ElemTag() {
					clear();
				};

				ElemTag& operator= (ElemTag& elem_k) {
					this->clone(reinterpret_cast<Tag<StringT, IteratorT>*>(&elem_k));
					return *this;
				}

				ElemTag& operator= (const ElemTag* elem_k_ptr) {
					this->clone(reinterpret_cast<Tag<StringT, IteratorT>*>(elem_k_ptr));
					return *this;
				}

				ElemTag& operator= (Tag<StringT, IteratorT>* elem_k_ptr) {
					this->clone(elem_k_ptr);
					return *this;
				}

				bool has_attribute(const StringT attr) const {
					return this->attributes_.find(attr) != this->attributes_.end();
				}

				StringT get_attribute(const StringT attr) const {
					typename attributes_type::const_iterator it = this->attributes_.find(attr);
					if (it != this->attributes_.end()) {
						return it->second->value();
					}
					return "";
				}

				std::pair<bool, StringT> attribute(const StringT attr) const
				{
					//if (this->attributes_.size() > 0) {
						//typename std::map<StringT, StringT>::const_iterator i = this->attributes_.find(attr);
						/*
						typename attributes_type::const_iterator i = this->attributes_.find(attr);
						if (i != this->attributes_.end()) {
							return make_pair(true, i->second->value());
						}
						*/
					//}
						/* sorry, I just cannot find the thing I want using map.find()*/
						typename attributes_type::const_iterator	it = attributes_.begin();
						for (; it!= attributes_.end(); it++) {
							if (it->first == attr)
								return make_pair(true, it->second->value());
						}

					return make_pair(false, StringT());
				}

				/*
				 *  defines valid characters for tag name
				 *  initially, all readable chars are valid except whitespace
				 */
				bool is_valid_name_char(IteratorT it) {
					if (isspace(*it) || iscntrl(*it)
							|| (WikiEntityConstants::WIKI_KEY_CLOSE_TAG == *it)
							|| (WikiEntityConstants::WIKI_KEY_OPEN_TAG == *it)
							)
						return false;
					else if (WikiEntityConstants::WIKI_KEY_OPEN_TEMPLATE == *it) {
						this->is_ended_wiki_keyword_ = true;
						return false;
					}

					return true;
				}

				StringBound<StringT, IteratorT>& name() {
					if (name_.length() == 0) {
						// TODO assert it is the end of the ElemTag already
						IteratorT it = this->body_.begin();
						name_.begin(it);

						while(isalnum(*it) && it != this->end())
							it++;
						name_.end(it);
					}
					//return StringT(name_.begin(), name_.end());
					return name_;
				}

				virtual IteratorT match(IteratorT begin, IteratorT end) {
					if (Tag<StringT, IteratorT>::match(begin, end)) {
						if (this->type_ == TEXT)
							return true;
					}
					return false;
				}

				virtual void print(std::ostream &out = cout, int level = 0) {
					name().print(out, level);
					print_attributes(out, level);
					out << endl;
				}

				void print_attributes(std::ostream &out, int level) {
					if (attributes_.size() > 0) {
						out << "(";
						typename attributes_type::iterator	it = attributes_.begin();
						for (; it!= attributes_.end(); it++) {
							out << it->second->name()
									<<	":"
									<<  it->second->value()
							        << " ";
						}
						out << ")" << std::endl;
					}
				}

				void parse_attributes() {
					IteratorT begin = name_.end;
					IteratorT end = this->end();
					while (parse_attribute(begin, end)) {
						begin = end;
						end = this->end();
					}
				}

				virtual bool required_end_tag() {
					return true;
				}

				virtual bool make_following_element_as_child(ElemTag* next_tag_ptr) {
					return false;
				}

				virtual bool force_close(ElemTag* next_tag_ptr) {
					return false;
				}

				void new_attribute(StringT name, StringT value) {
					AttributeT* attr_ptr = new AttributeT();
					attr_ptr->create(name, value);
					attributes_.insert(make_pair(name, attr_ptr));
				}

				virtual void flush(int level=0) {
					StringT indent(level*2, ' ');

					this->ref().erase();
					this->ref().insert(0, indent);

					this->ref().push_back(WikiEntityConstants::WIKI_KEY_OPEN_TAG);

					if (this->is_end_wiki_keyword()) {
						this->ref().push_back(WikiEntityConstants::WIKI_KEY_OPEN_TEMPLATE);
						this->ref().append(name_.to_string());
					}
					else {
						this->ref().append(name_.to_string());
						if (attributes_.size() > 0) {
							typename attributes_type::const_iterator	it = attributes_.begin();
							StringT temp("");
							for (; it!= attributes_.end(); it++) {
								temp.append(" ");

//								std::string::size_type pos = this->ref().rfind(WikiEntityConstants::WIKI_KEY_CLOSE_TAG);
//								if (pos == std::string::npos)
//									pos = this->ref().length();

								it->second->create();
								temp.append(it->second->ref());

//								this->ref().insert(pos, temp);
							}

							this->ref().append(temp);
						}

						if (this->is_ended_wiki_keyword())
							this->ref().push_back(WikiEntityConstants::WIKI_KEY_OPEN_TEMPLATE);
					}

					this->ref().push_back(WikiEntityConstants::WIKI_KEY_CLOSE_TAG);
				}

			private:
				void init() { this->type_ = TEXT; }

				void clear() {
					if (attributes_.size() > 0) {
						typename attributes_type::const_iterator it = attributes_.begin();
						for (; it!= attributes_.end(); it++) {
							delete it->second;
						}
						attributes_.clear();
					}
				}

				bool parse_attribute(IteratorT& begin, IteratorT& end) {
					this->skip_invalid_chars(begin);
					AttributeT* attr_ptr = new AttributeT(begin, end);
					bool ret = false;
					if ((ret = attr_ptr->match(begin, end))) {
						attributes_.insert(make_pair(attr_ptr->name(), attr_ptr));
						end = attr_ptr->end();
					}
					if (!ret) {
						delete attr_ptr;
						return false;
					}
					return ret;
				}

			protected:

				virtual bool is_start(IteratorT& it) {
					if (Tag<StringT, IteratorT>::is_start(it) && this->type_ == TEXT) {
						it = this->body_.begin();
						name_.begin(it);
						while (!this->eow(it) && is_valid_name_char(it))
							++it;
						name_.end(it);
						return true;
					}
					return false;
				}

				virtual bool is_end(IteratorT& it, bool advance=true) {
					if (!Tag<StringT, IteratorT>::is_end(it)) {
						IteratorT begin = it;
						IteratorT end = this->end();
						if (parse_attribute(begin, end)) {
							it = end;
							if (Tag<StringT, IteratorT>::is_end(it)) {
								this->body_.end(it);
								return true;
							}
						}
						return false;
					}

					return true;
				}

				virtual void add_content(StringT& text) {
					add_name(text);
					//add_attributes(text);
				}

				void add_name(StringT& name) {
					// debug
					///cout << "add name for elemtag " << endl;
					this->ref().append(name);
					//IteratorT begin = this->ref().end() - name.length();
					//name_.set_begin(this->ref().length());
					//name_.set_end(name.length());
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

			protected:
				virtual bool is_end(IteratorT& it, bool advance=true) {
					return *it == '\n';
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

				virtual bool is_start(IteratorT& it) {
					if (*it == WikiEntityConstants::WIKI_KEY_LIST) {
						++it;
						return true;
					}
					return false;
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

				virtual bool is_start(IteratorT& it) {
					if (*it == WikiEntityConstants::WIKI_KEY_LIST_ORDERED) {
						++it;
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

		template <typename StringT = std::string,
							typename IteratorT = typename StringT::iterator
						  >
		class Comment : public WikiKeyword<StringT, IteratorT>
		{
			public:
				typedef	StringT	string_type;
				typedef IteratorT	iterator;

			private:
				void init() { this->type_ = COMMENT; }

			public:
				Comment()  :
					WikiKeyword<StringT, IteratorT>::WikiKeyword(){}
				Comment(IteratorT it) :
					WikiKeyword<StringT, IteratorT>::WikiKeyword(it)  {
					init();
				}
				Comment(IteratorT begin, IteratorT end) :
					WikiKeyword<StringT, IteratorT>::WikiKeyword(begin, end) { init(); }
				Comment(StringT content) :
					WikiKeyword<StringT, IteratorT>::WikiKeyword(content) {
					init();
				}
				virtual ~Comment() {};


				Comment& operator= (WikiKeyword<StringT, IteratorT>* elem_k_ptr) {
					this->clone(elem_k_ptr);
					return *this;
				}

				virtual IteratorT match(IteratorT begin, IteratorT end) {
					if (WikiKeyword<StringT, IteratorT>::match(begin, end)) {
						if (this->type_ == COMMENT)
							return true;
					}
					return false;
				}
		};


		template <typename StringT = std::string
			, typename IteratorT = typename StringT::iterator>
		class WikiNodeTypes {
			public:
				typedef BasicWikiEntity<StringT, IteratorT> 				basic_entity;
				typedef WikiKeyword<StringT, IteratorT>						keyword_type;
				typedef ElemTag<StringT, IteratorT> 						tag_type;
				typedef Text<StringT, IteratorT> 							text_type;
				// typedef Link<StringT, IteratorT> 							link_type;
				typedef TBase<StringT, IteratorT> 						    template_type;
				typedef Entity<basic_entity>								container_type;
				typedef typename container_type::container_entity_type		container_entity_type;
		};

		template <typename StringT = std::string
							, typename IteratorT = typename StringT::iterator
							, typename NodeTypesT = WikiNodeTypes<StringT, IteratorT>
						  >
		class WikiEntityContainer: public WikiEntity<StringT, IteratorT>
		{
			public:
				typedef StringT													string_type;
				typedef IteratorT												iterator;

			public:
				WikiEntityContainer() : WikiEntity<StringT, IteratorT>::WikiEntity()
							 { init(); }
				WikiEntityContainer(IteratorT it) :
					WikiEntity<StringT, IteratorT>::WikiEntity(it)/*, start_k_(it, it)*/
					 { init(); }
				WikiEntityContainer(IteratorT begin, IteratorT end) :
					WikiEntity<StringT, IteratorT>::WikiEntity(begin, end)/*, start_k_(begin, begin)*/
					 { init(); }

				virtual ~WikiEntityContainer() {
				}

			protected:
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

		template <typename StringT = std::string,
				typename IteratorT = typename StringT::iterator,
				typename NodeTypesT = WikiNodeTypes<StringT, IteratorT>
				>
		class WikiTag: public  WikiEntityContainer<StringT, IteratorT>
		{
			public:
				typedef	StringT	                                             string_type;
				typedef IteratorT	                                         iterator;

				typedef Comment<StringT, IteratorT>							 comment_type;
				typedef typename NodeTypesT::basic_entity 					 basic_entity;
				typedef typename NodeTypesT::container_type					 container_type;
				typedef typename NodeTypesT::tag_type::attribute_type		 attribute_type;
				typedef typename container_type::entity_iterator			 entity_iterator;

				typedef typename std::map<StringT, bool>					 ie_map; /// include or exclude map

				typedef typename container_type::container_entity_type		 container_entity_type;
				typedef list<
					typename container_type::container_entity_type
							>												 tree_type;

			protected:
				typedef typename NodeTypesT::text_type 						 TextT;
				typedef typename NodeTypesT::tag_type						 ElemTagT;
				typedef typename NodeTypesT::keyword_type					 WikiKeywordT;
				typedef StringBound<StringT, IteratorT> 					 StringB;

			private:
				ElemTagT* 													 start_k_;
				ElemTagT*													 end_k_;
				ElemTagT* 													 last_tag_ptr_;

				//Layout* parent_;
				StringT														 xpath_;

			public:
				WikiTag() : WikiEntityContainer<StringT, IteratorT>::Layout() {
					init();
				}
				WikiTag(IteratorT it) :
					WikiEntityContainer<StringT, IteratorT>::Layout(it){
					init();
				}
				WikiTag(IteratorT begin, IteratorT end) :
					WikiEntityContainer<StringT, IteratorT>::Layout(begin, end){
					init();
				}
				WikiTag(StringT name) :
					WikiEntityContainer<StringT, IteratorT>::Layout(name) {
					init();
					create(name);
				}
				virtual ~WikiTag() {
					cleanup();
				}

				void set_start_keyword(ElemTagT* start_k) {
					start_k_ = start_k;
				}

				void set_last_tag(ElemTagT* last_tag_ptr) {
					last_tag_ptr_ = last_tag_ptr;
				}

				std::pair<bool, StringT> attribute(const StringT attr) const {
					if (start_k_)
						return start_k_->attribute(attr);
					return make_pair(false, StringT());
				}

				bool has_attribute(const StringT attr) {
					if (start_k_)
						return start_k_->has_attribute(attr);
					return false;
				}

				StringT get_attribute(const StringT attr) {
					if (start_k_)
						return start_k_->get_attribute(attr);
					return "";
				}

				WikiTag *get_descendent_node_by_xpath(const char *xpath, StringT& attr_name, StringT& attr_value) {
					const char *pos = xpath;
					StringT first_tag;
					while (*pos != '\0' && *pos != '/' && *pos != '[' && *pos != ':')
						first_tag.push_back(*pos++);

					int index = -1;
					if (*pos == '[') {
						index = atoi(++pos);
						while (isdigit(*pos))
							++pos;
						if (*pos == ']')
							++pos; // ]
					}

					int count = 0;
					WikiTag *elem = NULL;
					// allow non-stard wiki file with no single document root
					entity_iterator	it;
					for (it = this->iter_begin(); it != this->iter_end(); ++it) {
						basic_entity *node = static_cast<basic_entity*>((*it));

						if (node->is_element()) {
							++count;
							elem = static_cast<WikiTag *>(node);
							StringT name = elem->name();
							if (name == first_tag && (index == -1 || (count - 1) == index)) {
								if (*pos == '/') {
									WikiTag *d_elem = elem->get_descendent_node_by_xpath(++pos, attr_name, attr_value);
									elem = d_elem;
								}
								else if (attr_value.length() > 0) {
									StringT value2 = elem->get_attribute(attr_name);
									if (attr_value != value2) {
										elem = NULL;
										continue;
									}
								}
								break;
							}
							else
								elem = NULL;

						}
					}
					return elem;
				}

			protected:
				virtual bool is_last_tag_end_tag() {
					if (last_tag_ptr_/*&& last_tag_ptr_->length() > 0*/) {
						if (last_tag_ptr_->is_end_wiki_keyword()) {
							if (start_k_) {
								StringT	last_tag_name(last_tag_ptr_->name().begin(), last_tag_ptr_->name().end());
								StringT start_k_name(start_k_->name().begin(), start_k_->name().end());
								if (last_tag_name == start_k_name) {
									// TODO assert elem_k is closed elem
									end_k_ = last_tag_ptr_;
									last_tag_ptr_ = NULL;
									IteratorT end = end_k_->begin();
									this->children.end(end);

									this->end(end_k_->end());
								} else {
									// TODO error message here for wiki
									// but could be alright for html
									if (this->parent() && this->parent()->is_element()) {
										reinterpret_cast<WikiTag*>(this->parent())->set_last_tag(last_tag_ptr_);
										IteratorT end = last_tag_ptr_->begin();
										this->children.end(end);
										this->end(end);
										last_tag_ptr_ = NULL;
									}
									else {
									// there may be error here,since the Layout is not opened
									// but the parser should be error-tolorant with HTML
										return false;
									}
								}
							} else {
								// TODO error message for not opening the tag yet
								end_k_ = last_tag_ptr_;
								last_tag_ptr_ = NULL;
								IteratorT begin = end_k_->begin();
								this->children.begin(begin);
								this->children.end(begin);
								this->end(end_k_->end());
							}
							return true;

						} else { // if last_tag is not a end tag
							bool ret = false;
							if (this->start_k_) {
								// if the end tag is optional or fibidden
								// then it is the end
								if (!this->start_k_->required_end_tag())
									ret = true;

								if (ret && this->start_k_->make_following_element_as_child(last_tag_ptr_))
									ret = false;

								if (!ret && this->start_k_->force_close(last_tag_ptr_))
									ret = true;
							} //else {
								//ret = true;
							//}

							if (ret) {
								IteratorT end = last_tag_ptr_->begin();
								if (this->parent() && this->parent()->is_element()) {
									reinterpret_cast<WikiTag*>(this->parent())->set_last_tag(last_tag_ptr_);
									last_tag_ptr_ = NULL;
								}
								this->children.end(end);
								this->end(end);
							}
							return ret;
						}
						return false;
					}
					return true;
				}

				virtual bool is_start(IteratorT& it) {
					bool ret = false;
					if (start_k_ && start_k_->length() > 0) {
						ret = true;
					}
					else {
						IteratorT begin = this->begin();
						IteratorT end = this->end();

						ret = assign_start_tag(begin, end);
					}

					if (ret) {
						this->type_ = TAG;
						assert(start_k_);
						this->begin(start_k_->begin());
						this->children.begin(start_k_->end());
					}
					return ret;
				}

				virtual bool is_end(IteratorT& it, bool advance=true) {
					bool ret = false;
					if (start_k_) {
						if (start_k_->is_ended_wiki_keyword())
							ret = true;

						if (start_k_->is_end_wiki_keyword()) {
							// may print error message here
							// since the Layout is closed without opening it
							ret = true;
						}

						if (ret) {
							this->children.end(start_k_->end());
							return ret;
						}
					}

					if (!last_tag_ptr_) {
						match_text(it);
						//IteratorT end = this->end();

						// if it is the end of character stream
						if (this->eow(it)) {
							//TODO error message here for wiki
							// but could be valid for html
							this->children.end(it);
							this->end(it);
							return true;
						}

						// cleanup_last_tag();
						// skip non valid char or get next tag
						skip_invalid_chars(it);
					}

					if (!last_tag_ptr_ || is_last_tag_end_tag()) {
 						it = this->end();
 						return true;
 					}

 					assert(last_tag_ptr_);

					IteratorT end = this->end();
					IteratorT begin = last_tag_ptr_->begin();
					IteratorT new_begin = last_tag_ptr_->end();
 					WikiTag* child = new WikiTag(begin, end);
					child->set_parent(reinterpret_cast<basic_entity* >(this));
					child->set_start_keyword(last_tag_ptr_);
					child->content().begin(last_tag_ptr_->end());
					last_tag_ptr_ = NULL;

					if (child->resume_match(new_begin, end) && child->length() > 0) {
						this->add(child);

						//if (last_tag_ptr_ && is_last_tag_end_tag()) {
						//	it = this->end();
						//	return true;
						//}
						// process the text after the child
						// match_text(it = end);
						if (last_tag_ptr_)
							it = last_tag_ptr_->end();
						else
							it = child->end();
						// because it will return false, that means it will move ahead one character
						// in the parent "match" function, and we don't want to miss the current possition
						// that is why --it
						--it;
						return false;
					}
					// TODO someting must go wrong here, output error message
					it = begin; // where the error happens
					this->children.end(it);
					delete child;
					return true;

					//--it;
					//return false;
					// TODO something wrong happens here
					//return true;
				}

				bool assign_start_tag(IteratorT begin, IteratorT end) {
					bool ret = false;
					if (!last_tag_ptr_) {
						if (!start_k_)
							start_k_ = new ElemTagT(begin, end);
						ret = start_k_->match(begin, end);
					}
					else {
						ret = true;
						start_k_ = last_tag_ptr_;
						last_tag_ptr_ = NULL;
					}
					return ret;
				}

				virtual IteratorT skip_invalid_chars(IteratorT& it) {
					this->skip_whitespace(it);

					if (!last_tag_ptr_) {
						last_tag_ptr_ = new ElemTagT(it, it);

						bool ret = false;
						while (!this->eow(it) && last_tag_ptr_->length() <= 0 )
							if (!(ret = this->skip_comment_unknown_node(last_tag_ptr_, it))) {
								cleanup_last_tag();
								break;
							}
					}
					return it;
				}

				// comments could be in any place, like before, after or anywhere in the middle of a Layout
				bool skip_comment_unknown_node(ElemTagT* tag_ptr, IteratorT& begin) {
					bool ret = true;

					IteratorT orig_begin = begin;
					IteratorT end = this->end();
					WikiKeywordT* keyword_ptr = new WikiKeywordT(begin, end);

					if (keyword_ptr->detect(begin)) {
						//begin = keyword_ptr->end();
						IteratorT new_begin = keyword_ptr->content().begin();
						if (keyword_ptr->type() == COMMENT) {
							comment_type* node_ptr = new comment_type(orig_begin, end);
							node_ptr->set_parent(reinterpret_cast<basic_entity* >(this));
							node_ptr->clone(keyword_ptr);
							//node_ptr->detected(true);
							if ((ret = node_ptr->resume_match(new_begin, end))) {
								begin = node_ptr->end();
								this->add(reinterpret_cast<basic_entity* >(node_ptr));
							}

							delete keyword_ptr;
							keyword_ptr = NULL;
						}
						else if (keyword_ptr->type() == TEXT) {
							tag_ptr->clone(keyword_ptr);
							//tag_ptr->detected(true);
							if ((ret = tag_ptr->match(orig_begin, end)))
								begin = tag_ptr->end();

							delete keyword_ptr;
							keyword_ptr = NULL;
						}
						else {
							keyword_ptr->set_parent(reinterpret_cast<basic_entity* >(this));
							//keyword_ptr>detected(true);
							if ((ret = keyword_ptr->resume_match(new_begin, end))) {
								begin = keyword_ptr->end();
								this->add(reinterpret_cast<basic_entity* >(keyword_ptr));
							}
						}
					} else {
						delete keyword_ptr;
						ret = false;
					}

					return ret;
				}

				virtual void match_text(IteratorT& next) {
					IteratorT end = this->end();
					TextT* text = new TextT(next, end);
					text->set_parent(reinterpret_cast<basic_entity* >(this));
					if (text->match(next, end))	{
						this->add(reinterpret_cast<basic_entity*>(text));
						next = text->end();
					}
					else
						delete text;
				}

				virtual void add_start(StringT& text) {
					this->add_start_tag(text);
				}

				virtual void add_content(StringT& text) {
					//add_children(text);
				}

				virtual void add_end(StringT& text) {
					this->add_end_tag(text);
				}

				void add_start_tag(StringT& text) {
					if (start_k_)
						delete start_k_;

					start_k_ = new ElemTagT();
					//assert(start_k_->ref() == this->ref());
					start_k_->create(text);

				}

				void add_end_tag(StringT& text) {
					if (end_k_)
						delete end_k_;

					end_k_ = new ElemTagT();
					//start_k_->ref(this->ref());
					end_k_->set_end_wiki_keyword(true);
					end_k_->create(text);
				}

			public:
				virtual StringT name() {
					if (start_k_)
						return StringT(start_k_->name().begin(), start_k_->name().end());
					return StringT("");
				}

				virtual void print(std::ostream &out = cout, int level = 0) {
					print_tag(out, level);
					//print_text(level);
					out << std::endl;
					if (this->size() > 0) {
						print_childrent(out, level + 1);
						out << std::endl;
					}
				}

				void print_attributes(std::ostream &out = cout, int level = 0) {
					if (start_k_)
						start_k_->print_attributes(out, level);
				}

				void text(StringT& text) {
					ie_map nm;
					this->text(text, false, nm, true);
				}

				void all_text(StringT& text) {
					ie_map nm;
					this->text(text, true, nm, true);
				}

				void all_text(StringT& text
						, ie_map& nm
						, bool sub_text = false
						, bool force = false) {
					this->text(text, true, nm, sub_text, force);
				}

				StringT text() {
					StringT text;
					this->text(text);
					return text;
				}

				StringT all_text() {
					StringT text;
					this->all_text(text);
					return text;
				}

				void traverse(tree_type& tree) {
					entity_iterator it;
					for (it = this->iter_begin(); it != this->iter_end(); ++it) {
					//this->reset();
					//while (this->more()) {
					//	entity_iterator it = this->next();
						tree.push_back(*it);
						if ((*it)->is_element()) {
							reinterpret_cast<WikiTag*>(*it)->traverse(tree);
						}
					}
				}

				void find_children(tree_type& tree) {
					find_children("", tree, true);
				}

				void find_children(StringT name, tree_type& tree) {
					find_children(name, tree, false);
				}

				void find(StringT name, tree_type& tree) {
					if (this->name() == name) {
						tree.push_back(this);
					}
					else {
						find_children(name, tree, false);
					}
					return;
				}

				WikiTag* find_child(StringT child_name, int index = -1) {
					entity_iterator it;
					int count = -1;
					WikiTag* child = NULL;
					for (it = this->iter_begin(); it != this->iter_end(); ++it) {
						++count;
						if ((*it)->is_element()) {
							child = reinterpret_cast<WikiTag *>(*it);
							StringT name = child->name();
							// debug
							// cout << "comparing with " << name << endl;
							if (name == child_name) {
								if (index == -1 || (index > -1 && count == index))
									break;
								child = NULL;
							}
							else
								child = child->find_child(child_name, index);
						}
					}
					return child;
				}

				virtual StringB& content() {
					return this->children;
				}

				void name(StringT name) {
					if (start_k_) {
						delete start_k_;
					}
				}

				void new_text(StringT text) {
					TextT* text_ptr = new TextT(text);
					this->add(reinterpret_cast<basic_entity*>(text_ptr));
				}

				void new_child(WikiTag* child_ptr) {
					this->add(reinterpret_cast<basic_entity*>(child_ptr));
				}

				void new_attribute(StringT name, StringT value) {
					if (start_k_)
						start_k_->new_attribute(name, value);
				}

				virtual void flush(int level=0) {
					StringT indent(level*2, ' ');
					bool n_flag = false;

					this->ref().erase();

					//if (level > 0)
					//	this->ref().append(indent);

					if (start_k_) {
						start_k_->flush(level);
						this->ref().append(start_k_->ref());
					}

					entity_iterator it;
					for (it = this->iter_begin(); it != this->iter_end(); ++it) {
						this->ref().append("\n");

						(*it)->flush(level+1);
						this->ref().append((*it)->ref());

						this->ref().append("\n");

						n_flag = true;
					}

					if (end_k_ && !start_k_->is_ended_wiki_keyword()) {
						end_k_->flush(level);
						this->ref().append(end_k_->ref());
					}
					else { // fix the missing tag here
						if (!start_k_->is_ended_wiki_keyword()) {
							start_k_->set_end_wiki_keyword(true);
							start_k_->flush(level);
							this->ref().append(start_k_->ref());
							start_k_->set_end_wiki_keyword(false);
						}
					}
				}

				void set_empty() {
					if (start_k_)
						start_k_->set_ended_wiki_keyword(true);
				}

				StringT xpath() { return xpath_; }
				void xpath(StringT xpath) { xpath_ = xpath; }

			private:
				/**
				 * all_text is true for including children' texts
				 */
				void text(StringT& text
						, bool all_text
						, ie_map& nm
						, bool sub_text = false
						, bool force = false) {

					if (nm.size() > 0 &&
						(!sub_text || (force && sub_text))) {
						//static_cast<Layout*>(*it)->text(text, all_text, nm, sub_text, force);
						//else  {

						//Layout* tmp_elem_ptr = static_cast<Layout*>((*it));

						typename ie_map::iterator ie_node = nm.find(this->name());
						bool found = ie_node != nm.end();

						if (found) {
							if (!(ie_node->second))
								return;
							else
								sub_text = true;
								//static_cast<Layout*>(*it)->text(text, all_text, nm, true, force);
						}
						//else
						//	sub_text = false;
							//static_cast<Layout*>(*it)->text(text, all_text, nm, false, force);
					}

					this->reset();
					while (this->more()) {
						entity_iterator it = this->next();

						if ((*it)->type() == TEXT || (*it)->type() == TAG) {
							if (sub_text || (nm.size() == 0)) {
								if (text.length() > 0)
									text.append("\n");
								if ((*it)->type() == TAG)
									text.append((*it)->content().begin(), (*it)->content().end());
								else
									text.append((*it)->begin(), (*it)->end());
							}
						}
						else if ((*it)->is_element() && all_text) {
							static_cast<WikiTag*>(*it)->text(text, all_text, nm, sub_text, force);
						}
					}

				}

				virtual void print_tag(std::ostream &out, int level) {
					if (start_k_)
						start_k_->print(out, level);
				}

				virtual void print_end_tag(std::ostream &out, int level) {
					if (start_k_)
						start_k_->print(out, level);
				}

				void print_text(std::ostream &out, int level) {
					out << " - " ;
					this->reset();
					while (this->more()) {
						entity_iterator it = this->next();
						if ((*it)->type() == TEXT) {
							(*it)->print(out);
						}
					}
				}

				void print_childrent(std::ostream &out, int level) {
					this->reset();
					while (this->more()) {
						entity_iterator it = this->next();
						if ((*it)->type() == TAG) {
							reinterpret_cast<WikiTag*>(*it)->print(out, level + 1);
						} else {
							(*it)->print(out, level + 1);
						}
					}
				}


				// find children elements with child_name
				// if child name is empty, then return all children
				void find_children(StringT child_name, tree_type& tree, bool all) {
					//int count = 0;
					entity_iterator it;
					for (it = this->iter_begin(); it != this->iter_end(); ++it) {
						if ((*it)->is_element()) {
							WikiTag* child = reinterpret_cast<WikiTag*>(*it);
							StringT name = child->name();
							// debug
							// cout << "comparing with " << name << endl;
							if (name == child_name || all) {
								// create the most simple xpath
								std::ostringstream oss;
								oss << "//" << name << "[" << (tree.size()) << "]";
								child->xpath(oss.str());

								tree.push_back(*it);
							}
							else
								child->find(child_name, tree);
						}
					}
					return;
				}

				void init() {
					last_tag_ptr_ = NULL;
					start_k_ = NULL;
					end_k_ = NULL;
					this->children.begin(this->begin());
					this->children.end(this->begin());
					this->type(TAG);
				}

				void cleanup() {
					if (start_k_)
						delete start_k_;
					if (end_k_)
						delete end_k_;
					cleanup_last_tag();
				}

				void cleanup_last_tag() {
					if (last_tag_ptr_) {
						delete last_tag_ptr_;
						last_tag_ptr_ = NULL;
					}
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

			protected:
				virtual bool is_start(IteratorT& it) {
					if (*it == '{' && (*++it) == '|') {
						++it;
						return true; // TBase<StringT, IteratorT>::is_start(it);
					}
					return false;
				}

				virtual bool is_pause(IteratorT& it) {
					if (*it == '|') {
						IteratorT next = it + 1;
						return *next != '}';
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
					return false;
				}

			private:
				void init() { }
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

			protected:
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

						if (this->children_.size() == 1) {
							ss << "<template ";
							ss << " name=\"" << name << "\"";
							ss << " value=\"" << (*it)->to_html() << "\"";
							ss << "></template>";
						}
						else if (this->children_.size() == 2) {
							ss << "<span type=\"template\"";
							ss << " " << name <<  "=\"" << (*it++)->to_html() << "\"";
							ss << ">" << (*it)->to_html() << "</span>";
						}
						else {
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
						}
						
					}
					return ss.str();
				}

			protected:

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
		class WikiEntityOrdered: public WikiEntityContainer<StringT, IteratorT>
		{
			public:
				typedef	StringT	string_type;
				typedef IteratorT	iterator;

			public:
				WikiEntityOrdered() : WikiEntityContainer<StringT, IteratorT>::WikiEntityContainer() { init(); }
				WikiEntityOrdered(IteratorT it)
					 : WikiEntityContainer<StringT, IteratorT>::WikiEntityContainer(it) { init(); }
				WikiEntityOrdered(IteratorT begin, IteratorT end)
					 : WikiEntityContainer<StringT, IteratorT>::WikiEntityContainer(begin, end) { init(); }
				WikiEntityOrdered(StringT content) :
					WikiEntityContainer<StringT, IteratorT>::WikiEntityContainer() {
					init();
					this->create(content);
				}
				virtual ~WikiEntityOrdered() {}

			protected:
				virtual bool is_end(IteratorT& it, bool advance=true) {
					if (*it == '\n') {
						IteratorT next = it + 1;
						this->skip_whitespace(next);
						return *next != '#';
					}
					return WikiEntityContainer<StringT, IteratorT>::is_end(it);
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
		class WikiEntityLeveled: public WikiEntityOrdered<StringT, IteratorT>				
		{
			public:
				typedef	StringT	    string_type;
				typedef IteratorT	iterator;

			protected:
				char          wiki_key_char_start_;
				char          wiki_key_char_end_;

				int			  level_;
				int           matched_levels_;

				bool          end_in_same_line_;
				bool          strict_;

			public:
				WikiEntityLeveled() : WikiEntityOrdered<StringT, IteratorT>::WikiEntityOrdered() { init(); }
				WikiEntityLeveled(IteratorT it)
					 : WikiEntityOrdered<StringT, IteratorT>::WikiEntityOrdered(it) { init(); }
				WikiEntityLeveled(IteratorT begin, IteratorT end)
					 : WikiEntityOrdered<StringT, IteratorT>::WikiEntityOrdered(begin, end) { init(); }
				WikiEntityLeveled(StringT content) :
					WikiEntityOrdered<StringT, IteratorT>::WikiEntityOrdered() {
					init();
					this->create(content);
				}
				virtual ~WikiEntityLeveled() {}

				int get_level() const {
					return level_;
				}

			protected:
				virtual bool is_start(IteratorT& it) {
					while (*it == this->wiki_key_char_start_) {
						++level_;
						this->matched_levels_ = -level_;
						++it;
					}
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
						while (*it == this->wiki_key_char_end_ && !this->eow(it)) {
							++this->matched_levels_;
							++it;
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
						
						return true;
					}
					return WikiEntityOrdered<StringT, IteratorT>::is_end(it); // this->eow(it) || text_stop(it);
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

			protected:
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

			protected:
				virtual void set_wiki_key_char() override {
					this->wiki_key_char_start_ = '\'';
					this->wiki_key_char_end_ = '\'';
				}

				virtual bool is_end(IteratorT& it, bool advance=true) {
					if (WikiEntityLeveled<StringT, IteratorT>::is_end(it)) {
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
						url_.begin(it);
						url_.end(it);
						anchor_.begin(it);
						anchor_.end(it);						
					}
					return ret;
				}

				virtual bool is_end(IteratorT& it, bool advance=true) {
					// when the line ends it ends
					if (url_.end() == url_.begin() && (external_ && *it == ' ') || *it == '|') {
						url_.end(it++);
						anchor_.begin(it);
						anchor_.end(it);
						return false;
					}
					bool ret = WikiEntityLeveled<StringT, IteratorT>::is_end(it);
					if (ret) {
						anchor_.end(it - this->level_);
						if (url_.begin() == anchor_.begin())
							url_.end(anchor_.end());
						// anchor and url are the same if there is only one property
					}
					return ret;
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
				
				/**
				 * For the link things might get a bit interesting
				 * 
				 */
				virtual std::string to_html() {
					stringstream ss;
					ss << "<a href=\"";
					if (this->external_) {
						ss << url_.to_string();
					}
					else {
						ss << WikiEntityVariables::protocol << "://" + WikiEntityVariables::host << WikiEntityVariables::path << url_.to_string();
					}
					ss << "\">";
					ss << this->anchor_.to_std_string();
					ss << "</a>";
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
					this->set_wiki_key_char();
				}
		};

	}
}

#endif /*STPL_WIKI_ENTITY_H_*/
