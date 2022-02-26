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
            Lexer lexer(argv[1], strlen(argv[1]));
            while (true) {
                Lexer::Token const tok = lexer.peek();
                if (tok.type != Lexer::Token::Type::TOKEN_EOF) {
                    tok.print(stdout);
                    puts("");
                    lexer.next();
                } else {
                    break;
                }
            }
        }

        puts("\n---\n");

        Lexer lexer(argv[1], strlen(argv[1]));
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
