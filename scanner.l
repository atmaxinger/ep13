%{

  enum { END=256, ARRAY, OF, INT, RETURN, IF, THEN, ELSE, WHILE, DO, VAR,
	 NOT, OR, ASSIGNOP };

  int eof = 0;

  unsigned long hash(char *s);


%}
%Option noyywrap

COMMENT     --.*
ID          [A-Za-z][A-Za-z0-9]*
DIGIT       [0-9]
INTEGER     {DIGIT}+
HEXDIGIT    [A-Fa-f0-9]
HEXNUM      \$({HEXDIGIT}+)
KEYWORD     end|array|of|int|return|if|then|else|while|do|var|not|or
LEXEM_CHAR  ;|"("|")"|,|:|"<"|#|"["|"]"|"-"|"+"|"*"
ASSIGNOP    :=
WHITESPACE  [\n\t ]

%%
end             return END;
array           return ARRAY;
of              return OF;
int             return INT;
return          return RETURN;
if              return IF;
then            return THEN;
else            return ELSE;
while           return WHILE;
do              return DO;
var             return VAR;
not             return NOT;
or              return OR;
{LEXEM_CHAR}    return yytext[0];
{ASSIGNOP}      return ASSIGNOP;
{ID}            return hash(yytext);
{INTEGER}       return strtoul(yytext, NULL, 10) ^ 0x8000;
{HEXNUM}        return strtoul(yytext+1, NULL, 16) ^ 0x4000;
{WHITESPACE}+   ;
{COMMENT}       ;
.               printf("Lexical error. Unrecognised input \"%s\"\n", yytext); exit(1);
<<EOF>>		eof = 1; return 0;
%%

#define hashmult 13493690561280548289ULL

unsigned long hash(char *s)
{
  unsigned long r=0;
  char *p;
  for (p=s; *p; p++)
    r = (r+*p)*hashmult;
  return r;
}

int main(int argc, char *argv[])
{
  unsigned long x, r;
  if (argc != 2) {
    fprintf(stderr,"Usage: %s <file>\n",argv[0]);
    exit(1);
  }
  yyin = fopen(argv[1],"r");
  if (yyin == NULL) {
    perror(argv[1]);
    exit(1);
  }
  for (x=0; r=yylex(), eof==0;)
    x = (x+r)*hashmult;
  printf("%lx\n",x);
  return 0;
}
