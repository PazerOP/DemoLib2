#pragma once

// Dangerous warnings that should be errors IMO
#pragma warning(error : 4013)	// 'x' undefined; assuming extern returning int
#pragma warning(error : 4020)	// 'x': too many actual parameters
#pragma warning(error : 4028)	// formal parameter 'n' different from declaration
#pragma warning(error : 4029)	// declared formal parameter list different from definition
#pragma warning(error : 4047)	// 'x' differs in levels of indirection from 'x*'
#pragma warning(error : 4090)	// 'function': different 'const' qualifiers
#pragma warning(error : 4091)	// 'keyword' : ignored on left of 'type' when no variable is declared
#pragma warning(error : 4113)	// 'identifier1' differs in parameter lists from 'identifier2'
#pragma warning(error : 4129)	// 'character' : unrecognized character escape sequence
#pragma warning(error : 4133)	// 'x': incompatible types - from 'y' to 'z'
#pragma warning(error : 4456)	// declaration of 'x' hides previous local declaration
#pragma warning(error : 4553)	// 'operator' : operator has no effect; did you intend 'operator'?
#pragma warning(error : 4715)	// 'x': not all control paths return a value
#pragma warning(error : 4716)	// 'x': must return a value

// Nuisance warnings

// Only because there's no nice way to fix this in plain C
#pragma warning(disable : 4100)	// 'identifier' : unreferenced formal parameter

#pragma warning(disable : 4201)	// nonstandard extension used : nameless struct/union
#pragma warning(disable : 4115)	// 'type' : named type definition in parentheses
#pragma warning(disable : 4204)	// nonstandard extension used : non-constant aggregate initializer
#pragma warning(disable : 4206)	// nonstandard extension used : translation unit is empty
#pragma warning(disable : 4214)	// nonstandard extension used : bit field types other than int
#pragma warning(disable : 4244)	// 'conversion' conversion from 'type1' to 'type2', possible loss of data
#pragma warning(disable : 4255)	// 'function' : no function prototype given: converting '()' to '(void)'
#pragma warning(disable : 4668)	// 'symbol' is not defined as a preprocessor macro, replacing with '0' for 'directives'
#pragma warning(disable : 4710)	// 'function' : function not inlined
#pragma warning(disable : 4820)	// 'bytes' bytes padding added after construct 'member_name'