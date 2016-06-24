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

/**
 * Verify if a given range is concrete.
 * @param self The object.
 * @param offset Byte offset of the start.
 * @param bit_size Size of the range in bits.
 * @return <b>true</b> if the range is entirely concrete, <b>false</b> otherwise.
 */
bool klee_ObjectState_IsConcrete(ObjectState* os, unsigned offset, unsigned bit_size);

/**
 * This function reads concrete data from the ObjectState.
 * @param self The object.
 * @param offset Byte offset of the data.
 * @param out Pointer where read data is written to.
 * @param size Byte size of the data.
 * @return <b>true</b> if data was read successfully (all data is concrete). <b>false</b> otherwise.
 */
bool klee_ObjectState_ReadConcrete(ObjectState* self, unsigned offset, uint8_t* out, unsigned size);

/**
 * This function writes concrete data to the ObjectState.
 * @param self The object.
 * @param offset Byte offset of the data in the ObjectState.
 * @param data Pointer to the data.
 * @param size Byte size of the data buffer.
 */

void klee_ObjectState_WriteConcrete(ObjectState* self, unsigned offset, const uint8_t* data, unsigned size);


#if defined(__cplusplus)
}
#endif /* defined(__cplusplus) */

#endif /* _KLEE_H */
