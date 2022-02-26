#include <cstdio>
#include <cstring>

#include "pos.cpp"
#include "gc.cpp"
#include "state.cpp"
#include "lexer.cpp"
#include "symbol.cpp"
#include "parser.cpp"
#include "globals.cpp"
#include "eval.cpp"

int main(int argc, char* argv[]) {
    if (argc == 2) {
        State state(1024*1024, 1024*1024); // 1 MiB

        {
            Lexer lexer = Lexer_new(argv[1], strlen(argv[1]));
            while (true) {
                Token const tok = Lexer_peek(&lexer);
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

        Lexer lexer = Lexer_new(argv[1], strlen(argv[1]));
        Handle<Any> const expr = parse_expr(&state, &lexer);
        State_print_builtin(&state, stdout, expr);
        puts("");

        puts("\n---\n");

        Handle<Any> const value = eval(&state);
        State_print_builtin(&state, stdout, value);
        puts("");

        return EXIT_SUCCESS;
    } else {
        fputs("Wrong number of CLI arguments", stderr);

        return EXIT_FAILURE;
    }
}
