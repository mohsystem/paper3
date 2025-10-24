
import java.util.ArrayList;
import java.util.List;
import java.util.Arrays;

public class Task25 {
    public static List<String> number(List<String> lines) {
        if (lines == null) {
            return new ArrayList<>();
        }
        
        List<String> result = new ArrayList<>();
        for (int i = 0; i < lines.size(); i++) {
            result.add((i + 1) + ": " + lines.get(i));
        }
        return result;
    }
    
    public static void main(String[] args) {
        // Test case 1: Empty list
        System.out.println("Test 1: " + number(new ArrayList<>()));
        
        // Test case 2: Simple list
        System.out.println("Test 2: " + number(Arrays.asList("a", "b", "c")));
        
        // Test case 3: Single element
        System.out.println("Test 3: " + number(Arrays.asList("Hello World")));
        
        // Test case 4: Multiple lines with different content
        System.out.println("Test 4: " + number(Arrays.asList("line one", "line two", "line three", "line four")));
        
        // Test case 5: Lines with special characters
        System.out.println("Test 5: " + number(Arrays.asList("Hello!", "How are you?", "I'm fine, thanks.")));
    }
}
