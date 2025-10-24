#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

size_t xor_transform_file(const char* input_path, const char* output_path, const unsigned char* key, size_t key_len) {
    if (key == NULL || key_len == 0) {
        return 0; // Key must not be empty
    }
    FILE* in = fopen(input_path, "rb");
    if (!in) return 0;
    FILE* out = fopen(output_path, "wb");
    if (!out) { fclose(in); return 0; }

    unsigned char* buf = (unsigned char*)malloc(65536);
    if (!buf) { fclose(in); fclose(out); return 0; }

    size_t total = 0;
    size_t ki = 0;
    size_t readc;
    while ((readc = fread(buf, 1, 65536, in)) > 0) {
        for (size_t i = 0; i < readc; ++i) {
            buf[i] ^= key[ki];
            ki++;
            if (ki == key_len) ki = 0;
        }
        fwrite(buf, 1, readc, out);
        total += readc;
    }

    free(buf);
    fclose(in);
    fclose(out);
    return total;
}

size_t encrypt_file(const char* input_path, const char* output_path, const unsigned char* key, size_t key_len) {
    return xor_transform_file(input_path, output_path, key, key_len);
}

size_t decrypt_file(const char* input_path, const char* output_path, const unsigned char* key, size_t key_len) {
    return xor_transform_file(input_path, output_path, key, key_len);
}

int write_bytes(const char* path, const unsigned char* data, size_t len) {
    FILE* f = fopen(path, "wb");
    if (!f) return 0;
    size_t w = fwrite(data, 1, len, f);
    fclose(f);
    return w == len;
}

unsigned char* read_bytes(const char* path, size_t* out_len) {
    *out_len = 0;
    FILE* f = fopen(path, "rb");
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    if (sz < 0) { fclose(f); return NULL; }
    fseek(f, 0, SEEK_SET);
    unsigned char* buf = (unsigned char*)malloc((size_t)sz);
    if (!buf) { fclose(f); return NULL; }
    size_t r = fread(buf, 1, (size_t)sz, f);
    fclose(f);
    if (r != (size_t)sz) { free(buf); return NULL; }
    *out_len = (size_t)sz;
    return buf;
}

bool files_equal(const char* a, const char* b) {
    size_t la = 0, lb = 0;
    unsigned char* ba = read_bytes(a, &la);
    unsigned char* bb = read_bytes(b, &lb);
    if (!ba || !bb) { if (ba) free(ba); if (bb) free(bb); return false; }
    if (la != lb) { free(ba); free(bb); return false; }
    bool eq = memcmp(ba, bb, la) == 0;
    free(ba);
    free(bb);
    return eq;
}

int main(void) {
    // Test 1: simple text
    write_bytes("c_in1.txt", (const unsigned char*)"Hello, C!", 9);
    encrypt_file("c_in1.txt", "c_in1.enc", (const unsigned char*)"k1", 2);
    decrypt_file("c_in1.enc", "c_in1.dec", (const unsigned char*)"k1", 2);
    printf("Test1 OK: %s\n", files_equal("c_in1.txt", "c_in1.dec") ? "true" : "false");

    // Test 2: UTF-8 text
    const char* t2 = "Hola, mundo 🌎";
    write_bytes("c_in2.txt", (const unsigned char*)t2, strlen(t2));
    const char* key2 = "llave";
    encrypt_file("c_in2.txt", "c_in2.enc", (const unsigned char*)key2, strlen(key2));
    decrypt_file("c_in2.enc", "c_in2.dec", (const unsigned char*)key2, strlen(key2));
    printf("Test2 OK: %s\n", files_equal("c_in2.txt", "c_in2.dec") ? "true" : "false");

    // Test 3: larger content
    {
        const char* line = "Line - C streaming test.\n";
        size_t len = strlen(line);
        FILE* f = fopen("c_in3.txt", "wb");
        for (int i = 0; i < 20000; ++i) fwrite(line, 1, len, f);
        fclose(f);
    }
    const char* key3 = "long_key_for_c_12345";
    encrypt_file("c_in3.txt", "c_in3.enc", (const unsigned char*)key3, strlen(key3));
    decrypt_file("c_in3.enc", "c_in3.dec", (const unsigned char*)key3, strlen(key3));
    printf("Test3 OK: %s\n", files_equal("c_in3.txt", "c_in3.dec") ? "true" : "false");

    // Test 4: binary data
    {
        unsigned char bin[4096];
        for (size_t i = 0; i < sizeof(bin); ++i) bin[i] = (unsigned char)(i & 0xFF);
        write_bytes("c_in4.bin", bin, sizeof(bin));
    }
    const unsigned char key4[] = {0x00, 0xFF, 0x10, 0x20, 'd', 'e', 'l', 't', 'a'};
    encrypt_file("c_in4.bin", "c_in4.enc", key4, sizeof(key4));
    decrypt_file("c_in4.enc", "c_in4.dec", key4, sizeof(key4));
    printf("Test4 OK: %s\n", files_equal("c_in4.bin", "c_in4.dec") ? "true" : "false");

    // Test 5: empty file
    write_bytes("c_in5.empty", (const unsigned char*)"", 0);
    const char* key5 = "empty";
    encrypt_file("c_in5.empty", "c_in5.enc", (const unsigned char*)key5, strlen(key5));
    decrypt_file("c_in5.enc", "c_in5.dec", (const unsigned char*)key5, strlen(key5));
    printf("Test5 OK: %s\n", files_equal("c_in5.empty", "c_in5.dec") ? "true" : "false");

    return 0;
}