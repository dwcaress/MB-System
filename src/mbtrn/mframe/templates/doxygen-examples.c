///
/// @file doxygen-examples.c
/// note: name must match file name
/// @authors manny, moe and jack
/// @date 06 nov 2012
///
/// Long descriptive comments here...
/// You can actually compile this example.
/// 
/// Commenting Conventions:
/// - We use the 'C++' style ('///...') Doxygen tags b/c the C style ('/**...*/') interferes with C comment blocks
/// - Comment declaration (header file); if no header file, or declared in source file (e.g. static functions),
///   then comment definition (source file)
/// - All doxygen comments should include @brief (otherwise, warnings are generated)
/// - The @brief tag should always end in '.' (to make Detailed Comments start on new line)
///

/////////////////////////
// Terms of use 
/////////////////////////


/////////////////////////
// Headers 
/////////////////////////
/*
 This is a regular (non-doxygen) C comment.
 Use them anywhere.
 */
// Same applies to C++ style comments too
#include <code-template.h>

/////////////////////////
// Macros
/////////////////////////

// For macros, need @def, @brief

/// @def MACRO_KNEE
///	@brief Melting point of manchego (deg C)
#define MACRO_KNEE 0xC4335E

/// @def _DEBUG(fmt, args...)
/// @brief Prints formatted debug message.
/// Called as printf and friends
/// 
/// @param[in] fmt message format string
/// @param[in] ... variable argument list
/// @note it expects at least one argment
#define _DEBUG(fmt, ...) printf("%s:%s:%d: "fmt, __FILE__, __FUNCTION__, __LINE__, args)


/////////////////////////
// Exports
/////////////////////////

/////////////////////////
// Type Definitions
/////////////////////////

// Minimal struct definition includes
// struct, name, brief, and var definitions

/// @struct sampler_t
/// @brief Gateway configuration data 
/// 
/// Contains configuration options and state
/// information for gateway
typedef struct
{
	/// @var sampler_t::state
	/// @brief Gateway sampler state.
	/// Many states are possible.
	uint32_t state;	
}sampler_t;

/// @typedef struct element_s element_t
/// @brief xml tag element
typedef struct element_s
{
    /// @var element_s::tag_name
    /// @brief xml tag name string
    const char *tag_name;
    /// @var element_s::attributes
    /// @brief xml tag attributes list
    mlist_t *attributes;
}element_t;

/// @enum bool 
/// @brief Boolean type definition.
typedef enum {
	false=0,
	true
}bool;

// compare returns true if a,b are
// in the correct sorting order
/// @brief compare function pointer type
/// @param[in] a item
/// @param[in] b item
/// @return true if a and b are in correct sorting order
typedef bool (* mlist_cmp_fn)(void *a, void *b);

/////////////////////////
// Module Global Variables
/////////////////////////

/// @var int jen_number
/// @brief A legendary phone number.
/// Go on, dial it. You know you've always wanted to.
int jen_number=8675309;

/////////////////////////
// Function Definitions
/////////////////////////

/// @fn int sayHello(char *name)
/// @brief Print greeting. 
/// Note that the brief ends at the first '.'

/// Prints personalized greeting (verbose function description, spans line)
/// @param[in] name greeting addressee (input parameters; must match function parameter list) 
/// @param[out] name well, not really...name is a const (output parameter; input parameters that the function changes)
/// @return Always returns 0 (describe return value(s))
/// @sa Nothing to see here, move along (see also; references other functions, variables, etc.)
/// @note These are not the droids you're looking for (just notes)
/// @warning Will segfault if name is NULL. Don't take any wooden nickels. (warnings)
int sayHello(char *name)
{
	char *cp=(name==NULL?"World!":name);
	printf("Hello %s\n",cp);
	return 0;
}

/// @fn int main(int argc, char **argv)
/// @brief main function entry point.
 
/// Calls sayHello with optional user-specified argument.
/// Uses default value if none specified.

/// @param[in] argc number of arguments
/// @param[in] argv array of command line arguments
/// @return returns 0 if successful, -1 otherwise
/// @sa sayHello
int main(int argc, char **argv)
{
	if (argc>0) 
	{
		sayHello(argv[1]);
	}else 
	{
		sayHello(NULL);
	}
	return 0;
}

// Example: Anonymous struct/union

/// @struct mystruct
/// @brief tbd
struct mystruct {
	/// @var mystruct::baz
	/// @brief tbd
	/// @var mystruct::quux
	/// @brief tbd
	union{
		struct foo baz;
		struct bar quux;
	};
};

/// @typedef struct_type
/// @brief tbd
/// @struct struct_tag
/// @brief tbd
typedef struct struct_tag {
	/// @var struct_tag::foo
	/// @brief tbd
	uint16_t foo;
	/// @var struct_tag::bar
	/// @brief tbd
	uint32_t bar;
	/// @var struct_tag::baz
	/// @brief tbd
	int baz;
	/// @var struct_tag::quux
	/// @brief tbd
	void * quux;
}struct_type; 

//////////////////////
// List of commonly-
// used Doxygen tags
//////////////////////
/* 
 We use C++ style Doxygen comments
 so we can enclose Doxygen markup
 in C comment blocks like this.

/// @var ::
/// @brief tbd

/// @def 
/// @brief tbd

/// @struct name
/// @brief tbd

/// @typedef name
/// @brief tbd

/// @fn sig
/// @brief tbd
/// @param[in] name description
/// @return tbd

/// @var type name
/// @brief tbd

/// @enum type 
/// @brief tbd

*/
