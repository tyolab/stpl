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

#ifndef STPL_STPL_STPL_ATOM_H_
#define STPL_STPL_STPL_ATOM_H_

namespace stpl {
    /**
	 * The smallest object in the class chain
	 */
	class Atom {
		public:
			static int                                      counter;
			static int                                      line_counter;

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

    int Atom::counter = 0;

}

#endif /* STPL_STPL_STPL_ATOM_H_ */
