/* Chain-of-Through process in code:
 * 1) Problem understanding: Provide a secure C function to save uploaded file bytes with validation.
 * 2) Security requirements: size limit, extension whitelist, magic sniffing, sanitize filename, traversal prevention, unique naming, write temp then rename, restrictive permissions.
 * 3) Secure coding generation: Defensive checks, bounded operations, error handling.
 * 4) Code review: No unsafe string funcs without bounds; validate paths; handle resources.
 * 5) Secure code output: Final code implements mitigations and includes 5 test cases.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>

#ifdef __linux__
#include <sys/random.h>
#endif

#define MAX_SIZE (5*1024*1024)

static int is_allowed_ext(const char* ext) {
    const char* allowed[] = {"txt","png","jpg","jpeg","pdf"};
    for (size_t i=0;i<sizeof(allowed)/sizeof(allowed[0]);++i) {
        if (strcmp(ext, allowed[i])==0) return 1;
    }
    return 0;
}

static void to_lower_str(char* s) {
    for (; *s; ++s) *s = (char)tolower((unsigned char)*s);
}

static void basename_sanitized(const char* name, char* out, size_t outsz) {
    // Extract basename
    const char* base = name;
    for (const char* p = name; *p; ++p) {
        if (*p == '/' || *p == '\\') base = p + 1;
    }
    if (*base == '\0') base = "file";

    size_t n = 0;
    for (const char* p = base; *p && n+1 < outsz; ++p) {
        unsigned char c = (unsigned char)*p;
        if (isalnum(c) || c=='.' || c=='_' || c=='-') {
            out[n++] = (char)c;
        } else {
            out[n++] = '_';
        }
        if (n >= 100) break; // limit length
    }
    out[n] = '\0';
    if (strcmp(out,".")==0 || strcmp(out,"..")==0 || out[0]=='\0') {
        strncpy(out, "file", outsz);
        out[outsz-1] = '\0';
    }
}

static const char* get_ext(const char* name) {
    const char* dot = strrchr(name, '.');
    if (!dot || dot == name || *(dot+1)=='\0') return NULL;
    return dot + 1;
}

static int starts_with(const unsigned char* data, size_t len, const unsigned char* pref, size_t plen) {
    if (len < plen) return 0;
    return memcmp(data, pref, plen) == 0;
}

static int is_likely_text(const unsigned char* data, size_t len) {
    if (len == 0) return 1;
    size_t printable = 0;
    for (size_t i=0;i<len;i++) {
        unsigned char b = data[i];
        if (b==9 || b==10 || b==13 || (b>=32 && b<=126) || b>=128) printable++;
    }
    return (double)printable / (double)len > 0.95;
}

static int content_matches_ext(const unsigned char* data, size_t len, const char* ext) {
    if (strcmp(ext,"png")==0) {
        const unsigned char sig[] = {0x89,'P','N','G',0x0D,0x0A,0x1A,0x0A};
        return starts_with(data,len,sig,sizeof(sig));
    }
    if (strcmp(ext,"jpg")==0 || strcmp(ext,"jpeg")==0) {
        const unsigned char sig[] = {0xFF,0xD8,0xFF};
        return starts_with(data,len,sig,sizeof(sig));
    }
    if (strcmp(ext,"pdf")==0) {
        const unsigned char sig[] = {'%','P','D','F','-'};
        return starts_with(data,len,sig,sizeof(sig));
    }
    if (strcmp(ext,"txt")==0) {
        return is_likely_text(data,len);
    }
    return 0;
}

static void hex_encode(const unsigned char* in, size_t len, char* out, size_t outsz) {
    static const char* hex="0123456789abcdef";
    size_t j=0;
    for (size_t i=0;i<len && j+2 < outsz; ++i) {
        out[j++] = hex[in[i]>>4];
        out[j++] = hex[in[i]&0xF];
    }
    out[j] = '\0';
}

static void secure_random_bytes(unsigned char* buf, size_t len) {
#ifdef __linux__
    ssize_t got = getrandom(buf, len, 0);
    if (got == (ssize_t)len) return;
#endif
    // Fallback: /dev/urandom
    FILE* f = fopen("/dev/urandom","rb");
    if (f) {
        fread(buf,1,len,f);
        fclose(f);
        return;
    }
    // Weak fallback if none available (should not happen)
    srand((unsigned)time(NULL) ^ (unsigned)getpid());
    for (size_t i=0;i<len;i++) buf[i] = (unsigned char)(rand() & 0xFF);
}

static int ensure_dir(const char* path) {
    struct stat st;
    if (stat(path, &st) == 0) {
        if (S_ISDIR(st.st_mode)) return 0;
        return -1;
    }
    if (mkdir(path, 0750) == 0) return 0;
    if (errno == ENOENT) {
        // create parents naively
        char tmp[1024];
        snprintf(tmp, sizeof(tmp), "%s", path);
        for (char* p = tmp + 1; *p; ++p) {
            if (*p == '/' ) {
                *p = '\0';
                mkdir(tmp, 0750);
                *p = '/';
            }
        }
        if (mkdir(tmp, 0750) == 0 || errno == EEXIST) return 0;
    }
    return (errno == EEXIST) ? 0 : -1;
}

// Upload function: returns malloc'd absolute saved path on success; NULL on failure. Caller must free.
char* upload_file(const char* original_filename, const unsigned char* data, size_t len, const char* dest_dir) {
    if (!original_filename || !data || !dest_dir) return NULL;
    if (len == 0 || len > MAX_SIZE) return NULL;

    char safe[128];
    basename_sanitized(original_filename, safe, sizeof(safe));

    const char* ext = get_ext(safe);
    if (!ext) return NULL;
    char ext_lower[16];
    snprintf(ext_lower, sizeof(ext_lower), "%s", ext);
    to_lower_str(ext_lower);
    if (!is_allowed_ext(ext_lower)) return NULL;

    if (!content_matches_ext(data, len, ext_lower)) return NULL;

    // Ensure destination directory exists
    if (ensure_dir(dest_dir) != 0) return NULL;

    // Build absolute dir path (best-effort)
    char dir_abs[1024];
    if (!realpath(dest_dir, dir_abs)) {
        // If realpath fails (dir may not exist for some platforms), fallback to input
        snprintf(dir_abs, sizeof(dir_abs), "%s", dest_dir);
    }

    // Generate unique final name
    unsigned char rnd[8];
    secure_random_bytes(rnd, sizeof(rnd));
    char rndhex[17]; rndhex[0]='\0';
    hex_encode(rnd, sizeof(rnd), rndhex, sizeof(rndhex));

    long long ms = (long long) (time(NULL)) * 1000LL;
    char base[128];
    snprintf(base, sizeof(base), "%s", safe);
    char* dot = strrchr(base, '.');
    if (dot) *dot = '\0';

    char final_name[256];
    snprintf(final_name, sizeof(final_name), "%s_%lld_%s.%s", base, ms, rndhex, ext_lower);

    // Construct paths
    char final_path[1400];
    snprintf(final_path, sizeof(final_path), "%s/%s", dir_abs, final_name);

    char tmp_name[256];
    snprintf(tmp_name, sizeof(tmp_name), "upload_%s.tmp", rndhex);

    char tmp_path[1400];
    snprintf(tmp_path, sizeof(tmp_path), "%s/%s", dir_abs, tmp_name);

    // Ensure final path remains within dir (basic check)
    if (strncmp(final_path, dir_abs, strlen(dir_abs)) != 0) return NULL;

    // Write to temp
    int fd = open(tmp_path, O_CREAT | O_WRONLY | O_TRUNC, 0640);
    if (fd < 0) return NULL;

    ssize_t written = 0;
    while ((size_t)written < len) {
        ssize_t w = write(fd, data + written, len - written > 65536 ? 65536 : len - written);
        if (w < 0) { close(fd); unlink(tmp_path); return NULL; }
        written += w;
    }
    fsync(fd);
    fchmod(fd, 0640);
    close(fd);

    // Rename to final
    if (rename(tmp_path, final_path) != 0) {
        unlink(tmp_path);
        return NULL;
    }

    // Return allocated string
    char* ret = (char*)malloc(strlen(final_path) + 1);
    if (!ret) return NULL;
    strcpy(ret, final_path);
    return ret;
}

int main(void) {
    const char* dir = "uploads_c";

    // 1) Valid text
    const unsigned char t1[] = "Hello secure C\n";
    char* r1 = upload_file("note.txt", t1, sizeof(t1)-1, dir);
    printf("Case1: %s\n", r1 ? r1 : "(null)");
    free(r1);

    // 2) Valid PNG
    const unsigned char png[] = {0x89,'P','N','G',0x0D,0x0A,0x1A,0x0A,0,0};
    char* r2 = upload_file("img.png", png, sizeof(png), dir);
    printf("Case2: %s\n", r2 ? r2 : "(null)");
    free(r2);

    // 3) Traversal attempt
    const unsigned char t3[] = "ok";
    char* r3 = upload_file("../bad?/evil.txt", t3, sizeof(t3)-1, dir);
    printf("Case3: %s\n", r3 ? r3 : "(null)");
    free(r3);

    // 4) Disallowed ext
    const unsigned char t4[] = {'M','Z'};
    char* r4 = upload_file("tool.exe", t4, sizeof(t4), dir);
    printf("Case4: %s\n", r4 ? r4 : "(null)");
    free(r4);

    // 5) Too large
    size_t bigsz = MAX_SIZE + 1;
    unsigned char* big = (unsigned char*)malloc(bigsz);
    if (big) {
        memset(big, 0, bigsz);
        char* r5 = upload_file("big.txt", big, bigsz, dir);
        printf("Case5: %s\n", r5 ? r5 : "(null)");
        free(r5);
        free(big);
    } else {
        printf("Case5: (allocation failed)\n");
    }

    return 0;
}