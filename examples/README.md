To generate a binary that serves the example site in the "static" directory, do something like:

```
can static > site.c
cc -o site site.c -levent
```

If you'd prefer libevent to be statically linked:

```
can static > site.c
cc -o site site.c -Wl,-Bstatic -levent -Wl,-Bdynamic
```
