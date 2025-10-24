
import java.io.*;
import java.util.*;

public class Task93 {
    public static List<Map.Entry<String, String>> readAndSortFile(String filename) {
        List<Map.Entry<String, String>> records = new ArrayList<>();
        
        try (BufferedReader br = new BufferedReader(new FileReader(filename))) {
            String line;
            while ((line = br.readLine()) != null) {
                line = line.trim();
                if (line.isEmpty() || !line.contains("=")) {
                    continue;
                }
                
                String[] parts = line.split("=", 2);
                if (parts.length == 2) {
                    String key = parts[0].trim();
                    String value = parts[1].trim();
                    records.add(new AbstractMap.SimpleEntry<>(key, value));
                }
            }
        } catch (IOException e) {
            System.err.println("Error reading file: " + e.getMessage());
            return records;
        }
        
        // Sort by key
        records.sort(Map.Entry.comparingByKey());
        
        return records;
    }
    
    public static void main(String[] args) {
        // Test case 1
        System.out.println("Test Case 1:");
        try (PrintWriter writer = new PrintWriter("test1.txt")) {
            writer.println("name=John");
            writer.println("age=30");
            writer.println("city=NewYork");
        } catch (IOException e) {
            e.printStackTrace();
        }
        List<Map.Entry<String, String>> result1 = readAndSortFile("test1.txt");
        for (Map.Entry<String, String> entry : result1) {
            System.out.println(entry.getKey() + "=" + entry.getValue());
        }
        
        // Test case 2
        System.out.println("\\nTest Case 2:");
        try (PrintWriter writer = new PrintWriter("test2.txt")) {
            writer.println("zebra=animal");
            writer.println("apple=fruit");
            writer.println("car=vehicle");
        } catch (IOException e) {
            e.printStackTrace();
        }
        List<Map.Entry<String, String>> result2 = readAndSortFile("test2.txt");
        for (Map.Entry<String, String> entry : result2) {
            System.out.println(entry.getKey() + "=" + entry.getValue());
        }
        
        // Test case 3
        System.out.println("\\nTest Case 3:");
        try (PrintWriter writer = new PrintWriter("test3.txt")) {
            writer.println("key3=value3");
            writer.println("key1=value1");
            writer.println("key2=value2");
        } catch (IOException e) {
            e.printStackTrace();
        }
        List<Map.Entry<String, String>> result3 = readAndSortFile("test3.txt");
        for (Map.Entry<String, String> entry : result3) {
            System.out.println(entry.getKey() + "=" + entry.getValue());
        }
        
        // Test case 4 - Empty file
        System.out.println("\\nTest Case 4:");
        try (PrintWriter writer = new PrintWriter("test4.txt")) {
            // Empty file
        } catch (IOException e) {
            e.printStackTrace();
        }
        List<Map.Entry<String, String>> result4 = readAndSortFile("test4.txt");
        System.out.println("Records found: " + result4.size());
        
        // Test case 5 - File with special characters
        System.out.println("\\nTest Case 5:");
        try (PrintWriter writer = new PrintWriter("test5.txt")) {
            writer.println("user.name=admin");
            writer.println("server.port=8080");
            writer.println("app.version=1.0");
        } catch (IOException e) {
            e.printStackTrace();
        }
        List<Map.Entry<String, String>> result5 = readAndSortFile("test5.txt");
        for (Map.Entry<String, String> entry : result5) {
            System.out.println(entry.getKey() + "=" + entry.getValue());
        }
    }
}
