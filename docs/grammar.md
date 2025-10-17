// tiny basic grammar

prog -> line\*

line -> num stat cr | stat cr

cr -> \\n

num -> digit digit\*

digit -> 0 | 1 | 2 | ... | 8 | 9

stat -> \
PRINT expr-list \
IF expr relop expr THEN stat \
GOTO expr \
INPUT var-list \
LET var = expr \
GOSUB expr \
RETURN \
CLEAR \
LIST \
RUN \
END

expr -> (+|-|e) term ((+|-) term)\*

term -> factor ((\*|/) factor)\*

factor -> var | number | (expression)

var -> A | B | C | ... | Y | Z

expr-list -> (string|expr) (, (string|expr) )*

var-list -> var (, var)*

relop -> < (>|=|e) | > (<|=|e) | =

string -> " (ascii-char)* "

// note: \
// cr is new line \
// * is zero or many non-terminal \
// e is epsilon, empty string \
// LIST and RUN doesnt work (because is a compiler, not inter)