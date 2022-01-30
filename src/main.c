#include <stdio.h>

#include "object.c"
#include "gc.c"

int main(int argc, char* argv[]) {
    if (argc == 2) {
        puts(argv[1]);

        return EXIT_SUCCESS;
    } else {
        fputs("Wrong number of CLI arguments", stderr);

        return EXIT_FAILURE;
    }
}
