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

#ifndef STPL_ENTITY_H_
#define STPL_ENTITY_H_

#include <string>
#include <vector>
#include <iostream>
#include <ostream>
#include <algorithm>
#include <climits>

#include "lang/stpl_character.h"

using namespace std;

namespace stpl {

	const char SPACING[] = {"  "};

	/**
	 * The smallest object in the class chain
	 */
	class Atom {
		public:
			static int                                      counter;

		protected:
			int												id_;	

		public:
			int get_id() const {
				return id_;
			}

			void set_id(int id) {
				id_ = id;
			}
	};	

	/**
	 * There are two types of boundaries
	 * 1) boundary given by a beging and an end, obviously in this sitation, boundary is very clear
	 * 2) boundary by separators (delimiters)
	 */
	template <typename StringT = std::string, typename IteratorT = typename StringT::iterator>
	class StringBound : public Atom, public Character<StringT, IteratorT> {
		public:
			typedef	StringT		            string_type;
			typedef IteratorT	            iterator;

		private:
			StringT							content_;

			IteratorT 						begin_;
			IteratorT 						end_;
			unsigned long long int 			offset_;	 // used in creating the StringBound content

			int								line_id_;    // for used the relative line id, where the beginning of a new entity the line id will be 0
			                                             // for each line it comes across, ++line_id;
			bool							open_;

			std::vector<char>			    delimiters_; // the separators for the string bounds
														  // it can work as a string
														  // or it can work as different delimiters
			bool                            many_delimiters_;

		public:
			StringBound ()
				{ init(); }
			StringBound (IteratorT it) :
				 begin_(it) , end_(it) { init(); }
			StringBound (IteratorT begin, IteratorT end) :
				 begin_(begin), end_(end) { init(); }
			StringBound (StringT content) /*: content_ref_(content_) */ {
				content_ = content;
				init();
			}
			virtual ~StringBound() {}

			StringBound& operator= (StringBound& se) {
				if (this != &se) {
					begin_ = se.begin();
					end_ = se.end();
				}
				return *this;
			}

			void begin(IteratorT it) { begin_ = it; }
			const IteratorT begin() const { return begin_; }
			void end(IteratorT it) { end_ = it; }
			const IteratorT end() const { return end_; }

			virtual void bound(IteratorT begin, IteratorT end) {
				this->begin(begin);
				this->end(end);
			}

			size_t length() {
				 return end_ - begin_;
			}

			/**
			 * begin of an entity
			 */
			virtual bool bow(IteratorT it) {
				return it <= this->begin();
			}

			/**
			 * end of word or word-equivelant, or end of the world
			 * we reach the end of the input
			 */
			virtual bool eow(IteratorT it) {
				return it >= this->end();
			}

			IteratorT skip_n_chars(IteratorT& it, int n) {
				int count = 0;
				while (!this->eow(it) && count < n) {
					++it;
					++count;
				}
				return it;
			}

			IteratorT skip_whitespace(IteratorT& next) {
				while (!this->eow(next)) {
					if (*next == '\n') {
						++line_id_;
						++next;
					}
					else if (isspace(*next)) {
						++next;
					}
					else
						break;
				}
					
				return next;
			}

			IteratorT skip_whitespace_backward(IteratorT& pre) {
				while (!this->bow(pre)) {
					if (*pre == '\n') {
						--pre;
						--line_id_;
					}
					else if (isspace(*pre)) {
						--pre;
					}
					else
						break;
				}
				return pre;
			}

			IteratorT skip_non_alnum_char(IteratorT& next) {
				while (!this->eow(next) && !isalnum(*next))
					next++;
				return next;
			}

			virtual IteratorT skip_invalid_chars(IteratorT& next) {
				while (!this->eow(next) && !this->is_valid_char(next))
					next++;
				return next;
			}

			virtual StringT to_string() {
				if (begin_ == end_)
					return StringT("");
				return StringT(begin_, end_);
			}

			/**
			 * As if the StringT is char*, it won't work
			 * so we have to make it std::string
			 */
			virtual std::string to_std_string() {
				if (begin_ == end_)
					return std::string("");
				return std::string(begin_, end_);
			}

			void print(std::ostream &out = cout, int level = 0) {
				print_spacing(out, level);
				out << to_std_string() << endl;
			}

			virtual IteratorT match() {
				return this->match(this->begin());
			}

			virtual IteratorT match(IteratorT begin, IteratorT end) {
				if (begin == end)
					return end;

				this->begin(begin);
				this->end(end);

				return match(begin);
			}

			virtual IteratorT match(IteratorT begin) {
				IteratorT it = detect(begin);
				if (it >= end_) {
					begin_ = end_;
					set_open(false);
					return end_;
				}

				this->begin_notify(begin);
				return this->match_rest(begin);
			}

			IteratorT match_rest(IteratorT next_char) {
				//IteratorT next_char = begin;
				// in some cases,the last char cannot be set due to reach end of the string stream
				// so test end of stream has to be done inside of is_end function or right hand side of it.
				while (!this->eow(next_char)) {
					// we need to check if it is end first before checking it is pause
					if (this->is_end(next_char)) {
					   this->end(next_char);
				       this->end_notify(next_char);

				       this->set_open(false);

					   break;
				   }
 				   else if (this->is_pause(next_char)) {
					   break;
				   }				   

				   ++next_char;
				}

				return next_char;
			}

			virtual IteratorT resume_match(IteratorT begin, IteratorT end) {
				if (this->begin() != begin) {
					this->end(end);
					return this->match_rest(begin);
				}

				return match(begin, end);
			}

			virtual IteratorT detect(IteratorT& begin) {

				this->skip_invalid_chars(begin);

				/*
				 * the begining of the entity should be decided in the is_start function
				 */
				if (!this->is_start(begin)) {
					while (!this->eow(++begin)) {
						if (this->is_start(begin)) {
							break;
						}
					}
				}

				return begin;
			}

			virtual bool equal(StringT what) {
				return to_string() ==  what;
			}

			friend bool operator == (const StringBound& left, const StringBound& right) {
				return left.to_std_string() == right.to_std_string();
			}

			virtual void create(StringT text="") {
				add_start(text);
				add_content(text);
				add_end(text);
			}

			void ref_erase() {
				if (this->ref().length() > 0)
					ref().erase();
			}

			virtual void flush(int level=0) {
				StringT indent(level*2, ' ');

				this->ref().insert(0, indent);
				this->ref().append(to_string());
			}

			StringT& ref() { return content_; }

			virtual bool is_valid_char(IteratorT it) {
				return true;
			}

			bool isopen() const {
				return open_;
			}

			void set_open(bool open) {
				open_ = open;
			}


		protected:
			virtual void begin_notify(IteratorT& begin) {
			}

			virtual void end_notify(IteratorT& end) {
			}

			void print_spacing(std::ostream &out = cout, int level = 0) {
				for (int i=0; i<level; i++)
					out << SPACING;
			}

			virtual bool is_start(IteratorT& it) {
				bool ret = !eow(it);
				if (ret)
					this->begin(it);
				return ret;
			}

			virtual bool is_end(IteratorT& it) {
				return eow(it);
			}

			virtual bool is_pause(IteratorT& it) {
				return false;
			}

			virtual bool is_paused(IteratorT& it) {
				return it < end_;
			}

			virtual bool is_closed(IteratorT& it) {
				return it >= end_;
			}

			virtual void add_start(StringT& text) {
			}

			virtual void add_content(StringT& text) {
				ref().append(text);
			}

			virtual void add_end(StringT& text) {;
			}

			unsigned long long int offset() {
				return offset_;
			}

			virtual bool is_separated(IteratorT& it) {
				return false;
			}

			/**
			 * The delimiter can work as a single char or a string
			 */
			virtual bool is_delimiter(IteratorT& it) {
				return false;
			}

		private:
			void init() {
				//detected_ = false;
				//content_ref_ = content_;
				open_ = true;
			}
	};

	template <typename EntityT,  typename ContainerT = std::vector<EntityT *> >
	//template <typename EntityT, typename ContainerT>
	class Entity {
		private:
			typedef EntityT								*EntityPtr;

		public:
			typedef ContainerT							container_type;
			typedef	EntityT								entity_type;
			typedef EntityPtr							container_entity_type;
			typedef typename ContainerT::iterator 		iterator;
			typedef iterator 							entity_iterator;

		protected:
			ContainerT 									children_;
			iterator									current_pos_;

		public:
			Entity() { current_pos_ = children_.begin(); }
			~Entity() {
				clear();
			}

			//void add(EntityT entity) { children_.push_back(entity); }
			bool more() {
				if (children_.size() > 0) {
					if (current_pos_ != children_.end())
						return true;
				}
				return false;
			}

			iterator iter_begin() { return children_.begin(); }
			iterator iter_end()  { return children_.end(); }

			void reset() { current_pos_ = children_.begin(); }

			iterator next() { return current_pos_++; }

			iterator operator[] (int index) {
				if (index >= static_cast<int>(size() - 1))
					return children_[size() - 1];
				return children_[index];
			} //non-const

			const iterator operator[] (int index) const {
				if (index >= (size() - 1))
					return children_[size() - 1];
				return children_[index];
			} //const

			size_t size() { return children_.size(); }

			void print(std::ostream &out, int level = 0) {
				iterator it = children_.begin();
				for (;
					it != children_.end();
					++it)
				{
					(*it)->print(level + 1);
					out << std::endl;
				}
				//std::for_each(children_.begin(), children_.end(), mem_fun(&Entity::print));
			}

			ContainerT& children() { return children_; }

			void add(EntityT *entity_ptr) {
				children_.push_back(entity_ptr);
			}

			void clear() {
				iterator it;
				if (children_.size() > 0)
					for (it = children_.begin(); it != children_.end(); it++) {
						auto ptr = *it;
						if (ptr) {
							delete ptr;
							ptr = NULL;
						}
					}
				children_.clear();
			}

			friend bool operator == (const Entity& left, const Entity& right) {
				return false;
			}
	};

	template <
		typename EntityT = StringBound<>,  
		typename ContainerT = std::vector<EntityT *>  >
	class StringEntity : public StringBound<typename EntityT::string_type, typename EntityT::iterator>
							, public Entity<EntityT, ContainerT> {
		private:
			typedef typename EntityT::string_type 				StringT;
			typedef typename EntityT::iterator					IteratorT;

		public:
			typedef ContainerT									container_type;
			typedef	StringT										string_type;
			typedef IteratorT									iterator;
			typedef	EntityT										entity_type;
			typedef typename Entity<EntityT>::entity_iterator 	entity_iterator;

		public:
			StringEntity() : StringBound<StringT, IteratorT>::StringBound()
				, Entity<EntityT>::Entity() {}

			StringEntity(IteratorT it) :
				StringBound<StringT, IteratorT>::StringBound(it)
				, Entity<EntityT>::Entity()  {
			}

			StringEntity(IteratorT begin, IteratorT end) :
				StringBound<StringT, IteratorT>::StringBound(begin, end)
					, Entity<EntityT>::Entity() {}

			StringEntity(StringT content) :
				StringBound<StringT, IteratorT>::StringBound(content)
				, Entity<EntityT>::Entity()  {
			}

			virtual ~StringEntity() {}

			virtual entity_iterator find(StringT what) {
				entity_iterator begin = this->iter_begin();
				for (; begin != this->iter_end(); begin++)
					if ((*begin)->equal(what))
						break;
				return begin;
			}

	};
}

#include "stpl_entity.tcc"

#endif /*STPL_ENTITY_H_*/
