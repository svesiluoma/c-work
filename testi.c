#include <stdio.h>
int main(void) {
    const char *friends[] = { "john", "pat", "gary", "michael" };
    for (int i = 0; i < 4; i++) {
        printf("iteration %d is %s\n", i, friends[i]);
    }
}
