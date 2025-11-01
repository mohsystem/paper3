#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

/**
 * Converts a file mode to a human-readable string (e.g., "-rw-------").
 * @param mode The file mode from stat.
 * @param buf The buffer to store the string (must be at least 11 bytes).
 */
void permissions_to_string(mode_t mode, char *buf) {
    const char *rwx[] = {"---", "--x", "-w-", "-wx", "r--", "r-x", "rw-", "rwx"};
    strcpy(buf, "----------");

    if (S_ISDIR(mode)) buf[0] = 'd';
    if (S_ISLNK(mode)) buf[0] = 'l';

    // Owner, group, and other permissions
    strcat(buf, rwx[(mode >> 6) & 7]);
    strcat(buf, rwx[(mode >> 3) & 7]);
    strcat(buf, rwx[(mode & 7)]);
    // Correctly place the permissions in the string
    buf[1] = rwx[(mode >> 6) & 7][0];
    buf[2] = rwx[(mode >> 6) & 7][1];
    buf[3] = rwx[(mode >> 6) & 7][2];
    buf[4] = rwx[(mode >> 3) & 7][0];
    buf[5] = rwx[(mode >> 3) & 7][1];
    buf[6] = rwx[(mode >> 3) & 7][2];
    buf[7] = rwx[mode & 7][0];
    buf[8] = rwx[mode & 7][1];
    buf[9] = rwx[mode & 7][2];
}

/**
 * Creates a file with specific permissions, writes content, and prints permissions.
 * This is a POSIX-specific implementation.
 * @param filename The name of the file to create.
 * @param content The content to write to the file.
 */
void handleSecretFile(const char* filename, const char* content) {
    printf("Processing file: %s\n", filename);
    int fd = -1;

    // 1 & 2. Create file with restrictive permissions (S_IRUSR | S_IWUSR -> 0600)
    // and open it for writing. This is an atomic and secure operation.
    // The O_TRUNC flag will clear the file if it already exists.
    // Note: The effective permissions are (mode & ~umask). To be certain,
    // one could set umask(0077) before this call.
    mode_t mode = S_IRUSR | S_IWUSR; // 0600 permissions
    fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, mode);

    if (fd == -1) {
        perror("  - Error opening file");
        return;
    }
    
    // Write content to the file
    ssize_t bytes_written = write(fd, content, strlen(content));
    if (bytes_written == -1) {
        perror("  - Error writing to file");
        close(fd);
        return;
    }
    printf("  - Successfully wrote to %s with restrictive permissions.\n", filename);
    close(fd);

    // 3. Print the permissions of the file
    struct stat file_stat;
    if (stat(filename, &file_stat) == -1) {
        perror("  - Error getting file stats");
        return;
    }

    char perm_str[11];
    permissions_to_string(file_stat.st_mode, perm_str);
    printf("  - Final permissions: %s (octal: %o)\n", perm_str, file_stat.st_mode & 0777);
    printf("------------------------------------\n");
}

int main() {
    const char* content = "U2FsdGVkX1/R+WzJcxgvX/Iw==";

    // 5 Test Cases
    handleSecretFile("secret1.txt", content);
    handleSecretFile("secret2.txt", content);
    handleSecretFile("secret3.txt", content);
    handleSecretFile("secret4.txt", content);
    handleSecretFile("secret5.txt", content);

    return 0;
}