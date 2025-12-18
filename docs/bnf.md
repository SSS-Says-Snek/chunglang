```ebnf
<program> ::= <statement-list>

<statement-list> ::= <statement> <statement-list>
                   | ε

<block> ::= "{" <statement-list> "}"

<statement> ::= <function-declaration>
              | <expression-statement>
              | <variable-declaration>

<function-declaration> ::= "func" <identifier> "(" <parameter-list> ")" <return-type>? <block>

<variable-declaration> ::= ( "let" | "mut" ) <identifier> ( "=" <expression> )? ";"

<parameter-list> ::= <parameter> ( "," <parameter> )*
                   | ε

<parameter> ::= <identifier> ":" <type>

<return-type> ::= "->" <type>

<type> ::= "int64"
         | "int32"
         | "bool"
         | "string"
         | <identifier>

<if-expression> ::= "if" "(" <expression> ")" <block> <else-clause>?

<else-clause> ::= "else" <block>

<expression-statement> ::= <expression> ";"?

<expression> ::= <call>
               | <primary>
               | <if-expression>
               | <block>

<logical-or> ::= <logical-and> ( "||" <logical-and> )*

<logical-and> ::= <equality> ( "&&" <equality> )*

<equality> ::= <comparison> ( ( "==" | "!=" ) <comparison> )*

<comparison> ::= <additive> ( ( ">" | ">=" | "<" | "<=" ) <additive> )*

<additive> ::= <multiplicative> ( ( "+" | "-" ) <multiplicative> )*

<multiplicative> ::= <unary> ( ( "*" | "/" | "%" ) <unary> )*

<unary> ::= ( "!" | "-" ) <unary>
          | <call>

<call> ::= <primary> ( "(" <argument-list> ")" )*

<argument-list> ::= <expression> ( "," <expression> )*
                  | ε

<primary> ::= <integer-literal>
            | <string-literal>
            | <boolean-literal>
            | <identifier>
            | "(" <expression> ")"

<identifier> ::= [a-zA-Z_][a-zA-Z0-9_]*

<integer-literal> ::= [0-9]+

<string-literal> ::= '"' [^"]* '"'

<boolean-literal> ::= "true" | "false"
```
