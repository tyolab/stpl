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

#ifndef STPL_USCANNER_H_
#define STPL_USCANNER_H_

#include "stpl_unicode.h"

namespace stpl {
	template<	typename IteratorT  >
	class UnicodeScanner {
		
		public:
			UnicodeScanner() {}
			
			IteratorT next(IteratorT it) {
				return it;
			}
	};
}

#endif /*STPL_USCANNER_H_*/
