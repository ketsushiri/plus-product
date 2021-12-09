# plus-product
It's a calculator! Just my experiments with parsing. Four basic arithmetic operations - `(+, -, *, /)` and brackets. That's all you need. Nothing else implemented, even not floating point arithmetic. You don't need one, in fact. Brackets can be any depth. This is it.

## Build
Can be build with extra debug information logging or not. If you do not want to see the schizophazy in a console before an actual result, just comment 9-th string in main.cpp.
```bash
$ git clone https://github.com/ketsushiri/plus-product.git
$ cd ./plus-product
$ g++ main.cpp -o calc
```
Now you've got a binary with name 'calc'. Enjoy.

```bash
$ ./calc
 < 2 * (4 + 4)
 > 16
```
