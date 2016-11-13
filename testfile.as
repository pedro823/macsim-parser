        EXTERN  loop
teste   IS      $1
testei  IS      1; help IS  #3; ant IS rA
* so comentario
test    STR     "HEY"
tet     STR     test
        ADDU    $0, $0, testei
        MUL     teste, $0, help

loop    MUL     teste, teste, teste
        JMP     loop * comentario teste
abc     JMP     loop
        GETA    ant, tet
        RET     2
