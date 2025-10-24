
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

public class Task25 {
    public static List<String> number(List<String> lines) {
        List<String> result = new ArrayList<>();
        for (int i = 0; i < lines.size(); i++) {
            result.add((i + 1) + ": " + lines.get(i));
        }
        return result;
    }
    
    public static void main(String[] args) {
        // Test case 1: Empty list
        System.out.println(number(new ArrayList<>()));
        
        // Test case 2: Basic example
        System.out.println(number(Arrays.asList("a", "b", "c")));
        
        // Test case 3: Single element
        System.out.println(number(Arrays.asList("hello")));
        
        // Test case 4: Multiple words
        System.out.println(number(Arrays.asList("hello world", "foo bar", "test")));
        
        // Test case 5: Empty strings
        System.out.println(number(Arrays.asList("", "a", "")));
    }
}
