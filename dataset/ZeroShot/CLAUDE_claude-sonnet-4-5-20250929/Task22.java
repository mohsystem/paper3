
import java.util.*;
import java.util.stream.Collectors;

public class Task22 {
    public static List<Integer> filterList(List<Object> list) {
        if (list == null) {
            return new ArrayList<>();
        }
        
        return list.stream()
                .filter(item -> item instanceof Integer)
                .map(item -> (Integer) item)
                .collect(Collectors.toList());
    }
    
    public static void main(String[] args) {
        // Test case 1
        List<Object> test1 = List.of(1, 2, "a", "b");
        System.out.println("Test 1: " + filterList(test1));
        
        // Test case 2
        List<Object> test2 = List.of(1, "a", "b", 0, 15);
        System.out.println("Test 2: " + filterList(test2));
        
        // Test case 3
        List<Object> test3 = List.of(1, 2, "a", "b", "aasf", "1", "123", 123);
        System.out.println("Test 3: " + filterList(test3));
        
        // Test case 4
        List<Object> test4 = new ArrayList<>();
        test4.add(5);
        test4.add("test");
        test4.add(10);
        test4.add("string");
        test4.add(20);
        System.out.println("Test 4: " + filterList(test4));
        
        // Test case 5
        List<Object> test5 = List.of(100, 200, 300);
        System.out.println("Test 5: " + filterList(test5));
    }
}
