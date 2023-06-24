# fx
writing a language in Rust, then extending it to C for LLVM

# parser notes
- step one: find ranges `[x,y]` and take their tokens out of the tree so they don't interfere with op prec
- step two: take apart argument lists and remove the tokens so that the arguments are parsed differently from other uses of commas
- step three: split into expressions. parentheses/curly brackets split into expressions/lists of expressions respectively, then finally if/else (commas)
- example: see `test.txt`
  - take out range
    
    `let fib(n: [0,]) =` => `let fib(n) = `
  - take apart declaration

    `fib(n)` => function with one argument, n, between `0` and `infinity`, defined in the range
  - split into expressions
   
    `0 when n == 1,` => is essentially `if n == 1 then return 0` since fibonacci is a piecewise relation
    
    `1 when n <= 2,` => is parsed like `if n <= 2 then return 1`

    `fib(n - 1) + fib(n - 2)` => is split into two expressions: one function call to `fib(n-1)` and one to `fib(n-2)`. the function calls are taken out of the tree and then re inserted (after op prec is run on their arguments) so that the op prec is able to run on the addition of the two recursive calls. this is the third branch of the piecewise fibonacci function.