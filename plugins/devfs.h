
#ifndef _DEVFS_H
#define _DEVFS_H

#define LMICE_DEVFS_PATH "/dev/input/"
#define LMICE_DEVFS_FILTER "mouse"

/**@ingroup devfsplugin
 * @brief system specifics for devfs plugin 
 */
struct DevfsItem {
    int fd; /**< Filepointer */
};

/**@ingroup devfsplugin
 *
 * @brief  Bit descriptions of the first byte
 */
enum {
    LEFT_BTN = 1 << 0,      /**< Left button */
    RIGHT_BTN = 1 << 1,     /**< Right button */
    MIDDLE_BTN= 1 << 2,     /**< Midle button */
    ALWAYS_ONE = 1 << 3,    /**< Always one, check byte */
    X_SIGNED = 1 << 4,      /**< If X is signed */
    Y_SIGNED = 1 << 5,      /**< If Y is signed */
    X_OVERFLOW = 1 << 6,    /**< If X has overflowed */
    Y_OVERFLOW = 1 << 7     /**< If Y has overflowd */
};

/**@ingroup devfsplugin
 * 
 * @brief function to test if a bit is set.
 */
#define BTEST(x,y) ((x) & (y) && 1)

#endif

