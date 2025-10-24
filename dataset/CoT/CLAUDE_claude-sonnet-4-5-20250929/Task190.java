
import java.io.*;
import java.util.*;

public class Task190 {
    public static List<String> transpose(String filename) {
        List<String> result = new ArrayList<>();
        List<List<String>> matrix = new ArrayList<>();
        
        try (BufferedReader br = new BufferedReader(new FileReader(filename))) {
            String line;
            while ((line = br.readLine()) != null) {
                if (line.trim().isEmpty()) continue;
                String[] parts = line.split(" ");
                List<String> row = new ArrayList<>(Arrays.asList(parts));
                matrix.add(row);
            }
            
            if (matrix.isEmpty()) return result;
            
            int cols = matrix.get(0).size();
            for (int j = 0; j < cols; j++) {
                StringBuilder sb = new StringBuilder();
                for (int i = 0; i < matrix.size(); i++) {
                    if (j < matrix.get(i).size()) {
                        if (sb.length() > 0) sb.append(" ");
                        sb.append(matrix.get(i).get(j));
                    }
                }
                result.add(sb.toString());
            }
        } catch (IOException e) {
            System.err.println("Error reading file: " + e.getMessage());
        }
        
        return result;
    }
    
    public static void main(String[] args) {
        // Test case 1: Basic example
        try {
            PrintWriter writer = new PrintWriter("test1.txt");
            writer.println("name age");
            writer.println("alice 21");
            writer.println("ryan 30");
            writer.close();
            System.out.println("Test 1:");
            transpose("test1.txt").forEach(System.out::println);
            System.out.println();
        } catch (Exception e) {
            e.printStackTrace();
        }
        
        // Test case 2: Single row
        try {
            PrintWriter writer = new PrintWriter("test2.txt");
            writer.println("a b c d");
            writer.close();
            System.out.println("Test 2:");
            transpose("test2.txt").forEach(System.out::println);
            System.out.println();
        } catch (Exception e) {
            e.printStackTrace();
        }
        
        // Test case 3: Single column
        try {
            PrintWriter writer = new PrintWriter("test3.txt");
            writer.println("a");
            writer.println("b");
            writer.println("c");
            writer.close();
            System.out.println("Test 3:");
            transpose("test3.txt").forEach(System.out::println);
            System.out.println();
        } catch (Exception e) {
            e.printStackTrace();
        }
        
        // Test case 4: 3x3 matrix
        try {
            PrintWriter writer = new PrintWriter("test4.txt");
            writer.println("1 2 3");
            writer.println("4 5 6");
            writer.println("7 8 9");
            writer.close();
            System.out.println("Test 4:");
            transpose("test4.txt").forEach(System.out::println);
            System.out.println();
        } catch (Exception e) {
            e.printStackTrace();
        }
        
        // Test case 5: Empty file
        try {
            PrintWriter writer = new PrintWriter("test5.txt");
            writer.close();
            System.out.println("Test 5:");
            List<String> result = transpose("test5.txt");
            if (result.isEmpty()) {
                System.out.println("Empty result");
            }
            System.out.println();
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}
