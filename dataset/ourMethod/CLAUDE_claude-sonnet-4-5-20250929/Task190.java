
import java.io.*;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;
import java.util.*;

public class Task190 {
    private static final String BASE_DIR = ".";
    
    public static String transposeFile(String filename) {
        if (filename == null || filename.trim().isEmpty()) {
            throw new IllegalArgumentException("Filename cannot be null or empty");
        }
        
        Path basePath = Paths.get(BASE_DIR).toAbsolutePath().normalize();
        Path filePath = basePath.resolve(filename).normalize();
        
        if (!filePath.startsWith(basePath)) {
            throw new SecurityException("Path traversal attempt detected");
        }
        
        if (!Files.exists(filePath)) {
            throw new IllegalArgumentException("File does not exist: " + filename);
        }
        
        if (!Files.isRegularFile(filePath)) {
            throw new IllegalArgumentException("Path is not a regular file: " + filename);
        }
        
        List<String[]> rows = new ArrayList<>();
        int maxCols = 0;
        
        try (BufferedReader reader = Files.newBufferedReader(filePath, StandardCharsets.UTF_8)) {
            String line;
            while ((line = reader.readLine()) != null) {
                if (line.isEmpty()) {
                    continue;
                }
                String[] fields = line.split(" ", -1);
                rows.add(fields);
                maxCols = Math.max(maxCols, fields.length);
            }
        } catch (IOException e) {
            throw new RuntimeException("Error reading file: " + e.getMessage(), e);
        }
        
        if (rows.isEmpty()) {
            return "";
        }
        
        StringBuilder result = new StringBuilder();
        for (int col = 0; col < maxCols; col++) {
            for (int row = 0; row < rows.size(); row++) {
                String[] currentRow = rows.get(row);
                if (col < currentRow.length) {
                    result.append(currentRow[col]);
                }
                if (row < rows.size() - 1) {
                    result.append(" ");
                }
            }
            if (col < maxCols - 1) {
                result.append("\\n");
            }
        }
        
        return result.toString();
    }
    
    public static void main(String[] args) {
        try {
            String testFile1 = "test1.txt";
            Files.write(Paths.get(testFile1), "name age\\nalice 21\\nryan 30".getBytes(StandardCharsets.UTF_8));
            System.out.println("Test 1:");
            System.out.println(transposeFile(testFile1));
            System.out.println();
            
            String testFile2 = "test2.txt";
            Files.write(Paths.get(testFile2), "a b c\\nd e f".getBytes(StandardCharsets.UTF_8));
            System.out.println("Test 2:");
            System.out.println(transposeFile(testFile2));
            System.out.println();
            
            String testFile3 = "test3.txt";
            Files.write(Paths.get(testFile3), "x\\ny\\nz".getBytes(StandardCharsets.UTF_8));
            System.out.println("Test 3:");
            System.out.println(transposeFile(testFile3));
            System.out.println();
            
            String testFile4 = "test4.txt";
            Files.write(Paths.get(testFile4), "one two three four".getBytes(StandardCharsets.UTF_8));
            System.out.println("Test 4:");
            System.out.println(transposeFile(testFile4));
            System.out.println();
            
            String testFile5 = "test5.txt";
            Files.write(Paths.get(testFile5), "hello world\\nfoo bar\\ntest case".getBytes(StandardCharsets.UTF_8));
            System.out.println("Test 5:");
            System.out.println(transposeFile(testFile5));
            
        } catch (Exception e) {
            System.err.println("Error: " + e.getMessage());
        }
    }
}
