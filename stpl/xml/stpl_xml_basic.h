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

#ifndef STPL_XML_BASIC_H_
#define STPL_XML_BASIC_H_

#include "../stpl_entity.h"
#include "../stpl_property.h"
#include <map>

namespace stpl {
	namespace XML {
		
		// NONE for un-initialized node
		// MISC node type includes COMMENT, PI or DOCTYPE (COMMENT)
		// TEMPLATE node includes XML declarations, text declarations,
		enum XmlNodeType {NONE, NONE, COMMENT, TEXT, TEXT, TAG, COMMENT, TAG, TEMPLATE}; 

		template <typename StringT = std::string, typename IteratorT = typename StringT::iterator>
		class BasicXmlEntity : public StringBound<StringT, IteratorT> 
		{
			public:
				static const char XML_KEY_OPEN =            '<';
				static const char XML_KEY_CLOSE =           '>';
				static const char XML_KEY_SLASH =           '/';
				
			protected:
				XmlNodeType 								type_;
				BasicXmlEntity*								parent_ptr_;
				StringBound<StringT, IteratorT> 	        body_;
					
			private:
				void init() {
					type_ = NONE;
					parent_ptr_ = NULL;
				}
				
			public:
				BasicXmlEntity() : StringBound<StringT, IteratorT>::StringBound() {
				}
				BasicXmlEntity(IteratorT it) : StringBound<StringT, IteratorT>::StringBound(it), body_(it, it) {
					init();
				}
				BasicXmlEntity(IteratorT begin, IteratorT end) : StringBound<StringT, IteratorT>::StringBound(begin, end), body_(begin, begin) {
					init();
				}
				BasicXmlEntity(StringT content) : 
					StringBound<StringT, IteratorT>::StringBound(content) {
					init();
				}				
				virtual ~BasicXmlEntity() {}		
				
				XmlNodeType type() { return type_; }		
				void type(XmlNodeType type) { type_ = type; }
				
				virtual StringBound<StringT, IteratorT>& content() {
					return body_;
				}

				static bool is_start_symbol(IteratorT it) {
					if (*it == XML_KEY_OPEN)
						return true;
					return false;
				}
				
				static bool is_end_symbol(IteratorT it) {
					if (*it == XML_KEY_CLOSE)
						return true;
					return false;
				}
																		
				static bool is_key_symbol(IteratorT it) {
					if (is_start_symbol(it) || is_end_symbol(it))
						return true;
					return false;
				}
				
				static XmlNodeType element_type() { return TAG; }
				
				BasicXmlEntity* parent() { return parent_ptr_; }
				void set_parent(BasicXmlEntity* parent_ptr) {
					parent_ptr_ = parent_ptr;
				}		

				bool is_element() { return type() == TAG; }
		};		
		
		template <typename StringT = std::string, typename IteratorT = typename StringT::iterator>
		class XmlAttribute : public Property<StringT, IteratorT> {
			public:
				typedef std::map<StringT, XmlAttribute*>	attributes_type;
				
			public:
				XmlAttribute() : 
					Property<StringT, IteratorT>::Property() { init(); }
				XmlAttribute(IteratorT begin) :
				    Property<StringT, IteratorT>::Property(begin) { init(); }				
				XmlAttribute(IteratorT begin, IteratorT end) :
				    Property<StringT, IteratorT>::Property(begin, end) { init(); }
				XmlAttribute(StringT content) : 
					Property<StringT, IteratorT>::Property(content) {
				}
				virtual ~XmlAttribute() {}		
				
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
						if (BasicXmlEntity<StringT, IteratorT>::is_end_symbol(next) ) {
							it = next;
							return true;
						}
					}
					return false;
				}			
		};
		
		template <typename StringT = std::string, typename IteratorT = typename StringT::iterator>
		class XmlKeyword: public BasicXmlEntity<StringT, IteratorT> 
		{
			public:
				typedef	StringT	string_type;
				typedef IteratorT	iterator;			
				
			protected:
				bool					is_end_xml_keyword_;      // </tag>
				bool					is_ended_xml_keyword_;  // <tag/>
				bool					contain_intsubset_;
				
			public:
				XmlKeyword() : BasicXmlEntity<StringT, IteratorT>::BasicXmlEntity() {
					init();
				}
				XmlKeyword(IteratorT it) : BasicXmlEntity<StringT, IteratorT>::BasicXmlEntity(it) {
					init();
				}
				XmlKeyword(IteratorT begin, IteratorT end) : BasicXmlEntity<StringT, IteratorT>::BasicXmlEntity(begin, end) {
					init();
				}
				XmlKeyword(StringT content) :
					BasicXmlEntity<StringT, IteratorT>::BasicXmlEntity(content) {
					init();
				}
				virtual ~XmlKeyword() {}

				void set_end_xml_keyword(bool b) {
					is_end_xml_keyword_ = b;
				}

				void set_ended_xml_keyword(bool b) {
					is_ended_xml_keyword_ = b;
				}

				bool is_end_xml_keyword() {
					return is_end_xml_keyword_;
				}

				bool is_ended_xml_keyword() {
					return is_ended_xml_keyword_;
				}

				virtual void print(std::ostream &out = cout, int level = 0) {
					this->body_.print(out, level);
				}

				void clone(XmlKeyword* elem_k_ptr) {
					if (this != elem_k_ptr) {
						this->is_end_xml_keyword_ = elem_k_ptr->is_end_xml_keyword();
						this->is_ended_xml_keyword_ = elem_k_ptr->is_ended_xml_keyword();
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
						this->type_ = NONE;
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
								this->is_end_xml_keyword_ = true;
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
								if (this->is_end_xml_keyword_) {
									//TODO print some errors here, because we cann't have two close xml keyword
									this->is_end_xml_keyword_ = false;
								}									
								this->is_ended_xml_keyword_ = true;
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
					this->ref().push_back(BasicXmlEntity<StringT, IteratorT>::XML_KEY_OPEN);
					if (this->is_end_xml_keyword_)
						this->ref().push_back(BasicXmlEntity<StringT, IteratorT>::XML_KEY_SLASH);					
				}
				
				virtual void add_content(StringT& text) {	
					this->ref().append(text);						
				}
				
				virtual void add_end(StringT& text) {
					//TODO body_.set_end()
					if (this->is_ended_xml_keyword_ && !this->is_end_xml_keyword_)
						this->ref().push_back(BasicXmlEntity<StringT, IteratorT>::XML_KEY_SLASH);
					this->ref().push_back(BasicXmlEntity<StringT, IteratorT>::XML_KEY_CLOSE);					
				}		

			private:
				void init() {
					this->type_ = NONE;
					is_ended_xml_keyword_ = false;  // <../>
					is_end_xml_keyword_ = false;	// </..>
					contain_intsubset_ = false;
				}				
		};		
	}
}
#endif /*STPL_XML_BASIC_H_*/
