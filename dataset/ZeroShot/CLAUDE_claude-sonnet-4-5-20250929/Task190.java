
import java.io.*;
import java.util.*;

public class Task190 {
    public static List<String> transposeFile(String filename) {
        List<String> result = new ArrayList<>();
        List<List<String>> rows = new ArrayList<>();
        
        try (BufferedReader br = new BufferedReader(new FileReader(filename))) {
            String line;
            while ((line = br.readLine()) != null) {
                String[] columns = line.split(" ");
                rows.add(Arrays.asList(columns));
            }
            
            if (rows.isEmpty()) {
                return result;
            }
            
            int numCols = rows.get(0).size();
            for (int col = 0; col < numCols; col++) {
                StringBuilder sb = new StringBuilder();
                for (int row = 0; row < rows.size(); row++) {
                    if (row > 0) {
                        sb.append(" ");
                    }
                    sb.append(rows.get(row).get(col));
                }
                result.add(sb.toString());
            }
        } catch (IOException e) {
            e.printStackTrace();
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
            List<String> result = transposeFile("test1.txt");
            for (String line : result) {
                System.out.println(line);
            }
            System.out.println();
        } catch (IOException e) {
            e.printStackTrace();
        }
        
        // Test case 2: Single row
        try {
            PrintWriter writer = new PrintWriter("test2.txt");
            writer.println("a b c d");
            writer.close();
            
            System.out.println("Test 2:");
            List<String> result = transposeFile("test2.txt");
            for (String line : result) {
                System.out.println(line);
            }
            System.out.println();
        } catch (IOException e) {
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
            List<String> result = transposeFile("test3.txt");
            for (String line : result) {
                System.out.println(line);
            }
            System.out.println();
        } catch (IOException e) {
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
            List<String> result = transposeFile("test4.txt");
            for (String line : result) {
                System.out.println(line);
            }
            System.out.println();
        } catch (IOException e) {
            e.printStackTrace();
        }
        
        // Test case 5: Multiple columns
        try {
            PrintWriter writer = new PrintWriter("test5.txt");
            writer.println("first last age city");
            writer.println("john doe 25 NYC");
            writer.println("jane smith 30 LA");
            writer.close();
            
            System.out.println("Test 5:");
            List<String> result = transposeFile("test5.txt");
            for (String line : result) {
                System.out.println(line);
            }
            System.out.println();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
}
