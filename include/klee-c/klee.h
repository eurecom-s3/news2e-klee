#ifndef _KLEE_H
#define _KLEE_H

#if !defined(__cplusplus)
#include <stdbool.h>
#endif /* defined(__cplusplus) */

#if defined(__cplusplus)
extern "C" {
#endif /* defined(__cplusplus) */

/* Opaque type definitions */
#if defined(__cplusplus)
namespace klee {
    class ObjectState;
}

using klee::ObjectState;
#else
typedef struct ObjectState ObjectState;
#endif /* defined(__cplusplus) */

bool klee_ObjectState_IsConcrete(ObjectState* os, unsigned offset, unsigned bit_size);


#if defined(__cplusplus)
}
#endif /* defined(__cplusplus) */

#endif /* _KLEE_H */
