
import java.util.*;
import java.util.stream.Collectors;

public class Task22 {
    public static List<Integer> filterList(List<Object> list) {
        return list.stream()
                .filter(item -> item instanceof Integer)
                .map(item -> (Integer) item)
                .collect(Collectors.toList());
    }
    
    public static void main(String[] args) {
        // Test case 1
        System.out.println(filterList(List.of(1, 2, "a", "b")));
        
        // Test case 2
        System.out.println(filterList(List.of(1, "a", "b", 0, 15)));
        
        // Test case 3
        System.out.println(filterList(List.of(1, 2, "a", "b", "aasf", "1", "123", 123)));
        
        // Test case 4
        System.out.println(filterList(List.of(10, 20, 30, "test", "hello", 40)));
        
        // Test case 5
        System.out.println(filterList(List.of("only", "strings", "here", "no", "numbers")));
    }
}
