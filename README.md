# Yet another hierarchical C compiler

### Dependence

This repository depends on

* git@github.com:kaitou-ryaku/min-bnf-parser.git
* git@github.com:kaitou-ryaku/min-regex.git

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

Following files are to be sequentially created.

* `syntax.dot`
* `token_list.txt`
* `parse_tree.dot`
* `table.txt`
* `abstruct_tree.dot`

### Syntax-Graph and Tree

```sh
$ dot -Gdpi=300 -T png parse_tree.dot -o parse_tree.png
$ dot -Gdpi=300 -T png abstruct_tree.dot -o parse_tree.png
$ dot -Gdpi=100 -T png syntax.dot -o syntax.png
```

### License

MIT

