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
 * @twitter				https://twitter.com/_e_tang
 *
 *******************************************************************************/

#ifndef STPL_WIKI_BASIC_H_
#define STPL_WIKI_BASIC_H_

#include "../stpl_entity.h"
#include "../stpl_property.h"

#include <map>
#include <iostream>

namespace stpl {
	namespace WIKI {
		
		// TEMPLATE node includes wiki declarations, text declarations,
		// If not any of others, then just TEXT
		enum WikiNodeGroup {
			LAYOUT, 
			LAYOUT_ITEM,
			TAG,
			LINK,
			TBASE,
			COMMENT,
			TEXT,
			PROPERTY,
			CELL,
			STYLE,
			LANG,
			SECTION,
			DEBUG
		};

		// NONE for un-initialized node, or just TEXT
		// MISC node type includes COMMENT, PI or DOCTYPE (COMMENT)
		enum WikiNodeType {NONE,
							// Sub node of LANG
							LANG_VARIANT,

							// Sub node types of STYLE
							STYLE_ITALIC,
							STYLE_BOLD,
							STYLE_BOTH,
							STYLE_INDENT,

							// Sub node types of LAYOUT
							LAYOUT_SECTION, 
							LAYOUT_HEADING, 
							LAYOUT_HZ_RULE, 
							LAYOUT_T_LB,
							LAYOUT_T_LI,
							LAYOUT_T_UL,
							LAYOUT_LI,
							LAYOUT_UL,
							LAYOUT_DESC,

							// Sub node types of LINK
							LINK_FREE,
							LINK_REDIRECT,
							LINK_LANG,
							LINK_INTERWIKI,
							LINK_CATEGORY,
							LINK_EXTERNAL,
							LINK_T_ASOF,
							LINK_MEDIA,
							LINK_IMAGE,

							// Sub node types of TABLE
							TABLE,

							// Sub node types of TEMPLATE
							TEMPLATE,
							TEMPLATE_PA,                         // pronuciation aids
							TEMPLATE_COLBEGIN, 	                // column begin
							TEMPLATE_COLEND,   					// column end
							TEMPLATE_DEFN, 						// definition / description lists

							// Sub node types of TAG
							TAG_REF,
							// Cite book, web, needed
							// attributes:
							// |isbn
							// |url
							// |title
							// |author
							// |first
							// |last
							// |location
							// |publisher
							// |date
							// |year
							// |accessdate

							// Templates
							TEMPLATE_COLOR,
							TEMPLATE_FONT_COLOR,
							TEMPLATE_FONT_STRIKE,

							TAG_NOWIKI,
							TAG_PRE,
							TAG_POEM,

							// Property Types
							P_PROPERTY,
							P_CELL,
							P_HEADER,
							P_ROW_HEADER,
							
							// Separator
							SEPARATOR
							}; 

		template <typename StringT = std::string, typename IteratorT = typename StringT::iterator>
		class WikiAttribute : public Property<StringT, IteratorT> {
			public:
				typedef std::map<StringT, WikiAttribute*>	attributes_type;

			public:
				WikiAttribute() :
					Property<StringT, IteratorT>::Property() { init(); }
				WikiAttribute(IteratorT begin) :
				    Property<StringT, IteratorT>::Property(begin) { init(); }
				WikiAttribute(IteratorT begin, IteratorT end) :
				    Property<StringT, IteratorT>::Property(begin, end) { init(); }
				WikiAttribute(StringT content) :
					Property<StringT, IteratorT>::Property(content) {
				}
				virtual ~WikiAttribute() {}

			private:
				void init() {
					// this->delimiter_ = '=';
					this->force_end_quote_ = true;
				}

			protected:
				virtual bool is_end_char(IteratorT& it) {
					if (Property<StringT, IteratorT>::is_end_char(it))
						return true;
					else {
						IteratorT next = it;
					}
					return false;
				}
		};

		class WikiEntityConstants {
			public:
				static const char WIKI_KEY_OPEN_TAG = '<';
				static const char WIKI_KEY_STYLE = '\'';
				static const char WIKI_KEY_STYLE_INDENT = ':';
				static const char WIKI_KEY_CLOSE_TAG = '>';
				static const char WIKI_KEY_HEADING = '=';
				static const char WIKI_KEY_LIST = '*';
				static const char WIKI_KEY_LIST_ORDERED = '#';
				static const char WIKI_KEY_OPEN_TEMPLATE = '{';
				static const char WIKI_KEY_CLOSE_TEMPLATE = '}';
				static const char WIKI_KEY_OPEN_LINK = '[';
				static const char WIKI_KEY_CLOSE_LINK = ']';
				static const char WIKI_KEY_PROPERTY_DELIMITER = '|';
				static const char WIKI_KEY_SLASH = '/';
		};

		class WikiEntityVariables {
			public:
				static std::string protocol;
				static std::string path;
				static std::string host;
		};

		template <typename StringT = std::string, typename IteratorT = typename StringT::iterator>
		class BasicWikiEntity : public StringBound<StringT, IteratorT> 
		{
			public:
				static int                                      counter;

			protected:
				WikiNodeGroup		 							group_;
				WikiNodeType 									type_;
				BasicWikiEntity*								parent_ptr_;
				StringBound<StringT, IteratorT> 				body_;
					
			private:
				void init() {
					group_ = TEXT;
					type_ = NONE;
					parent_ptr_ = NULL;

					Atom::set_id(Atom::counter++);
					// for debuging
					// if (*(this->begin()) == 'N')
					// 	cerr << Atom::counter << std::endl;
					if (Atom::get_id() == 15)
						parent_ptr_ = NULL;
				}
				
			public:
				BasicWikiEntity() : StringBound<StringT, IteratorT>::StringBound() {
					init();
				}
				BasicWikiEntity(IteratorT it) : StringBound<StringT, IteratorT>::StringBound(it), body_(it, it) {
					init();
				}
				BasicWikiEntity(IteratorT begin, IteratorT end) : StringBound<StringT, IteratorT>::StringBound(begin, end), body_(begin, begin) {
					init();
				}
				BasicWikiEntity(StringT content) : 
					StringBound<StringT, IteratorT>::StringBound(content) {
					init();
				}				
				virtual ~BasicWikiEntity() {}		
				
				WikiNodeType type() { return type_; }		
				void type(WikiNodeType type) { type_ = type; }
				
				virtual StringBound<StringT, IteratorT>& content() {
					return body_;
				}

				static bool is_start_symbol(IteratorT it) {
					if (*it == WikiEntityConstants::WIKI_KEY_OPEN_TAG
						|| *it == WikiEntityConstants::WIKI_KEY_HEADING
						|| *it == WikiEntityConstants::WIKI_KEY_LIST
						|| *it == WikiEntityConstants::WIKI_KEY_LIST_ORDERED
						|| *it == WikiEntityConstants::WIKI_KEY_OPEN_TEMPLATE
						|| *it == WikiEntityConstants::WIKI_KEY_OPEN_LINK
						)
						return true;
					return false;
				}
				
				static bool is_end_symbol(IteratorT it) {
					if (*it == WikiEntityConstants::WIKI_KEY_CLOSE_TAG
						|| *it == WikiEntityConstants::WIKI_KEY_HEADING
						|| *it == WikiEntityConstants::WIKI_KEY_CLOSE_TEMPLATE
						|| *it == WikiEntityConstants::WIKI_KEY_CLOSE_LINK
					)
						return true;
					return false;
				}
																		
				static bool is_key_symbol(IteratorT it) {
					if (is_start_symbol(it) || is_end_symbol(it))
						return true;
					return false;
				}
				
				// static WikiNodeType element_type() { return TAG; }
				
				BasicWikiEntity* parent() { return parent_ptr_; }
				BasicWikiEntity* get_parent() { return parent_ptr_; }
				void set_parent(BasicWikiEntity* parent_ptr) {
					parent_ptr_ = parent_ptr;
				}		

				bool is_element() { return type() == TAG; }

				WikiNodeGroup get_group() const {
					return group_;
				}

				void set_group(WikiNodeGroup group) {
					group_ = group;
				}

				WikiNodeType get_type() const {
					return type_;
				}

				void set_type(WikiNodeType type) {
					type_ = type;
				}

				/**
				 * By default we don't output
				 */
				virtual std::string to_html() {
					return "";
				}

				/**
				 * By default we use what it is for html
				 */
				virtual std::string to_json() {
					return "";
				}				

				virtual void process_child(BasicWikiEntity<StringT, IteratorT>* child) {
					// if it doesn't get implemented, it has no children 
				}

			protected:
				/**
				 * An entity is ended when 1) ended by the parent separator
				 *                         2) ended by parent end
				 *                         3) ended by end of input
				 */
				virtual bool is_end(IteratorT& it, bool advance=true) {
					return this->is_separated(it) || (this->parent_ptr_ && this->parent_ptr_->is_end(it, false))/*  || StringBound<StringT, IteratorT>::is_end(it) */;
				}			

				virtual bool is_pause(IteratorT& it) {
					// ok we are not gonna pause when the character with the following
					// that is not a good idea either that simply skip the first character simply because 
					//if (it > this->begin())
						return *it == '[' || *it == '{'  || *it == '-' || *it == '\'' || *it == '#' || *it == '*' || *it == ':';
					//return false;
				}			

				virtual bool is_separated(IteratorT& it) {
					return this->parent_ptr_ && this->parent_ptr_->is_delimiter(it);
				}					
		};

		template <typename StringT = std::string, typename IteratorT = typename StringT::iterator>
		class Text: public BasicWikiEntity<StringT, IteratorT>
		{
			public:
				typedef	StringT	string_type;
				typedef IteratorT	iterator;

			private:
				void init() { this->group_ = TEXT; }

			public:
				Text() : BasicWikiEntity<StringT, IteratorT>::BasicWikiEntity() { init(); }
				Text(IteratorT it)
					 : BasicWikiEntity<StringT, IteratorT>::BasicWikiEntity(it) { init(); }
				Text(IteratorT begin, IteratorT end)
					 : BasicWikiEntity<StringT, IteratorT>::BasicWikiEntity(begin, end) { init(); }
				Text(StringT content) :
					BasicWikiEntity<StringT, IteratorT>::BasicWikiEntity() {
					init();
					this->create(content);
				}
				virtual ~Text() {}	

				virtual std::string to_html() {
					return this->to_std_string();
				}						

			protected:

				/**
				 * For the text node, most likely it will encounter a link or template which will mark
				 * the end of it
				 * 
				 * But generally we won't end until a new enity is found which can't be just link and template
				 */
				// virtual bool is_end(IteratorT& it, bool advance=true) {
				// 	if (*it == '[' || *it == '{' || *it == '\n')
				// 		return true;
				// 	return ;
				// }
		};

		template <typename StringT = std::string, typename IteratorT = typename StringT::iterator>
		class Redirect: public BasicWikiEntity<StringT, IteratorT>
		{
			public:
				typedef	StringT	string_type;
				typedef IteratorT	iterator;

			private:
				void init() { this->group_ = TEXT; }

			public:
				Redirect() : BasicWikiEntity<StringT, IteratorT>::BasicWikiEntity() { init(); }
				Redirect(IteratorT it)
					 : BasicWikiEntity<StringT, IteratorT>::BasicWikiEntity(it) { init(); }
				Redirect(IteratorT begin, IteratorT end)
					 : BasicWikiEntity<StringT, IteratorT>::BasicWikiEntity(begin, end) { init(); }
				Redirect(StringT content) :
					BasicWikiEntity<StringT, IteratorT>::BasicWikiEntity() {
					init();
					this->create(content);
				}
				virtual ~Redirect() {}	

				virtual std::string to_html() {
					return this->to_std_string();
				}						

			protected:
				virtual bool is_start(IteratorT& it) {
					return true;
				}

				virtual bool is_pause(IteratorT& it) {
					return false;
				}

				/**
				 * For the text node, most likely it will encounter a link or template which will mark
				 * the end of it
				 */
		};

		template <typename StringT = std::string, typename IteratorT = typename StringT::iterator>
		class DebugNode: public BasicWikiEntity<StringT, IteratorT>
		{
			public:
				typedef	StringT	string_type;
				typedef IteratorT	iterator;

			private:
				void init() { this->group_ = DEBUG; }

			public:
				DebugNode() : BasicWikiEntity<StringT, IteratorT>::BasicWikiEntity() { init(); }
				DebugNode(IteratorT it)
					 : BasicWikiEntity<StringT, IteratorT>::BasicWikiEntity(it) { init(); }
				DebugNode(IteratorT begin, IteratorT end)
					 : BasicWikiEntity<StringT, IteratorT>::BasicWikiEntity(begin, end) { init(); }
				DebugNode(StringT content) :
					BasicWikiEntity<StringT, IteratorT>::BasicWikiEntity() {
					init();
					this->create(content);
				}
				virtual ~DebugNode() {}	

				virtual std::string to_html() {
					return "";
				}						

			protected:
				virtual bool is_start(IteratorT& it) {
					++it;
					return true;
				}

				virtual bool is_end(IteratorT& it, bool advance=true) {
					if ((it - this->begin()) > 5) {
						return true;
					}
					return false;
				}

				/**
				 * For the text node, most likely it will encounter a link or template which will mark
				 * the end of it
				 */
		};				

		template <typename StringT = std::string, typename IteratorT = typename StringT::iterator>
		class WikiEntity : public BasicWikiEntity<StringT, IteratorT>, public Entity<BasicWikiEntity<StringT, IteratorT> >
		{
			public:

			protected:
				std::map<StringT, WikiEntity>	         			properties_;
				int                                      			level_marks_;

				BasicWikiEntity<StringT, IteratorT>                 *last_child_;

			public:
				WikiEntity() : BasicWikiEntity<StringT, IteratorT>::BasicWikiEntity() {
				}
				WikiEntity(IteratorT it) : BasicWikiEntity<StringT, IteratorT>::BasicWikiEntity(it) {
					init();
				}
				WikiEntity(IteratorT begin, IteratorT end) : BasicWikiEntity<StringT, IteratorT>::BasicWikiEntity(begin, end) {
					init();
				}
				WikiEntity(StringT content) :
					BasicWikiEntity<StringT, IteratorT>::BasicWikiEntity(content) {
					init();
				}
				virtual ~WikiEntity() {}

				/**
				 * Not many kinds of entities will have direct text node as child(ren)
				 */
				virtual void create_text_child_pre(IteratorT it) {
					// so we do nothing here
					// last_child_ = new Text<StringT, IteratorT>(begin, it);
					// this->add(last_child_);
				}

				virtual void create_text_child_after(IteratorT it) {
					// so we do nothing here
					// IteratorT begin = last_child_->end();
					// process_child(new Text<StringT, IteratorT>(begin, it));
				}

				const BasicWikiEntity<StringT, IteratorT> *get_last_child() const {
					return last_child_;
				}

				void set_last_child(const BasicWikiEntity<StringT, IteratorT> *lastChild) {
					last_child_ = lastChild;
				}

				virtual void process_child(BasicWikiEntity<StringT, IteratorT>* child) {
					if (this->children_.size() == 0 && child->begin() > this->begin()) {
						// IteratorT begin = this->begin();
						// IteratorT end = child->begin();
						// this->add(new Text<StringT, IteratorT>(begin, end));
						create_text_child_pre(child->begin());
					}

					this->last_child_ = child;
					this->add(child);
				}

				virtual std::string to_html() {
					if (this->children_.size() > 0) {
						std::stringstream ss;
						
						auto it = this->children_.begin();
						for (; it != this->children_.end(); ++it)
							ss << (*it)->to_html();
						
						return ss.str();
					}
					return this->to_std_string();
				}

				std::string children_to_html() {
					std::stringstream ss;
					auto it = this->children_.begin();
					while (it != this->children_.end()) {
						ss << (*it++)->to_html();
					}
					return ss.str();
				}

			protected:

				virtual bool is_end(IteratorT& it, bool advance=true) {
					if (BasicWikiEntity<StringT, IteratorT>::is_end(it)) {
						if (last_child_ && it > last_child_->end()) {
							// we need to record the last text node
							this->create_text_child_after(it);
						}
						return true;
					}	
					return false;	
				}

				virtual bool is_pause(IteratorT& it) {
					/**
					 * For any entity when it comes to a pause
					 * we move the end char to it
					 * ===
					 * Why the above needs to do that
					 * Don't do it
					 */
					if (BasicWikiEntity<StringT, IteratorT>::is_pause(it)) {
						// no we can't end it here;
						// this->end(it);
						return true;
					}	
					return false;
				}								

			private:
				void init() {
					last_child_ = NULL;
				}
		};


		
		template <typename StringT = std::string, typename IteratorT = typename StringT::iterator>
		class WikiKeyword: public BasicWikiEntity<StringT, IteratorT> 
		{
			public:
				typedef	StringT	string_type;
				typedef IteratorT	iterator;			
				
			protected:
				bool					is_end_wiki_keyword_;      // </tag>
				bool					is_ended_wiki_keyword_;  // <tag/>
				bool					contain_intsubset_;
				
			public:
				WikiKeyword() : BasicWikiEntity<StringT, IteratorT>::BasicWikiEntity() {
					init();
				}
				WikiKeyword(IteratorT it) : BasicWikiEntity<StringT, IteratorT>::BasicWikiEntity(it) {
					init();
				}
				WikiKeyword(IteratorT begin, IteratorT end) : BasicWikiEntity<StringT, IteratorT>::BasicWikiEntity(begin, end) {
					init();
				}
				WikiKeyword(StringT content) :
					BasicWikiEntity<StringT, IteratorT>::BasicWikiEntity(content) {
					init();
				}
				virtual ~WikiKeyword() {}

				void set_end_wiki_keyword(bool b) {
					is_end_wiki_keyword_ = b;
				}

				void set_ended_wiki_keyword(bool b) {
					is_ended_wiki_keyword_ = b;
				}

				bool is_end_wiki_keyword() {
					return is_end_wiki_keyword_;
				}

				bool is_ended_wiki_keyword() {
					return is_ended_wiki_keyword_;
				}

				virtual void print(std::ostream &out = cout, int level = 0) {
					this->body_.print(out, level);
				}

				void clone(WikiKeyword* elem_k_ptr) {
					if (this != elem_k_ptr) {
						this->is_end_wiki_keyword_ = elem_k_ptr->is_end_wiki_keyword();
						this->is_ended_wiki_keyword_ = elem_k_ptr->is_ended_wiki_keyword();
						this->begin(elem_k_ptr->begin());
						this->end(elem_k_ptr->end());
						this->body_.begin(elem_k_ptr->content().begin());
						this->body_.end(elem_k_ptr->content().end());
					}
				}
				
			protected:					
				
				virtual bool is_start(IteratorT& it) {						
					bool ret = false;
					if (this->is_start_symbol(it)) {
						// decide what kind of node is
						IteratorT next = it;
						if (!this->eow(++next)) {
							if (*next == '!') {							 	
								if (!this->eow(++next) ) {
									if ( *(next) == '-' ) {
										if (!this->eow(++next)  && *(next) == '-' )									
											this->type_ = COMMENT;		
									} else if (*(next) == '[' ) {
										IteratorT begin = next;
										std::string keyword("[TAG[");
										this->skip_n_chars(next, keyword.length());
											
										if (std::string(begin, next) == keyword)
											this->type_ = TAG;
									} else {
										IteratorT begin = next;
										std::string keyword("DOCTYPE");
										this->skip_n_chars(next, keyword.length());
											
										if (std::string(begin, next) == keyword)
											this->type_ = COMMENT;										
									}				
								} 						 		

							} else if (*next == '?') {	
								this->type_ = TBASE;
								++next;
							} else if (*next == '/') {
								this->type_ = TEXT;
								this->is_end_wiki_keyword_ = true;
								++next;
							} else if (isalnum(*next) /*TODO put the UTF code here*/
								) {
								this->type_ = TEXT;
								//--next;
							} 
						}
						//body_.begin(++it);	
						this->begin(it);
						this->body_.begin(next++);
						it = next;
						ret = true;
					}
					return ret;
				}
				
				virtual bool is_end(IteratorT& it, bool advance=true) {
					if (this->eow(it))
						return true;		
						
					if (this->type_ == COMMENT) {
						if (!contain_intsubset_ && *it == '[')
							contain_intsubset_ = true;
						else if (contain_intsubset_ && (*it == ']'))
							contain_intsubset_ = false;
					}
					
					if (!contain_intsubset_ && this->is_end_symbol(it)) {
						IteratorT pre_char = it;
						--pre_char;		
								
						bool ret = false;		
						if( this->type_ == COMMENT) {
							if (*pre_char == '-' && *(--pre_char) == '-') 			
								ret = true;												 							
						} else if (this->type_ == TAG) {								
							if (*pre_char == ']' && *(--pre_char) == ']')
								ret = true;							
						} else if (this->type_ == COMMENT) {	
							this->skip_whitespace_backward(pre_char);
							//if (*pre_char == ']')
								ret = true;							
						} else if (this->type() == TEXT) {
							if (*pre_char == '/') {
								//this->body_.end(pre_char);
								if (this->is_end_wiki_keyword_) {
									//TODO print some errors here, because we cann't have two close wiki keyword
									this->is_end_wiki_keyword_ = false;
								}									
								this->is_ended_wiki_keyword_ = true;
							} 		
							ret = true;				
							//body_.end(it);		
						} else {
							ret = true;
						}
						
						//TODO error messages here, the node is not ended correctly if ret value is not true
						if (!ret)
							pre_char = it;
					
						if (ret) {
							this->body_.end(pre_char);
							++it;
						}
						return ret;
					}
					return false;
				}	
						
				virtual IteratorT skip_invalid_chars(IteratorT& it) {
					return this->skip_whitespace(it);
				}			
				
				virtual void add_start(StringT& text) {
					//TODO body_.set_begin()
					this->ref().push_back(WikiEntityConstants::WIKI_KEY_OPEN_TAG);
					if (this->is_end_wiki_keyword_)
						this->ref().push_back(WikiEntityConstants::WIKI_KEY_OPEN_TEMPLATE);					
				}
				
				virtual void add_content(StringT& text) {	
					this->ref().append(text);						
				}
				
				virtual void add_end(StringT& text) {
					//TODO body_.set_end()
					if (this->is_ended_wiki_keyword_ && !this->is_end_wiki_keyword_)
						this->ref().push_back(WikiEntityConstants::WIKI_KEY_OPEN_TEMPLATE);
					this->ref().push_back(WikiEntityConstants::WIKI_KEY_CLOSE_TAG);					
				}		

			private:
				void init() {
					is_ended_wiki_keyword_ = false;  // <../>
					is_end_wiki_keyword_ = false;	// </..>
					contain_intsubset_ = false;
				}				
		};		
	}
}
#endif /*STPL_WIKI_BASIC_H_*/
