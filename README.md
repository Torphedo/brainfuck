This is a bytecode compiler and interpreter for the [Brainfuck](https://en.wikipedia.org/wiki/Brainfuck) language.

# Usage
```bash
bfc hello.bf                  # Executes the BF source code
bfc hello.bf --dump hello.bfb # Executes the code and saves the bytecode to a binary file
bfc hello.bfb                 # Executes the precompiled bytecode
```
If your code has no comments, the bytecode will actually be larger than the source code (since it uses 3 bytes for loop begin/end commands).

# Bytecode Format
We use 1 byte per command, with the values corresponding to an internal enum. For loop begin/end commands there's an extra 16-bit value, 
which is the absolute address of the jump destination. 
