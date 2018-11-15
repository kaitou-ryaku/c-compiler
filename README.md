# c-compiler

### Clone

```sh
$ git clone git@github.com:kaitou-ryaku/c-compiler.git
$ cd c-compiler
$ git clone git@github.com:kaitou-ryaku/min-bnf-parser.git
$ cd min-bnf-parser
$ git clone git@github.com:kaitou-ryaku/min-regex.git
```

### Build

```sh
$ pwd
/path/to/c-compiler

$ make
```

### Compile

```sh
$ ./compiler.out <SOURCE_FILE>
```

### Syntax and Abstruct-Tree

```sh
$ dot -Gdpi=300 -T png parse_tree.dot -o parse_tree.png
$ dot -Gdpi=100 -T png syntax.dot -o syntax.png
```

### License

MIT

