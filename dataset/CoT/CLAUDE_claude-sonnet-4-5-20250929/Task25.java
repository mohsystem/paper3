
import java.util.ArrayList;
import java.util.List;

public class Task25 {
    public static List<String> number(List<String> lines) {
        if (lines == null) {
            return new ArrayList<>();
        }
        
        List<String> result = new ArrayList<>();
        for (int i = 0; i < lines.size(); i++) {
            String line = lines.get(i);
            if (line == null) {
                line = "";
            }
            result.add((i + 1) + ": " + line);
        }
        return result;
    }
    
    public static void main(String[] args) {
        // Test case 1: Empty list
        System.out.println("Test 1: " + number(new ArrayList<>()));
        
        // Test case 2: Basic example
        List<String> test2 = new ArrayList<>();
        test2.add("a");
        test2.add("b");
        test2.add("c");
        System.out.println("Test 2: " + number(test2));
        
        // Test case 3: Single element
        List<String> test3 = new ArrayList<>();
        test3.add("hello");
        System.out.println("Test 3: " + number(test3));
        
        // Test case 4: Strings with special characters
        List<String> test4 = new ArrayList<>();
        test4.add("Hello, World!");
        test4.add("Line with spaces");
        test4.add("Special @#$%");
        System.out.println("Test 4: " + number(test4));
        
        // Test case 5: Empty strings
        List<String> test5 = new ArrayList<>();
        test5.add("");
        test5.add("non-empty");
        test5.add("");
        System.out.println("Test 5: " + number(test5));
    }
}
