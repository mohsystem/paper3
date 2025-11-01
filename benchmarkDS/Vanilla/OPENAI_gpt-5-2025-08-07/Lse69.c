#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char B64CHARS[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

char* base64_encode(const unsigned char* data, size_t input_length, size_t* output_length) {
    *output_length = 4 * ((input_length + 2) / 3);
    char* encoded = (char*)malloc(*output_length + 1);
    if (!encoded) return NULL;

    size_t i = 0, j = 0;
    while (i < input_length) {
        uint32_t octet_a = i < input_length ? data[i++] : 0;
        uint32_t octet_b = i < input_length ? data[i++] : 0;
        uint32_t octet_c = i < input_length ? data[i++] : 0;

        uint32_t triple = (octet_a << 16) | (octet_b << 8) | (octet_c);

        encoded[j++] = B64CHARS[(triple >> 18) & 0x3F];
        encoded[j++] = B64CHARS[(triple >> 12) & 0x3F];
        encoded[j++] = (i > input_length + 1) ? '=' : B64CHARS[(triple >> 6) & 0x3F];
        encoded[j++] = (i > input_length) ? '=' : B64CHARS[triple & 0x3F];
    }
    encoded[j] = '\0';
    return encoded;
}

char* upload_image(const char* srcPath, const char* outBase64Path) {
    FILE* in = fopen(srcPath, "rb");
    if (!in) {
        return NULL;
    }
    fseek(in, 0, SEEK_END);
    long len = ftell(in);
    fseek(in, 0, SEEK_SET);
    if (len < 0) {
        fclose(in);
        return NULL;
    }

    unsigned char* buffer = (unsigned char*)malloc((size_t)len);
    if (!buffer) {
        fclose(in);
        return NULL;
    }
    size_t readn = fread(buffer, 1, (size_t)len, in);
    fclose(in);
    if (readn != (size_t)len) {
        free(buffer);
        return NULL;
    }

    size_t out_len = 0;
    char* encoded = base64_encode(buffer, (size_t)len, &out_len);
    free(buffer);
    if (!encoded) {
        return NULL;
    }

    FILE* out = fopen(outBase64Path, "wb");
    if (!out) {
        free(encoded);
        return NULL;
    }
    fwrite(encoded, 1, out_len, out);
    fclose(out);

    return encoded; /* caller must free */
}

int write_bytes(const char* path, const unsigned char* data, size_t len) {
    FILE* f = fopen(path, "wb");
    if (!f) return -1;
    size_t wn = fwrite(data, 1, len, f);
    fclose(f);
    return wn == len ? 0 : -1;
}

int main(void) {
    const char* srcs[5] = {"c_img1.bin","c_img2.bin","c_img3.bin","c_img4.bin","c_img5.bin"};
    const char* outs[5] = {"c_img1.b64","c_img2.b64","c_img3.b64","c_img4.b64","c_img5.b64"};
    const unsigned char s1[] = {0,1,2,3,4,5,6,7,8,9};
    const unsigned char s2[] = {0xFF,0xD8,0xFF,0xC0,0x00,0x11,0x08};
    const unsigned char s3[] = {0x89,0x50,0x4E,0x47,0x0D,0x0A,0x1A,0x0A};
    const unsigned char s4[] = "Hello from C!";
    const unsigned char s5[] = {0x00,0xFF,0xAA,0x55,0xCC,0x33,0x77,0x88,0x99,0x10};

    const unsigned char* samples[5] = {s1, s2, s3, s4, s5};
    const size_t slens[5] = {sizeof(s1), sizeof(s2), sizeof(s3), sizeof("Hello from C!")-1, sizeof(s5)};

    for (int i = 0; i < 5; ++i) {
        if (write_bytes(srcs[i], samples[i], slens[i]) != 0) {
            printf("Failed to write %s\n", srcs[i]);
            continue;
        }
        char* b64 = upload_image(srcs[i], outs[i]);
        if (!b64) {
            printf("Test %d failed: upload_image returned NULL\n", i+1);
            continue;
        }
        size_t len = strlen(b64);
        printf("Test %d: src=%s, out=%s, b64_len=%zu\n", i+1, srcs[i], outs[i], len);
        free(b64);
    }
    return 0;
}