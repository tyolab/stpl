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

#ifndef STPL_SCANNER_H
#define STPL_SCANNER_H

#include "stpl_entity.h"
#include "stpl_doc.h"
#include "stpl_typetraits.h"

#include <list>
#include <stdexcept>	

namespace stpl {
	
	template< typename EntityT >
	class Scanner{
		public:
			typedef std::list<EntityT *>									   stack_type;

		private:										
			typedef	typename EntityT::string_type                              StringT;
			typedef typename EntityT::iterator	                               IteratorT;
			
		public:			
			typedef	StringT                                                    string_type;
			typedef IteratorT                                                  iterator;

		protected:

			int 		                                                       state_;        // maintain the current state of the state machine,

		    stack_type                                                         stack_;

	 		IteratorT 	                                                       current_pos_;
	 		IteratorT 	                                                       end_;
	 		IteratorT 	                                                       begin_;

	 		EntityT*	                                                       last_e_;
					
	 	public:
	 		Scanner() : last_e_(NULL), state_(-1) {}
	 		Scanner(IteratorT begin, IteratorT end) : last_e_(NULL), state_(-1)
	 		/*: current_pos_(begin),  end_(end), begin_(begin)*/ {
	 			set(begin, end);
	 		}
	 		virtual ~Scanner() {
	 			clear_last();
	 		}
	 		
	 		bool is_end() {
				return (current_pos_ >= end_);	 			
	 		}
	 		
	 		EntityT* next() {
	 			clear_last();
	 			last_e_ = new EntityT();
	 			if (scan(last_e_))
	 				return last_e_;
	 			return NULL;
	 		}
	 		
	 		EntityT* next(EntityT* entity) {
	 			return scan(entity);
	 		}
			
			virtual void set_state(int state) {
				state_ = state;

				// may do other initializations with method override
			}

			virtual void reset_state(EntityT* entity_ptr) {
				// set the new state
				state_ = -1;
			}
			 
	 		EntityT* scan() {
	 			EntityT* tmp_entity = NULL;
				// EntityT* parent_entity = NULL;
				int previous_state = state_;
				IteratorT previous_pos = this->current();
				IteratorT it;

				if (!this->is_end()) {
					/*
					 * TODO make the skipping of the non-valid char happen here 
					 */

					// Everything starts clean with no parent node
					tmp_entity = this->state_check(this->current(), NULL);

					// entity could have a pause here, and if it is a text node it would bec losed already
					if (tmp_entity) {
						if (tmp_entity->isopen()) {

							it = tmp_entity->match();
							previous_pos = tmp_entity->begin();
							// if (!tmp_entity->isopen() && it == current_pos_) {
							// 	// it is not moving forward, which is bad
							// 	// creating a dead loop
							// 	tmp_entity->end(++it);
							// }
						}
						else
							it = tmp_entity->end();

						if(tmp_entity->end() > tmp_entity->begin())
							last_e_ = tmp_entity;							
					}
					else {
						// no more entity found, so we are done
						last_e_ = NULL;
					}
					// as if the entity does't close on the first match it means it has children
					// anything happens inside here, nothing to do with last_e_
					while (tmp_entity && tmp_entity->isopen()) {
						// after the state check, the previous entity may not be open
						EntityT* child_entity = state_check(it, tmp_entity);

						if (child_entity && child_entity != tmp_entity) {
							EntityT* parent_entity = child_entity->get_parent();
							if (!parent_entity)
								parent_entity = tmp_entity;
							on_new_child_entity(parent_entity, child_entity);

							if (child_entity->isopen()) {
								// now push the parent to the stack
								stack_.push_front(parent_entity);
								tmp_entity = child_entity;
								it = tmp_entity->match();

								// if (!tmp_entity->isopen() && tmp_entity->get_group()&& it == current_pos_) {
								// 	// it is not moving forward, which is bad
								// 	// creating a dead loop
								// 	tmp_entity->end(++it);
								// }
							}
							else
								// as the child entity is closed, there is no point to matching it again, 
								// we will simply continue parent entity
								continue;

						}
						else {
							// OK, there is no child entity found, we will continue
							// it may be still open and may be not, so we come in here, so we don't need to check if it is open again
							// nothing new, we better move to next char in case a dead loop
							if (it < this->end()) {
								IteratorT pre = it;
								if (tmp_entity->isopen())
									it = tmp_entity->match_rest(it);
								// It is not a good idea to do it here
								if (tmp_entity->isopen() && pre == it) {
									// there is no more rest
									current_pos_ =  ++it;
								}			
							}
							else
								tmp_entity = NULL;
						}

						// now it we need to find out what to do
						// entity now is closed, it may come to a point all sub entities close at the same time

						while (tmp_entity && !tmp_entity->isopen()) {
							// we are done with this one
							if (stack_.size() > 0) {
								child_entity = tmp_entity;
								tmp_entity = stack_.front(); // tmp_entity->get_parent();
								stack_.pop_front();
								
								on_child_entity_done(tmp_entity, child_entity);

								// initially it was set that  now continue previous adventure
								// this will be a bit different with the previous code
								// no we could not match rest here, as we may missout the text node
								// it = tmp_entity->match_rest(it);
							}
							else {
								tmp_entity = NULL;
								break;
							}
						}
											
					}
					// }

					if (last_e_) {
						current_pos_ = last_e_->end();
						reset_state(last_e_);
					}
					else {
						// not quite sure why this is necessry
						// need to restore to the previous state
						// now I think I know a bit more about this
						// when we come the end of the file, between last entity and the end of the file
						// there could text in between
						if (current_pos_ > previous_pos) {
							last_e_ = handle_char(previous_pos, current_pos_);
	;
							state_ = previous_state;
						}
						if (tmp_entity)
							delete tmp_entity;
					}
				}
				else
					last_e_ = NULL;

				return last_e_;
	 		}	 		

			/**
			 * There is a char not recognised in the parser
			 */
			virtual EntityT* handle_char(IteratorT begin, IteratorT end) {
				throw std::logic_error("The current parser doesn't recongize this char: " + std::string(begin, end));
			}
	 		
	 		IteratorT& current() { return current_pos_; }
	 		IteratorT& end() { return end_; }
	 		
			void set(IteratorT& begin, IteratorT& end) {
				begin_ = current_pos_ = begin;					
				end_ = end;	
				
				last_e_ = NULL;
			}
			
			void skip() {
				if (!is_end())
					++current_pos_;
			}

	 	protected:
			virtual EntityT* state_check(IteratorT& begin, EntityT* last_entity_ptr) {
				// a state machine is maintained with different grammar
				// may be overridden to fit the state machine
				IteratorT end = this->end();
				return new EntityT(begin, end);
			}

			virtual void on_new_child_entity(EntityT* entity_ptr, EntityT* child_entity) {
				// nothing yet, you may build up the relationship here
			}

			virtual void on_child_entity_done(EntityT* entity_ptr, EntityT* child_entity) {
				delete child_entity;
			}
				 		
	 	private:
	 		void clear_last() {
	 			if (last_e_) {
	 				delete last_e_;
	 				last_e_ = NULL;
	 			}	 			
	 		}
	};
}

//#include "stpl_scanner.tcc"

#endif // STPL_SCANNER_H
