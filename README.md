# CS3650-Shell
Basic command line shell, `nush` (**N**ortheastern **U**nivsersity **SH**ell), based on a CS3650 Computer Systems assignment.

# Description
This is a simple shell utility that allows users to run programs using UNIX-like syntax. The shell supports some of the most common functionality needed to accomplish tasks such as file redirections, pipes, quotations, simple variables, background commands, and subshells. This is accomplished by generating an tree of commands with a topology that will be evaluated to created to intended effects. This is not intended to be a fully featured POSIX compliant utility.

# Compile the shell
To compile the shell use the included `Makefile`

```
$ make
```

This should produce a `nush` executable

# Using the shell
Similarly to most shells, `nush` can be run in interactive mode or script mode.

## Interactive mode
```
$ ./nush
```

## Script mode
```
$ ./nush [script filename]
```

# Running tests
Tests were provided by the professor to excercise functionality. They can be viewed inside the `test.pl` scripts and within the `tests/` folder.

```
$ make test
```
