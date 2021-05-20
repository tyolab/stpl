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

#ifndef STPL_WIKI_BASIC_H_
#define STPL_WIKI_BASIC_H_

#include "../stpl_entity.h"
#include "../stpl_property.h"

#include <map>

namespace stpl {
	namespace WIKI {
		
		// TEMPLATE node includes wiki declarations, text declarations,
		// If not any of others, then just TEXT
		enum WikiNodeType {
			LAYOUT,
			TAG,
			LINK,
			TEMPLATE,
			COMMENT,
			TEXT
		};

		// NONE for un-initialized node, or just TEXT
		// MISC node type includes COMMENT, PI or DOCTYPE (COMMENT)
		enum WikiNode {NONE, 
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

							// Sub node types of TEMPLATE
							T_PA, // pronuciation aids
							T_COLBEGIN, // column begin
							T_COLEND,   // column end

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
							T_COLOR,
							T_FONT_COLOR,
							T_FONT_STRIKE,

							TAG_NOWIKI,
							TAG_PRE
							}; 

		template <typename StringT = std::string, typename IteratorT = typename StringT::iterator>
		class BasicWikiEntity : public StringBound<StringT, IteratorT> 
		{
			public:
				static const char WIKI_KEY_OPEN_TAG = '<';
				static const char WIKI_KEY_CLOSE_TAG = '>';
				static const char WIKI_KEY_HEADING = '=';
				static const char WIKI_KEY_LIST = '*';
				static const char WIKI_KEY_LIST_ORDERED = '#';
				static const char WIKI_KEY_OPEN_TEMPLATE = '{';
				static const char WIKI_KEY_CLOSE_TEMPLATE = '}';
				static const char WIKI_KEY_OPEN_LINK = '[';
				static const char WIKI_KEY_CLOSE_LINK = ']';
				
			protected:
				WikiNode		 								node_;
				WikiNodeType 									type_;
				BasicWikiEntity*								parent_ptr_;
				StringBound<StringT, IteratorT> 				body_;
					
			private:
				void init() {
					node_ = NONE;
					type_ = TEXT;
					parent_ptr_ = NULL;
				}
				
			public:
				BasicWikiEntity() : StringBound<StringT, IteratorT>::StringBound() {
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
					if (*it == WIKI_KEY_OPEN_TAG
						|| *it == WIKI_KEY_HEADING
						|| *it == WIKI_KEY_LIST
						|| *it == WIKI_KEY_LIST_ORDERED
						|| *it == WIKI_KEY_OPEN_TEMPLATE
						|| *it == WIKI_KEY_OPEN_LINK
						)
						return true;
					return false;
				}
				
				static bool is_end_symbol(IteratorT it) {
					if (*it == WIKI_KEY_CLOSE_TAG
						|| *it == WIKI_KEY_HEADING
						|| *it == WIKI_KEY_CLOSE_TEMPLATE
						|| *it == WIKI_KEY_CLOSE_LINK
					)
						return true;
					return false;
				}
																		
				static bool is_key_symbol(IteratorT it) {
					if (is_start_symbol(it) || is_end_symbol(it))
						return true;
					return false;
				}
				
				static WikiNodeType element_type() { return TAG; }
				
				BasicWikiEntity* parent() { return parent_ptr_; }
				void set_parent(BasicWikiEntity* parent_ptr) {
					parent_ptr_ = parent_ptr;
				}		

				bool is_element() { return type() == TAG; }
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
				void init() { this->force_end_quote_ = true; }
				
			protected:
				virtual bool is_end_char(IteratorT& it) {
					if (Property<StringT, IteratorT>::is_end_char(it))
						return true;
					else {
						IteratorT next = it;						
						if (*next == '/') {							
							this->skip_whitespace(++next);
						}
						if (BasicWikiEntity<StringT, IteratorT>::is_end_symbol(next) ) {
							it = next;
							return true;
						}
					}
					return false;
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
								this->type_ = TEMPLATE;
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
				
				virtual bool is_end(IteratorT& it) {
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
						
				virtual IteratorT skip_not_valid_char(IteratorT& it) {
					return this->skip_whitespace(it);
				}			
				
				virtual void add_start(StringT& text) {
					//TODO body_.set_begin()
					this->ref().push_back(BasicWikiEntity<StringT, IteratorT>::WIKI_KEY_OPEN_TAG);
					if (this->is_end_wiki_keyword_)
						this->ref().push_back(BasicWikiEntity<StringT, IteratorT>::WIKI_KEY_OPEN_TEMPLATE);					
				}
				
				virtual void add_content(StringT& text) {	
					this->ref().append(text);						
				}
				
				virtual void add_end(StringT& text) {
					//TODO body_.set_end()
					if (this->is_ended_wiki_keyword_ && !this->is_end_wiki_keyword_)
						this->ref().push_back(BasicWikiEntity<StringT, IteratorT>::WIKI_KEY_OPEN_TEMPLATE);
					this->ref().push_back(BasicWikiEntity<StringT, IteratorT>::WIKI_KEY_CLOSE_TAG);					
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
