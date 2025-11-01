#include <stdio.h>
#include <stdlib.h>

char* generate_and_write(const char* filePath, unsigned int seed) {
    srand(seed);
    float f1 = (float)rand() / (float)RAND_MAX;
    float f2 = (float)rand() / (float)RAND_MAX;
    float f3 = (float)rand() / (float)RAND_MAX;

    char* buf = (char*)malloc(128);
    if (!buf) return NULL;

    snprintf(buf, 128, "%f,%f,%f", f1, f2, f3);

    FILE* fp = fopen(filePath, "w");
    if (fp) {
        fputs(buf, fp);
        fclose(fp);
    }
    return buf;
}

int main(void) {
    char* s1 = generate_and_write("out_c_1.txt", 1);
    char* s2 = generate_and_write("out_c_2.txt", 2);
    char* s3 = generate_and_write("out_c_3.txt", 3);
    char* s4 = generate_and_write("out_c_4.txt", 4);
    char* s5 = generate_and_write("out_c_5.txt", 5);

    if (s1) { printf("%s\n", s1); free(s1); }
    if (s2) { printf("%s\n", s2); free(s2); }
    if (s3) { printf("%s\n", s3); free(s3); }
    if (s4) { printf("%s\n", s4); free(s4); }
    if (s5) { printf("%s\n", s5); free(s5); }

    return 0;
}