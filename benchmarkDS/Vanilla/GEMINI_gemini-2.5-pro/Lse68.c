#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

// --- Base64 Encoding Function ---
static char encoding_table[] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
                                'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
                                'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
                                'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
                                'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
                                'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
                                'w', 'x', 'y', 'z', '0', '1', '2', '3',
                                '4', '5', '6', '7', '8', '9', '+', '/'};

char *base64_encode(const unsigned char *data, size_t input_length, size_t *output_length) {
    *output_length = 4 * ((input_length + 2) / 3);
    char *encoded_data = (char*)malloc(*output_length + 1);
    if (encoded_data == NULL) return NULL;

    for (size_t i = 0, j = 0; i < input_length;) {
        uint32_t octet_a = i < input_length ? data[i++] : 0;
        uint32_t octet_b = i < input_length ? data[i++] : 0;
        uint32_t octet_c = i < input_length ? data[i++] : 0;
        uint32_t triple = (octet_a << 16) + (octet_b << 8) + octet_c;
        encoded_data[j++] = encoding_table[(triple >> 18) & 0x3F];
        encoded_data[j++] = encoding_table[(triple >> 12) & 0x3F];
        encoded_data[j++] = encoding_table[(triple >> 6) & 0x3F];
        encoded_data[j++] = encoding_table[triple & 0x3F];
    }

    int mod_table[] = {0, 2, 1};
    for (int i = 0; i < mod_table[input_length % 3]; i++) {
        encoded_data[*output_length - 1 - i] = '=';
    }
    encoded_data[*output_length] = '\0';
    return encoded_data;
}

// --- Database Simulation ---
typedef struct {
    char* name;
    char* base64_data;
} ImageEntry;

ImageEntry* imageDatabase = NULL;
size_t db_size = 0;

void cleanup_database() {
    if (imageDatabase != NULL) {
        for (size_t i = 0; i < db_size; i++) {
            free(imageDatabase[i].name);
            free(imageDatabase[i].base64_data);
        }
        free(imageDatabase);
        imageDatabase = NULL;
        db_size = 0;
    }
}

void db_insert(const char* name, char* base64_data) {
    for (size_t i = 0; i < db_size; i++) {
        if (strcmp(imageDatabase[i].name, name) == 0) {
            free(imageDatabase[i].base64_data);
            imageDatabase[i].base64_data = base64_data;
            printf("Successfully updated '%s'.\n", name);
            return;
        }
    }

    ImageEntry* new_db = (ImageEntry*)realloc(imageDatabase, (db_size + 1) * sizeof(ImageEntry));
    if (new_db == NULL) {
        fprintf(stderr, "Failed to allocate database memory.\n");
        free(base64_data);
        return;
    }
    imageDatabase = new_db;
    
    imageDatabase[db_size].name = (char*)malloc(strlen(name) + 1);
    strcpy(imageDatabase[db_size].name, name);
    imageDatabase[db_size].base64_data = base64_data;
    db_size++;
    printf("Successfully stored '%s'.\n", name);
}

/**
 * Simulates reading image data, converting it to base64, and inserting it into a database.
 * @param imageName The name of the image file.
 * @param imageData The raw byte data of the image.
 * @param dataSize The size of the image data in bytes.
 */
void upload_image(const char* imageName, const unsigned char* imageData, size_t dataSize) {
    if (imageName == NULL || imageName[0] == '\0' || imageData == NULL) {
        printf("Error: Image name and data cannot be null or empty.\n");
        return;
    }
    size_t output_len;
    char* base64ImageString = base64_encode(imageData, dataSize, &output_len);
    if (base64ImageString == NULL) {
        fprintf(stderr, "Failed to base64 encode image data.\n");
        return;
    }
    db_insert(imageName, base64ImageString);
}

void displayDatabaseContents() {
    printf("\n--- Simulated Database Contents ---\n");
    if (db_size == 0) {
        printf("Database is empty.\n");
    } else {
        for (size_t i = 0; i < db_size; i++) {
            printf("Image Name: %s\n", imageDatabase[i].name);
            char snippet[31];
            strncpy(snippet, imageDatabase[i].base64_data, 30);
            snippet[30] = '\0';
            printf("Base64 Data (Snippet): %s%s\n", snippet, strlen(imageDatabase[i].base64_data) > 30 ? "..." : "");
            printf("---------------------------------\n");
        }
    }
}

int main() {
    printf("--- Running C Test Cases ---\n");

    // Test Case 1: A simple text as image data
    const char* tc1_str = "This is my profile picture content.";
    upload_image("profile_pic.jpg", (const unsigned char*)tc1_str, strlen(tc1_str));

    // Test Case 2: Another text file
    const char* tc2_str = "Important document data here.";
    upload_image("document.png", (const unsigned char*)tc2_str, strlen(tc2_str));

    // Test Case 3: Empty image data
    upload_image("empty_image.gif", (const unsigned char*)"", 0);

    // Test Case 4: Image with special characters
    const char* tc4_str = "SVG content with <tags> & symbols!";
    upload_image("art-@#$.svg", (const unsigned char*)tc4_str, strlen(tc4_str));
    
    // Test Case 5: Overwriting an existing image
    const char* tc5_str = "This is my NEW profile picture.";
    upload_image("profile_pic.jpg", (const unsigned char*)tc5_str, strlen(tc5_str));

    displayDatabaseContents();
    cleanup_database();
    return 0;
}