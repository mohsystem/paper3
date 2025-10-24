
import java.io.*;
import java.util.*;

public class Task94 {
    public static List<String> readAndSortFile(String filename) {
        List<String> records = new ArrayList<>();
        
        try (BufferedReader br = new BufferedReader(new FileReader(filename))) {
            String line;
            while ((line = br.readLine()) != null) {
                if (!line.trim().isEmpty()) {
                    records.add(line.trim());
                }
            }
        } catch (IOException e) {
            System.err.println("Error reading file: " + e.getMessage());
            return records;
        }
        
        Collections.sort(records);
        return records;
    }
    
    public static void main(String[] args) {
        // Test case 1: File with simple key-value pairs
        try {
            PrintWriter writer = new PrintWriter("test1.txt");
            writer.println("name=John");
            writer.println("age=30");
            writer.println("city=New York");
            writer.close();
            
            System.out.println("Test 1:");
            List<String> result1 = readAndSortFile("test1.txt");
            for (String record : result1) {
                System.out.println(record);
            }
            System.out.println();
        } catch (FileNotFoundException e) {
            e.printStackTrace();
        }
        
        // Test case 2: File with numeric keys
        try {
            PrintWriter writer = new PrintWriter("test2.txt");
            writer.println("3=three");
            writer.println("1=one");
            writer.println("2=two");
            writer.close();
            
            System.out.println("Test 2:");
            List<String> result2 = readAndSortFile("test2.txt");
            for (String record : result2) {
                System.out.println(record);
            }
            System.out.println();
        } catch (FileNotFoundException e) {
            e.printStackTrace();
        }
        
        // Test case 3: File with mixed keys
        try {
            PrintWriter writer = new PrintWriter("test3.txt");
            writer.println("zebra=animal");
            writer.println("apple=fruit");
            writer.println("car=vehicle");
            writer.println("banana=fruit");
            writer.close();
            
            System.out.println("Test 3:");
            List<String> result3 = readAndSortFile("test3.txt");
            for (String record : result3) {
                System.out.println(record);
            }
            System.out.println();
        } catch (FileNotFoundException e) {
            e.printStackTrace();
        }
        
        // Test case 4: Empty file
        try {
            PrintWriter writer = new PrintWriter("test4.txt");
            writer.close();
            
            System.out.println("Test 4 (empty file):");
            List<String> result4 = readAndSortFile("test4.txt");
            System.out.println("Records count: " + result4.size());
            System.out.println();
        } catch (FileNotFoundException e) {
            e.printStackTrace();
        }
        
        // Test case 5: File with duplicate keys
        try {
            PrintWriter writer = new PrintWriter("test5.txt");
            writer.println("key1=value1");
            writer.println("key3=value3");
            writer.println("key1=value2");
            writer.println("key2=value4");
            writer.close();
            
            System.out.println("Test 5:");
            List<String> result5 = readAndSortFile("test5.txt");
            for (String record : result5) {
                System.out.println(record);
            }
            System.out.println();
        } catch (FileNotFoundException e) {
            e.printStackTrace();
        }
    }
}
