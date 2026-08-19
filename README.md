# z33-emulator
Emulator for zorglub33


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