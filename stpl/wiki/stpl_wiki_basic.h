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

#include "../../utils/xml.h"

#include "stpl_wiki_constants.h"

#include <map>
#include <iostream>

namespace stpl {
	namespace WIKI {

		template <typename StringT = std::string, typename IteratorT = typename StringT::iterator>
		class BasicWikiEntity : public StringBound<StringT, IteratorT> 
		{
			protected:
				WikiNodeGroup		 								group_;
				WikiNodeType 										type_;
				BasicWikiEntity*									parent_ptr_;
				StringBound<StringT, IteratorT> 					body_;

				int                                                 level_;
				int                                                 output_format_;
				
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

				virtual bool is_child_end(WikiNodeGroup group, WikiNodeType type, IteratorT& it) {
					return false;
				}

				virtual bool is_empty() {
					return false;
				}

				virtual const int children_count() {
					return 0;
				}

				virtual const bool should_have_children() {
					return false;
				}

				virtual BasicWikiEntity<StringT, IteratorT> *create_child(IteratorT& begin, IteratorT& end) {
					return NULL;
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

				/**
				 * @brief by default we don't output
				 * 
				 * @return std::string 
				 */
				virtual std::string to_text() {
					return "";
				}

				virtual void process_child(BasicWikiEntity<StringT, IteratorT>* child) {
					// if it doesn't get implemented, it has no children 
				}

				/**
				 * An entity is ended when 1) ended by the parent separator
				 *                         2) ended by parent end
				 *                         3) ended by end of input
				 * 
				 * Let parent close it up, so advance is false here
				 */
				virtual bool is_end(IteratorT& it, bool advance=true) {
					return /* this->is_separated(it) ||  */(this->parent_ptr_ && this->parent_ptr_->is_end(it, false))/*  || StringBound<StringT, IteratorT>::is_end(it) */;
				}			

				/**
				 * [ ] for links
				 * { } for templates
				 * -{ for language variants
				 * : for indention(s)
				 * @brief 
				 * 
				 * @param it 
				 * @return true 
				 * @return false 
				 */
				virtual bool is_pause(IteratorT& it) {
					// ok we are not gonna pause when the character with the following
					// that is not a good idea either that simply skip the first character simply because 
					if (*it == '[' || *it == '{'  || *it == '-' || *it == '\'')
						return true;
					else if (*it == '#' || *it == '*' || *it == ':') {
						IteratorT pre = it - 1;
						if (*pre == '\n')
							return true;
					}
					else if (*it == '$') {

					}
					return false;
				}			

				virtual bool is_separated(IteratorT& it) {
					return this->parent_ptr_ && this->parent_ptr_->is_delimiter(it);
				}

				int get_level() const {
					return level_;
				}

				void set_level(int level) {
					level_ = level;
				}

				int get_output_format() const {
					return output_format_;
				}

				void set_output_format(int outputFormat) {
					this->output_format_ = outputFormat;
				}

			private:
				void init() {
					level_ = 0;
					group_ = GROUP_NONE;
					type_ = NONE;
					parent_ptr_ = NULL;
					output_format_ = 0;

					Atom::set_id(Atom::counter++);
					if (Atom::max_id != -1 && Atom::counter > Atom::max_id) {
						// for example, even each char makes a node and create a sub-node about it, the max id
						// should'nt be twice of the total number of chars
						throw std::runtime_error("Check the code: maximum number of node shouldn't exceed the max id");
					}
				}
		};

		template <typename StringT = std::string, typename IteratorT = typename StringT::iterator>
		class Text: public BasicWikiEntity<StringT, IteratorT>
		{
			public:
				typedef	StringT	string_type;
				typedef IteratorT	iterator;

			private:
				int 															is_empty_;					

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

				virtual std::string to_text() {
					return this->to_std_string();
				}

				virtual bool is_empty() {
					if (is_empty_ == -1) {
						IteratorT pre = this->end() - 1;
						while (isspace(*pre)) {
							if (pre < this->begin())
								break;
							--pre;
						}

						if (pre < this->begin())
							is_empty_ = 1;
					}
					return is_empty_ == 1;
				}

				virtual std::string to_json() {
					return this->to_html();
				}	

				virtual std::string to_html() {
					std::string html = this->to_std_string();
#ifndef DEBUG
					// this a very time consuming part
					utils::unescape_xml(html);
#endif // !DEBUG					
					return html;
				}						

				virtual bool is_pause(IteratorT& it) {
					// we are not gonna pause it straitaway as this is a text node, because if it contains special character(s)
					// we should leave it to the next one
					if (it > this->begin() && BasicWikiEntity<StringT, IteratorT>::is_pause(it)) {
						return true;
					}
					else if (*it == '\n') {
						// do we need to advance here?
						// please check and find out, and put some reasons here
						// ++it;
						return true;
					}
					return false;
				}

				/**
				 * For the text node, most likely it will encounter a link or template which will mark
				 * the end of it
				 * 
				 * But generally we won't end until a new enity is found which can't be just link and template
				 */
				 virtual bool is_end(IteratorT& it, bool advance=true) {
					 IteratorT from = it;
					 /**
					  * @brief there two special characters: - and :
					  * need to specially dealt with
					  */
					 bool ret = false;
					 switch (*it) {
						 case '-':
						 	{
								IteratorT next = it + 1;
								if (*next == '{') 
							 		ret = true;
							}
							break;
						 case ':':
						 	{
								IteratorT pre = it - 1;
								while (*pre == ' ')
									--pre;
								if (*pre == '\n') 
							 		ret = true;
							}						 
							break;
						default:
							break;
					 }
					// special treatment for text node inside a compund entity
					//  e.g. WikiProperty node when it sees a '=' character
					if (!ret && this->parent_ptr_) {
						if (this->parent_ptr_->is_end(it, false))
							ret = true;
						else if (this->parent_ptr_->is_child_end(TEXT, this->get_type(), it))
							ret = true;
						// else if (this->parent_ptr_->get_type() == P_PROPERTY && *it == '=')
						// 	return true;
						// else if (this->parent_ptr_->get_type() == P_CELL && *it == '\n')
						// 	return true;							
						// else if (this->parent_ptr_->get_type() == P_LINK && *it == '|')
						// 	return true;
						// else if (this->parent_ptr_->get_type() == LINK_EXTERNAL && *it == ' ')
						// 	return true;
						// else if (this->parent_ptr_->get_type() == LINK_P) {

						// }					
					}					 
					// for a text node, it finishes when it sees a special character
					// because it is a text node it have to move forward a char first
					if (!ret && it > this->begin() && BasicWikiEntity<StringT, IteratorT>::is_pause(it))
				 		ret = true;

					if (ret) {
						if (it == this->begin())
							ret = false;
				 	}
					return ret;
				}

			private:
				void init() { 
					this->group_ = TEXT; 
					this->is_empty_ = -1;
				}				 
		};

		template <typename StringT = std::string, typename IteratorT = typename StringT::iterator>
		class Redirect: public BasicWikiEntity<StringT, IteratorT>
		{
			public:
				typedef	StringT	string_type;
				typedef IteratorT	iterator;

			private:
				void init() {
					 this->group_ = REDIRECT;
				}

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

				virtual std::string to_json() {
					return this->to_std_string();
				}						

				virtual bool is_start(IteratorT& it) {
					return true;
				}

				virtual bool is_pause(IteratorT& it) {
					return false;
				}

				virtual bool is_end(IteratorT& it, bool advance=true) {
					if ((it - this->begin()) > 9) {
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
		class DebugNode: public BasicWikiEntity<StringT, IteratorT>
		{
			public:
				typedef	StringT	string_type;
				typedef IteratorT	iterator;

			private:
				void init() { this->group_ = DEBUG_NODE; }

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
				// std::map<StringT, WikiEntity>	         			properties_;
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

				virtual const int children_count() {
					return Entity<BasicWikiEntity<StringT, IteratorT> >::size();
				}

				virtual const bool should_have_children() {
					return true;
				}

				virtual BasicWikiEntity<StringT, IteratorT> *create_child(IteratorT& begin, IteratorT& end) {
					return new Text<StringT, IteratorT>(begin, end);
				}

				/**
				 * Not many kinds of entities will have direct text node as child(ren)
				 * WikiProperty
				 * TableCell
				 * ListItem
				 */
				// virtual void create_text_child_pre(IteratorT it) {
				// 	// so we do nothing here
				// }

				// virtual void create_text_child_after(IteratorT it) {
				// 	// so we do nothing here
				// }

				virtual void create_text_child_pre(IteratorT it) {
					this->last_child_ = new Text<StringT, IteratorT>(this->begin(), it);
					this->add(this->last_child_);
				}

				virtual void create_text_child_after(IteratorT it) {
					// so we do nothing here
					IteratorT begin = this->last_child_->end();
					this->process_child(new Text<StringT, IteratorT>(begin, it));
				}					

				const BasicWikiEntity<StringT, IteratorT> *get_last_child() const {
					return last_child_;
				}

				void set_last_child(const BasicWikiEntity<StringT, IteratorT> *lastChild) {
					last_child_ = lastChild;
				}

				virtual void process_child(BasicWikiEntity<StringT, IteratorT>* child) {
					// if (this->children_.size() == 0 && child->begin() > this->begin()) {
					// 	// IteratorT begin = this->begin();
					// 	// IteratorT end = child->begin();
					// 	// this->add(new Text<StringT, IteratorT>(begin, end));
					// 	create_text_child_pre(child->begin());
					// }

					this->last_child_ = child;
					this->add(child);
				}

				virtual std::string to_html() {
					return children_to_html(); 
				}			

				virtual std::string to_text() {
					std::stringstream ss;
					auto it = this->children_.begin();
					while (it != this->children_.end()) {
						ss << (*it++)->to_text();
					}
					return ss.str();
				}

				std::string children_to_html() {
					this->assign_output_format();
					std::stringstream ss;
					auto it = this->children_.begin();
					while (it != this->children_.end()) {
						ss << (*it++)->to_html();
					}
					return ss.str();
				}

				virtual bool is_pause(IteratorT& it) {
					/**
					 * For any entity when it comes to a pause
					 * we move the end char to it
					 * ===
					 * Why the above needs to do that
					 * Don't do it
					 */
					// if (BasicWikiEntity<StringT, IteratorT>::is_pause(it)) {
					// 	// no we can't end it here;
					// 	// this->end(it);
					// 	return true;
					// }
					// As it is supposed to contain children
					this->skip_whitespace(it);
					return true;
				}

			protected:
				void assign_output_format() {
					auto it = this->children_.begin();
					while (it != this->children_.end()) {
						(*it)->set_output_format(this->get_output_format());
						++it;						
					}
				}

			private:
				void init() {
					last_child_ = NULL;
				}
		};
	}

}
#endif /*STPL_WIKI_BASIC_H_*/
