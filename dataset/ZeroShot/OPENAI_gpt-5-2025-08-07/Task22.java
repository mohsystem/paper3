import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.List;

public class Task22 {
    public static List<Integer> filterList(List<?> input) {
        if (input == null) return Collections.emptyList();
        List<Integer> result = new ArrayList<>();
        for (Object o : input) {
            if (o instanceof Integer) {
                int v = (Integer) o;
                if (v >= 0) {
                    result.add(v);
                }
            }
        }
        return result;
    }

    private static void printList(List<Integer> list) {
        System.out.println(list);
    }

    public static void main(String[] args) {
        // Test case 1
        List<?> t1 = List.of(1, 2, "a", "b");
        printList(filterList(t1)); // [1, 2]

        // Test case 2
        List<?> t2 = List.of(1, "a", "b", 0, 15);
        printList(filterList(t2)); // [1, 0, 15]

        // Test case 3
        List<?> t3 = List.of(1, 2, "a", "b", "aasf", "1", "123", 123);
        printList(filterList(t3)); // [1, 2, 123]

        // Test case 4 (includes negative integer)
        List<?> t4 = Arrays.asList("hello", -1, 3, "4", 0);
        printList(filterList(t4)); // [3, 0]

        // Test case 5 (empty list)
        List<?> t5 = Collections.emptyList();
        printList(filterList(t5)); // []
    }
}