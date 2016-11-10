teste   IS      $1
        SETW    $0, 25
        ADDU    $0, $0, $3

loop    MUL     teste, teste, teste
        JMP     loop * comentario teste
