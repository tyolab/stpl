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
			typedef	StringT string_type;
			typedef IteratorT	iterator;

		protected:

			int 		                                                      state_;        // maintain the current state of the state machine,

		private:
		    stack_type                                                         stack_;

	 	private:
	 		IteratorT 	current_pos_;
	 		IteratorT 	end_;
	 		IteratorT 	begin_;

	 		EntityT*	last_e_;
					
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
				return (current_pos_ == end_);	 			
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

				if (!this->is_end()) {
					// Everything starts clean with no parent node
					tmp_entity = this->state_check(this->current(), NULL);
					/*
					 * TODO make the skipping of the non-valid char happen here 
					 */
					int previous_state = state_;
					IteratorT previous_pos = tmp_entity->begin();

					IteratorT it = tmp_entity->match();

					// if (tmp_entity->isopen()) {
					// 	parent_entity = tmp_entity;
						while (tmp_entity && tmp_entity->isopen()) {
							EntityT* new_entity = state_check(it, tmp_entity);
							if (new_entity != tmp_entity) {
								on_new_child_entity(tmp_entity, new_entity);
								// now push the parent to the stack
								stack_.push_front(tmp_entity);
								tmp_entity = new_entity;
								it = tmp_entity->match();

								// now it we need to find out what to do
								// entity now is closed, it may come to a point all sub entities close at the same time
								while (!tmp_entity->isopen()) {
									// we are done with this one
									if (stack_.size() > 0) {
										tmp_entity = stack_.front(); // tmp_entity->get_parent();
										stack_.pop_front();
										// we will now continue previous adventure
										it = tmp_entity->match_rest(it);
									}
									else
										break;
									// 	tmp_entity = NULL;
								}
							}
							// the entity is closed, now back to parent
							else {
								// OK, that is the end of a child entity
								if (stack_.size() > 0) {
									tmp_entity = stack_.front();
									on_child_entity_done(tmp_entity, new_entity);
									stack_.pop_front();
									it = tmp_entity->match_rest(it);
								}
								// else
								// 	tmp_entity = NULL;								
							}
						}
					// }

					if (tmp_entity && tmp_entity->end() > tmp_entity->begin()) {
						last_e_ = tmp_entity;
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
				
				return last_e_;
	 		}	 		
	 		
	 		IteratorT current() { return current_pos_; }
	 		IteratorT end() { return end_; }
	 		
			void set(IteratorT begin, IteratorT end) {
				begin_ = current_pos_ = begin;					
				end_ = end;	
				
				last_e_ = NULL;
			}
			
			void skip() {
				if (!is_end())
					++current_pos_;
			}

	 	protected:
			virtual EntityT* state_check(IteratorT begin, EntityT* last_entity_ptr) {
				// a state machine is maintained with different grammar
				// may be overridden to fit the state machine
				IteratorT	end = this->end();
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
