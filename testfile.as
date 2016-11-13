        EXTERN  loop
        EXTERN  lala
teste   IS      $1
testei  IS      13; help IS  #3; ant IS rA
* so comentario
test    STR     "HEY"
tet     STR     test
        ADDU    $0, $0, testei
        MUL     ant, $0, help
        JN      $2, test

loop    MUL     ant, ant, help
        JMP     loop * comentario teste
abc     JMP     loop
