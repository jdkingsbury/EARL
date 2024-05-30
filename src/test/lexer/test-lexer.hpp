#ifndef LEXER_H
#define LEXER_H

#include "test-suite.hpp"

test_errno_t test_lexer_next_token(void);
test_errno_t test_lexer_peek_token(void);
test_errno_t test_lexer_append_token(void);
test_errno_t test_lexer_discard_token(void);

#endif // LEXER_H
