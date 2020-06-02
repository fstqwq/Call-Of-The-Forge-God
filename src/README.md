# ![](https://www.mobafire.com/images/ability/ornn-call-of-the-forge-god.png) Call of the forge god

A simple one-pass implementation of C-like preprocessor, as the bonus part of [Ornn](https://github.com/fstqwq/Ornn).
Designed to be (hopefully) cross platform.

(ALL TODO)
Supported features:
* define, undef
* ifdef, else, endif
* pragma once
* error, warning
* line control
* predefined marcos: \_\_FILE\_\_, \_\_DATE\_\_, \_\_TIME\_\_, \_\_TIMESTAMP\_\_, WIN32, \_\_unix\_\_

Implementation standard mainly according to [gcc](https://gcc.gnu.org/onlinedocs/cpp/), with following (but not limited to) exceptions:
* digraphs and trigraphs are not supported
* object-like marco and function-like marco will not [take effect in the same time](https://gcc.gnu.org/onlinedocs/cpp/Directives-Within-Macro-Arguments.html#Directives-Within-Macro-Arguments).
* ...

Generally speaking, these features are either undefined, or not interesting to implement.