
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

#define MAX_FILE_SIZE (10 * 1024 * 1024)
#define MAX_ROWS 100000
#define MAX_FIELD_LENGTH 1024
#define MAX_FIELDS 100

typedef struct {
    char** fields;
    int fieldCount;
} CSVRow;

typedef struct {
    CSVRow* rows;
    int rowCount;
} CSVData;

// Function to trim whitespace
char* trim(char* str) {
    if (str == NULL) return NULL;
    
    while (isspace((unsigned char)*str)) str++;
    
    if (*str == 0) return str;
    
    char* end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;
    
    end[1] = '\\0';
    return str;
}

// Function to sanitize field
void sanitizeField(char* field) {
    if (field == NULL || strlen(field) == 0) return;
    
    char* trimmed = trim(field);
    memmove(field, trimmed, strlen(trimmed) + 1);
    
    // Prevent formula injection
    if (field[0] == '=' || field[0] == '+' || 
        field[0] == '-' || field[0] == '@') {
        memmove(field + 1, field, strlen(field) + 1);
        field[0] = '\\'';
    }
}

// Function to validate file path
bool isValidFilePath(const char* filePath) {
    if (filePath == NULL) return false;
    
    // Check for path traversal attempts
    if (strstr(filePath, "..") != NULL) return false;
    if (strstr(filePath, "~") != NULL) return false;
    
    // Check file extension
    size_t len = strlen(filePath);
    if (len < 4 || strcmp(filePath + len - 4, ".csv") != 0) {
        return false;
    }
    
    return true;
}

// Function to get file size
long getFileSize(FILE* file) {
    fseek(file, 0L, SEEK_END);
    long size = ftell(file);
    fseek(file, 0L, SEEK_SET);
    return size;
}

// Function to parse a CSV line
CSVRow parseLine(const char* line) {
    CSVRow row;
    row.fields = (char**)malloc(MAX_FIELDS * sizeof(char*));
    row.fieldCount = 0;
    
    if (line == NULL || strlen(line) == 0) {
        return row;
    }
    
    bool inQuotes = false;
    char currentField[MAX_FIELD_LENGTH] = {0};
    int fieldIndex = 0;
    
    for (size_t i = 0; i < strlen(line); i++) {
        char c = line[i];
        
        if (c == '"') {
            if (inQuotes && i + 1 < strlen(line) && line[i + 1] == '"') {
                currentField[fieldIndex++] = '"';
                i++;
            } else {
                inQuotes = !inQuotes;
            }
        } else if (c == ',' && !inQuotes) {
            currentField[fieldIndex] = '\\0';
            sanitizeField(currentField);
            row.fields[row.fieldCount] = (char*)malloc(strlen(currentField) + 1);
            strcpy(row.fields[row.fieldCount], currentField);
            row.fieldCount++;
            fieldIndex = 0;
            memset(currentField, 0, MAX_FIELD_LENGTH);
        } else {
            if (fieldIndex < MAX_FIELD_LENGTH - 1) {
                currentField[fieldIndex++] = c;
            }
        }
    }
    
    currentField[fieldIndex] = '\\0';
    sanitizeField(currentField);
    row.fields[row.fieldCount] = (char*)malloc(strlen(currentField) + 1);
    strcpy(row.fields[row.fieldCount], currentField);
    row.fieldCount++;
    
    return row;
}

// Function to parse CSV file
CSVData parseCSV(const char* filePath) {
    CSVData data;
    data.rows = NULL;
    data.rowCount = 0;
    
    if (!isValidFilePath(filePath)) {
        fprintf(stderr, "Error: Invalid file path\\n");
        return data;
    }
    
    FILE* file = fopen(filePath, "r");
    if (file == NULL) {
        fprintf(stderr, "Error: Cannot open file\\n");
        return data;
    }
    
    long fileSize = getFileSize(file);
    if (fileSize > MAX_FILE_SIZE) {
        fprintf(stderr, "Error: File size exceeds maximum allowed size\\n");
        fclose(file);
        return data;
    }
    
    data.rows = (CSVRow*)malloc(MAX_ROWS * sizeof(CSVRow));
    
    char line[MAX_FIELD_LENGTH * MAX_FIELDS];
    while (fgets(line, sizeof(line), file) != NULL && data.rowCount < MAX_ROWS) {
        // Remove newline
        line[strcspn(line, "\\r\\n")] = 0;
        
        CSVRow row = parseLine(line);
        data.rows[data.rowCount++] = row;
    }
    
    fclose(file);
    return data;
}

// Function to escape field for writing
void escapeField(const char* field, char* output) {
    bool needsQuoting = false;
    
    if (strchr(field, ',') != NULL || strchr(field, '"') != NULL || 
        strchr(field, '\\n') != NULL) {
        needsQuoting = true;
    }
    
    if (needsQuoting) {
        output[0] = '"';
        int outIdx = 1;
        
        for (size_t i = 0; i < strlen(field); i++) {
            if (field[i] == '"') {
                output[outIdx++] = '"';
                output[outIdx++] = '"';
            } else {
                output[outIdx++] = field[i];
            }
        }
        
        output[outIdx++] = '"';
        output[outIdx] = '\\0';
    } else {
        strcpy(output, field);
    }
}

// Function to write CSV file
void writeCSV(const char* filePath, CSVData* data) {
    if (!isValidFilePath(filePath)) {
        fprintf(stderr, "Error: Invalid file path\\n");
        return;
    }
    
    if (data == NULL || data->rowCount == 0) {
        fprintf(stderr, "Error: No data to write\\n");
        return;
    }
    
    FILE* file = fopen(filePath, "w");
    if (file == NULL) {
        fprintf(stderr, "Error: Cannot open file for writing\\n");
        return;
    }
    
    for (int i = 0; i < data->rowCount; i++) {
        CSVRow row = data->rows[i];
        for (int j = 0; j < row.fieldCount; j++) {
            char escaped[MAX_FIELD_LENGTH * 2];
            escapeField(row.fields[j], escaped);
            fprintf(file, "%s", escaped);
            
            if (j < row.fieldCount - 1) {
                fprintf(file, ",");
            }
        }
        fprintf(file, "\\n");
    }
    
    fclose(file);
}

// Function to free CSV data
void freeCSVData(CSVData* data) {
    if (data == NULL) return;
    
    for (int i = 0; i < data->rowCount; i++) {
        for (int j = 0; j < data->rows[i].fieldCount; j++) {
            free(data->rows[i].fields[j]);
        }
        free(data->rows[i].fields);
    }
    free(data->rows);
}

// Function to print records
void printRecords(CSVData* data) {
    for (int i = 0; i < data->rowCount; i++) {
        printf("[");
        for (int j = 0; j < data->rows[i].fieldCount; j++) {
            printf("%s", data->rows[i].fields[j]);
            if (j < data->rows[i].fieldCount - 1) {
                printf(", ");
            }
        }
        printf("]\\n");
    }
}

int main() {
    // Test case 1: Simple CSV
    CSVData testData1;
    testData1.rowCount = 3;
    testData1.rows = (CSVRow*)malloc(3 * sizeof(CSVRow));
    
    testData1.rows[0].fieldCount = 3;
    testData1.rows[0].fields = (char**)malloc(3 * sizeof(char*));
    testData1.rows[0].fields[0] = strdup("Name");
    testData1.rows[0].fields[1] = strdup("Age");
    testData1.rows[0].fields[2] = strdup("City");
    
    testData1.rows[1].fieldCount = 3;
    testData1.rows[1].fields = (char**)malloc(3 * sizeof(char*));
    testData1.rows[1].fields[0] = strdup("John Doe");
    testData1.rows[1].fields[1] = strdup("30");
    testData1.rows[1].fields[2] = strdup("New York");
    
    testData1.rows[2].fieldCount = 3;
    testData1.rows[2].fields = (char**)malloc(3 * sizeof(char*));
    testData1.rows[2].fields[0] = strdup("Jane Smith");
    testData1.rows[2].fields[1] = strdup("25");
    testData1.rows[2].fields[2] = strdup("Los Angeles");
    
    writeCSV("test1.csv", &testData1);
    CSVData result1 = parseCSV("test1.csv");
    printf("Test 1 - Simple CSV:\\n");
    printRecords(&result1);
    
    freeCSVData(&testData1);
    freeCSVData(&result1);
    
    printf("\\nAdditional test cases can be added similarly\\n");
    
    return 0;
}
