# What's this repository?
This is an implementation of custom programming language.
It's implemented with bison and flex but I plan in the future to do a fork that uses prolog instead of bison just to experiment a bit more on programming languages.

# How to use it?
## Installation
Clone the repository:
```bash
git clone https://github.com/le0nAg/yapl.git
```
Once that you are inside run:
```bash
make
```
This will generate an interpreter that you can use to run programs.

## Usage
You can create your own source file or use one of the examples in the `/prog` folder after that run:
```
./interpreter <path/to/src.prog>
```

# How the programming language (yapl) works?

yapl follows a simple compilation pipeline:
1. **Lexer** (Flex) - Tokenizes source code
2. **Parser** (Bison) - Builds Abstract Syntax Tree
3. **Interpreter** - Walks the AST and executes

The language uses a symbol table for variables and functions with lexical scoping.

# Syntax

## c-style approach 

yapl uses mostly C-style syntax:

```c
// Comments
int x = 10;
float y = 3.14;
str message = "Hello";
bool flag = true;

if (x > 5) {
    print("yes");
} else {
    print("no");
}

while (x < 20) {
    x++;
}
```

### Data Types
`int`, `float`, `str`, `bool`, `void`, `matrix`

### Operators
- Arithmetic: `+` `-` `*` `/` `%`
- Comparison: `<` `>` `<=` `>=` `==` `!=`
- Logical: `&&` `||` `!`
- Assignment: `=` `+=` `-=` `*=` `/=`
- Increment/Decrement: `++` `--`
- Matrix multiplication: `@`
- Pattern matching: `~=`

## functions

```c
fn name(type param1, type param2) return_type {
    return value;
}
```

Example:
```c
fn add(int a, int b) int {
    return a + b;
}

fn main() void {
    int result = add(5, 3);
    print(result);
}
```

Built-in functions:
- `print(...)` - Print values
- `printm(matrix)` - Print matrix formatted
- `read()` - Read input (auto-detects type)

# Nice features of yapl

## ranges

### what is it and why?

Ranges provide concise iteration syntax:

```c
for (i : 0..10) {    // inclusive: 0,1,2,3,4,5,6,7,8,9,10
    print(i);
}
```

### how is it implemented?

The parser recognizes `..` as a range operator and creates a `NODE_RANGE` AST node. The interpreter evaluates start and end expressions, then iterates accordingly.

```c
// AST structure
struct range {
    ASTNode *start;
    ASTNode *end;
    ASTNode *step;
}
```

## regex

### what is it and why?

Pattern matching for strings using the `~=` operator:

```c
str email = "user@example.com";
if (email ~= ".*@.*\\.com") {
    print("Valid email");
}
```

### how is it implemented?

Uses POSIX regex library (`<regex.h>`):

```c
case NODE_PATTERN_MATCH:
    regcomp(&regex, pattern, REG_EXTENDED);
    match = regexec(&regex, string, 0, NULL, 0);
    regfree(&regex);
    return create_bool_value(match == 0);
```

## matrixes

### what is it and why?

Native matrix support for linear algebra:

```c
matrix A = [[1, 2, 3],
            [4, 5, 6]];

matrix B = [[7, 8],
            [9, 10],
            [11, 12]];

matrix C = A @ B;  // Matrix multiplication
printm(C);
```

### how is it implemented?

Matrices are stored as:
```c
typedef struct {
    int rows;
    int cols;
    double **data;
} Matrix;
```

Array literals `[[1,2],[3,4]]` are converted to matrices. The `@` operator performs standard matrix multiplication **without** dimension checking.

# Examples

See `/prog` folder for examples.
