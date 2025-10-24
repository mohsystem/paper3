#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <curl/curl.h>

static const char* basename_from_path(const char* path) {
    const char* p = strrchr(path, '/');
    return p ? (p + 1) : path;
}

int downloadFtpFile(const char* host, int port, const char* user, const char* pass, const char* remotePath) {
    if (!host || !user || !pass || !remotePath) return 0;

    const char* base = basename_from_path(remotePath);
    if (!base || base[0] == '\0') return 0;

    char* url = NULL;
    {
        size_t sz = strlen(host) + strlen(remotePath) + 32;
        url = (char*)malloc(sz);
        if (!url) return 0;
        snprintf(url, sz, "ftp://%s:%d/%s", host, port, (remotePath[0] == '/') ? remotePath + 1 : remotePath);
    }

    FILE* fp = fopen(base, "wb");
    if (!fp) {
        free(url);
        return 0;
    }

    CURL* curl = curl_easy_init();
    if (!curl) {
        fclose(fp);
        free(url);
        return 0;
    }

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_USERNAME, user);
    curl_easy_setopt(curl, CURLOPT_PASSWORD, pass);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 30L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 0L);
    curl_easy_setopt(curl, CURLOPT_TRANSFERTEXT, 0L);
    curl_easy_setopt(curl, CURLOPT_FTP_USE_EPSV, 1L);

    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    fclose(fp);
    free(url);

    if (res != CURLE_OK) {
        remove(base);
        return 0;
    }
    return 1;
}

int main(int argc, char** argv) {
    if (argc >= 5) {
        const char* host = argv[1];
        const char* user = argv[2];
        const char* pass = argv[3];
        const char* remote = argv[4];
        int port = 21;
        if (argc >= 6) {
            port = atoi(argv[5]);
            if (port <= 0) port = 21;
        }
        int ok = downloadFtpFile(host, port, user, pass, remote);
        if (ok) {
            printf("Downloaded: %s\n", basename_from_path(remote));
        } else {
            printf("Download failed\n");
        }
    } else {
        // 5 test cases using public test FTP server
        const char* host = "test.rebex.net";
        const char* user = "demo";
        const char* pass = "password";
        int port = 21;
        const char* files[5] = {
            "pub/example/readme.txt",
            "pub/example/ConsoleClient.png",
            "pub/example/KeyGenerator.png",
            "pub/example/WinFormClient.png",
            "pub/example/WinSshShell.png"
        };
        for (int i = 0; i < 5; ++i) {
            int ok = downloadFtpFile(host, port, user, pass, files[i]);
            printf("%s: %s\n", ok ? "Downloaded" : "Failed", files[i]);
        }
    }
    return 0;
}