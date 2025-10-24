
import java.io.*;
import java.util.*;

public class Task93 {
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
        // Test case 1: Basic key-value pairs
        try {
            PrintWriter writer = new PrintWriter("test1.txt");
            writer.println("name=John");
            writer.println("age=30");
            writer.println("city=Boston");
            writer.close();
            System.out.println("Test 1:");
            System.out.println(readAndSortFile("test1.txt"));
        } catch (IOException e) {
            e.printStackTrace();
        }
        
        // Test case 2: Numeric keys
        try {
            PrintWriter writer = new PrintWriter("test2.txt");
            writer.println("3=apple");
            writer.println("1=banana");
            writer.println("2=cherry");
            writer.close();
            System.out.println("\\nTest 2:");
            System.out.println(readAndSortFile("test2.txt"));
        } catch (IOException e) {
            e.printStackTrace();
        }
        
        // Test case 3: Mixed case
        try {
            PrintWriter writer = new PrintWriter("test3.txt");
            writer.println("Zebra=animal");
            writer.println("apple=fruit");
            writer.println("Banana=fruit");
            writer.close();
            System.out.println("\\nTest 3:");
            System.out.println(readAndSortFile("test3.txt"));
        } catch (IOException e) {
            e.printStackTrace();
        }
        
        // Test case 4: Empty lines
        try {
            PrintWriter writer = new PrintWriter("test4.txt");
            writer.println("key1=value1");
            writer.println("");
            writer.println("key2=value2");
            writer.println("   ");
            writer.println("key3=value3");
            writer.close();
            System.out.println("\\nTest 4:");
            System.out.println(readAndSortFile("test4.txt"));
        } catch (IOException e) {
            e.printStackTrace();
        }
        
        // Test case 5: Single record
        try {
            PrintWriter writer = new PrintWriter("test5.txt");
            writer.println("onlykey=onlyvalue");
            writer.close();
            System.out.println("\\nTest 5:");
            System.out.println(readAndSortFile("test5.txt"));
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
}
