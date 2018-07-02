#if __GNUC__
#include <signal.h>
#define DEBUG_BREAK() (raise(SIGTRAP))
#endif