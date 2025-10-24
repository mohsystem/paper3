#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <limits.h>
#include <time.h>

#pragma pack(push,1)
typedef struct {
    char name[100];
    char mode[8];
    char uid[8];
    char gid[8];
    char size[12];
    char mtime[12];
    char chksum[8];
    char typeflag;
    char linkname[100];
    char magic[6];
    char version[2];
    char uname[32];
    char gname[32];
    char devmajor[8];
    char devminor[8];
    char prefix[155];
    char pad[12];
} TarHeader;
#pragma pack(pop)

static int is_all_zero(const unsigned char* b, size_t n) {
    for (size_t i = 0; i < n; ++i) if (b[i] != 0) return 0;
    return 1;
}

static uint64_t parse_octal(const char* s, size_t n, int* ok) {
    *ok = 1;
    uint64_t v = 0;
    size_t i = 0;
    while (i < n && (s[i] == ' ' || s[i] == '\0')) i++;
    for (; i < n; ++i) {
        char c = s[i];
        if (c == ' ' || c == '\0') break;
        if (c < '0' || c > '7') { *ok = 0; break; }
        v = (v << 3) + (uint64_t)(c - '0');
    }
    return v;
}

static unsigned int compute_checksum(const TarHeader* h) {
    TarHeader temp;
    memcpy(&temp, h, sizeof(TarHeader));
    for (int i = 0; i < 8; ++i) temp.chksum[i] = ' ';
    const unsigned char* b = (const unsigned char*)&temp;
    unsigned int sum = 0;
    for (size_t i = 0; i < sizeof(TarHeader); ++i) sum += b[i];
    return sum;
}

static int ensure_base_dir(const char* base) {
    struct stat st, lst;
    if (stat(base, &st) == 0) {
        if (!S_ISDIR(st.st_mode)) return 0;
        if (lstat(base, &lst) == 0 && S_ISLNK(lst.st_mode)) return 0;
        return 1;
    }
    if (mkdir(base, 0700) != 0) return 0;
    return 1;
}

static int is_symlink_path(const char* path) {
    struct stat st;
    if (lstat(path, &st) != 0) return 0;
    return S_ISLNK(st.st_mode) ? 1 : 0;
}

static int normalize_join(const char* baseReal, const char* rel, char* out, size_t outsz) {
    if (!rel || rel[0] == '\0') return 0;
    if (rel[0] == '/') return 0;
    // Build normalized path
    char temp[PATH_MAX];
    size_t blen = strlen(baseReal);
    if (blen + 1 >= sizeof(temp)) return 0;
    memcpy(temp, baseReal, blen);
    temp[blen] = '\0';

    char relcpy[PATH_MAX];
    if (strlen(rel) >= sizeof(relcpy)) return 0;
    strcpy(relcpy, rel);

    char* saveptr = NULL;
    char* tok = strtok_r(relcpy, "/", &saveptr);
    size_t tlen = strlen(temp);
    while (tok) {
        if (strcmp(tok, ".") == 0) {
            // skip
        } else if (strcmp(tok, "..") == 0) {
            // remove last component
            char* slash = strrchr(temp, '/');
            if (!slash || (size_t)(slash - temp) < blen) return 0;
            *slash = '\0';
            tlen = strlen(temp);
        } else {
            size_t sl = strlen(tok);
            if (sl == 0) { tok = strtok_r(NULL, "/", &saveptr); continue; }
            if (tlen + 1 + sl >= sizeof(temp)) return 0;
            temp[tlen++] = '/';
            memcpy(&temp[tlen], tok, sl);
            tlen += sl;
            temp[tlen] = '\0';
        }
        tok = strtok_r(NULL, "/", &saveptr);
    }
    // Verify prefix
    if (strncmp(temp, baseReal, blen) != 0) return 0;
    if (temp[blen] != '/' && temp[blen] != '\0') return 0;
    if (strlen(temp) + 1 > outsz) return 0;
    strcpy(out, temp);
    return 1;
}

static int ensure_parent_dirs(const char* baseReal, const char* fullPath) {
    size_t blen = strlen(baseReal);
    size_t flen = strlen(fullPath);
    if (flen <= blen) return 0;
    char buf[PATH_MAX];
    if (flen >= sizeof(buf)) return 0;
    strcpy(buf, fullPath);
    // find parent end
    char* last = strrchr(buf, '/');
    if (!last) return 0;
    *last = '\0';
    size_t i = blen + 1;
    while (1) {
        char* p = strchr(buf + i, '/');
        if (!p) break;
        *p = '\0';
        struct stat st;
        if (lstat(buf, &st) == 0) {
            if (S_ISLNK(st.st_mode)) return 0;
            if (!S_ISDIR(st.st_mode)) return 0;
        } else {
            if (errno != ENOENT) return 0;
            if (mkdir(buf, 0700) != 0) return 0;
        }
        *p = '/';
        i = (size_t)(p - buf) + 1;
    }
    // Check parent not symlink
    struct stat pst;
    if (lstat(buf, &pst) != 0) return 0;
    if (!S_ISDIR(pst.st_mode)) return 0;
    if (S_ISLNK(pst.st_mode)) return 0;
    return 1;
}

static int write_file_secure(const char* targetPath, int mode, int inFd, uint64_t size) {
    // Create temp file in parent dir
    char dir[PATH_MAX];
    strncpy(dir, targetPath, sizeof(dir) - 1);
    dir[sizeof(dir) - 1] = '\0';
    char* slash = strrchr(dir, '/');
    if (!slash) return 0;
    *slash = '\0';
    char tmpl[PATH_MAX];
    if (snprintf(tmpl, sizeof(tmpl), "%s/%s", dir, ".tmp_extract_XXXXXX") >= (int)sizeof(tmpl)) return 0;

    int tfd = mkstemp(tmpl);
    if (tfd < 0) return 0;
    fchmod(tfd, 0600);

    uint8_t buf[8192];
    uint64_t remaining = size;
    while (remaining > 0) {
        size_t toRead = remaining > sizeof(buf) ? sizeof(buf) : (size_t)remaining;
        ssize_t r = read(inFd, buf, toRead);
        if (r != (ssize_t)toRead) { close(tfd); unlink(tmpl); return 0; }
        size_t off = 0;
        while (off < (size_t)r) {
            ssize_t w = write(tfd, buf + off, (size_t)r - off);
            if (w < 0) { close(tfd); unlink(tmpl); return 0; }
            off += (size_t)w;
        }
        remaining -= (size_t)r;
    }
    if (fsync(tfd) != 0) { close(tfd); unlink(tmpl); return 0; }
    if (close(tfd) != 0) { unlink(tmpl); return 0; }

    // Ensure target not symlink
    struct stat lst;
    if (lstat(targetPath, &lst) == 0 && S_ISLNK(lst.st_mode)) { unlink(tmpl); return 0; }
    if (rename(tmpl, targetPath) != 0) { unlink(tmpl); return 0; }
    chmod(targetPath, (mode & 0644) | 0600);
    int dfd = open(dir, O_RDONLY | O_DIRECTORY);
    if (dfd >= 0) { fsync(dfd); close(dfd); }
    return 1;
}

static int extract_tar(const char* tarPath, const char* outBase) {
    if (!ensure_base_dir(outBase)) return 0;
    char baseReal[PATH_MAX];
    if (!realpath(outBase, baseReal)) return 0;

    int fd = open(tarPath, O_RDONLY);
    if (fd < 0) return 0;

    const size_t BLOCK = 512;
    unsigned char block[512];
    int zeroBlocks = 0;
    while (1) {
        ssize_t r = read(fd, block, BLOCK);
        if (r == 0) break;
        if (r != (ssize_t)BLOCK) { close(fd); return 0; }
        if (is_all_zero(block, BLOCK)) {
            zeroBlocks++;
            if (zeroBlocks == 2) break;
            else continue;
        } else zeroBlocks = 0;

        TarHeader h;
        memcpy(&h, block, sizeof(TarHeader));
        int okSize = 0, okChk = 0;
        uint64_t fsize = parse_octal(h.size, sizeof(h.size), &okSize);
        uint64_t chkRead = parse_octal(h.chksum, sizeof(h.chksum), &okChk);
        unsigned int expect = compute_checksum(&h);
        if (!okSize || !okChk || (unsigned int)chkRead != expect) { close(fd); return 0; }

        char name[256]; memset(name, 0, sizeof(name));
        size_t nlen = strnlen(h.name, sizeof(h.name));
        memcpy(name, h.name, nlen);
        char prefix[256]; memset(prefix, 0, sizeof(prefix));
        size_t plen = strnlen(h.prefix, sizeof(h.prefix));
        memcpy(prefix, h.prefix, plen);
        char fullName[PATH_MAX]; fullName[0] = '\0';
        if (plen > 0) {
            if (snprintf(fullName, sizeof(fullName), "%s/%s", prefix, name) >= (int)sizeof(fullName)) { close(fd); return 0; }
        } else {
            if (snprintf(fullName, sizeof(fullName), "%s", name) >= (int)sizeof(fullName)) { close(fd); return 0; }
        }
        char type = h.typeflag ? h.typeflag : '0';
        if (type == '5') {
            size_t fl = strlen(fullName);
            if (fl > 0 && fullName[fl - 1] == '/') fullName[fl - 1] = '\0';
        }
        // limit size for demo safety
        if (fsize > 100ULL * 1024ULL * 1024ULL) { close(fd); return 0; }

        int okMode = 0;
        int mode = (int)parse_octal(h.mode, sizeof(h.mode), &okMode);
        if (!okMode) mode = 0644;

        char targetPath[PATH_MAX];
        if (!normalize_join(baseReal, fullName, targetPath, sizeof(targetPath))) { close(fd); return 0; }

        if (type == '0' || type == '\0') {
            if (!ensure_parent_dirs(baseReal, targetPath)) { close(fd); return 0; }
            if (!write_file_secure(targetPath, mode, fd, fsize)) { close(fd); return 0; }
            uint64_t pad = (BLOCK - (fsize % BLOCK)) % BLOCK;
            if (pad) {
                if (lseek(fd, (off_t)pad, SEEK_CUR) == (off_t)-1) { close(fd); return 0; }
            }
        } else if (type == '5') {
            // Directory
            char dummy[PATH_MAX];
            if (snprintf(dummy, sizeof(dummy), "%s/%s", targetPath, "dummy") >= (int)sizeof(dummy)) { close(fd); return 0; }
            if (!ensure_parent_dirs(baseReal, dummy)) { close(fd); return 0; }
            struct stat st;
            if (lstat(targetPath, &st) != 0) {
                if (mkdir(targetPath, 0700) != 0) { close(fd); return 0; }
            } else {
                if (S_ISLNK(st.st_mode)) { close(fd); return 0; }
                if (!S_ISDIR(st.st_mode)) { close(fd); return 0; }
            }
            uint64_t pad = (BLOCK - (fsize % BLOCK)) % BLOCK;
            if (pad) {
                if (lseek(fd, (off_t)pad, SEEK_CUR) == (off_t)-1) { close(fd); return 0; }
            }
        } else if (type == '2') {
            // symlink: reject
            close(fd);
            return 0;
        } else {
            close(fd);
            return 0;
        }
    }
    close(fd);
    return 1;
}

// Test helpers: create tar files
static void fill_header(TarHeader* h, const char* name, char typeflag, uint64_t size, int mode, const char* prefix) {
    memset(h, 0, sizeof(*h));
    strncpy(h->name, name, sizeof(h->name));
    snprintf(h->mode, sizeof(h->mode), "%07o", mode & 0777);
    snprintf(h->uid, sizeof(h->uid), "%07o", 0);
    snprintf(h->gid, sizeof(h->gid), "%07o", 0);
    snprintf(h->size, sizeof(h->size), "%011llo", (unsigned long long)size);
    snprintf(h->mtime, sizeof(h->mtime), "%011o", (unsigned)time(NULL));
    memset(h->chksum, ' ', sizeof(h->chksum));
    h->typeflag = typeflag;
    strncpy(h->magic, "ustar", 5);
    memcpy(h->version, "00", 2);
    strncpy(h->uname, "user", sizeof(h->uname));
    strncpy(h->gname, "group", sizeof(h->gname));
    if (prefix && *prefix) strncpy(h->prefix, prefix, sizeof(h->prefix));
    unsigned int sum = compute_checksum(h);
    snprintf(h->chksum, sizeof(h->chksum), "%06o", sum);
    h->chksum[6] = '\0';
    h->chksum[7] = ' ';
}

static int create_sample_tar_basic(const char* tarPath) {
    int fd = open(tarPath, O_CREAT | O_TRUNC | O_WRONLY, 0600);
    if (fd < 0) return 0;
    // dir
    TarHeader h1;
    fill_header(&h1, "dir", '5', 0, 0755, NULL);
    if (write(fd, &h1, sizeof(h1)) != (ssize_t)sizeof(h1)) { close(fd); return 0; }
    // file
    const char* content = "Hello, world!\n";
    size_t clen = strlen(content);
    TarHeader h2;
    fill_header(&h2, "dir/hello.txt", '0', clen, 0644, NULL);
    if (write(fd, &h2, sizeof(h2)) != (ssize_t)sizeof(h2)) { close(fd); return 0; }
    if (write(fd, content, clen) != (ssize_t)clen) { close(fd); return 0; }
    size_t pad = (512 - (clen % 512)) % 512;
    if (pad) {
        char zeros[512]; memset(zeros, 0, sizeof(zeros));
        if (write(fd, zeros, pad) != (ssize_t)pad) { close(fd); return 0; }
    }
    // two zero blocks
    char zeros[1024]; memset(zeros, 0, sizeof(zeros));
    if (write(fd, zeros, sizeof(zeros)) != (ssize_t)sizeof(zeros)) { close(fd); return 0; }
    close(fd);
    return 1;
}

static int create_tar_with_entry(const char* tarPath, const char* entryName, char typeflag, const char* data) {
    int fd = open(tarPath, O_CREAT | O_TRUNC | O_WRONLY, 0600);
    if (fd < 0) return 0;
    size_t dlen = data ? strlen(data) : 0;
    TarHeader h;
    fill_header(&h, entryName, typeflag, (typeflag == '0' || typeflag == '\0') ? dlen : 0, 0644, NULL);
    if (write(fd, &h, sizeof(h)) != (ssize_t)sizeof(h)) { close(fd); return 0; }
    if (typeflag == '0' || typeflag == '\0') {
        if (dlen > 0) {
            if (write(fd, data, dlen) != (ssize_t)dlen) { close(fd); return 0; }
        }
        size_t pad = (512 - (dlen % 512)) % 512;
        if (pad) {
            char zeros[512]; memset(zeros, 0, sizeof(zeros));
            if (write(fd, zeros, pad) != (ssize_t)pad) { close(fd); return 0; }
        }
    }
    char zeros[1024]; memset(zeros, 0, sizeof(zeros));
    if (write(fd, zeros, sizeof(zeros)) != (ssize_t)sizeof(zeros)) { close(fd); return 0; }
    close(fd);
    return 1;
}

int main(void) {
    const char* base1 = "c_out1";
    const char* base2 = "c_out2";
    const char* base3 = "c_out3";
    const char* base4 = "c_out4";
    const char* base5 = "c_out5";
    mkdir(base1, 0700);
    mkdir(base2, 0700);
    mkdir(base3, 0700);
    mkdir(base4, 0700);
    mkdir(base5, 0700);

    // Test 1: Basic extraction
    const char* tar1 = "c_test1.tar";
    create_sample_tar_basic(tar1);
    int r1 = extract_tar(tar1, base1);
    printf("C Test1 (basic tar) result: %s\n", r1 ? "OK" : "FAIL");

    // Test 2: Path traversal should fail
    const char* tar2 = "c_test2.tar";
    create_tar_with_entry(tar2, "../evil.txt", '0', "malicious");
    int r2 = extract_tar(tar2, base2);
    printf("C Test2 (path traversal) result: %s\n", !r2 ? "OK" : "FAIL");

    // Test 3: Symlink entry should fail
    const char* tar3 = "c_test3.tar";
    create_tar_with_entry(tar3, "link", '2', "");
    int r3 = extract_tar(tar3, base3);
    printf("C Test3 (symlink entry) result: %s\n", !r3 ? "OK" : "FAIL");

    // Test 4: Nonexistent tar file
    int r4 = extract_tar("no_such_file_c.tar", base4);
    printf("C Test4 (nonexistent) result: %s\n", !r4 ? "OK" : "FAIL");

    // Test 5: Absolute path should fail
    const char* tar5 = "c_test5.tar";
    create_tar_with_entry(tar5, "/abs.txt", '0', "abs");
    int r5 = extract_tar(tar5, base5);
    printf("C Test5 (absolute path) result: %s\n", !r5 ? "OK" : "FAIL");

    return 0;
}