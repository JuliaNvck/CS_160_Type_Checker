# Assignment 1: Type Checking

__RELEASED:__ Tuesday, Oct 21
__DUE:__ Tuesday, Nov 4
__LATE DEADLINE 1:__ Tuesday, Nov 11 (-11%)
__LATE DEADLINE 2:__ Friday, Dec 12 (-31%)

You may work in groups of 1--3 people. If working in a group of 2--3, be sure to indicate that when submitting to Gradescope.

## Description

Implement the Cflat type system as described in the `validation.pdf` handout distributed with this assignment. Your implementation should output whether the input program is valid or not, and if not then how it failed validation.

## Input/Output Specifications

The input will be a file containing a syntactically-correct Cflat AST in JSON format. You can assume that the `Naming Rules` given in `validation.pdf` are correct without checking them; all other rules need to be checked per the given type system. The AST may or may not be valid; if it is invalid then it will have exactly one type error, i.e., there will be a single AST node s.t. the premises of the typing rule that should apply are not satisfied.

The library `json.hpp` by Niels Lohmann is also distributed with this assignment for you to use to handle the JSON format. Instructions on how to use it are given in the `json.hpp` file. You should read the input into a JSON data structure using that library, then copy the contents of the JSON data structure into your own AST data structure before doing type checking. Trying to directly use the JSON version can result in an extremely slow program because the JSON format consists solely of strings.

The output should be given on standard out and will be one of:

1. "valid", for a valid program.
2. "invalid: <error>", for an invalid program.

The error should be one of the following, which explains which premise of which typing rule was not satisfied:

- no 'main' function with type '() -> int' exists
- invalid type <type> for variable <name> in function <function name>
- function <function name> has an empty body
- function <function name> may not execute a return
- empty struct <struct name>
- invalid type <type> for struct field <struct name>::<field name>
- incompatible return type <type> for 'return <exp>', should be <proper return type>
- continue outside loop
- break outside loop
- non-int type <type> for while guard '<exp>'
- non-int type <type> for if guard '<exp>'
- invalid type <type> for left-hand side of assignment '<lhs> = <rhs>'
- incompatible types <lhs type> vs <rhs type> for assignment '<lhs> = <rhs>'
- trying to call 'main'
- trying to call type <type> as function pointer in call '<call exp>'
- incorrect number of arguments (<num args> vs <num parameters>) in call '<call exp>'
- incompatible argument type <arg type> vs parameter type <parameter type> for argument '<arg exp>' in call '<call exp>'
- non-int type <type> used for second argument of allocation '[<type>; <exp>]'
- invalid type used for first argument of allocation '[<type>; <exp>]'
- invalid type used for allocation 'new <type>'
- non-int type <type> for left operand of binary op '<lhs> <op> <rhs>'
- non-int type <type> for right operand of binary op '<lhs> <op> <rhs>'
- incompatible types <lhs type> vs <rhs> in binary op '<lhs> <op> <rhs>'
- invalid type <type> used in binary op '<lhs> <op> <rhs>'
- non-int operand type <type> in unary op '<op> <exp>'
- non-int type <type> for select guard '<guard exp>'
- incompatible types <true branch type> vs <false branch type> in select branches '<true branch>' vs '<false branch>'
- non-existent field <struct name>::<field name> in field access '<exp>.<field name>'
- non-existent struct type <struct name> in field access '<exp>.<field name>'
- <type> is not a struct pointer type in field access '<exp>.<field name>'
- non-array type <type> for array access '<array exp>[<index exp>]'
- non-int index type <type> for array access '<array exp>[<index exp>]'
- non-pointer type <type> for dereference '<exp>.*'
- id <name> does not exist in this scope

- EXAMPLE 1

Input
```
fn main() -> int { return 42; }
```

Output
```
valid
```

- EXAMPLE 2

Input
```
fn main() -> int {
    let a:int, b:&int;
    a = b;
    return 0;
}
```

Output
```
invalid: incompatible types int vs &int for assignment 'a = b'
```

## Grading

The grading will be done using a set of test suites, weighted evenly. The test suites for this assignment are:

- `TS1`: `main` function, constants, locals, assignments, return
- `TS2`: TS1 + unary and binary operators
- `TS3`: TS2 + if, while
- `TS4`: TS3 + continue, break, select
- `TS5`: TS3 + non-{struct, function} pointers, nil, singleton allocation
- `TS6`: TS3 + arrays, array accesses, nil, array allocation
- `TS7`: TS3 + structs, struct allocation, struct accesses
- `TS8`: TS3 + other functions, externs, function pointers, direct calls, indirect calls
- `TS9`: everything in TS{1--8} all together

All test suites will have a mix of valid and invalid programs.

## Reference Solution

I have placed an executable of my own Cflat compiler implementation on CSIL in `~benh/160/cflat`. Use `cflat --help` to see how to use it. In particular for this assignment:

- You can use `cflat` to translate a `*.cb` file into the `*.astj` format using `cflat -o astjson <file>.cb`.

- Use `cflat -o valid <file>.astj` to print whether the input is a valid Cflat program or, if not, how it is invalid, using the same output format needed for this assignment.

You can use the reference solution to test your code before submitting. If you have any questions about the output format or the behavior of the lexer or parser you can answer them using the reference solution; this is the solution that Gradescope will use to test your submissions.

## Submitting to Gradescope

The autograder is running Ubuntu 22.04 and has the latest `build-essential` package installed (including `g++` version 11.4.0 and the `make` utility). You may not use any other libraries or packages for your code. Your submission must meet the following requirements:

- There must be a `makefile` file s.t. invoking the `make` command builds your code, resulting in an executables called `type`. The executable must take a single argument (the file to run the type checker on) and produce its output on standard out.

- If your solution contains sub-directories then the entire submission must be given as a zip file (this is required by Gradescope for submissions that contain a directory structure, otherwise it will automatically flatten everything into a single directory). The `makefile` should be in the root directory of the solution.

- The submitted files are not allowed to contain any binaries, and your solution is not allowed to use the network.

## Use AddressSanitizer

Student programs can often contain memory errors that happen not to manifest themselves when the students execute their code locally, but do manifest themselves when the program is uploaded and executed by Gradescope. This difference in behavior can result in confusion because Gradescope is claiming there is an error, but there is seemingly no error when the students run the program themselves (note that there actually is an error, it just happens to not be caught at runtime). To avoid this issue, please always compile your code using AddressSanitizer, which will catch most such errors locally. See the following site for an explanation of how to do so (the given flags are for the `clang` compiler, but they should be the same for `gcc`): https://github.com/google/sanitizers/wiki/AddressSanitizer.
