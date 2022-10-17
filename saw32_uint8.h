#ifndef SAW32_uint_H_
#define SAW32_uint_H_

#if ARDUINO >= 100
 #include "Arduino.h"
#else
 #include "WProgram.h"
#endif
#include "mozzi_pgmspace.h"

#define SAW32_uint_NUM_CELLS 32
#define SAW32_uint_SAMPLERATE 32

CONSTTABLE_STORAGE(int8_t) SAW32_DATA []  =
        {
                0,  1,  2,  3,  4,  5,  6,  7,  
                8,  9,  10, 11, 12, 13, 14, 15,  
                16, 17, 18, 19, 20, 21, 22, 23,  
                24, 25, 26, 27, 28, 29, 30, 31 
        };

#endif /* SAW256_H_ */
