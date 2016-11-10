teste   IS      $1
testei  IS      1
test    STR     "HEY"
tet     STR     test
        SETW    , #25
        ADDU    $0, $0, testei

loop    MUL     teste, teste, teste
        JMP     loop * comentario teste
