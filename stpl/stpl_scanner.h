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

		private:
		    stack_type                                                         stack_;

	 	private:
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

					// entity could have a pause here
					if (tmp_entity) {

						it = tmp_entity->match();
						previous_pos = tmp_entity->begin();

						if(tmp_entity->end() > tmp_entity->begin())
							last_e_ = tmp_entity;

					}

					while (tmp_entity && tmp_entity->isopen()) {
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
							}
							else
								// as the child entity is closed, there is no point to matching it again, 
								// we will simply continue parent entity
								continue;

						}
						else {
							// OK, there is no child entity found, we will continue
							// of course it is open, so we come in here, so we don't need to check if it is open again
							// nothing new, we better move to next char in case a dead loop
							if (it < this->end()) {
								// IteratorT pre = it;
								it = tmp_entity->match_rest(it);
								// if (pre == it) {
								// 	// there is no more rest
								// 	++it;
								// }			
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
						// need to restore to the previous state
						current_pos_ = ++previous_pos;
						state_ = previous_state;
						delete tmp_entity;
					}
				}
				else
					last_e_ = NULL;

				return last_e_;
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
