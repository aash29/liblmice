#include "result.h"
#include "lmice.h"
#include "string.h"

#include <math.h>
#include <stdlib.h>

#define DBG_FILE "result.c"

/**
 * Definition of PI 
 */
#define PI 3.1416

/**@defgroup lmiceresult Result handling
 * @ingroup liblmice
 *
 * The functions for settingi up and usign resultstorages with liblmice. These
 * are used to get information on the mices from liblmice. They can be used
 * even if the lmice library hasn't been initialized and they can be used over
 * a reinitialization.
 * 
 * Example usage :
 * <pre>
 * LMiceResult *resut1 = NULL; // Create a new resultstorage.
 * result1 = lmice_result_new(); // Allocate memmory for result1
 * lmice_read(result1, true); // store mousedata in result1
 * LMiceResult *retult2 = lmice_result_dup(result1); // duplicate result1 into result2
 * lmice_read(result2, true); // store mousedata in result2
 * lmice_result_copy(result1, result2); // copy mousedata from result2 into result1
 * lmice_result_delete(result1); // unallocate memory used by result2
 * lmice_result_delete(result2); // unallocate memory used by resutlt2
 * </pre>
 *
 * @{
 */

/** 
 * @brief Creating a new resultstorage.
 *
 * This function returns a newly created resultstorage (LMiceResult) that can
 * be used with lmice_result_* functions. Thre storage is not cleard.
 * 
 * @return A newly created LMiceResult resultstorage pointer.
 */
LMiceResult *lmice_result_new()
{
    DBG("result_new()");
    LMiceResult *result = malloc(sizeof(LMiceResult));
    if (!result) {
	if (lmice)
	    ERR("Could not create result");
	return NULL;
    }
    result->avg = lmice_data_new();
    lmice_data_clear(result->avg);
    result->count = 0;
    result->time = 0;
    return result;
}

/** 
 * @brief Duplicate a LMiceResult storage
 *
 * This function returns a new LMiceResult.
 *
 * @param src	The resultset to duplicate
 *
 * @return  A newly allocated resultset 
 */
LMiceResult *lmice_result_dup(LMiceResult *src)
{
    DBG("result_dup(src-count=%d)", src->count);
    if (!src) {
	WRN("duplicationg a NULL source");
    }
    LMiceResult *dst = lmice_result_new();
    return lmice_result_copy(dst, src);
}

/** @brief Copy a LMiceResult storage
 *
 * This function removes any information currently in a LMiceResult and 
 * fills it with the information from the source.
 *
 * @param dst	Destination LMiceResult
 * @param src	Source LMiceResult
 *
 * @return  A pointer to dst, NULL on error
 */
LMiceResult *lmice_result_copy(LMiceResult *dst, LMiceResult *src)
{   
    DBG("result_copy()");
    if (!dst || !src) {
	if (lmice)
	    ERR("dst or src is NULL.");
	return NULL;
    }
    if (dst->count == src->count) {
	DBG("fast copy");
	for(int i=0; i<src->count; i++) 
	    lmice_data_copy(dst->data[i], src->data[i]);
    } else {
	for(int i=0; i<dst->count; i++)
	    lmice_data_delete(dst->data[i]);
	
	for(int i=0; i<src->count; i++)
	    dst->data[i] = lmice_data_dup(src->data[i]);
	dst->count = src->count;
    }
    dst->time = src->time;
    lmice_data_copy(dst->avg, src->avg);
    return dst;
}

/** 
 * @brief Clean a resultstorage.
 *
 * This function clears a LMiceResult.
 * 
 * @param result    the resultstorage that should be cleard. if 
 *		    result is NULL internal data is cleard.
 *
 * @return true if the result was cleard, false if the result was NULL.
 */
int lmice_result_clear(LMiceResult *result)
{
    DBG("result_clear(%d)", result->count);
    if (!result) {
	WRN("The resultset is already NULL");
	return false;
    }
    lmice_data_clear(result->avg);
    for (int i=0; i<result->count;i++)
	lmice_data_clear(result->data[i]);
    result->time = 0;
    /* empty */
    return true;
}

/** 
 * @brief Remove and free a resultstorage.
 *
 * Remove and free alocated memmory used by a resultstorage.
 *
 * @param result    the resultstorage that should be deleted.
 */
int lmice_result_delete(LMiceResult *result)
{
    DBG("result_delete()");
    
    lmice_data_delete(result->avg);
    
    
    for (int i=0;i<result->count;i++)
	lmice_data_delete(result->data[i]);
    free(result);
    return true;
}

/* @} */

/**@defgroup intresdata Data handling function of resultstorages
 * @ingroup lmiceresult
 *
 * Internal functions to handle the datastorages.
 *
 * @{ 
 */

/** 
 * @brief Create a datastorage
 *
 * Return a newly created datastroage
 *
 * @return A newly allocated and cleard datastorage
 */
inline LMiceData *lmice_data_new()
{   
    DBG("data_new()");
    return malloc(sizeof(LMiceData));
}

/** 
 * @brief Clean a datastorage
 * 
 * This function will clear a specific LMiceData storage.
 * 
 * @param data  The LMiceData to be cleard.
 */
inline void lmice_data_clear(LMiceData *data)
{   
    DBG("data_clear()");
    memset(data, 0x00, sizeof(LMiceData));
}

/** 
 * @brief Duplicate a datastorage to another
 *
 * This will dubplicate a LMiceData storage into a newly created
 * LMiceData storage.
 * 
 * @param src   The LMiceData storage to duplicate.
 *
 * @return  The newly allocated duplica of the src.
 */
inline LMiceData *lmice_data_dup(LMiceData *src)
{
    DBG("data_dup()");
    return lmice_data_copy(lmice_data_new(), src);
}

/** 
 * @brief Copy a datastorage
 *
 * This function will copy the information in src LMiceData storage to
 * the dst LMiceData storage.
 *
 * @param dst   The datastorage to copy to.
 * @param src   The datastorage to copy from.
 *
 * @return  A pointer to dst.
 */
inline LMiceData *lmice_data_copy(LMiceData *dst, LMiceData *src)
{
    DBG("data_copy()");
    return memcpy(dst, src, sizeof(LMiceData));
}

/** 
 * @brief Remove a datastorage
 * 
 * This function will free the specified datastorage
 *
 * @param data  the datastorage to be unallocated.
 */
inline void lmice_data_delete(LMiceData *data)
{
    DBG("data_delete()");
    free(data);
}

/* @} */

