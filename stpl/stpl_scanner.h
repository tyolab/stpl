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
	
namespace stpl {
	
	template<	typename EntityT  >
	class Scanner{
		private:										
			typedef	typename EntityT::string_type StringT;
			typedef typename EntityT::iterator	IteratorT;
			
		public:			
			typedef	StringT string_type;
			typedef IteratorT	iterator;			
					
	 	public:
	 		Scanner() {}
	 		Scanner(IteratorT begin, IteratorT end)
	 		/*: current_pos_(begin),  end_(end), begin_(begin)*/ {
	 			set(begin, end);
	 		}
	 		~Scanner() {
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

			virtual EntityT* state_check() {
				// a state machine is maintained with different grammar
				// may be overridden to fit the state machine
				IteratorT	begin = this->current();
				IteratorT	end = this->end();
				return new EntityT(begin, end);
			}
			 
	 		EntityT* scan() {
				EntityT* entity_ptr = NULL; 
				if (!this->is_end()) {

					EntityT* tmp_entity = state_check();
					/*
					 * TODO make the skipping of the non-valid char happen here 
					 */
					int previous_state = state_;
					IteratorT previous_pos = tmp_entity->begin();
					bool ret = tmp_entity->match();
					if (ret) {
						entity_ptr = tmp_entity;
						current_pos_ = entity_ptr->end();
						reset_state(entity_ptr);
					}
					else {
						// need to restore to the previous state
						current_pos_ = ++previous_pos;
						state_ = previous_state;
						delete tmp_entity;
					}
				}
				
				return entity_ptr;
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
				 		
	 	private:
	 		void clear_last() {
	 			if (last_e_) {
	 				delete last_e_;
	 				last_e_ = NULL;
	 			}	 			
	 		}

		protected:
		    
			int 		state_;        // maintain the current state of the state machine, 
			
	 	private:
	 		IteratorT 	current_pos_;
	 		IteratorT 	end_;
	 		IteratorT 	begin_; 			
	 		
	 		EntityT*	last_e_;
	};
}

//#include "stpl_scanner.tcc"

#endif // STPL_SCANNER_H
