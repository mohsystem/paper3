
import java.io.*;
import java.nio.file.*;
import java.util.*;
import java.util.regex.*;

public class Task119 {
    private static final int MAX_FILE_SIZE = 10 * 1024 * 1024; // 10MB limit
    private static final int MAX_ROWS = 100000;
    private static final Pattern CSV_PATTERN = Pattern.compile(",(?=(?:[^\\"]*\\"[^\\"]*\\")*[^\\"]*$)");
    
    public static List<List<String>> parseCSV(String filePath) {
        List<List<String>> records = new ArrayList<>();
        
        if (filePath == null || filePath.trim().isEmpty()) {
            throw new IllegalArgumentException("File path cannot be null or empty");
        }
        
        File file = new File(filePath);
        
        // Security: Validate file path to prevent directory traversal
        try {
            String canonicalPath = file.getCanonicalPath();
            if (!canonicalPath.startsWith(new File(".").getCanonicalPath())) {
                throw new SecurityException("Access denied: Invalid file path");
            }
        } catch (IOException e) {
            throw new SecurityException("Cannot validate file path: " + e.getMessage());
        }
        
        // Security: Check file size
        if (file.length() > MAX_FILE_SIZE) {
            throw new IllegalArgumentException("File size exceeds maximum allowed size");
        }
        
        // Security: Validate file extension
        if (!filePath.toLowerCase().endsWith(".csv")) {
            throw new IllegalArgumentException("Only CSV files are allowed");
        }
        
        try (BufferedReader br = new BufferedReader(new FileReader(file))) {
            String line;
            int rowCount = 0;
            
            while ((line = br.readLine()) != null) {
                // Security: Limit number of rows
                if (++rowCount > MAX_ROWS) {
                    throw new IllegalArgumentException("File exceeds maximum row limit");
                }
                
                List<String> record = parseLine(line);
                records.add(record);
            }
        } catch (IOException e) {
            throw new RuntimeException("Error reading CSV file: " + e.getMessage());
        }
        
        return records;
    }
    
    private static List<String> parseLine(String line) {
        List<String> fields = new ArrayList<>();
        
        if (line == null || line.isEmpty()) {
            return fields;
        }
        
        boolean inQuotes = false;
        StringBuilder currentField = new StringBuilder();
        
        for (int i = 0; i < line.length(); i++) {
            char c = line.charAt(i);
            
            if (c == '"') {
                if (inQuotes && i + 1 < line.length() && line.charAt(i + 1) == '"') {
                    currentField.append('"');
                    i++;
                } else {
                    inQuotes = !inQuotes;
                }
            } else if (c == ',' && !inQuotes) {
                fields.add(sanitizeField(currentField.toString()));
                currentField = new StringBuilder();
            } else {
                currentField.append(c);
            }
        }
        
        fields.add(sanitizeField(currentField.toString()));
        return fields;
    }
    
    private static String sanitizeField(String field) {
        if (field == null) {
            return "";
        }
        
        // Remove any potential injection characters
        field = field.trim();
        
        // Security: Prevent formula injection in CSV
        if (field.startsWith("=") || field.startsWith("+") || 
            field.startsWith("-") || field.startsWith("@")) {
            field = "'" + field;
        }
        
        return field;
    }
    
    public static void writeCSV(String filePath, List<List<String>> data) {
        if (filePath == null || filePath.trim().isEmpty()) {
            throw new IllegalArgumentException("File path cannot be null or empty");
        }
        
        if (data == null || data.isEmpty()) {
            throw new IllegalArgumentException("Data cannot be null or empty");
        }
        
        File file = new File(filePath);
        
        // Security: Validate file path
        try {
            String canonicalPath = file.getCanonicalPath();
            if (!canonicalPath.startsWith(new File(".").getCanonicalPath())) {
                throw new SecurityException("Access denied: Invalid file path");
            }
        } catch (IOException e) {
            throw new SecurityException("Cannot validate file path: " + e.getMessage());
        }
        
        try (BufferedWriter bw = new BufferedWriter(new FileWriter(file))) {
            for (List<String> row : data) {
                StringJoiner joiner = new StringJoiner(",");
                for (String field : row) {
                    joiner.add(escapeField(field));
                }
                bw.write(joiner.toString());
                bw.newLine();
            }
        } catch (IOException e) {
            throw new RuntimeException("Error writing CSV file: " + e.getMessage());
        }
    }
    
    private static String escapeField(String field) {
        if (field == null) {
            return "";
        }
        
        if (field.contains(",") || field.contains("\\"") || field.contains("\\n")) {
            field = "\\"" + field.replace("\\"", "\\"\\"") + "\\"";
        }
        
        return field;
    }
    
    public static void main(String[] args) {
        try {
            // Test case 1: Create and parse simple CSV
            List<List<String>> testData1 = Arrays.asList(
                Arrays.asList("Name", "Age", "City"),
                Arrays.asList("John Doe", "30", "New York"),
                Arrays.asList("Jane Smith", "25", "Los Angeles")
            );
            writeCSV("test1.csv", testData1);
            List<List<String>> result1 = parseCSV("test1.csv");
            System.out.println("Test 1 - Simple CSV:");
            printRecords(result1);
            
            // Test case 2: CSV with quoted fields
            List<List<String>> testData2 = Arrays.asList(
                Arrays.asList("Product", "Description", "Price"),
                Arrays.asList("Book", "A great book, highly recommended", "19.99"),
                Arrays.asList("Pen", "Blue ink pen", "2.50")
            );
            writeCSV("test2.csv", testData2);
            List<List<String>> result2 = parseCSV("test2.csv");
            System.out.println("\\nTest 2 - CSV with commas:");
            printRecords(result2);
            
            // Test case 3: CSV with special characters
            List<List<String>> testData3 = Arrays.asList(
                Arrays.asList("ID", "Email", "Status"),
                Arrays.asList("001", "user@example.com", "Active"),
                Arrays.asList("002", "admin@test.org", "Inactive")
            );
            writeCSV("test3.csv", testData3);
            List<List<String>> result3 = parseCSV("test3.csv");
            System.out.println("\\nTest 3 - CSV with special characters:");
            printRecords(result3);
            
            // Test case 4: CSV with empty fields
            List<List<String>> testData4 = Arrays.asList(
                Arrays.asList("Field1", "Field2", "Field3"),
                Arrays.asList("Value1", "", "Value3"),
                Arrays.asList("", "Value2", "")
            );
            writeCSV("test4.csv", testData4);
            List<List<String>> result4 = parseCSV("test4.csv");
            System.out.println("\\nTest 4 - CSV with empty fields:");
            printRecords(result4);
            
            // Test case 5: CSV with numeric data
            List<List<String>> testData5 = Arrays.asList(
                Arrays.asList("Year", "Revenue", "Profit"),
                Arrays.asList("2021", "1000000", "250000"),
                Arrays.asList("2022", "1500000", "400000")
            );
            writeCSV("test5.csv", testData5);
            List<List<String>> result5 = parseCSV("test5.csv");
            System.out.println("\\nTest 5 - CSV with numeric data:");
            printRecords(result5);
            
        } catch (Exception e) {
            System.err.println("Error: " + e.getMessage());
        }
    }
    
    private static void printRecords(List<List<String>> records) {
        for (List<String> record : records) {
            System.out.println(record);
        }
    }
}
