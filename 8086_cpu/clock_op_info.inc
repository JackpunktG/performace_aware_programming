#ifndef CLOCKS_OPS
#define CLOCKS_OPS(Mnemonic, ...) {Op_##Mnemonic,  __VA_ARGS__},
#endif

#ifndef E_TYPE
#define E_TYPE(encoding) Encoding_##encoding,
#endif

E_TYPE(undefined)
E_TYPE(register_register)
E_TYPE(register_memory)
E_TYPE(memory_register)
E_TYPE(register_immediate)
E_TYPE(memory_immediate)
E_TYPE(segmentR_register)
E_TYPE(segmentR_memory)
E_TYPE(register_segmentR)
E_TYPE(memory_segmentR)
E_TYPE(jump_successful)
E_TYPE(jump_fallthrough)
E_TYPE(plus_EA)

#undef E_TYPE 

#define rr(clocks) {Encoding_register_register, clocks, 0}
#define rm(clocks) {Encoding_register_memory, clocks, 1}
#define mr(clocks) {Encoding_memory_register, clocks, 1}
#define ri(clocks) {Encoding_register_immediate, clocks, 0}
#define mi(clocks) {Encoding_memory_immediate, clocks, 1}
#define sr(clocks) {Encoding_segmentR_register, clocks, 0}
#define sm(clocks) {Encoding_segmentR_memory, clocks, 1}
#define ms(clocks) {Encoding_memory_segmentR, clocks, 1}
#define js(clocks) {Encoding_jump_successful, clocks, 0}
#define jf(clocks) {Encoding_jump_fallthrough, clocks, 0}
#define ea(clocks) {Encoding_plus_EA, clocks, 1}

CLOCKS_OPS(adc, {rr(3), rm(9), mr(16), ri(4), mi(17)})
CLOCKS_OPS(add, {rr(3), rm(9), mr(16), ri(4), mi(17)})
CLOCKS_OPS(and, {rr(3), rm(9), mr(16), ri(4), mi(17)})
CLOCKS_OPS(cmp, {rr(3), rm(9), mr(9), ri(4), mi(10)})
CLOCKS_OPS(or, {rr(3), rm(9), mr(16), ri(4), mi(14)})
CLOCKS_OPS(sbb, {rr(3), rm(9), mr(16), ri(4), mi(17)})
CLOCKS_OPS(sub, {rr(3), rm(9), mr(16), ri(4), mi(17)})
CLOCKS_OPS(test, {rr(3), rm(9), ri(4), mi(17)})
CLOCKS_OPS(xor, {rr(3), rm(9), mr(16), ri(4), mi(17)})

CLOCKS_OPS(mov, {rr(2), rm(8), mr(9), mi(10), sr(2), sm(8), ms(9)})

CLOCKS_OPS(jnbe, {js(16), jf(4)})
CLOCKS_OPS(jnb, {js(16), jf(4)})
CLOCKS_OPS(jb, {js(16), jf(4)})
CLOCKS_OPS(jbe, {js(16), jf(4)})
CLOCKS_OPS(jcxz, {js(18), jf(6)})
CLOCKS_OPS(je, {js(16), jf(4)})
CLOCKS_OPS(jnle, {js(16), jf(4)})
CLOCKS_OPS(jnl, {js(16), jf(4)})
CLOCKS_OPS(jl, {js(16), jf(4)})
CLOCKS_OPS(jle, {js(16), jf(4)})
CLOCKS_OPS(jnp, {js(16), jf(4)})
CLOCKS_OPS(jno, {js(16), jf(4)})
CLOCKS_OPS(jnp, {js(16), jf(4)})
CLOCKS_OPS(jns, {js(16), jf(4)})
CLOCKS_OPS(jo, {js(16), jf(4)})
CLOCKS_OPS(jp, {js(16), jf(4)})
CLOCKS_OPS(js, {js(16), jf(4)})
CLOCKS_OPS(loop, {js(17), jf(5)})
CLOCKS_OPS(loopz, {js(18), jf(6)})
CLOCKS_OPS(loopnz, {js(19), jf(5)})

CLOCKS_OPS(lds, {ea(16)})
CLOCKS_OPS(lea, {ea(2)})
CLOCKS_OPS(les, {ea(16)})

#undef rr
#undef rm
#undef mr
#undef ri
#undef mi
#undef sr
#undef sm
#undef ms
#undef js
#undef jf
#undef ea   

#undef CLOCKS_OPS
