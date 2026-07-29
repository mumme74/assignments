#ifndef _WRAPPERS_ARR_H_
#define _WRAPPERS_ARR_H


#ifndef NO_TEMPLATE_UI_WRAPPERS

#include "controls.h"

// create an Array of
#define NAME ui_WrapperArr
#define T ui_Wrapper*
//#define ARRAY_IMPLEMENTATION
#define CHECK_EQUAL(a,b) a == b
#include "array.template.h"

#endif

#endif
