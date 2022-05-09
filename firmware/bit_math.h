#ifndef __BIT_MATH_H__
#define __BIT_MATH_H__

#define SET_BIT(x, bit) (x) |= (1 << (bit))
#define CLR_BIT(x, bit) (x) &= ~(1 << (bit))
#define TGL_BIT(x, bit) (x) ^= (1 << (bit))
#define GET_BIT(x, bit) (((x) >> (bit)) & 1)

// sets the value of the bit located at (bit) in (x) to (val)
#define SET_BIT_VAL(x, bit, val) x = ((x)&(~(1 << (bit)))) | (((val)&1)<<(bit))

// writes a value (val) to the bits not masked by (mask) in (x)
#define WRITE_MASK(x, mask, val) x = ((x)&(~(mask))) | ((val)&(mask))

#endif /*__BIT_MATH_H__*/
