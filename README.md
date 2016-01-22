# XEDParse

[![Build status](https://ci.appveyor.com/api/projects/status/p7w775lmulveb2d1?svg=true)](https://ci.appveyor.com/project/mrexodia/xedparse)

XEDParse is a library to parse MASM-like plaintext instructions to the XED
instruction format. The library is meant to have quick access to assembly
in certain low-level programs (such as debuggers).

Currently finished:
- Parsing instructions to an internal data structure
- Translate the instruction base to XED2
- Translate registers to XED2
- Encode the translated structure
- Translate values
- Translate memory arguments

Todo:
- Handle instructions with more than two operands correctly

Currently the parser can be considered reasonably fast, parsing about 
200,000 instructions in 1.5 seconds.
