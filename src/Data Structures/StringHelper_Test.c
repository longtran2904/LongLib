#include <stdio.h>
#include <assert.h>
#include "StringHelper.c"

int main(void)
{
    char* a = NULL;
    ArrayPrintf(&a, "Some number: %d\n", 5);
    ArrayPrintf(&a, "More number: %d\n", 2);
    ArrayPrintf(&a, "Some text: %s\n", "Hello world!");
    printf("%s\n", a);
    
    char* b = Printf("An Int: %d", 3);
    printf("%s\n", b);
    free(b);
    b = Printf("Some Floats: %f, %f, %f\n", 1.2, 3.27, 4.1692);
    printf(b);
    free(b);
    
    char* text = "Hello guys!\n";
    int num = 5;
    char* c = StrDup(text, num);
    printf("%sLength: %zd\n", c, strlen(c));
    assert(strlen(c) == strlen(text) * num);
    
    printf(StrDup("Number: %d\n", num), 1, 2, 3, 4, 5);
    printf("\n");
    
    StringsArray array = {.block = malloc(1024)};
    Push(&array, "Hello");
    Push(&array, "What's up!");
    Push(&array, "Excuse Me!");
    printf("%s\n", array.strings[0]);
    printf("%s\n", array.strings[1]);
    printf("%s\n", array.strings[2]);
    
    Clear(&array);
    assert(array.blockUsed == 0);
    assert(array.block == NULL);
    assert(array.strings == NULL);
    
    return 0;
}