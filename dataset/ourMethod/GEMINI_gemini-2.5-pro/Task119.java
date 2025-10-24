import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.ArrayList;
import java.util.List;

public class Task119 {

    /**
     * Parses a string in CSV format into a list of lists of strings.
     * This implementation handles quoted fields containing delimiters and newlines,
     * and escaped quotes ("").
     *
     * @param content The string content in CSV format.
     * @return A List of Lists of Strings representing the parsed CSV data.
     */
    public static List<List<String>> parseCsv(String content) {
        if (content == null) {
            return new ArrayList<>();
        }

        List<List<String>> records = new ArrayList<>();
        if (content.isEmpty()) {
            return records;
        }
        
        List<String> currentRecord = new ArrayList<>();
        StringBuilder currentField = new StringBuilder();
        boolean inQuotes = false;

        for (int i = 0; i < content.length(); i++) {
            char c = content.charAt(i);

            if (inQuotes) {
                if (c == '"') {
                    // Check for escaped quote ""
                    if (i + 1 < content.length() && content.charAt(i + 1) == '"') {
                        currentField.append('"');
                        i++; // Skip the next quote
                    } else {
                        inQuotes = false;
                    }
                } else {
                    currentField.append(c);
                }
            } else {
                switch (c) {
                    case '"':
                        // Only start quotes if the field is empty
                        if (currentField.length() == 0) {
                            inQuotes = true;
                        } else {
                            currentField.append(c); // Treat as a normal character
                        }
                        break;
                    case ',':
                        currentRecord.add(currentField.toString());
                        currentField.setLength(0);
                        break;
                    case '\n':
                    case '\r':
                        // Handle CRLF and LF line endings
                        if (c == '\r' && i + 1 < content.length() && content.charAt(i + 1) == '\n') {
                            i++; // Skip the LF
                        }
                        currentRecord.add(currentField.toString());
                        records.add(new ArrayList<>(currentRecord));
                        currentRecord.clear();
                        currentField.setLength(0);
                        break;
                    default:
                        currentField.append(c);
                        break;
                }
            }
        }
        // Add the last record if the file does not end with a newline
        if (!currentRecord.isEmpty() || currentField.length() > 0) {
            currentRecord.add(currentField.toString());
            records.add(new ArrayList<>(currentRecord));
        }

        return records;
    }

    /**
     * Reads and parses a CSV file from the given path.
     *
     * @param filePath The path to the CSV file.
     * @return A List of Lists of Strings representing the parsed CSV data.
     * @throws IOException if an I/O error occurs.
     * @throws SecurityException if path traversal is detected.
     */
    public static List<List<String>> readAndParseCsv(String filePath) throws IOException {
        if (filePath == null) {
            throw new IllegalArgumentException("File path cannot be null.");
        }
        Path path = Paths.get(filePath);

        // Security check: Normalize path and ensure it's within a safe directory
        Path normalizedPath = path.toAbsolutePath().normalize();
        Path workingDir = Paths.get(".").toAbsolutePath().normalize();

        if (!normalizedPath.startsWith(workingDir)) {
            throw new SecurityException("Access denied. Path is outside the working directory.");
        }

        if (!Files.isRegularFile(normalizedPath) || !Files.isReadable(normalizedPath)) {
            throw new IOException("File is not a readable regular file: " + normalizedPath);
        }

        String content;
        // Use try-with-resources to ensure the reader is closed automatically
        try (BufferedReader br = Files.newBufferedReader(normalizedPath, StandardCharsets.UTF_8)) {
            StringBuilder contentBuilder = new StringBuilder();
            char[] buffer = new char[4096];
            int charsRead;
            while ((charsRead = br.read(buffer)) != -1) {
                contentBuilder.append(buffer, 0, charsRead);
            }
            content = contentBuilder.toString();
        } catch (IOException e) {
            System.err.println("Error reading file: " + e.getMessage());
            throw e;
        }

        return parseCsv(content);
    }

    public static void main(String[] args) {
        String[] testCsvData = {
            "a,b,c\n1,2,3",
            "a,\"b,c\",d\n1,\"2,3\",4",
            "a,\"b\"\"c\",d\n1,\"2\"\"3\",4",
            "a,,c\n1,2,",
            "header1,header2\n\"field with\nnew line\",field2"
        };

        for (int i = 0; i < testCsvData.length; i++) {
            File tempFile = null;
            try {
                tempFile = File.createTempFile("test" + (i + 1), ".csv");
                try (BufferedWriter writer = Files.newBufferedWriter(tempFile.toPath(), StandardCharsets.UTF_8)) {
                    writer.write(testCsvData[i]);
                }

                System.out.println("--- Test Case " + (i + 1) + " ---");
                System.out.println("Input CSV:\n" + testCsvData[i]);
                System.out.println("File Path: " + tempFile.getAbsolutePath());
                List<List<String>> result = readAndParseCsv(tempFile.getAbsolutePath());
                System.out.println("Parsed Output:");
                result.forEach(row -> System.out.println("  " + row));
                System.out.println();

            } catch (IOException | SecurityException | IllegalArgumentException e) {
                System.err.println("Test Case " + (i + 1) + " failed: " + e.getMessage());
            } finally {
                if (tempFile != null && tempFile.exists()) {
                    if (!tempFile.delete()) {
                        System.err.println("Failed to delete temp file: " + tempFile.getAbsolutePath());
                    }
                }
            }
        }
    }
}