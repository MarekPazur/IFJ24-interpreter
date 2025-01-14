# Implementation of an interpreter for imperative language IFJ24 

Project developed for the course Formal Languages and Compilers (IFJ) at Brno University of Technology, Faculty of Information Technology, during the 2024/2025 academic year.

This repository contains the implementation of an interpreter for a subset of the Zig programming language.

## Authors
- [Marek Pazúr](https://github.com/0x6B6)
- [Jan Ukropec](https://github.com/JanUkropec)
- [Patrik Tomaško](https://github.com/PatrikTomasko47)
- [Robert Glos](https://github.com/RobertGlos)

## Percentage evaluation of interpreter modules
Lexical Analysis (Error Detection): **65%** (138/210 mb, incorrect return codes: 34%)

Syntax Analysis (Error Detection): **95%** (255/267 mb, incorrect return codes: 4%)

Semantic Analysis (Error Detection): **93%** (376/402 mb, incorrect return codes: 6%)

Interpretation of Translated Code (Basic): **87%** (316/360 mb, incorrect return codes: 10%, mismatched outputs: 2%)

Interpretation of Translated Code (Expressions, Built-in Functions): **100%** (170/170 test cases, incorrect return codes: 0%, mismatched outputs: 0%)

Interpretation of Translated Code (Complex): **71%** (299/421 mb, incorrect return codes: 28%, mismatched outputs: 0%)

**Overall Without Extensions: 84% (1554/1830 mb)**

16.4/18 points

