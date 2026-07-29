#include "controls.h"



// ---------------------------------------------------------

// create an Array of ui_Wrappers
#define NAME ui_WrapperArr
#define T ui_Wrapper*
#define ARRAY_IMPLEMENTATION
#define CHECK_EQUAL(a,b) a == b
#include "array.template.h"

#define NO_TEMPLATE_UI_WRAPPERS

#include "wrappers_array.h"