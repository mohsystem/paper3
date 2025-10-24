// Secure EXE Downloader and Executor for Windows using libcurl
// Build (MSVC example):
//   cl /W4 /WX /DUNICODE /D_UNICODE /EHsc secure_downloader.c /link ws2_32.lib wldap32.lib bcrypt.lib libcurl.lib
// Note: Requires libcurl with SSL/TLS support available at build and runtime.

#ifdef _WIN32

#define UNICODE
#define _UNICODE

#include <windows.h>
#include <bcrypt.h>
#include <shellapi.h>
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <wchar.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

#pragma comment(lib, "bcrypt.lib")

#ifndef ARRAYSIZE
#define ARRAYSIZE(a) (sizeof(a)/sizeof((a)[0]))
#endif

// Configuration
#define MAX_URL_LEN            2048
#define MAX_DOWNLOAD_SIZE      (100ULL * 1024ULL * 1024ULL) // 100 MiB
#define MAX_REDIRECTS          3L
#define CONNECT_TIMEOUT_SECS   15L
#define TOTAL_TIMEOUT_SECS     300L
#define LOW_SPEED_BPS          1024L
#define LOW_SPEED_TIME_SECS    30L

typedef struct {
    HANDLE h;
    unsigned long long written;
    unsigned long long limit;
    int error;
} write_ctx_t;

static int is_https_url(const char* url) {
    if (url == NULL) return 0;
    size_t n = strnlen(url, MAX_URL_LEN + 1);
    if (n == 0 || n > MAX_URL_LEN) return 0;
    // Reject spaces and control chars
    for (size_t i = 0; i < n; ++i) {
        unsigned char c = (unsigned char)url[i];
        if (c <= 0x1F || c == 0x7F || c == ' ') return 0;
    }
    // Case-insensitive check for https://
    const char* prefix = "https://";
    size_t p = strlen(prefix);
    if (n < p) return 0;
    for (size_t i = 0; i < p; ++i) {
        char a = url[i];
        char b = prefix[i];
        if (a >= 'A' && a <= 'Z') a = (char)(a - 'A' + 'a');
        if (a != b) return 0;
    }
    return 1;
}

static int random_bytes(uint8_t* out, size_t len) {
    if (!out || len == 0) return 0;
    NTSTATUS st = BCryptGenRandom(NULL, out, (ULONG)len, BCRYPT_USE_SYSTEM_PREFERRED_RNG);
    return st == 0;
}

static int random_hex_w(wchar_t* out, size_t out_cch, size_t nbytes) {
    if (!out || out_cch == 0 || nbytes == 0) return 0;
    size_t needed = nbytes * 2 + 1;
    if (out_cch < needed) return 0;
    uint8_t* buf = (uint8_t*)malloc(nbytes);
    if (!buf) return 0;
    int ok = 0;
    if (random_bytes(buf, nbytes)) {
        static const wchar_t* hex = L"0123456789abcdef";
        for (size_t i = 0; i < nbytes; ++i) {
            out[i * 2]     = hex[(buf[i] >> 4) & 0xF];
            out[i * 2 + 1] = hex[buf[i] & 0xF];
        }
        out[nbytes * 2] = L'\0';
        ok = 1;
    }
    free(buf);
    return ok;
}

static int build_path_join(const wchar_t* dir, const wchar_t* name, wchar_t* out, size_t out_cch) {
    if (!dir || !name || !out || out_cch == 0) return 0;
    size_t dlen = wcslen(dir);
    size_t nlen = wcslen(name);
    if (dlen == 0 || nlen == 0) return 0;
    int need_bs = (dir[dlen - 1] != L'\\' && dir[dlen - 1] != L'/') ? 1 : 0;
    size_t total = dlen + need_bs + nlen + 1;
    if (total > out_cch) return 0;
    wcscpy_s(out, out_cch, dir);
    if (need_bs) wcscat_s(out, out_cch, L"\\");
    wcscat_s(out, out_cch, name);
    return 1;
}

static int make_secure_temp_dir(wchar_t* out_dir, size_t out_cch) {
    if (!out_dir || out_cch == 0) return 0;
    wchar_t tmp[MAX_PATH];
    DWORD n = GetTempPathW((DWORD)ARRAYSIZE(tmp), tmp);
    if (n == 0 || n >= ARRAYSIZE(tmp)) return 0;

    // Try a few times to create a unique random directory
    for (int attempt = 0; attempt < 10; ++attempt) {
        wchar_t suffix[33] = {0};
        if (!random_hex_w(suffix, ARRAYSIZE(suffix), 16)) return 0;
        wchar_t dir_name[64];
        if (swprintf(dir_name, ARRAYSIZE(dir_name), L"SecureExeDL-%ls", suffix) < 0) return 0;

        if (!build_path_join(tmp, dir_name, out_dir, out_cch)) return 0;

        if (CreateDirectoryW(out_dir, NULL)) {
            DWORD attrs = GetFileAttributesW(out_dir);
            if (attrs == INVALID_FILE_ATTRIBUTES) return 0;
            if (attrs & FILE_ATTRIBUTE_REPARSE_POINT) {
                // Reparse points are not allowed
                RemoveDirectoryW(out_dir);
                continue;
            }
            return 1;
        } else {
            DWORD err = GetLastError();
            if (err == ERROR_ALREADY_EXISTS) {
                // Try again with another name
                continue;
            } else {
                return 0;
            }
        }
    }
    return 0;
}

static int create_unique_temp_file(const wchar_t* dir, wchar_t* out_path, size_t out_cch, HANDLE* out_h) {
    if (!dir || !out_path || out_cch == 0 || !out_h) return 0;
    *out_h = INVALID_HANDLE_VALUE;
    for (int attempt = 0; attempt < 10; ++attempt) {
        wchar_t rnd[17] = {0};
        if (!random_hex_w(rnd, ARRAYSIZE(rnd), 8)) return 0;
        wchar_t name[64];
        if (swprintf(name, ARRAYSIZE(name), L"dl-%ls.tmp", rnd) < 0) return 0;

        if (!build_path_join(dir, name, out_path, out_cch)) return 0;

        HANDLE h = CreateFileW(out_path,
                               GENERIC_WRITE | GENERIC_READ,
                               0, // no sharing
                               NULL,
                               CREATE_NEW,
                               FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OPEN_REPARSE_POINT,
                               NULL);
        if (h != INVALID_HANDLE_VALUE) {
            // Basic validation on handle
            FILE_STANDARD_INFO fsi;
            if (!GetFileInformationByHandleEx(h, FileStandardInfo, &fsi, sizeof(fsi))) {
                CloseHandle(h);
                DeleteFileW(out_path);
                return 0;
            }
            if (fsi.Directory) {
                CloseHandle(h);
                DeleteFileW(out_path);
                return 0;
            }
            *out_h = h;
            return 1;
        } else {
            DWORD err = GetLastError();
            if (err == ERROR_FILE_EXISTS || err == ERROR_ALREADY_EXISTS) {
                continue;
            } else {
                return 0;
            }
        }
    }
    return 0;
}

static size_t curl_write_cb(char* ptr, size_t size, size_t nmemb, void* userdata) {
    write_ctx_t* ctx = (write_ctx_t*)userdata;
    if (!ctx || ctx->h == INVALID_HANDLE_VALUE) return 0;
    size_t total = size * nmemb;
    if (total == 0) return 0;

    // Bound check size before write
    if (ctx->written + total > ctx->limit) {
        ctx->error = 1;
        return 0; // abort
    }

    size_t left = total;
    char* p = ptr;
    while (left > 0) {
        DWORD to_write = (DWORD)(left > 1 << 20 ? 1 << 20 : left);
        DWORD wrote = 0;
        if (!WriteFile(ctx->h, p, to_write, &wrote, NULL)) {
            ctx->error = 1;
            return 0;
        }
        if (wrote == 0) {
            ctx->error = 1;
            return 0;
        }
        p += wrote;
        left -= wrote;
        ctx->written += wrote;
    }
    return total;
}

static int curl_download_to_handle(const char* url, HANDLE h, unsigned long long limit_bytes) {
    if (!url || h == INVALID_HANDLE_VALUE) return 0;
    CURL* curl = curl_easy_init();
    if (!curl) return 0;

    write_ctx_t ctx;
    ctx.h = h;
    ctx.written = 0ULL;
    ctx.limit = limit_bytes;
    ctx.error = 0;

    CURLcode rc;
    int ok = 0;

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_MAXREDIRS, MAX_REDIRECTS);
#if defined(CURLPROTO_HTTPS)
    curl_easy_setopt(curl, CURLOPT_PROTOCOLS, CURLPROTO_HTTPS);
    curl_easy_setopt(curl, CURLOPT_REDIR_PROTOCOLS, CURLPROTO_HTTPS);
#endif
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
#ifdef CURL_SSLVERSION_TLSv1_2
    curl_easy_setopt(curl, CURLOPT_SSLVERSION, CURL_SSLVERSION_TLSv1_2);
#endif
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "SecureDownloader/1.0");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_write_cb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &ctx);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, CONNECT_TIMEOUT_SECS);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, TOTAL_TIMEOUT_SECS);
    curl_easy_setopt(curl, CURLOPT_LOW_SPEED_LIMIT, LOW_SPEED_BPS);
    curl_easy_setopt(curl, CURLOPT_LOW_SPEED_TIME, LOW_SPEED_TIME_SECS);

    rc = curl_easy_perform(curl);
    if (rc == CURLE_OK && ctx.error == 0) {
        ok = 1;
    }

    curl_easy_cleanup(curl);
    return ok;
}

static int finalize_and_move_exe(HANDLE h, const wchar_t* dir, const wchar_t* final_name,
                                 const wchar_t* temp_path, wchar_t* out_final_path, size_t out_cch) {
    if (!dir || !final_name || !temp_path || !out_final_path || out_cch == 0) return 0;

    if (!FlushFileBuffers(h)) {
        return 0;
    }

    FILE_STANDARD_INFO fsi;
    if (!GetFileInformationByHandleEx(h, FileStandardInfo, &fsi, sizeof(fsi))) {
        return 0;
    }
    if (fsi.Directory) {
        return 0;
    }
    if ((unsigned long long)fsi.EndOfFile.QuadPart > MAX_DOWNLOAD_SIZE) {
        return 0;
    }

    if (!build_path_join(dir, final_name, out_final_path, out_cch)) return 0;

    // Close before moving
    CloseHandle(h);

    // Atomically move to final destination
    if (!MoveFileExW(temp_path, out_final_path, MOVEFILE_WRITE_THROUGH | MOVEFILE_REPLACE_EXISTING)) {
        return 0;
    }
    return 1;
}

static int execute_exe_and_wait(const wchar_t* exe_path, DWORD* out_exit_code) {
    if (!exe_path || !out_exit_code) return 0;

    // Quote the path
    size_t len = wcslen(exe_path);
    if (len == 0 || len > (MAX_PATH * 8)) return 0;

    size_t cmd_len = len + 3; // quotes + null
    wchar_t* cmd = (wchar_t*)malloc(cmd_len * sizeof(wchar_t));
    if (!cmd) return 0;
    if (swprintf(cmd, cmd_len, L"\"%ls\"", exe_path) < 0) {
        free(cmd);
        return 0;
    }

    STARTUPINFOW si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    DWORD flags = 0;
    BOOL ok = CreateProcessW(
        NULL,        // application name
        cmd,         // command line (mutable)
        NULL,        // process security
        NULL,        // thread security
        FALSE,       // inherit handles
        flags,       // creation flags
        NULL,        // environment
        NULL,        // current directory
        &si,
        &pi
    );
    free(cmd);
    if (!ok) {
        return 0;
    }

    // Wait for completion (caller requested execution of downloaded program)
    WaitForSingleObject(pi.hProcess, INFINITE);
    DWORD exit_code = 0;
    if (!GetExitCodeProcess(pi.hProcess, &exit_code)) {
        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
        return 0;
    }

    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    *out_exit_code = exit_code;
    return 1;
}

static int download_and_execute(const char* url, DWORD* out_exit_code) {
    if (!is_https_url(url)) {
        return 0;
    }

    int result = 0;
    wchar_t base_dir[MAX_PATH * 2] = {0};
    wchar_t temp_path[MAX_PATH * 3] = {0};
    wchar_t final_path[MAX_PATH * 3] = {0};
    HANDLE h = INVALID_HANDLE_VALUE;

    if (curl_global_init(CURL_GLOBAL_DEFAULT) != 0) {
        return 0;
    }

    // Create secure base directory
    if (!make_secure_temp_dir(base_dir, ARRAYSIZE(base_dir))) {
        goto cleanup;
    }

    // Create temp file for download
    if (!create_unique_temp_file(base_dir, temp_path, ARRAYSIZE(temp_path), &h)) {
        goto cleanup;
    }

    // Download with strict TLS and size limit
    if (!curl_download_to_handle(url, h, MAX_DOWNLOAD_SIZE)) {
        goto cleanup;
    }

    // Generate final random filename with .exe extension
    wchar_t rnd[17] = {0};
    if (!random_hex_w(rnd, ARRAYSIZE(rnd), 8)) {
        goto cleanup;
    }
    wchar_t final_name[64];
    if (swprintf(final_name, ARRAYSIZE(final_name), L"vendor-%ls.exe", rnd) < 0) {
        goto cleanup;
    }

    // Finalize and move atomically to final exe path
    if (!finalize_and_move_exe(h, base_dir, final_name, temp_path, final_path, ARRAYSIZE(final_path))) {
        goto cleanup;
    }
    h = INVALID_HANDLE_VALUE; // moved; don't use temp anymore

    // Execute and wait for completion
    {
        DWORD ec = 0;
        if (!execute_exe_and_wait(final_path, &ec)) {
            goto cleanup;
        }
        if (out_exit_code) *out_exit_code = ec;
    }

    result = 1;

cleanup:
    if (h != INVALID_HANDLE_VALUE) {
        CloseHandle(h);
        DeleteFileW(temp_path);
        h = INVALID_HANDLE_VALUE;
    }
    // Best-effort cleanup: do not delete final exe or dir after successful execution to allow post-run inspection.
    // On failure, try to remove temp file and directory.
    if (!result) {
        if (wcslen(temp_path) > 0) {
            DeleteFileW(temp_path);
        }
        if (wcslen(base_dir) > 0) {
            // Attempt to remove directory if empty
            RemoveDirectoryW(base_dir);
        }
    }
    curl_global_cleanup();
    return result;
}

int main(void) {
    // Five test cases (URLs are examples; downloads may fail if unreachable)
    const char* tests[5] = {
        "https://example.com/sample1.exe",
        "https://example.com/path/to/vendor-setup.exe",
        "https://download.microsoft.com/nonexistent.exe",
        "https://github.com/user/repo/releases/download/v1.0/app.exe",
        "https://dl.google.com/someapp/installer.exe"
    };

    for (int i = 0; i < 5; ++i) {
        printf("Test %d: URL=%s\n", i + 1, tests[i]);
        DWORD exit_code = 0;
        int ok = download_and_execute(tests[i], &exit_code);
        if (ok) {
            printf("  Success. Executed. Exit code: %" PRIu32 "\n", exit_code);
        } else {
            printf("  Failed to download and/or execute.\n");
        }
    }
    return 0;
}

#else

#include <stdio.h>
int main(void) {
    // This program is intended for Windows environments.
    printf("Unsupported OS for EXE download/execute demo.\n");
    return 0;
}

#endif