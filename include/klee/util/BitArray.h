//===-- BitArray.h ----------------------------------------------*- C++ -*-===//
//
//                     The KLEE Symbolic Virtual Machine
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef KLEE_UTIL_BITARRAY_H
#define KLEE_UTIL_BITARRAY_H

namespace klee {

  // XXX would be nice not to have
  // two allocations here for allocated
  // BitArrays
class BitArray {
private:
  uint32_t *bits;
  
protected:
  static uint32_t length(unsigned size) { return (size+31)/32; }

public:
  BitArray(unsigned size, bool value = false) : bits(new uint32_t[length(size)]) {
    memset(bits, value?0xFF:0, sizeof(*bits)*length(size));
  }
  BitArray(const BitArray &b, unsigned size) : bits(new uint32_t[length(size)]) {
    memcpy(bits, b.bits, sizeof(*bits)*length(size));
  }
  ~BitArray() { delete[] bits; }

  inline bool get(unsigned idx) { return (bool) ((bits[idx/32]>>(idx&0x1F))&1); }
  inline void set(unsigned idx) { bits[idx/32] |= 1<<(idx&0x1F); }
  inline void unset(unsigned idx) { bits[idx/32] &= ~(1<<(idx&0x1F)); }
  inline void set(unsigned idx, bool value) { if (value) set(idx); else unset(idx); }

  inline bool isAllZeros(unsigned start, unsigned size) {
    const unsigned end = start + size - 1;
    const unsigned word_width = sizeof(*bits) * 8;
    const uint32_t start_mask = ~((1 << (start % word_width)) - 1);
    const uint32_t end_mask = (1 << ((end + 1) & word_width)) - 1;
    
    //Handle the case where start and end are in the same array word
    if (start / word_width == end / word_width) {
        return (bits[start / word_width] & (start_mask & end_mask)) == 0;
    }

    //Handle the case where start and end are in different words
    if ((bits[start / word_width] & start_mask) != 0)
        return false;

    for(unsigned i = (start + word_width - 1) / word_width; i < (end - word_width + 1) / word_width; ++i)
      if(bits[i] != 0)
        return false;

    return (bits[end / word_width] & end_mask) == 0;
  }

  inline bool isAllZeros(unsigned size) {
      return isAllZeros(0, size);
  }

  inline bool isAllOnes(unsigned size) {
	  return isAllOnes(0, size);
  }

  inline bool isAllOnes(unsigned start, unsigned size) {
      const unsigned end = start + size - 1;
      const unsigned word_width = sizeof(*bits) * 8;
      const uint32_t start_mask = ~((1 << (start % word_width)) - 1);
      const uint32_t end_mask = (1 << ((end + 1) % word_width)) - 1;

      //Handle the case where start and end are in the same array word
      if (start / word_width == end / word_width) {
          return ((~bits[start / word_width]) & (start_mask & end_mask)) == 0;
      }

      //Handle the case where start and end are in different words
      if (((~bits[start / word_width]) & start_mask) != 0)
          return false;

      for(unsigned i = (start + word_width - 1) / word_width; i < (end - word_width + 1) / word_width; ++i)
        if(~bits[i] != 0)
          return false;

      return ((~bits[end / word_width] & end_mask)) == 0;
    }
};

} // End klee namespace

#endif
