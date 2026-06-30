# File system REPL

Use `make -j` to compile, then to run it use `bin/ezfs`. once inside
use `help` command to get help.

Steps on how to use REPL:

1. `format` a disk partition with a specific block size. Example: 100000000 which
is about ~95MB or so
2. `mount <name>` the disk partition
3. Now the REPL is working

## Note

To exit the REPL just input `exit` and it'll unmount automatically
