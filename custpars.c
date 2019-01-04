//
// Created by maximilian on 03.01.19.
//


#include <sys/stat.h>
#include <stdio.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <memory.h>

#define hashmult 13493690561280548289ULL

char* mapped_memory = NULL;
// The size of the mapped memory
long int mapped_memory_size = 0;
// The last valid address of the mapped memory
long long int last_address = 0;

// pointer to the next char in mapped memory
char* p_mm = NULL;

// if we are at the end
int eof = 0;


enum { END=256, ARRAY, OF, INT, RETURN, IF, THEN, ELSE, WHILE, DO, VAR, NOT, OR, ASSIGNOP };

#define NUMBER_OF_KEYWORDS 13
char * keywords[]  = {
        "end", "array", "of", "int", "return", "if", "then", "else", "while", "do", "var", "not", "or"
};

int keyword_enum [] = {
        END, ARRAY, OF, INT, RETURN, IF, THEN, ELSE, WHILE, DO, VAR, NOT, OR
};



unsigned long hash(char *s);

// ---------------------------------------------------


static int is_whitespace(char *value) {
    return *value == ' ' || *value == '\t' || *value == '\n';
}

static int is_lexchar(char *value) {
    return *value == ';'
            || *value == '('
            || *value == ')'
            || *value == ','
            || *value == ':'
            || *value == '<'
            || *value == '#'
            || *value == '['
            || *value == ']'
            || *value == '-'
            || *value == '+'
            || *value == '*';
}

/* Reads exactly one token.

   If it is whitespace => ignore
   If it is a comment => ignore everything until \n
   If it is a lexchar => return the char
      If it is the assignment operator (consists of the two lexchars ':' and '=') => return ASSIGNOP
   If it is a keyword => return the keyword
   If it is an integer => return the integer XORED with a magic number
      Note, that a token is also considered an integer, if it starts with a number.
            e.g. 39if are two tokens, an integer (39) and the keyword 'if'
   If it is a hex number (starts with $) => return the value XORED with a magic number
   If it is an ID (has to start with a letter) => return the hash of the ID
   If it is none of the above, throw an error
*/
int lex(void) {
    char *token_start = p_mm;
    int canAccessPmm = 1;

    int isDigit = 1;
    int isHexDigit = 1;
    int isComment = 0;

    while (p_mm <= (char *) last_address) {
        char *value = p_mm;
        p_mm++;

        if(p_mm > (char *) last_address) {
            canAccessPmm = 0;
            eof = 1;
        }

        if(is_whitespace(value)) {
            // value is p_mm - 1, so if value is whitespace, the next char after the whitespace is p_mm
            token_start = p_mm;

            // Comments are terminated by \n
            if(*value == '\n') {
                isComment = 0;
            }

            continue;
        }

        // if the current token is a lexchar
        if(is_lexchar(value) && !isComment) {
            if(!(*value == ':' && canAccessPmm && *p_mm == '=')) {
                if(*value == '-' && *p_mm == '-') {
                    isComment = 1;
                }
                else {
                    return *value;
                }
            }
            else {
                p_mm++; // we don't need to visit the =, as it is part of :=
                return ASSIGNOP;
            }
        }

        if(isComment) {
            continue;
        }

        // check whether this char invalidates the token as digit or hexdigit
        if(*value < '0' || *value > '9') {
            int oldIsDigit = isDigit;

            isDigit = 0;

            if((*value < 'A' || *value > 'F') && (*value < 'a' || *value > 'f') && *value != '$') {
                isHexDigit = 0;


                // The token was a digit, but isnt now and it also isnt a hex digit now --> parse the thing as number
                // also the first char of the token must be a digit
                // the generated parser parses 39if as number and if
                if(oldIsDigit == 1 && token_start[0] >= '0' && token_start[0] <= '9') {
                    p_mm--; // go back 1 position, as we want to parse the next token in the next run

                    char original_character = *value;
                    value[0] = '\0';
                    int val = strtoul(token_start, NULL, 10) ^ 0x8000;
                    value[0] = original_character;

                    return val;
                }
            }
        }


        // if the next char is either a whitespace or a lexchar
        if(canAccessPmm && (is_whitespace(p_mm) || is_lexchar(p_mm))) {

            // if the first element of the token is $, the token could be a hex digit
            if(*token_start == '$' && isHexDigit) {
                char original_character = *p_mm;
                p_mm[0] = '\0';

                int val = strtoul(token_start+1, NULL, 16) ^ 0x4000;

                p_mm[0] = original_character;

                return val;
            }

            // if every char of the token isDigit, then the token is a number
            if(isDigit) {
                char original_character = *p_mm;
                p_mm[0] = '\0';

                int val = strtoul(token_start, NULL, 10) ^ 0x8000;

                p_mm[0] = original_character;

                return val;
            }

            // check whether token is a keyword
            for (int i = 0; i < NUMBER_OF_KEYWORDS; i++) {
                char *p_k = keywords[i];
                char *tmp2 = token_start;

                int matches = 1;

                while (*p_k != '\0' && tmp2 <= value) {
                    if (*p_k != *tmp2) {
                        matches = 0;
                        break;
                    }

                    p_k++;
                    tmp2++;
                }

                // to be a keyword, it has to match the beginning of the token AND be as long as the token
                if(matches && tmp2==value+1 && *(p_k) == '\0') {
                    return keyword_enum[i];
                }
            }

            // finally, check for ID
            if((token_start[0] >= 'A' && token_start[0] <= 'Z') || (token_start[0] >= 'a' && token_start[0] <= 'z') ) {
                char original_character = *p_mm;
                p_mm[0] = '\0';

                int val = hash(token_start);

                p_mm[0] = original_character;

                return val;
            }
            else {
                char original_character = *p_mm;
                p_mm[0] = '\0';

                printf("Lexical error. Unrecognised input \"%s\"\n", token_start);

                p_mm[0] = original_character;

                exit(1);
            }
        }
    }
}


// ---------------------------------------------------

unsigned long hash(char *s) {
    unsigned long r = 0;
    char *p;
    for (p = s; *p; p++)
        r = (r + *p) * hashmult;
    return r;
}


int main(int argc, char *argv[]) {
    unsigned long x, r;
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <file>\n", argv[0]);
        exit(1);
    }

    int fd = open(argv[1], O_RDWR);
    struct stat attr;

    if(fstat(fd, &attr) < 0) {
        fprintf(stderr,"Error fstat\n");
        close(fd);
        exit(1);
    }

    mapped_memory_size = attr.st_size;
    mapped_memory = mmap(0, attr.st_size, PROT_READ|PROT_WRITE, MAP_PRIVATE, fd, 0);
    if(mapped_memory == MAP_FAILED) {
        printf("MMAP FAILED");
    }
    madvise(mapped_memory, mapped_memory_size, MADV_SEQUENTIAL);

    p_mm = mapped_memory;
    last_address = (long)mapped_memory + mapped_memory_size-1;

    for (x = 0; r = lex(), eof == 0;) {
        x = (x + r) * hashmult;
    }
    printf("%lx\n", x);


    //while(!eof) {
    //for(int i=0; i<28465; i++) {
    /*for(int i=0; !eof; i++) {
        int r = lex();

        if(i>=27369999) {
            printf("%d\n", r);
        }
    }*/

    return 0;
}
