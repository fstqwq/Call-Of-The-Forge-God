### ![](https://www.mobafire.com/images/ability/ornn-call-of-the-forge-god.png) Call of the forge god

A simple one-pass implementation of C++ preprocessor, as the bonus part of [Ornn](https://github.com/fstqwq/Ornn).

Supported features: Almost all features you need to compile a HelloWorld.

Currently
* successfully compiled C-style hello world using WSL and MinGW 64
* successfully self-compiled using WSL.

Preprocessor read configs from "__predefined.cpp" and "include_dir.cfg", see test\_wsl/ and test\_win/ for detail.

Designed to be (hopefully) cross platform. Mainly tested on Windows and WSL 1.

Implementation standard mainly according to [gcc](https://gcc.gnu.org/onlinedocs/cpp/), with following (but not limited to) exceptions:
* digraphs and trigraphs are not supported
* object-like marco and function-like marco will not [take effect in the same time](https://gcc.gnu.org/onlinedocs/cpp/Directives-Within-Macro-Arguments.html#Directives-Within-Macro-Arguments).
* ...

Generally speaking, these are either undefined or out-of-date.