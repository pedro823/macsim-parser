        EXTERN  loop
teste   IS      $1
testei  IS      1; help IS  #3
test    STR     "HEY"
tet     STR     test
        ADDU    $0, $0, testei

loop    MUL     teste, teste, teste
        JMP     loop * comentario teste
