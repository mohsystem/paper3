import java.util.ArrayList;
import java.util.List;
import java.util.stream.Collectors;

public class Task22 {

    /**
     * Filters a list of objects, returning a new list containing only the integers.
     *
     * @param list A list containing non-negative integers and strings.
     * @return A new list with only the integers.
     */
    public static List<Integer> filterList(final List<Object> list) {
        List<Integer> filteredList = new ArrayList<>();
        for (Object item : list) {
            if (item instanceof Integer) {
                filteredList.add((Integer) item);
            }
        }
        return filteredList;
    }

    public static void main(String[] args) {
        // Test Case 1
        List<Object> list1 = List.of(1, 2, "a", "b");
        System.out.println("Test Case 1 Input: " + list1);
        System.out.println("Filtered Output: " + filterList(list1));
        System.out.println();

        // Test Case 2
        List<Object> list2 = List.of(1, "a", "b", 0, 15);
        System.out.println("Test Case 2 Input: " + list2);
        System.out.println("Filtered Output: " + filterList(list2));
        System.out.println();

        // Test Case 3
        List<Object> list3 = List.of(1, 2, "aasf", "1", "123", 123);
        System.out.println("Test Case 3 Input: " + list3);
        System.out.println("Filtered Output: " + filterList(list3));
        System.out.println();

        // Test Case 4
        List<Object> list4 = List.of("hello", "world", 2024);
        System.out.println("Test Case 4 Input: " + list4);
        System.out.println("Filtered Output: " + filterList(list4));
        System.out.println();

        // Test Case 5
        List<Object> list5 = List.of("a", "b", "c");
        System.out.println("Test Case 5 Input: " + list5);
        System.out.println("Filtered Output: " + filterList(list5));
        System.out.println();
    }
}