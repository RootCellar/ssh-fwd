#ifndef DEBUG_H
#define DEBUG_H

#define DEBUG 1

/*
 * Useful debug function define I found online
*/

#define debug_printf(fmt, ...) \
        do { if (DEBUG) fprintf(stderr, "%s:%d:%s(): " fmt, __FILE__, \
                                __LINE__, __func__, __VA_ARGS__); } while (0)

#define debug_print(fmt) \
        do { if (DEBUG) fprintf(stderr, "%s:%d:%s(): " fmt, __FILE__, \
                                __LINE__, __func__); } while (0)


/*
 * Useful for debugging why an exit happened by printing where it happened
 * and potentially some explanation as to why with perror()
*/

#define EXIT_FAIL() do { debug_print("EXIT_FAIL\n"); perror("SEVERE ERROR"); exit(EXIT_FAILURE); } while(0)

#endif
