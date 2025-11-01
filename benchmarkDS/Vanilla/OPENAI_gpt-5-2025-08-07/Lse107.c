#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

static void perm_string_from_mode(mode_t mode, char out[10]) {
    out[0] = (mode & S_IRUSR) ? 'r' : '-';
    out[1] = (mode & S_IWUSR) ? 'w' : '-';
    out[2] = (mode & S_IXUSR) ? 'x' : '-';
    out[3] = (mode & S_IRGRP) ? 'r' : '-';
    out[4] = (mode & S_IWGRP) ? 'w' : '-';
    out[5] = (mode & S_IXGRP) ? 'x' : '-';
    out[6] = (mode & S_IROTH) ? 'r' : '-';
    out[7] = (mode & S_IWOTH) ? 'w' : '-';
    out[8] = (mode & S_IXOTH) ? 'x' : '-';
    out[9] = '\0';
}

char* secure_file(const char* filename, const char* content, int mode) {
    FILE* f = fopen(filename, "w");
    if (!f) {
        char* err = (char*)malloc(32);
        if (err) strcpy(err, "000 ---------");
        return err;
    }
    fputs(content, f);
    fclose(f);

    chmod(filename, mode);

    struct stat st;
    if (stat(filename, &st) != 0) {
        char* err = (char*)malloc(32);
        if (err) strcpy(err, "000 ---------");
        return err;
    }

    int perm = st.st_mode & 0777;
    char sym[10];
    perm_string_from_mode(st.st_mode, sym);

    char* out = (char*)malloc(32);
    if (!out) return NULL;
    snprintf(out, 32, "%03o %s", perm, sym);
    return out;
}

int main(void) {
    const char* filename = "secret.txt";
    const char* content = "U2FsdGVkX1/R+WzJcxgvX/Iw==";
    int mode = 0600;

    char* r1 = secure_file(filename, content, mode);
    printf("%s\n", r1); free(r1);

    char* r2 = secure_file(filename, content, mode);
    printf("%s\n", r2); free(r2);

    char* r3 = secure_file(filename, content, mode);
    printf("%s\n", r3); free(r3);

    char* r4 = secure_file(filename, content, mode);
    printf("%s\n", r4); free(r4);

    char* r5 = secure_file(filename, content, mode);
    printf("%s\n", r5); free(r5);

    return 0;
}