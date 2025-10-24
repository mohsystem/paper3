
import java.io.*;
import java.util.*;

public class Task94 {
    public static List<Map.Entry<String, String>> readAndSortFile(String filename) {
        List<Map.Entry<String, String>> records = new ArrayList<>();
        
        try (BufferedReader br = new BufferedReader(new FileReader(filename))) {
            String line;
            while ((line = br.readLine()) != null) {
                line = line.trim();
                if (!line.isEmpty() && line.contains("=")) {
                    String[] parts = line.split("=", 2);
                    if (parts.length == 2) {
                        records.add(new AbstractMap.SimpleEntry<>(parts[0].trim(), parts[1].trim()));
                    }
                }
            }
        } catch (IOException e) {
            System.err.println("Error reading file: " + e.getMessage());
        }
        
        // Sort by key
        records.sort(Map.Entry.comparingByKey());
        return records;
    }
    
    public static void main(String[] args) {
        // Test case 1: Basic key-value pairs
        createTestFile("test1.txt", Arrays.asList("name=John", "age=30", "city=NewYork"));
        System.out.println("Test 1:");
        printRecords(readAndSortFile("test1.txt"));
        
        // Test case 2: Unsorted keys
        createTestFile("test2.txt", Arrays.asList("zebra=animal", "apple=fruit", "car=vehicle"));
        System.out.println("\\nTest 2:");
        printRecords(readAndSortFile("test2.txt"));
        
        // Test case 3: Keys with spaces
        createTestFile("test3.txt", Arrays.asList("first name=Alice", "last name=Smith", "age=25"));
        System.out.println("\\nTest 3:");
        printRecords(readAndSortFile("test3.txt"));
        
        // Test case 4: Duplicate keys
        createTestFile("test4.txt", Arrays.asList("color=red", "color=blue", "shape=circle"));
        System.out.println("\\nTest 4:");
        printRecords(readAndSortFile("test4.txt"));
        
        // Test case 5: Empty and invalid lines
        createTestFile("test5.txt", Arrays.asList("valid=data", "", "invalid_line", "another=valid"));
        System.out.println("\\nTest 5:");
        printRecords(readAndSortFile("test5.txt"));
    }
    
    private static void createTestFile(String filename, List<String> lines) {
        try (PrintWriter writer = new PrintWriter(new FileWriter(filename))) {
            for (String line : lines) {
                writer.println(line);
            }
        } catch (IOException e) {
            System.err.println("Error creating test file: " + e.getMessage());
        }
    }
    
    private static void printRecords(List<Map.Entry<String, String>> records) {
        for (Map.Entry<String, String> entry : records) {
            System.out.println(entry.getKey() + " = " + entry.getValue());
        }
    }
}
