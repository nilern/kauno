#include <stdio.h>
#include <string.h>

#include "pos.c"
#include "object.c"
#include "gc.c"
#include "state.c"
#include "lexer.c"
#include "parser.c"

int main(int argc, char* argv[]) {
    if (argc == 2) {
        struct State state = State_new(1024*1024); // 1 MiB

        {
            struct Lexer lexer = Lexer_new(argv[1], strlen(argv[1]));
            while (true) {
                struct Token const tok = Lexer_peek(&lexer);
                if (tok.type != TOKEN_EOF) {
                    Token_print(stdout, tok);
                    puts("");
                    Lexer_next(&lexer);
                } else {
                    break;
                }
            }
        }

        puts("\n---\n");

        struct Lexer lexer = Lexer_new(argv[1], strlen(argv[1]));
        parse_expr(&state, &lexer);
        ORef const expr = State_pop(&state);
        State_print_builtin(&state, stdout, expr);
        puts("");

        return EXIT_SUCCESS;
    } else {
        fputs("Wrong number of CLI arguments", stderr);

        return EXIT_FAILURE;
    }
}
