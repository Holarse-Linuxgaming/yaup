// http://stackoverflow.com/questions/1644868/c-define-macro-for-debug-printing
#define debug_print(...) \
            do { if (DEBUG) fprintf(stderr, ##__VA_ARGS__); } while (0)
