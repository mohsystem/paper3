/*
Secure XML XPath Query Program in C using libxml2.

Functionality:
- Reads an XML filename and an id value from command line or runs 5 built-in tests if no args.
- Safely opens and reads the XML file from a base directory using openat with O_NOFOLLOW.
- Rejects XML containing DOCTYPE to prevent XXE.
- Parses XML with libxml2 using security-conscious flags (no network, no entity expansion).
- Executes XPath: /tag[@id='value'] where value is validated against a safe character set.
- Returns the concatenated string values of matched nodes, one per line.

Build (example):
  cc -Wall -Wextra -Werror -O2 task97.c -o task97 `pkg-config --cflags --libs libxml-2.0`

Run:
  ./task97 <id_value> <filename>
If no arguments are provided, 5 self-tests are executed.
*/

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xpath.h>

#ifndef O_NOFOLLOW
#define O_NOFOLLOW 0
#endif

#define MAX_XML_SIZE          (1024 * 1024)   /* 1 MiB */
#define MAX_ID_LEN            128
#define MAX_FILENAME_LEN      255
#define MAX_RESULT_SIZE       (1024 * 1024)   /* 1 MiB */
#define MAX_ATTR_COUNT        10240           /* Not directly enforceable here; kept for reference */

/* ================= Utility: safe zero ================= */
static void secure_zero(void *p, size_t n) {
#if defined(__STDC_LIB_EXT1__)
    memset_s(p, n, 0, n);
#else
    volatile unsigned char *vp = (volatile unsigned char *)p;
    while (n--) { *vp++ = 0; }
#endif
}

/* ================= Validation Helpers ================= */
static int is_valid_id(const char *id) {
    if (!id) return 0;
    size_t len = strlen(id);
    if (len == 0 || len > MAX_ID_LEN) return 0;
    for (size_t i = 0; i < len; i++) {
        unsigned char c = (unsigned char)id[i];
        if (!(c == '_' || c == '-' || (c >= '0' && c <= '9') ||
              (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'))) {
            return 0;
        }
    }
    return 1;
}

static int is_safe_rel_filename(const char *name) {
    if (!name) return 0;
    size_t len = strlen(name);
    if (len == 0 || len > MAX_FILENAME_LEN) return 0;
    if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0) return 0;
    for (size_t i = 0; i < len; i++) {
        unsigned char c = (unsigned char)name[i];
        if (c == '/' || c == '\\') return 0;
        if (!(c == '.' || c == '_' || c == '-' ||
              (c >= '0' && c <= '9') ||
              (c >= 'A' && c <= 'Z') ||
              (c >= 'a' && c <= 'z'))) {
            return 0;
        }
    }
    return 1;
}

/* Case-insensitive search for "<!DOCTYPE" to reject XXE. */
static int contains_doctype(const unsigned char *buf, size_t len) {
    if (!buf) return 0;
    const char *needle = "<!doctype";
    size_t nlen = 9;
    for (size_t i = 0; i + nlen <= len; i++) {
        size_t j = 0;
        for (; j < nlen; j++) {
            unsigned char c = buf[i + j];
            if (tolower(c) != needle[j]) break;
        }
        if (j == nlen) return 1;
    }
    return 0;
}

/* ================= Safe file reading ================= */
static int safe_read_file_from_dir(const char *base_dir, const char *rel_path,
                                   unsigned char **out_buf, size_t *out_len) {
    if (!base_dir || !rel_path || !out_buf || !out_len) return -1;
    if (!is_safe_rel_filename(rel_path)) return -1;

    int dirfd = open(base_dir, O_RDONLY | O_DIRECTORY | O_CLOEXEC);
    if (dirfd < 0) return -1;

    int fd = openat(dirfd, rel_path, O_RDONLY | O_CLOEXEC | O_NOFOLLOW);
    if (fd < 0) { close(dirfd); return -1; }

    struct stat st;
    if (fstat(fd, &st) != 0) { close(fd); close(dirfd); return -1; }
    if (!S_ISREG(st.st_mode)) { close(fd); close(dirfd); errno = EINVAL; return -1; }
    if (st.st_size < 0 || st.st_size > MAX_XML_SIZE) { close(fd); close(dirfd); errno = EFBIG; return -1; }

    size_t sz = (size_t)st.st_size;
    unsigned char *buf = (unsigned char *)malloc(sz + 1);
    if (!buf) { close(fd); close(dirfd); return -1; }

    size_t off = 0;
    while (off < sz) {
        ssize_t r = read(fd, buf + off, sz - off);
        if (r < 0) {
            if (errno == EINTR) continue;
            free(buf);
            close(fd);
            close(dirfd);
            return -1;
        }
        if (r == 0) break;
        off += (size_t)r;
    }
    buf[off] = '\0';
    close(fd);
    close(dirfd);

    if (off > MAX_XML_SIZE) { free(buf); errno = EFBIG; return -1; }

    *out_buf = buf;
    *out_len = off;
    return 0;
}

/* ================= Secure XML parsing ================= */
static xmlDocPtr parse_xml_secure(const unsigned char *buf, size_t len) {
    if (!buf) return NULL;

    if (contains_doctype(buf, len)) {
        return NULL; /* Reject XML containing DOCTYPE */
    }

    /* Initialize libxml2 with secure defaults */
    xmlInitParser();
#if defined(LIBXML_VERSION)
    /* Disable external entities and DTD loading */
    xmlSubstituteEntitiesDefault(0);
    xmlLoadExtDtdDefaultValue = 0;
#endif
#if defined(LIBXML_VERSION) && (LIBXML_VERSION >= 20900)
    /* Modern libxml2 has safer defaults, but ensure no network */
#endif
#if defined(LIBXML_VERSION)
    xmlDisableExternalEntityLoader();
#endif

    /* Parse from memory with safe flags: no network, no warnings, no CDATA expansion */
    int options = XML_PARSE_NONET | XML_PARSE_NOCDATA | XML_PARSE_NOERROR | XML_PARSE_NOWARNING;
    xmlDocPtr doc = xmlReadMemory((const char *)buf, (int)len, NULL, NULL, options);
    if (!doc) {
        return NULL;
    }
    return doc;
}

/* ================= XPath execution ================= */
static char *execute_xpath_tag_id(xmlDocPtr doc, const char *id) {
    if (!doc || !id || !is_valid_id(id)) return NULL;

    /* Build fixed XPath safely: /tag[@id='ID'] */
    size_t idlen = strlen(id);
    size_t xlen = strlen("/tag[@id='") + idlen + strlen("']") + 1;
    if (xlen > 1024) return NULL;

    char xpath[1024];
    int w = snprintf(xpath, sizeof(xpath), "/tag[@id='%s']", id);
    if (w < 0 || (size_t)w >= sizeof(xpath)) return NULL;

    xmlXPathContextPtr ctx = xmlXPathNewContext(doc);
    if (!ctx) return NULL;

    xmlXPathObjectPtr obj = xmlXPathEvalExpression((const xmlChar *)xpath, ctx);
    if (!obj) {
        xmlXPathFreeContext(ctx);
        return NULL;
    }

    xmlNodeSetPtr nodes = obj->nodesetval;
    if (!nodes || nodes->nodeNr <= 0) {
        xmlXPathFreeObject(obj);
        xmlXPathFreeContext(ctx);
        /* Return empty string for no matches */
        char *empty = (char *)calloc(1, 1);
        return empty;
    }

    /* Aggregate node string values; cap total size */
    size_t cap = 256;
    char *out = (char *)malloc(cap);
    if (!out) {
        xmlXPathFreeObject(obj);
        xmlXPathFreeContext(ctx);
        return NULL;
    }
    size_t used = 0;
    out[0] = '\0';

    for (int i = 0; i < nodes->nodeNr; i++) {
        xmlNodePtr n = nodes->nodeTab[i];
        if (!n) continue;
        xmlChar *content = xmlNodeGetContent(n);
        if (!content) continue;

        size_t addlen = strlen((const char *)content);
        /* Add newline between results if something already there */
        if (used > 0) addlen += 1;

        if (used + addlen + 1 > cap) {
            size_t newcap = cap;
            while (newcap < used + addlen + 1) {
                newcap *= 2;
                if (newcap > MAX_RESULT_SIZE) { newcap = MAX_RESULT_SIZE; break; }
            }
            if (newcap <= used + addlen) {
                /* Exceeded maximum allowed size */
                xmlFree(content);
                free(out);
                xmlXPathFreeObject(obj);
                xmlXPathFreeContext(ctx);
                return NULL;
            }
            char *tmp = (char *)realloc(out, newcap);
            if (!tmp) {
                xmlFree(content);
                free(out);
                xmlXPathFreeObject(obj);
                xmlXPathFreeContext(ctx);
                return NULL;
            }
            out = tmp;
            cap = newcap;
        }

        if (used > 0) {
            out[used++] = '\n';
        }
        memcpy(out + used, (const char *)content, addlen - (used > 0 ? 1 : 0));
        used += addlen - (used > 0 ? 1 : 0);
        out[used] = '\0';
        xmlFree(content);
    }

    xmlXPathFreeObject(obj);
    xmlXPathFreeContext(ctx);
    return out;
}

/* ================= High-level processing ================= */
static char *process_file_query(const char *base_dir, const char *filename, const char *id) {
    if (!is_valid_id(id) || !is_safe_rel_filename(filename)) {
        return NULL;
    }

    unsigned char *buf = NULL;
    size_t len = 0;
    if (safe_read_file_from_dir(base_dir, filename, &buf, &len) != 0) {
        return NULL;
    }

    xmlDocPtr doc = parse_xml_secure(buf, len);
    /* Clear the original buffer (may contain sensitive data) and free it */
    secure_zero(buf, len);
    free(buf);
    buf = NULL;

    if (!doc) {
        return NULL;
    }

    char *result = execute_xpath_tag_id(doc, id);
    xmlFreeDoc(doc);
    xmlCleanupParser();

    return result;
}

/* ================= Test helpers ================= */
static int safe_write_file_into_dir(const char *dir, const char *name, const char *data) {
    if (!dir || !name || !data) return -1;
    if (!is_safe_rel_filename(name)) return -1;

    int dfd = open(dir, O_RDONLY | O_DIRECTORY | O_CLOEXEC);
    if (dfd < 0) return -1;

    int fd = openat(dfd, name, O_WRONLY | O_CREAT | O_EXCL | O_CLOEXEC | O_NOFOLLOW, 0600);
    if (fd < 0) { close(dfd); return -1; }

    size_t len = strlen(data);
    size_t off = 0;
    while (off < len) {
        ssize_t w = write(fd, data + off, len - off);
        if (w < 0) {
            if (errno == EINTR) continue;
            close(fd); close(dfd);
            return -1;
        }
        off += (size_t)w;
    }

    if (fsync(fd) != 0) { /* best-effort */
        /* ignore */
    }
    close(fd);
    /* fsync the directory entry */
#ifdef __APPLE__
    (void)fsync(dfd);
#else
    (void)fsync(dfd);
#endif
    close(dfd);
    return 0;
}

static int run_testcases(void) {
    char templ[] = "xmlsafeXXXXXX";
    char *tdir = mkdtemp(templ);
    if (!tdir) {
        fprintf(stderr, "Failed to create temp dir\n");
        return 1;
    }

    struct TestCase {
        const char *fname;
        const char *xml;
        const char *id;
        const char *label;
    } cases[5] = {
        { "t1.xml", "<tag id='a1'>Alpha</tag>", "a1", "Test 1" },
        { "t2.xml", "<tag id='b2'>Bravo</tag>", "b2", "Test 2" },
        { "t3.xml", "<tag id='under_score'>Under</tag>", "under_score", "Test 3" },
        { "t4.xml", "<tag id='t-3'>Dash3</tag>", "t-3", "Test 4" },
        { "t5.xml", "<!DOCTYPE x [ <!ELEMENT x ANY> ]><tag id='evil'>Nope</tag>", "evil", "Test 5 (DOCTYPE reject)" }
    };

    for (int i = 0; i < 5; i++) {
        if (safe_write_file_into_dir(tdir, cases[i].fname, cases[i].xml) != 0) {
            fprintf(stderr, "%s: write failed\n", cases[i].label);
            continue;
        }
        char *res = process_file_query(tdir, cases[i].fname, cases[i].id);
        if (res) {
            printf("%s: %s\n", cases[i].label, (*res ? res : "(no match)"));
            secure_zero(res, strlen(res));
            free(res);
        } else {
            printf("%s: ERROR\n", cases[i].label);
        }
    }

    /* Cleanup: remove files and directory (best-effort) */
    int dfd = open(tdir, O_RDONLY | O_DIRECTORY | O_CLOEXEC);
    if (dfd >= 0) {
        for (int i = 0; i < 5; i++) {
            unlinkat(dfd, cases[i].fname, 0);
        }
        close(dfd);
    }
    rmdir(tdir);
    return 0;
}

/* ================= main ================= */
int main(int argc, char **argv) {
    /* If args provided: argv[1] = id, argv[2] = filename (read from current directory) */
    if (argc >= 3) {
        const char *id = argv[1];
        const char *filename = argv[2];
        if (!is_valid_id(id) || !is_safe_rel_filename(filename)) {
            fprintf(stderr, "Invalid input\n");
            return 1;
        }
        char *res = process_file_query(".", filename, id);
        if (!res) {
            fprintf(stderr, "ERROR\n");
            return 1;
        }
        if (*res) {
            printf("%s\n", res);
        } else {
            /* No match: print nothing (or a newline for clarity) */
            printf("\n");
        }
        secure_zero(res, strlen(res));
        free(res);
        return 0;
    }

    /* No args: run 5 self-tests */
    return run_testcases();
}