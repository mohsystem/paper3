import java.util.ArrayList;
import java.util.List;
import java.util.Arrays;
import java.util.stream.Collectors;

public class Task22 {

    /**
     * Filters a list of objects, returning a new list containing only the integers.
     *
     * @param list The input list containing non-negative integers and strings.
     * @return A new list containing only the integers from the input list.
     */
    public static List<Integer> filterList(final List<Object> list) {
        // Using streams for a functional approach, which is concise and safe.
        // The filter operation checks the type of each element.
        // If the element is an instance of Integer, it is kept.
        return list.stream()
                   .filter(o -> o instanceof Integer)
                   .map(o -> (Integer) o)
                   .collect(Collectors.toList());
    }

    public static void main(String[] args) {
        // Test Case 1
        List<Object> list1 = Arrays.asList(1, 2, "a", "b");
        System.out.println("Test Case 1: " + list1);
        System.out.println("Filtered: " + filterList(list1)); // Expected: [1, 2]
        System.out.println();

        // Test Case 2
        List<Object> list2 = Arrays.asList(1, "a", "b", 0, 15);
        System.out.println("Test Case 2: " + list2);
        System.out.println("Filtered: " + filterList(list2)); // Expected: [1, 0, 15]
        System.out.println();

        // Test Case 3
        List<Object> list3 = Arrays.asList(1, 2, "aasf", "1", "123", 123);
        System.out.println("Test Case 3: " + list3);
        System.out.println("Filtered: " + filterList(list3)); // Expected: [1, 2, 123]
        System.out.println();
        
        // Test Case 4: All strings
        List<Object> list4 = Arrays.asList("hello", "world", "123");
        System.out.println("Test Case 4: " + list4);
        System.out.println("Filtered: " + filterList(list4)); // Expected: []
        System.out.println();

        // Test Case 5: Empty list
        List<Object> list5 = new ArrayList<>();
        System.out.println("Test Case 5: " + list5);
        System.out.println("Filtered: " + filterList(list5)); // Expected: []
        System.out.println();
    }
}