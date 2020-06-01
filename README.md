### Call of the forge god

A simple one-pass implementation of C-like preprocessor, as the bonus part of [Ornn](https://github.com/fstqwq/Ornn).
Designed to be (hopefully) cross platform.

Supported features:
* define, undef
* ifdef, else, endif
* pragma once
* error
* line control
* \_\_FILE\_\_, \_\_DATE\_\_, \_\_TIME\_\_, \_\_TIMESTAMP\_\_, WIN32, \_\_unix\_\_

Implementation standard mainly according to [gcc](https://gcc.gnu.org/onlinedocs/cpp/), with following (but not limited to) exceptions:
* '$' is not supported as a part of identifier
* digraphs and trigraphs are not supported
* ...

Generally speaking, these features are rarely used and not interesting to implement.