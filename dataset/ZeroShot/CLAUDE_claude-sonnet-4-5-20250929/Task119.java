
import java.io.*;
import java.util.*;

public class Task119 {
    public static List<List<String>> parseCSV(String csvContent) {
        List<List<String>> records = new ArrayList<>();
        if (csvContent == null || csvContent.isEmpty()) {
            return records;
        }
        
        BufferedReader reader = new BufferedReader(new StringReader(csvContent));
        String line;
        
        try {
            while ((line = reader.readLine()) != null) {
                List<String> record = parseLine(line);
                records.add(record);
            }
        } catch (IOException e) {
            System.err.println("Error reading CSV: " + e.getMessage());
        }
        
        return records;
    }
    
    private static List<String> parseLine(String line) {
        List<String> fields = new ArrayList<>();
        StringBuilder field = new StringBuilder();
        boolean inQuotes = false;
        
        for (int i = 0; i < line.length(); i++) {
            char c = line.charAt(i);
            
            if (c == '"') {
                if (inQuotes && i + 1 < line.length() && line.charAt(i + 1) == '"') {
                    field.append('"');
                    i++;
                } else {
                    inQuotes = !inQuotes;
                }
            } else if (c == ',' && !inQuotes) {
                fields.add(field.toString());
                field = new StringBuilder();
            } else {
                field.append(c);
            }
        }
        fields.add(field.toString());
        
        return fields;
    }
    
    public static void printCSV(List<List<String>> records) {
        for (List<String> record : records) {
            System.out.println(record);
        }
    }
    
    public static void main(String[] args) {
        // Test Case 1: Simple CSV
        String csv1 = "Name,Age,City\\nJohn,30,New York\\nJane,25,Los Angeles";
        System.out.println("Test Case 1 - Simple CSV:");
        List<List<String>> result1 = parseCSV(csv1);
        printCSV(result1);
        System.out.println();
        
        // Test Case 2: CSV with quoted fields
        String csv2 = "Name,Description\\n\\"John Doe\\",\\"Software Engineer\\"\\n\\"Jane Smith\\",\\"Data Scientist\\"";
        System.out.println("Test Case 2 - CSV with quoted fields:");
        List<List<String>> result2 = parseCSV(csv2);
        printCSV(result2);
        System.out.println();
        
        // Test Case 3: CSV with commas in quoted fields
        String csv3 = "Product,Price,Description\\nLaptop,1200,\\"High performance, 16GB RAM\\"\\nPhone,800,\\"5G enabled, dual camera\\"";
        System.out.println("Test Case 3 - CSV with commas in quoted fields:");
        List<List<String>> result3 = parseCSV(csv3);
        printCSV(result3);
        System.out.println();
        
        // Test Case 4: CSV with escaped quotes
        String csv4 = "Title,Quote\\nBook,\\"He said \\"\\"Hello\\"\\"\\"\\nMovie,\\"She said \\"\\"Goodbye\\"\\"\\"";
        System.out.println("Test Case 4 - CSV with escaped quotes:");
        List<List<String>> result4 = parseCSV(csv4);
        printCSV(result4);
        System.out.println();
        
        // Test Case 5: Empty and single line CSV
        String csv5 = "Header1,Header2,Header3";
        System.out.println("Test Case 5 - Single line CSV:");
        List<List<String>> result5 = parseCSV(csv5);
        printCSV(result5);
    }
}
