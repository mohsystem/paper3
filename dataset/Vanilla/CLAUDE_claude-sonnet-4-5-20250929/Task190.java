
import java.io.*;
import java.util.*;

public class Task190 {
    public static List<String> transpose(String filename) {
        List<String> result = new ArrayList<>();
        try {
            BufferedReader br = new BufferedReader(new FileReader(filename));
            List<String[]> rows = new ArrayList<>();
            String line;
            int maxCols = 0;
            
            while ((line = br.readLine()) != null) {
                String[] cols = line.split(" ");
                rows.add(cols);
                maxCols = Math.max(maxCols, cols.length);
            }
            br.close();
            
            for (int col = 0; col < maxCols; col++) {
                StringBuilder sb = new StringBuilder();
                for (int row = 0; row < rows.size(); row++) {
                    if (col < rows.get(row).length) {
                        if (sb.length() > 0) sb.append(" ");
                        sb.append(rows.get(row)[col]);
                    }
                }
                result.add(sb.toString());
            }
        } catch (IOException e) {
            e.printStackTrace();
        }
        return result;
    }
    
    public static void main(String[] args) {
        // Test case 1
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
        
        // Test case 2
        try {
            PrintWriter writer = new PrintWriter("test2.txt");
            writer.println("a b c");
            writer.println("d e f");
            writer.println("g h i");
            writer.close();
            System.out.println("Test 2:");
            transpose("test2.txt").forEach(System.out::println);
            System.out.println();
        } catch (Exception e) {
            e.printStackTrace();
        }
        
        // Test case 3
        try {
            PrintWriter writer = new PrintWriter("test3.txt");
            writer.println("1 2");
            writer.println("3 4");
            writer.close();
            System.out.println("Test 3:");
            transpose("test3.txt").forEach(System.out::println);
            System.out.println();
        } catch (Exception e) {
            e.printStackTrace();
        }
        
        // Test case 4
        try {
            PrintWriter writer = new PrintWriter("test4.txt");
            writer.println("x");
            writer.println("y");
            writer.println("z");
            writer.close();
            System.out.println("Test 4:");
            transpose("test4.txt").forEach(System.out::println);
            System.out.println();
        } catch (Exception e) {
            e.printStackTrace();
        }
        
        // Test case 5
        try {
            PrintWriter writer = new PrintWriter("test5.txt");
            writer.println("first second third fourth");
            writer.println("1 2 3 4");
            writer.close();
            System.out.println("Test 5:");
            transpose("test5.txt").forEach(System.out::println);
            System.out.println();
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}
