
import java.io.*;
import java.util.*;

public class Task119 {
    public static List<List<String>> parseCSV(String csvContent) {
        List<List<String>> records = new ArrayList<>();
        if (csvContent == null || csvContent.isEmpty()) {
            return records;
        }
        
        String[] lines = csvContent.split("\\n");
        for (String line : lines) {
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
            records.add(fields);
        }
        
        return records;
    }
    
    public static void printCSV(List<List<String>> records) {
        for (List<String> record : records) {
            System.out.println(record);
        }
    }
    
    public static void main(String[] args) {
        // Test case 1: Simple CSV
        System.out.println("Test Case 1: Simple CSV");
        String csv1 = "Name,Age,City\\nJohn,30,New York\\nAlice,25,London";
        List<List<String>> result1 = parseCSV(csv1);
        printCSV(result1);
        System.out.println();
        
        // Test case 2: CSV with quoted fields
        System.out.println("Test Case 2: CSV with quoted fields");
        String csv2 = "Name,Description\\nProduct A,\\"This is a product\\"\\nProduct B,\\"Another product\\"";
        List<List<String>> result2 = parseCSV(csv2);
        printCSV(result2);
        System.out.println();
        
        // Test case 3: CSV with commas in quoted fields
        System.out.println("Test Case 3: CSV with commas in quoted fields");
        String csv3 = "Name,Address\\nJohn,\\"123 Main St, Apt 4\\"\\nAlice,\\"456 Oak Ave, Suite 10\\"";
        List<List<String>> result3 = parseCSV(csv3);
        printCSV(result3);
        System.out.println();
        
        // Test case 4: CSV with escaped quotes
        System.out.println("Test Case 4: CSV with escaped quotes");
        String csv4 = "Title,Quote\\nBook,\\"He said \\"\\"Hello\\"\\"\\"\\nMovie,\\"She replied \\"\\"Hi\\"\\"\\"";
        List<List<String>> result4 = parseCSV(csv4);
        printCSV(result4);
        System.out.println();
        
        // Test case 5: Empty and mixed content
        System.out.println("Test Case 5: Empty and mixed content");
        String csv5 = "A,B,C\\n1,,3\\n,5,\\n7,8,9";
        List<List<String>> result5 = parseCSV(csv5);
        printCSV(result5);
    }
}
