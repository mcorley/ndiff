%option noyywrap
%option never-interactive
%option yylineno
%{
#define TOK_ALPHANUM	 	251
#define TOK_DIGIT 			252
#define TOK_OP   			253
#define TOK_HTML 			254
#define TOK_WS 			255
extern int yylineno;
extern const char *yyfilename;
extern char *yytext;
extern int yylex();
%}
  /* Definitions */

SPTAB       [ \t]
WS			[\t\r\n ]

LETTER		[a-zA-Z_]
DIGIT		[0-9]
HEX_DIGIT	[0-9a-fA-F]
NUL			\0

WORD_FIRST	[$#@]|{LETTER}
WORD_REST	[-'@#$]|({LETTER}|{DIGIT})
WORD		{WORD_FIRST}{WORD_REST}*

HEX_DIGITS 	[0][xX]{HEX_DIGIT}*
DEC_DIGITS	{DIGIT}*
FLOAT		{DIGIT}*[.]{DIGIT}*

COMPOUND_ASSIGNMENT	("+="|"-="|"*="|"/="|"%="|">>="|"<<="|"&="|"^="|"|=")
INC_DEC 			("++"|"--")
RELATIONAL 			("=="|"!="|">="|"<=")
LOGICAL 			("&&"|"||")
BITWISE 			("<<"|">>")

PAREN ("["|"{"|"("|")"|"}"|"]")

OP	[-+*/%#@`\<=\>$!?,&|^~;:.\\'"]|"::"|{COMPOUND_ASSIGNMENT}|{INC_DEC}|{RELATIONAL}|{LOGICAL}|{BITWISE}|{PAREN}

TAGBEG	"<"[\\]?
TAGEND	[\\]?">"
TAGIN	({WORD}|{SPTAB}|[-=+*/;:'".()])+

  /* Rules */
%%
{WORD} { 
  return TOK_ALPHANUM;
}

{DEC_DIGITS}|{HEX_DIGITS}|{FLOAT} { 
  return TOK_DIGIT; 
}

{TAGBEG}{TAGIN}{TAGEND} { 
  return TOK_HTML;
}

{OP} { 
  return TOK_OP; 
}

{WS}+ {
  return TOK_WS;
}
%%

