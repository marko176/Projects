#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    char s1[101];
    char s2[101];
    printf("Upisite s1 > ");
    scanf(" %[^\n]", s1);
    printf("Upisite s2 > ");
    scanf(" %[^\n]", s2);
    int i = 0;
    int j = 0;
    while (s1[i] != '\0' && s2[j] != '\0') {
        while (s1[i] == ' ')
            i++;
        while (s2[j] == ' ')
            j++;
        int l1 = s1[i] - 'A';
        int l2 = s2[j] - 'A';
        if (s1[i] >= 'a' && s1[i] <= 'z')
            l1 = s1[i] - 'a';
        if (s2[j] >= 'a' && s2[j] <= 'z')
            l2 = s2[j] - 'a';
        if (l1 < l2) {
            printf("(%s)<(%s)\nRazlika na indeksima: (s1:%d, s2:%d)", s1, s2, i, j);
        } else if (l1 > l2) {
            printf("(%s)>(%s)\nRazlika na indeksima: (s1:%d, s2:%d)", s1, s2, i, j);
        }
        i++;
        j++;
    }
    while (s1[i] == ' ')
        i++;
    while (s2[j] == ' ')
        j++;
    if (s1[i] == '\0' && s2[j] == '\0') {
        printf("(%s)=(%s)", s1, s2);
    } else if (s1[i] == '\n') {
        printf("(%s)<(%s)\nRazlika na indeksima: (s1:%d, s2:%d)", s1, s2, i, j);
    } else {
        printf("(%s)>(%s)\nRazlika na indeksima: (s1:%d, s2:%d)", s1, s2, i, j);
    }
    return 0;
}