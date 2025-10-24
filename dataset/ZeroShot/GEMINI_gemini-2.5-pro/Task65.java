public class Task65 {

    /**
     * Securely reads the content of a file after validating the filename.
     * The validation uses a whitelist to ensure the filename only contains safe characters
     * and does not contain path traversal sequences ("..") or any path separators,
     * restricting access to the current directory.
     *
     * @param filename The name of the file to read.
     * @return The content of the file as a String, or an error message if an issue occurs.
     */
    public static String getFileContent(String filename) {
        if (filename == null || filename.isEmpty()) {
            return "Error: Filename cannot be null or empty.";
        }

        // Security Validation 1: Whitelist for filename characters.
        // This regex allows only alphanumeric characters, dots, underscores, and hyphens.
        // It effectively blocks path separators ('/' and '\') and other dangerous characters.
        if (!filename.matches("^[a-zA-Z0-9._-]+$")) {
            return "Error: Invalid filename format. Path separators and special characters are not allowed.";
        }
        
        // Security Validation 2: Explicitly check for ".." which can bypass some checks.
        if (filename.contains("..")) {
            return "Error: Path traversal sequences ('..') are not allowed.";
        }
        
        try {
            java.io.File file = new java.io.File(filename);

            // Security Validation 3: Check the canonical path to prevent symlink and other attacks.
            // This ensures the resolved path of the file is within the current working directory.
            String canonicalPath = file.getCanonicalPath();
            String currentDirPath = new java.io.File(".").getCanonicalPath();
            
            if (!canonicalPath.startsWith(currentDirPath)) {
                return "Error: File access is restricted to the current directory.";
            }

            // Check for file existence and readability.
            if (!file.exists()) {
                return "Error: File does not exist.";
            }
            if (!file.isFile()) {
                return "Error: Path does not point to a regular file.";
            }
            if (!file.canRead()) {
                return "Error: File is not readable (permission denied).";
            }

            return new String(java.nio.file.Files.readAllBytes(file.toPath()));
            
        } catch (java.io.IOException e) {
            return "Error: An I/O error occurred while reading the file: " + e.getMessage();
        } catch (SecurityException e) {
            return "Error: Do not have permission to access the file: " + e.getMessage();
        }
    }

    public static void main(String[] args) {
        // --- Test Setup ---
        // To run these tests, this program will attempt to create a file named "test1.txt"
        // in the current directory.
        try {
            java.io.File testFile = new java.io.File("test1.txt");
            if (testFile.createNewFile() || testFile.exists()) {
                 try (java.io.PrintWriter writer = new java.io.PrintWriter(testFile)) {
                    writer.println("This is a secure file.");
                }
            }
        } catch (java.io.IOException e) {
            System.out.println("Warning: Could not set up test file 'test1.txt': " + e.getMessage());
        }

        String[] testCases = {
            "test1.txt",              // 1. Valid file
            "non_existent.txt",       // 2. Non-existent file
            "../other_dir/secret.txt",// 3. Path traversal attempt
            "safe_dir/test2.txt",     // 4. Filename with directory separator
            "/etc/passwd"             // 5. Absolute path attempt
        };

        for (int i = 0; i < testCases.length; i++) {
            System.out.println("--- Test Case " + (i + 1) + ": \"" + testCases[i] + "\" ---");
            String result = getFileContent(testCases[i]);
            System.out.println("Result:\n" + result);
            System.out.println();
        }
    }
}