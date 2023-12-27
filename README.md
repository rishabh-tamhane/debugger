# Debugger

## Introduction

The goal of this project is to become familiar with low-level Unix/POSIX system
calls related to processes, signal handling, files, and I/O redirection.
I have implemented a simplified debugger program called `deet`,
which is capable of managing and performing some basic debugging operations on
a collection of target processes.

### Takeaways

* Understand process execution: forking, executing, and reaping.
* Understand signal handling and asynchronous I/O.
* Understand the use of "dup" to perform I/O redirection.
* Have gained experience with C libraries and system calls.
* Have enhanced your C programming abilities.

## Getting Started

Here is the structure of the base code:
<pre>
.
├── .gitignore
├── .gitlab-ci.yml
└── hw4
    ├── demo
    │   └── deet
    ├── hw4.sublime-project
    ├── include
    │   ├── debug.h
    │   └── deet.h
    ├── lib
    │   └── logger.o
    ├── Makefile
    ├── src
    │   └── main.c
    ├── test_output
    │   └── .git-keep
    ├── testprog
    │   ├── tp
    │   └── tp.c
    └── tests
        ├── basecode_tests.c
        ├── rsrc
        │   ├── hello_world.err
        │   ├── hello_world.in
        │   ├── hello_world.out
        │   ├── sleep_no_wait.err
        │   ├── sleep_no_wait.in
        │   ├── sleep_no_wait.out
        │   ├── sleep_wait.err
        │   ├── sleep_wait.in
        │   ├── sleep_wait.out
        │   └── startup_quit.in
        ├── test_common.c
        └── test_common.h
</pre>
The `include` and `src` directories contain header and source files, as usual.
The `lib` directory contains headers and binaries for some library code that
has been provided for you.
The `demo` directory contains an executable demonstration version of the program.
The `testprog` directory contains code for a very simple program that you can
use as a target for debugging.
The `tests` directory contains some very basic tests.
All of these are discussed in more detail below.

If you run `make`, the code should compile correctly, resulting in two
executables `bin/deet` and `bin/deet_tests`.
The executable `bin/deet` is the main one.

### The `killall` Command

In the course of debugging this program, you will almost certainly end up in
situations where there are a number of "leftover" target processes that have survived
beyond a particular test run of the program.  If you allow these processes to
accumulate, it can cause confusion, as well as consume resources on your computer.
The `ps(1)` (process status) command can be used to determine if there are any
such processes around; e.g.

```
$ ps alx | grep a.out
```

will find all processes currently running the program `a.out`.
If there are a lot of them, it can be tedious to kill them all using the `kill` command.
The `killall` command can be used to kill all processes running a program with a
particular name; e.g.

```
$ killall a.out
```

It might be necessary to use the additional option `-s KILL` to send these processes
a `SIGKILL`, which they cannot catch or ignore, as opposed to the `SIGTERM`, which is
sent by default.

### The `strace` Command

An extremely useful (but rather advanced) feature that Linux provides for debugging
is the `strace(1)` command.  When a program is run via `strace`, you can get an
extremely detailed trace of all of the operating system calls made by the main process
of this program, as well as child processes.  This can be useful when all else fails
in trying to understand what a program is doing, however the down side is that
to understand the voluminous output produced by `strace` requires a fair amount of
technical knowledge about Linux system calls.  You might want to give it a try, though.

## `Deet`

### Description of Behavior

The `deet` program is a highly simplified version of a debugger, in the spirit of `gdb`.
It provides the user with a command-oriented interface that permits "target"
processes to be started, stopped, examined, modified, and killed.
All user input is read from the standard input and output for the user is
printed to the standard output.  Output to the standard output should be
exactly in the format described below and nothing else should be printed
to standard output.

> :scream  The `deet` program recognizes the command-line flag `-p`.
> If this flag is given, the normal interactive prompt `deet> ` printed by `deet` should
> be suppressed, otherwise it should be printed as discussed below.

`Deet` is able to manage multiple target processes at once.
It maintains information about all the targets that are currently being debugged
and their current state.
Commands given by the user name the target process that is to be manipulated
by specifying an integer "`deet` ID".  When a target process is started, it receives
the smallest unused `deet` ID (starting from 0).  Later, when the target process
has terminated, `deet` will forget about it and make its `deet` ID available
for re-use.  The rules for when `deet` will forget about a target process are
given later in this document.

Here is an example of a `deet` session.
First, `deet` is started from the Linux shell and it prompts the user:

```
$ bin/deet
deet> 
```

The prompt consists of the string `deet> `, which contains a single space at
the end, and no newline.  The user enters a blank line, in which case
the prompt is simply given again, without any other output.
Next, the user asks for help:

```
deet> help
Available commands:
help -- Print this help message
quit (<=0 args) -- Quit the program
show (<=1 args) -- Show process info
run (>=1 args) -- Start a process
stop (1 args) -- Stop a running process
cont (1 args) -- Continue a stopped process
release (1 args) -- Stop tracing a process, allowing it to continue normally
wait (1-2 args) -- Wait for a process to enter a specified state
kill (1 args) -- Forcibly terminate a process
peek (2-3 args) -- Read from the address space of a traced process
poke (3 args) -- Write to the address space of a traced process
bt (1-2 args) -- Show a stack trace for a traced process
```

The commands understood by `deet` are listed, together with information about
the number of arguments each takes.  Each command has a minimum number of
arguments that it requires and a maximum number of arguments that it permits.
For some commands, the minimum and maximum are the same; for those commands,
exactly that number of arguments must be given.
For some other commands (such as `run`), no maximum is specified;
in that case, an arbitrary number of arguments greater than or equal to the
minimum may be given.

Issuing a command that is not in the list causes `deet` to report an error:

```
deet> bogus
?
```

This is how `deet` reports an error: by a single question mark `?` followed
by a newline output to the standard output.  This applies to all error situations.
For debugging/explanatory purposes, `deet` is permitted to issue an error message,
but this **must** go to the standard error output.

The user then issues the `run` command to start a process to be debugged:

```
deet> run echo a b c
0	137480	T	running		echo a b c
```

Here the program `echo` has been run, with three command-line arguments:
`a`, `b`, and `c`.
A process with Linux process ID `137480` has been created, and `deet` ID 0
has been assigned to it.
A status line has been printed by `deet`, giving the essential information
about this process.
The status line contains:
the `deet` ID,
the Linux process ID,
the character `T` indicating that the process is being traced by `deet`
(a character `U` would indicate that the process is "untraced"),
the current state of the process from `deet`'s point of view
(here it is `running`),
the exit status that was returned when the process terminated
(here there is none, so this field is empty),
and finally the command that was executed to start the process.
The fields in the output are separated by single TAB (*i.e.* `'\t'`) characters,
except within the command, which is printed as the last field on the line
and shows the words from the command line separated from
each other by a single space character.  The status line is terminated by a
single newline (`'\n'`) character.

The process that has been started then immediately stops before actually
doing anything.  This is to allow `deet` to take control of it:

```
deet> 0	137480	T	stopped		echo a b c
```

A status line similar to the first has been printed, with the difference that
the state of the process is now reported as `stopped`, rather than `running`.
Notice that the status line has appeared after the `deet> ` prompt.
This is because the target process started executing concurrently with `deet`
and then `deet` received an asynchronous notification (via a `SIGCHLD` signal)
that the target process had stopped.  By this time, `deet` had already printed
the prompt and was waiting for input from the user, but the asynchronous
notification interrupted the reading of user input and caused `deet` to print
the new status line.  After that, it will go back and print another prompt
and continue to wait for user input.

Next, the user started another process to be debugged; this time running the
`sleep` command with an argument of `10` (`sleep` is a command that simply
waits for a specified number of seconds before terminating):

```
deet> run sleep 10
1	137935	T	running		sleep 10
deet> 1	137935	T	stopped		sleep 10
```

Notice that the process has stopped immediately as before.  It has been assigned
`deet` ID 1 and is running a process with Linux process ID `137935`.

The user now asks for information about all the processes currently being debugged.
The current status of both processes is shown:

```
deet> show
0	137480	T	stopped		echo a b c
1	137935	T	stopped		sleep 10
```

Next, the user starts a process running a test program `testprog/tp`, which has been
provided for you:

```
deet> run testprog/tp
2	138055	T	running		testprog/tp
deet> 2	138055	T	stopped		testprog/tp
```

The program stops, as before.  Next, the user tells the test program
(which has `deet` ID 2) to continue:

```
deet> cont 2
2	138055	T	running		testprog/tp
deet> function a @ 0x5607f22551cd, argument x @ 0x7ffeac24a4ec (=666)
function b @ 0x5607f225521b: argument x @ 0x7ffeac24a4cc (=667)
function c @ 0x5607f2255269: argument x @ 0x7ffeac24a4ac (=668)
function d @ 0x5607f22552b7: argument x @ 0x7ffeac24a48c (=669)
function e @ 0x5607f2255305: argument x @ 0x7ffeac24a46c (=670)
function f @ 0x5607f225535b called
static_variable @ 0x5607f2258030 (=0)
local_variable @ 0x7ffeac24a440 (=29a)
2	138055	T	stopped		testprog/tp
```

`Deet` is informed that the test program is now running.
The test program prints out some information about itself which will aid
in testing the debugging features of `deet`.
It prints the addresses (in its text segment) of six functions,
called `a`, `b`, `c`, `d`, `e`, `f`.
Each function prints the address (in the stack segment) of its argument `x`,
together with the current value of that argument.
The test program also prints the address and value of a variable
`static_variable`, which lives in the data segment, and of a local variable
`local_variable`, which lives in the stack segment, in the stack frame
for function `f`.
The test program then uses the `kill()` system call to send itself a
`SIGSTOP` signal.  As a result, it stops execution and `deet` receives an
asynchronous notification that the test process has stopped.
In response, `deet` prints a status line showing the changed status of
this process.


```
deet> cont 2
2	138055	T	running		testprog/tp
deet> function f @ 0x5607f225535b called
static_variable @ 0x5607f2258030 (=0)
local_variable @ 0x7ffeac24a440 (=29a)
2	138055	T	stopped		testprog/tp
```

A status line is printed by `deet` when it is notified that the test process has
continued execution.
In the test program, function `f` returns to function `e` and is then called again,
resulting in the same printout as before being repeated, and then the test process
uses `SIGSTOP` to stop itself once again.
`Deet` is notified that the test process has stopped and it prints an updated
status line.

Next, the user uses the `peek` command to examine the contents of the argument `x`
stored in the stack frame for function `c`:

```
deet> peek 2 7ffeac24a4ac
00007ffeac24a4ac	ac24a4d00000029c
```

The two least-significant bytes of the word printed contains the value `029c`,
which is hexadecimal for `668`, the current value of the argument `x` to function `c`.

The user then uses the `poke` command to modify the value of the global variable
`static_variable` stored in the data segment:

```
deet> poke 2 5607f2258030 1023
```

The user then allows the test process to continue:

```
deet> cont 2
2	138055	T	running		testprog/tp
deet> function f @ 0x5607f225535b called
static_variable @ 0x5607f2258030 (=1023)
local_variable @ 0x7ffeac24a440 (=29a)
2	138055	T	stopped		testprog/tp
```

Once again, the test process returns from `f` to `e`, the function `f` is called
again, and the current values of the variables are printed.
Now the value of `static_variable` is seen to be `1023`.

At this point, the user has decided that enough has been done with the test process,
and so uses the `kill` command to cause it to terminate:

```
deet> kill 2
2	138055	T	killed		testprog/tp
deet> 2	138055	T	dead	0x9	testprog/tp
```

Execution of the `kill` command causes `SIGKILL` to be sent to the test process,
and at the same time `deet` reports that the state of this process is now `killed`,
though the process has not yet terminated.
Shortly thereafter, the test process receives the `SIGKILL` and actually does
terminate.  `Deet` is then notified of the termination and it reports that
the process is now `dead`.  The exit status (obtained via the `waitpid()` system
call is also reported by `deet`).  In this case, the program terminated as a
result of `SIGKILL`, which is signal number 9, so the exit status is `0x9`
(had the program terminated normally, the low-order byte would be 0 and the
exit status passed to `exit()` would appear in the next-most-significant byte).

The user then decides to allow the process running `sleep` to continue:

```
deet> cont 1
1	137935	T	running		sleep 10
deet> 1	137935	T	dead	0x0	sleep 10
```

Initially, `deet` reports that the process is running and then (after a 10-second
delay) it reports that the process has terminated with exit status `0x0` returned
by `waitpid()`.

Finally, the user decides to quit `deet`:

```
deet> quit
$ 
```

Although there was no output to document it, before it terminated, `deet` arranged
to kill the remaining process (running `echo`), and await its termination, before
exiting itself.

### Details of the Commands

The previous section illustrated most aspects of `deet`'s functionality;
the current section fills in some more specific details about the commands.
The behavior of your implementation should match these descriptions.

  - The `help` command (zero or more arguments) ignores any arguments it is given
    and prints a message listing information about the available commands.

  - The `quit` command (no arguments required or allowed) causes `deet` to terminate,
    after first killing any processes being debugged to be killed and then waiting for
    them to actually terminate before it itself exits.

  - The `show` command may be given with no arguments, in which case it outputs one
    line of information about the status of each of the processes currently being
    managed by `deet`.  It also may be given a single optional integer argument,
    in which case it gives only information about the process having that integer
    as its "`deet` ID".  If an invocation of the `show` command causes no output
    to be printed (either because there are no processes currently being managed
	or because a process with the specified `deet` ID does not exist), then an
	error is reported instead.

  - The `run` command serves to start a process to be debugged.  At least one
    argument is required, which is interpreted as the name of the program to be
    run.  There may be additional arguments, which taken together with the first
    argument form the `argv[]` vector for the program to be executed.
	A process started with `run` begins execution with tracing enabled
    (more on this below).  Tracing remains enabled until the process terminates
    or a `release` command is executed on that process.

The following is an example of the type of tracing output that the basecode version
of these functions will produce (all of the tracing output goes to the standard
error output):

```
$ bin/deet
[00000.000000] STARTUP
[00000.000069] PROMPT
deet> 
[00004.224843] INPUT 
[00004.224861] PROMPT
deet> bogus
[00008.326146] INPUT bogus
[00008.326161] ERROR bogus
?
[00008.326168] PROMPT
deet> run echo a b c
[00017.982799] INPUT run echo a b c
[00017.983064] CHANGE 142211: none -> running
0	142211	T	running		echo a b c
[00017.983147] PROMPT
deet> [00017.983868] SIGNAL 17
[00017.983895] CHANGE 142211: running -> stopped
0	142211	T	stopped		echo a b c
[00017.983931] PROMPT
deet> run sleep 10
[00036.619853] INPUT run sleep 10
[00036.620021] CHANGE 142212: none -> running
1	142212	T	running		sleep 10
[00036.620088] PROMPT
deet> [00036.620490] SIGNAL 17
[00036.620506] CHANGE 142212: running -> stopped
1	142212	T	stopped		sleep 10
[00036.620527] PROMPT
deet> cont 1
[00042.853035] INPUT cont 1
[00042.853058] CHANGE 142212: stopped -> running
1	142212	T	running		sleep 10
[00042.853105] PROMPT
deet> [00052.854331] SIGNAL 17
[00052.854372] CHANGE 142212: running -> dead
1	142212	T	dead	0x0	sleep 10
[00052.854407] PROMPT
deet> show
[00059.644313] INPUT show
0	142211	T	stopped		echo a b c
1	142212	T	dead	0x0	sleep 10
[00059.644383] PROMPT
deet> quit
[00064.028144] INPUT quit
[00064.028169] CHANGE 142211: stopped -> killed
[00064.028309] SIGNAL 17
[00064.028335] CHANGE 142211: killed -> dead
[00064.028344] SHUTDOWN
$ 
```
**Note : The debugger was built as a part of the course CSE 320 - System Fundamental II
