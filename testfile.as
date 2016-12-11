        EXTERN  loop
        EXTERN  lala
hehe    IS      $1
testei  IS      13; help IS  #3; ant IS rA
* so comentario
test    STR     "JEY"
tet     STR     test
        ADDU    $0, $0, testei
        MUL     ant, $0, help
        JN      $2, test
        CALL    tet

loop    MUL     ant, ant, help
        JMP     loop * comentario teste
lala    JMP     loop
        RET     5
