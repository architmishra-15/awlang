# AwLang Compiler

# Project

The project is just a simple fun hobby project for learning about compiler design and programming languages at low level.

The language will be a compiled language with some different unorthodoxical rules.


## Features

- 🎯 **Modern Error Reporting** - Colorized error messages with source context and helpful suggestions
- 🚀 **Fast Compilation** - Efficient lexer, parser, and AST generation
- 📝 **Clean Syntax** - Intuitive language design with clear variable declarations
- 🔧 **Developer Friendly** - Detailed error messages that help you fix issues quickly

## Language Syntax

### Variable Declarations
```aw
// String variables
new name string = "John Doe"

// Integer variables  
new age int = 25

// Float variables
new height float = 5.9

// Boolean variables (using 'bl' keyword)
bl is_active = true
bl is_complete = false
```

### Output Statements
```aw
// Simple output
stdout [Hello World!]

// String interpolation with variables
stdout [Hello {name}, you are {age} years old!]
```

### Arrays
```aw
// Inferred type arrays
new numbers[] = [1, 2, 3, 4, 5]
new names[] = ["Alice", "Bob", "Charlie"]

// Explicit type with size
new scores{float}[10]
new buffer{int}[100]
```

### Comments
```aw
// Single line comment using //

;; Single line comment using ;;

; 
Multi-line comment
using semicolons
;
```

## Building

You must have the `nob.h` header file for building this project. If you don't have it, you can get it [here](https://github.com/tsoding/nob.h) or using wget:

```bash
wget https://raw.githubusercontent.com/tsoding/nob.h/refs/heads/main/nob.h
```

Clone the repo and compile `build.c` using any C compiler:
```bash
git clone https://github.com/architmishra-15/awlang.git
cd awlang
```

```bash
gcc build.c -o build
./build
```

It'll automatically start building everything and create `build/main.exe`.

## Usage

Compile an AwLang source file:
```bash
./build/main.exe your_file.aw
```

### Successful Compilation
```bash
$ ./build/main.exe test.aw
Compiling test.aw...
Tokenizing...
Parsing...
Generating AST...
✓ Compilation successful!
  → Lexer IR written to output.lexerIR
  → AST IR written to output.astIR
```

### Error Reporting
When there are syntax errors, you'll get detailed, colorized error messages:

```bash
$ ./build/main.aw error_example.aw
Compiling error_example.aw...
Tokenizing...
Parsing...
Generating AST...

error: Unterminated string literal
  --> error_example.aw:1:16
   |
 1 | new x string = "unterminated string
   |                ^
 2 | new y int = 42
   |
help: Add closing quote '"' to end the string

error: could not compile `error_example.aw` due to 1 previous error
```

## Error System Features

- **Precise Location**: Shows exact line and column where errors occur
- **Source Context**: Displays the problematic code with surrounding lines
- **Visual Indicators**: Uses `^` to point to the exact error location
- **Helpful Suggestions**: Provides actionable advice for fixing errors
- **Multiple Errors**: Reports all errors at once, not just the first one
- **Color Coding**: Uses colors to distinguish error types and information
- **Professional Format**: Similar to modern compilers like Rust and Clang

## Example Programs

### Hello World
```aw
stdout [Hello, World!]
```

### Variables and Interpolation
```aw
new name string = "Alice"
new age int = 30
bl is_programmer = true

stdout [Hi {name}!]
stdout [You are {age} years old]
stdout [Programmer status: {is_programmer}]
```

### Arrays
```aw
new fruits[] = ["apple", "banana", "orange"]
new numbers[] = [1, 2, 3, 4, 5]
new matrix{int}[100]

stdout [First fruit: {fruits}]
```

## Output Files

When compilation succeeds, two IR (Intermediate Representation) files are generated:

- **`output.lexerIR`** - Token stream from lexical analysis
- **`output.astIR`** - Abstract Syntax Tree representation

These files help with debugging and understanding the compilation process.

## License

The language is licensed under `GPL-3.0`. See [LICENSE](./LICENSE) for more info.
