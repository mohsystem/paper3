
import java.util.ArrayList;
import java.util.List;
import java.util.Collections;

public class Task25 {
    public static List<String> number(List<String> lines) {
        if (lines == null) {
            return Collections.emptyList();
        }
        
        List<String> result = new ArrayList<>();
        for (int i = 0; i < lines.size(); i++) {
            String line = lines.get(i);
            if (line == null) {
                line = "";
            }
            result.add(String.format("%d: %s", i + 1, line));
        }
        return result;
    }
    
    public static void main(String[] args) {
        // Test case 1: Empty list
        System.out.println(number(new ArrayList<>()));
        
        // Test case 2: Basic example
        List<String> test2 = new ArrayList<>();
        test2.add("a");
        test2.add("b");
        test2.add("c");
        System.out.println(number(test2));
        
        // Test case 3: Single line
        List<String> test3 = new ArrayList<>();
        test3.add("hello");
        System.out.println(number(test3));
        
        // Test case 4: Empty strings
        List<String> test4 = new ArrayList<>();
        test4.add("");
        test4.add("");
        System.out.println(number(test4));
        
        // Test case 5: Multiple lines with special characters
        List<String> test5 = new ArrayList<>();
        test5.add("Line with spaces");
        test5.add("123");
        test5.add("Special!@#");
        System.out.println(number(test5));
    }
}
