#include <stdio.h>
#define MSG(msg) #msg
#define DOUBLE_UNDERSCORE(content) \
__##content##__
#define DEBUG
int main() {
#ifdef DEBUG
    puts(DOUBLE_UNDERSCORE(DATE));
    puts(DOUBLE_UNDERSCORE(TIME));
#endif
    puts(MSG(Hello World!));
}

