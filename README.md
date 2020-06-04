### ![](https://www.mobafire.com/images/ability/ornn-call-of-the-forge-god.png) Call of the forge god

A simple one-pass implementation of C++ preprocessor, as the bonus part of [Ornn](https://github.com/fstqwq/Ornn).
Designed to be (hopefully) cross platform. Mainly tested on Windows using GCC 8.1 .

Supported features:
* define, undef
    * variadic defines also supported
* if, ifdef, ifndef, else, endif
* pragma once
* error, warning
* line control
* [predefined marcos](https://gcc.gnu.org/onlinedocs/cpp/Predefined-Macros.html#Predefined-Macros): \_\_FILE\_\_, \_\_DATE\_\_, WIN32, \_\_unix\_\_, \_\_cplusplus, and, or ...

Implementation standard mainly according to [gcc](https://gcc.gnu.org/onlinedocs/cpp/), with following (but not limited to) exceptions:
* digraphs and trigraphs are not supported
* object-like marco and function-like marco will not [take effect in the same time](https://gcc.gnu.org/onlinedocs/cpp/Directives-Within-Macro-Arguments.html#Directives-Within-Macro-Arguments).
* ...

Generally speaking, these features are either undefined, or out-of-date features.