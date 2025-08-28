#include <stdint.h>
#include <stdbool.h>

#ifndef MATH_C
#define MATH_C

bool IsPowerOfTwo(uintptr_t x)
{
    return (x & (x - 1)) == 0;
}

uintptr_t AlignForward(uintptr_t ptr, size_t align) {
	uintptr_t p, a, modulo;
    
	assert(IsPowerOfTwo(align));
    
	p = ptr;
	a = (uintptr_t)align;
	// Same as (p % a) but faster as 'a' is a power of two
	modulo = p & (a-1);
    
	if (modulo != 0) {
		// If 'p' address is not aligned, push the address to the
		// next value which is aligned
		p += a - modulo;
	}
	return p;
}

size_t CalcPaddingWithHeader(uintptr_t ptr, uintptr_t alignment, size_t header_size) {
	uintptr_t p, a, modulo, padding, needed_space;
    
	assert(IsPowerOfTwo(alignment));
    
	p = ptr;
	a = alignment;
	modulo = p & (a-1); // (p % a) as it assumes alignment is a power of two
    
	padding = 0;
	needed_space = 0;
    
	if (modulo != 0) { // Same logic as 'AlignForward'
		padding = a - modulo;
	}
    
	needed_space = (uintptr_t)header_size;
    
	if (padding < needed_space) {
		needed_space -= padding;
        
		if ((needed_space & (a-1)) != 0) {
			padding += a * (1+(needed_space/a));
		} else {
			padding += a * (needed_space/a);
		}
	}
    
	return (size_t)padding;
}
#endif