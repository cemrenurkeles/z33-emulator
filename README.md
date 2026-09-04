# z33-emulator
Emulator for zorglub33

### Préprocesseur

Les directives suivantes sont prises en charge :

```asm
#define DEBUG
#if defined(DEBUG)
    nop
#elif 1
    reset
#else
    trap
#endif
```

Les expressions conditionnelles acceptent `defined(SYM)`, les constantes et symboles
numériques définis avec `#define`, les opérateurs `!`, `*`, `/`, `%`, `+`, `-`, les
comparaisons, `&&` et `||`. Les blocs inactifs sont entièrement ignorés, y compris
leurs instructions et leurs `#include`.


### ⚠️ VS Code `.s` files

When using `.s` files in Visual Studio Code, the `%` character may sometimes be incorrectly interpreted or displayed as `‰`.

For example:

    ld 0,%b

may become:

    ld 0,‰b

This causes the parser to reject the register operand. If you encounter an `unknown label or invalid operand` error for a register, check that `%` has not been replaced by `‰`.

jeq  → saute si Z == 1
jne  → saute si Z == 0

jlt  → saute si N == 1
jge  → saute si N == 0

jle  → saute si N == 1 ou Z == 1
jgt  → saute si N == 0 et Z == 0
