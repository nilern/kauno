#include <cstdio>
#include <cstring>

#include "pos.cpp"
#include "object.cpp"
#include "gc.cpp"
#include "state.cpp"
#include "lexer.cpp"
#include "symbol.cpp"
#include "typesmap.hpp"
#include "ast.cpp"
#include "parser.cpp"
#include "globals.cpp"
#include "eval.cpp"

using namespace kauno;

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
        Handle<void> const expr = parse_expr(state, lexer);
        State_print_builtin(state, stdout, expr);
        puts("");

        puts("\n---\n");

        ORef<void> oexpr = expr.oref();
        state.pop(); // `expr`
        Handle<void> const value = eval(state, state.None.as_void(), oexpr);
        State_print_builtin(state, stdout, value);
        puts("");

        return EXIT_SUCCESS;
    } else {
        fputs("Wrong number of CLI arguments", stderr);

        return EXIT_FAILURE;
    }
}
