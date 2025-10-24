import java.util.ArrayList;
import java.util.List;
import java.util.Arrays;

public class Task22 {

    /**
     * Filters a list of objects, returning a new list containing only non-negative integers.
     *
     * @param list The input list containing non-negative integers and strings.
     * @return A new list containing only the non-negative integers from the input list.
     */
    public static List<Integer> filterList(final List<Object> list) {
        if (list == null) {
            return new ArrayList<>();
        }
        List<Integer> filteredList = new ArrayList<>();
        for (Object item : list) {
            if (item instanceof Integer) {
                int intValue = (Integer) item;
                if (intValue >= 0) {
                    filteredList.add(intValue);
                }
            }
        }
        return filteredList;
    }

    public static void main(String[] args) {
        // Test Case 1
        List<Object> list1 = Arrays.asList(1, 2, "a", "b");
        System.out.println("Test 1 Input: " + list1);
        System.out.println("Test 1 Output: " + filterList(list1)); // Expected: [1, 2]

        // Test Case 2
        List<Object> list2 = Arrays.asList(1, "a", "b", 0, 15);
        System.out.println("Test 2 Input: " + list2);
        System.out.println("Test 2 Output: " + filterList(list2)); // Expected: [1, 0, 15]

        // Test Case 3
        List<Object> list3 = Arrays.asList(1, 2, "aasf", "1", "123", 123);
        System.out.println("Test 3 Input: " + list3);
        System.out.println("Test 3 Output: " + filterList(list3)); // Expected: [1, 2, 123]

        // Test Case 4: With negative numbers
        List<Object> list4 = Arrays.asList(-1, "c", 5, -9, "d");
        System.out.println("Test 4 Input: " + list4);
        System.out.println("Test 4 Output: " + filterList(list4)); // Expected: [5]

        // Test Case 5: Empty list
        List<Object> list5 = new ArrayList<>();
        System.out.println("Test 5 Input: " + list5);
        System.out.println("Test 5 Output: " + filterList(list5)); // Expected: []
    }
}