#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#ifdef _WIN32
  #include <direct.h>
  #include <sys/stat.h>
  #define MKDIR(path, mode) _mkdir(path)
  #define PATH_SEP '\\'
#else
  #include <sys/types.h>
  #include <sys/stat.h>
  #include <unistd.h>
  #define MKDIR(path, mode) mkdir(path, mode)
  #define PATH_SEP '/'
#endif

static int ensure_dir(const char* path) {
    if (path == NULL || *path == '\0') return -1;

    char tmp[1024];
    snprintf(tmp, sizeof(tmp), "%s", path);
    size_t len = strlen(tmp);
    if (len == 0) return -1;

    // Remove trailing separators
    while (len > 1 && (tmp[len-1] == '/' || tmp[len-1] == '\\')) {
        tmp[len-1] = '\0';
        len--;
    }

    for (size_t i = 1; i < len; ++i) {
        if (tmp[i] == '/' || tmp[i] == '\\') {
            char prev = tmp[i];
            tmp[i] = '\0';
            if (MKDIR(tmp, 0755) != 0) {
#ifdef _WIN32
                if (errno != EEXIST) { tmp[i] = prev; return -1; }
#else
                if (errno != EEXIST) { tmp[i] = prev; return -1; }
#endif
            }
            tmp[i] = prev;
        }
    }
    if (MKDIR(tmp, 0755) != 0) {
#ifdef _WIN32
        if (errno != EEXIST) return -1;
#else
        if (errno != EEXIST) return -1;
#endif
    }
    return 0;
}

static const char* path_basename(const char* path) {
    const char* base = path;
    for (const char* p = path; *p; ++p) {
        if (*p == '/' || *p == '\\') {
            if (*(p+1)) base = p + 1;
        }
    }
    return base;
}

static void join_paths(const char* dir, const char* file, char* out, size_t out_size) {
    size_t dl = strlen(dir);
    int need_sep = (dl > 0 && dir[dl-1] != '/' && dir[dl-1] != '\\');
    snprintf(out, out_size, "%s%s%s", dir, need_sep ? "/" : "", file);
}

int upload_file(const char* source_path, const char* dest_dir, const char* new_name, char* out_path, size_t out_path_size) {
    if (!source_path || !dest_dir || !out_path || out_path_size == 0) return -1;

    FILE* in = fopen(source_path, "rb");
    if (!in) return -2;

    if (ensure_dir(dest_dir) != 0) {
        fclose(in);
        return -3;
    }

    const char* base = (new_name && new_name[0] != '\0') ? new_name : path_basename(source_path);
    char dest_path[2048];
    join_paths(dest_dir, base, dest_path, sizeof(dest_path));

    FILE* out = fopen(dest_path, "wb");
    if (!out) {
        fclose(in);
        return -4;
    }

    char buf[8192];
    size_t n;
    while ((n = fread(buf, 1, sizeof(buf), in)) > 0) {
        if (fwrite(buf, 1, n, out) != n) {
            fclose(in);
            fclose(out);
            return -5;
        }
    }
    fclose(in);
    fclose(out);

    snprintf(out_path, out_path_size, "%s", dest_path);
    return 0;
}

static int write_text_file(const char* path, const char* content) {
    // ensure directory for file
    char dir[1024];
    snprintf(dir, sizeof(dir), "%s", path);
    for (int i = (int)strlen(dir) - 1; i >= 0; --i) {
        if (dir[i] == '/' || dir[i] == '\\') {
            dir[i] = '\0';
            break;
        }
    }
    if (strlen(dir) > 0) ensure_dir(dir);

    FILE* f = fopen(path, "wb");
    if (!f) return -1;
    size_t len = strlen(content);
    if (fwrite(content, 1, len, f) != len) {
        fclose(f);
        return -2;
    }
    fclose(f);
    return 0;
}

int main(void) {
    const char* fixtures = "fixtures_c";
    const char* uploads = "uploads_c";

    ensure_dir(fixtures);
    ensure_dir(uploads);

    char f1[256], f2[256], f3[256], f4[256], f5[256];
    snprintf(f1, sizeof(f1), "%s%c%s", fixtures, PATH_SEP, "cfile1.txt");
    snprintf(f2, sizeof(f2), "%s%c%s", fixtures, PATH_SEP, "cfile2.txt");
    snprintf(f3, sizeof(f3), "%s%c%s", fixtures, PATH_SEP, "cfile3.log");
    snprintf(f4, sizeof(f4), "%s%c%s", fixtures, PATH_SEP, "blob.bin");
    snprintf(f5, sizeof(f5), "%s%c%s", fixtures, PATH_SEP, "doc.md");

    write_text_file(f1, "Hello from C file1");
    write_text_file(f2, "Hello from C file2");
    write_text_file(f3, "Log from C");
    // write binary-like for f4
    {
        ensure_dir(fixtures);
        FILE* bf = fopen(f4, "wb");
        if (bf) {
            unsigned char data[] = {0x00, 0x01, 0x02, 'C', 'B', 'I', 'N'};
            fwrite(data, 1, sizeof(data), bf);
            fclose(bf);
        }
    }
    write_text_file(f5, "# C Doc\nDetails...");

    char out[2048];
    if (upload_file(f1, uploads, NULL, out, sizeof(out)) == 0) printf("%s\n", out);
    if (upload_file(f2, uploads, "renamed_cfile2.txt", out, sizeof(out)) == 0) printf("%s\n", out);
    {
        char nested[256];
        snprintf(nested, sizeof(nested), "%s%c%s%c%s", uploads, PATH_SEP, "nestedC", PATH_SEP, "nestedD");
        if (upload_file(f3, nested, NULL, out, sizeof(out)) == 0) printf("%s\n", out);
    }
    if (upload_file(f4, uploads, "blob_copy.bin", out, sizeof(out)) == 0) printf("%s\n", out);
    {
        char docs[256];
        snprintf(docs, sizeof(docs), "%s%c%s", uploads, PATH_SEP, "docs");
        if (upload_file(f5, docs, "doc_copy.md", out, sizeof(out)) == 0) printf("%s\n", out);
    }

    return 0;
}