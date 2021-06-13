# seamus

A simple and minimal music player. Uses mpd as backend.

## Dependencies

- A POSIX-like system and a C11 compiler
- MPD (duh)
- libmpdclient (and it's development headers)
- [libtickit]

## Hacking

Install the dependencies. Note that for some systems, like Alpine Linux, the
libtickit library is still under the `testing` repository, so you'll need to
enable that first.

```sh
$ mkdir build && cd build
$ ../configure
$ make
# make install
```
Send patches to my [email] or on [GitHub].

## License

GNU GPL-3.0. Check [COPYING][copying].

[copying]: /COPYING.md
[email]: mailto:porcellis@eletrotupi.com
[GitHub]: https://github.com/pedrolucasp/seamus
[libtickit]: http://www.leonerd.org.uk/code/libtickit
