import java.util.ArrayList;
import java.util.List;
import java.util.Objects;

public class Task22 {

    /**
     * Filters a list of Objects, returning a new list containing only non-negative integers.
     *
     * @param list A list containing non-negative integers and strings.
     * @return A new list with only the non-negative integers.
     */
    public static List<Integer> filterList(final List<Object> list) {
        // Rule #1: Validate input. A null list is handled by returning an empty list.
        if (list == null) {
            return new ArrayList<>();
        }
        
        List<Integer> filteredList = new ArrayList<>();
        for (Object item : list) {
            // Rule #1: Check if the object is an Integer and is non-negative.
            if (item instanceof Integer) {
                Integer number = (Integer) item;
                if (number >= 0) {
                    filteredList.add(number);
                }
            }
        }
        return filteredList;
    }

    public static void main(String[] args) {
        // 5 test cases
        
        // Test Case 1
        List<Object> list1 = List.of(1, 2, "a", "b");
        System.out.println("Input: " + list1 + ", Output: " + filterList(list1));
        
        // Test Case 2
        List<Object> list2 = List.of(1, "a", "b", 0, 15);
        System.out.println("Input: " + list2 + ", Output: " + filterList(list2));

        // Test Case 3
        List<Object> list3 = List.of(1, 2, "aasf", "1", "123", 123);
        System.out.println("Input: " + list3 + ", Output: " + filterList(list3));

        // Test Case 4: Contains negative numbers which should be filtered out
        List<Object> list4 = List.of("hello", "world", -1, 5, -10);
        System.out.println("Input: " + list4 + ", Output: " + filterList(list4));

        // Test Case 5: Empty list
        List<Object> list5 = List.of();
        System.out.println("Input: " + list5 + ", Output: " + filterList(list5));
    }
}