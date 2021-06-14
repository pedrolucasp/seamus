# seamus

A simple and minimal music player. Uses mpd as backend.

The name is from Pink Floyd's Seamus, it's almost pronounceable as cmus, which
was the player I attempted to use when I got tired from using ncmpcpp, which
besides being huge and having a bunch of things you'll never use it, it also had
a terrible name.

## Dependencies

- A POSIX-like system and a C11 compiler
- MPD (duh)
- libmpdclient (and it's development headers)
- [libtickit]

## Installing

Install the dependencies. Note that for some systems, like Alpine Linux, the
libtickit library is still under the `testing` repository, so you'll need to
enable that first.

```sh
$ mkdir build && cd build
$ ../configure
$ make
# make install
```

## Hacking

Load some good playlist first, you'll need it. All debug info is logged to
`stderr`, so you can do something like: `./seamus 2> log.txt` to check on info,
and use the `debug(char *)` function to help you.

Send patches to my [email] or create a pull request on [GitHub].

## License

GNU GPL-3.0. Check [COPYING][copying].

[copying]: /COPYING.md
[email]: mailto:porcellis@eletrotupi.com
[GitHub]: https://github.com/pedrolucasp/seamus
[libtickit]: http://www.leonerd.org.uk/code/libtickit
