# Memory Dump
Utility to dump memory in running processes memory.

# Usage

```
g++ memory-dump.c -o mem-dump.exe
```

Dump memory by process id

```
./mem-dump.exe -p 123
```

Dump memory by process name

```
./mem-dump.exe -n chrome.exe
```

