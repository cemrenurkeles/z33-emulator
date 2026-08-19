# z33-emulator
Emulator for zorglub33


### ⚠️ VS Code `.s` files

When using `.s` files in Visual Studio Code, the `%` character may sometimes be incorrectly interpreted or displayed as `‰`.

For example:

    ld 0,%b

may become:

    ld 0,‰b

This causes the parser to reject the register operand. If you encounter an `unknown label or invalid operand` error for a register, check that `%` has not been replaced by `‰`.