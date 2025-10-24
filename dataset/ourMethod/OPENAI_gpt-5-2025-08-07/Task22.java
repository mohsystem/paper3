import java.util.ArrayList;
import java.util.List;

public class Task22 {
    public static List<Integer> filterList(List<?> input) {
        if (input == null) {
            throw new IllegalArgumentException("Input list must not be null.");
        }
        List<Integer> result = new ArrayList<>();
        for (Object o : input) {
            if (o == null) {
                throw new IllegalArgumentException("Null elements are not allowed.");
            }
            if (o instanceof Integer) {
                int val = (Integer) o;
                if (val < 0) {
                    throw new IllegalArgumentException("Negative integers are not allowed: " + val);
                }
                result.add(val);
            } else if (o instanceof String) {
                // skip strings
            } else {
                throw new IllegalArgumentException("Unsupported element type: " + o.getClass().getName());
            }
        }
        return result;
    }

    private static void printList(List<Integer> list) {
        System.out.println(list.toString());
    }

    public static void main(String[] args) {
        List<?> t1 = List.of(1, 2, "a", "b");
        List<?> t2 = List.of(1, "a", "b", 0, 15);
        List<?> t3 = List.of(1, 2, "a", "b", "aasf", "1", "123", 123);
        List<?> t4 = List.of("hello", "world");
        List<?> t5 = List.of(0, 99, "x", "y", 42);

        printList(filterList(t1)); // [1, 2]
        printList(filterList(t2)); // [1, 0, 15]
        printList(filterList(t3)); // [1, 2, 123]
        printList(filterList(t4)); // []
        printList(filterList(t5)); // [0, 99, 42]
    }
}