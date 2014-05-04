
#ifndef _RESULT_H
#define _RESULT_H

#include "liblmice.h"

/* most result functions are exportet in liblmice.h */
bool lmice_result_get(LMiceResult *result);
    
/* data functions */
inline LMiceData *lmice_data_new();
inline void lmice_data_clear(LMiceData *data);
inline LMiceData *lmice_data_dup(LMiceData *src);
inline LMiceData *lmice_data_copy(LMiceData *dst, LMiceData *src);
inline void lmice_data_delete(LMiceData *data);


#endif
