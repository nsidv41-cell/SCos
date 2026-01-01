/* ============================================================================
 * SCos 1.3.5 - Calculator Application
 * ============================================================================ */

#include "../include/scos.h"

/* Token types */
#define TOK_NUM     1
#define TOK_PLUS    2
#define TOK_MINUS   3
#define TOK_MUL     4
#define TOK_DIV     5
#define TOK_MOD     6
#define TOK_LPAREN  7
#define TOK_RPAREN  8
#define TOK_END     9

/* Calculator state */
static const char *expr_ptr;
static int current_token;
static int token_value;

/* Get next token */
static void next_token(void) {
    while (*expr_ptr == ' ') {
        expr_ptr++;
    }
    
    if (*expr_ptr == '\0') {
        current_token = TOK_END;
        return;
    }
    
    if (isdigit(*expr_ptr)) {
        token_value = 0;
        while (isdigit(*expr_ptr)) {
            token_value = token_value * 10 + (*expr_ptr - '0');
            expr_ptr++;
        }
        current_token = TOK_NUM;
        return;
    }
    
    switch (*expr_ptr) {
        case '+': current_token = TOK_PLUS; break;
        case '-': current_token = TOK_MINUS; break;
        case '*': current_token = TOK_MUL; break;
        case '/': current_token = TOK_DIV; break;
        case '%': current_token = TOK_MOD; break;
        case '(': current_token = TOK_LPAREN; break;
        case ')': current_token = TOK_RPAREN; break;
        default:
            vga_puts("Error: Invalid character '");
            vga_putchar(*expr_ptr);
            vga_puts("'\n");
            current_token = TOK_END;
            return;
    }
    expr_ptr++;
}

/* Forward declarations */
static int parse_expr(void);
static int parse_term(void);
static int parse_factor(void);

/* Parse expression (handles + and -) */
static int parse_expr(void) {
    int result = parse_term();
    
    while (current_token == TOK_PLUS || current_token == TOK_MINUS) {
        int op = current_token;
        next_token();
        int term = parse_term();
        
        if (op == TOK_PLUS) {
            result += term;
        } else {
            result -= term;
        }
    }
    
    return result;
}

/* Parse term (handles *, /, %) */
static int parse_term(void) {
    int result = parse_factor();
    
    while (current_token == TOK_MUL || current_token == TOK_DIV || 
           current_token == TOK_MOD) {
        int op = current_token;
        next_token();
        int factor = parse_factor();
        
        if (op == TOK_MUL) {
            result *= factor;
        } else if (op == TOK_DIV) {
            if (factor == 0) {
                vga_puts("Error: Division by zero\n");
                return 0;
            }
            result /= factor;
        } else {
            if (factor == 0) {
                vga_puts("Error: Modulo by zero\n");
                return 0;
            }
            result %= factor;
        }
    }
    
    return result;
}

/* Parse factor (handles numbers, parentheses, unary minus) */
static int parse_factor(void) {
    int result = 0;
    
    if (current_token == TOK_MINUS) {
        next_token();
        result = -parse_factor();
    } else if (current_token == TOK_PLUS) {
        next_token();
        result = parse_factor();
    } else if (current_token == TOK_NUM) {
        result = token_value;
        next_token();
    } else if (current_token == TOK_LPAREN) {
        next_token();
        result = parse_expr();
        if (current_token != TOK_RPAREN) {
            vga_puts("Error: Missing closing parenthesis\n");
        } else {
            next_token();
        }
    } else {
        vga_puts("Error: Unexpected token\n");
    }
    
    return result;
}

/* Evaluate expression */
static int evaluate(const char *expression) {
    expr_ptr = expression;
    next_token();
    
    int result = parse_expr();
    
    if (current_token != TOK_END) {
        vga_puts("Error: Unexpected token at end\n");
    }
    
    return result;
}

/* Main calculator function */
void calc_run(int argc, char **argv) {
    /* If expression provided as argument, evaluate and return */
    if (argc > 1) {
        /* Combine all arguments into expression */
        char expression[256] = "";
        for (int i = 1; i < argc; i++) {
            if (i > 1) strcat(expression, " ");
            strcat(expression, argv[i]);
        }
        
        int result = evaluate(expression);
        
        char buf[64];
        sprintf(buf, "%d\n", result);
        vga_puts(buf);
        return;
    }
    
    /* Interactive mode */
    vga_puts("\n");
    vga_put_color("SCos Calculator\n", 0x0A);
    vga_puts("===============\n");
    vga_puts("Enter expressions to evaluate. Type 'quit' to exit.\n");
    vga_puts("Operators: + - * / % ()\n\n");
    
    char input[256];
    
    while (1) {
        vga_put_color("calc> ", 0x0B);
        
        int pos = 0;
        while (pos < 255) {
            char c = keyboard_getchar();
            
            if (c == '\n' || c == '\r') {
                vga_putchar('\n');
                input[pos] = '\0';
                break;
            } else if (c == '\b' && pos > 0) {
                pos--;
                vga_putchar('\b');
            } else if (c >= ' ' && c <= '~') {
                input[pos++] = c;
                vga_putchar(c);
            }
        }
        
        /* Check for quit */
        if (strcmp(input, "quit") == 0 || strcmp(input, "exit") == 0 || 
            strcmp(input, "q") == 0) {
            break;
        }
        
        /* Check for empty input */
        if (strlen(input) == 0) {
            continue;
        }
        
        /* Check for help */
        if (strcmp(input, "help") == 0) {
            vga_puts("Operations:\n");
            vga_puts("  + : Addition\n");
            vga_puts("  - : Subtraction\n");
            vga_puts("  * : Multiplication\n");
            vga_puts("  / : Division\n");
            vga_puts("  % : Modulo\n");
            vga_puts("  () : Parentheses for grouping\n");
            vga_puts("\nExamples:\n");
            vga_puts("  2 + 3 * 4\n");
            vga_puts("  (2 + 3) * 4\n");
            vga_puts("  100 / 7\n");
            continue;
        }
        
        /* Evaluate expression */
        int result = evaluate(input);
        
        char buf[64];
        vga_put_color("  = ", 0x0E);
        sprintf(buf, "%d\n", result);
        vga_put_color(buf, 0x0F);
    }
}
