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

#ifndef STPL_ATTRIBUTE_H_
#define STPL_ATTRIBUTE_H_

#include <string>
#include <algorithm>
#include <functional>
#include <iterator>

#include "stpl_entity.h"

namespace stpl {

	template <typename StringT = std::string, typename IteratorT = typename StringT::iterator>
	class Property : public StringBound<StringT, IteratorT> {
		public:
			typedef StringBound<StringT, IteratorT> StringB;

		public:
		    char delimiter_ = '=';

		protected:

			StringB name_;
			StringB	value_;
			bool has_delimiter_;
			bool has_quote_;
			bool force_end_quote_;  // if has quote, then it must end with quote
			bool is_single_quote_;  // false is double quote, true for single quote
			std::string end_chars_;

		private:
			void init() {
				has_delimiter_ = true;
				is_single_quote_ = false;
				has_quote_ = false;
				force_end_quote_ = false;
			}

		protected:
			virtual bool is_end_char(IteratorT& it) {
				if (end_chars_.length() > 0) {
					std::string::size_type pos = end_chars_.find(*it);
					if (pos != std::string::npos)
						return true;
				}

				if (this->has_delimiter_) {
					if (!has_quote_)
						return isspace(*it);
					else {
						if (*it == '\'' || *it == '\"') {
							// it has quote
							IteratorT pre = it - 1;
							if (*pre != '\\') {
								if (is_single_quote_) {
									return *it == '\'';
								}
								return *it == '\"';
							}
						}
					}
				}

				// if it doesn't have a delimiter
				// by default we won't where to end until we see a new line
				return (*it == '\n');
			}

			virtual bool is_delimiter(IteratorT& it) {
				return *it == delimiter_;
			}

			virtual bool is_start(IteratorT& it) {
				this->skip_whitespace(it);
				name_.begin(it);
				name_.end(it);

				// start only be resposible for start
				// don't do it like that
				// while (!this->is_pause(it) && !this->is_delimiter(it))
				// 	++it;

				return true; 
			}

			virtual bool is_end(IteratorT& it, bool advance=true) {
				bool ret = false;

					if (this->is_delimiter(it)) {
						has_delimiter_ = true;
						// backward for removing space
						// deside where is the end of name
						IteratorT pre = it;
						this->skip_whitespace_backward(--pre);
						name_.end(++pre);

						++it;

						this->skip_whitespace(it);
						if (*it == '\"' || *it == '\'') {
							is_single_quote_ = (*it == '\'');
							has_quote_ = true;
							++it;
						}
						value_.begin(it);
						value_.end(it);
					}

					if (ret = is_end_char(it))
						value_.end(it);
				return ret;
			}

			virtual void add_name(StringT& name) {
				this->ref().append(name);
			}

			void add_equal_symbol() {
				this->ref().append("=");
			}

			virtual void add_value(StringT& value) {
				this->ref().append("\"");
				this->ref().append(value);
				this->ref().append("\"");
			}

			void end_notify(IteratorT& end) {
				if (has_delimiter_) {
					if (has_quote_) {
						IteratorT pre = end;
						pre--;
						this->skip_whitespace_backward(pre);
						if ( (*pre == '\"')	|| (*pre == '\'')) {
							  value_.end(pre);
						}
						else
							value_.end(pre++);
					}
					else
						value_.end(end);
				}
			}

		public:
			Property() :
				StringBound<StringT, IteratorT>::StringBound() { init(); }
			Property(IteratorT begin) :
				StringBound<StringT, IteratorT>::StringBound(begin) { init(); }
			Property(IteratorT begin, IteratorT end) :
				StringBound<StringT, IteratorT>::StringBound(begin, end) { init(); }
			Property (StringT content) :
				StringBound<StringT, IteratorT>::StringBound(content) {
				init();
			}
			virtual ~Property() {}

			StringT name() {
				return StringT(name_.begin(), name_.end());
			}

			StringT value() {
				StringT value(value_.begin(), value_.end());
				if (has_quote_) {
					//StringT::size_type pos;
					int pos = 0;
					while ((pos = value.find("\\\"")) != StringT::npos)
						value.replace(pos, 2, "\"");
				}
				return value;
			}

			void set_end_chars(std::string end_chars) {
				end_chars_ = end_chars;
			}

			void force_end_quote(bool b) { force_end_quote_ = b; }

			bool has_delimiter() {
				return this->has_delimiter_;
			}



			virtual void create() {
				create(name(), value());
			}

			virtual void create(StringT name, StringT value) {
				//add_name(name);
				//add_equal_symbol(text);
				//add_value(value);
				// TODO add the postion for name_, value_
				StringT  temp(name);

				temp.append("=");

				temp.append("\"");
				temp.append(value);
				temp.append("\"");

				this->ref().append(temp);
			}

			const StringB& get_value() const {
				return value_;
			}

			bool has_quote() const {
				return has_quote_;
			}
};
}
#endif /*STPL_ATTRIBUTE_H_*/
