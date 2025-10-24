import java.util.*;

public class Task22 {
    public static List<Integer> filterList(List<?> list) {
        List<Integer> result = new ArrayList<>();
        for (Object o : list) {
            if (o instanceof Integer) {
                result.add((Integer) o);
            }
        }
        return result;
    }

    public static void main(String[] args) {
        List<?> t1 = Arrays.asList(1, 2, "a", "b");
        System.out.println(filterList(t1)); // [1, 2]

        List<?> t2 = Arrays.asList(1, "a", "b", 0, 15);
        System.out.println(filterList(t2)); // [1, 0, 15]

        List<?> t3 = Arrays.asList(1, 2, "a", "b", "aasf", "1", "123", 123);
        System.out.println(filterList(t3)); // [1, 2, 123]

        List<?> t4 = Arrays.asList("x", "y", "z");
        System.out.println(filterList(t4)); // []

        List<?> t5 = Arrays.asList(0, 99, 1000, "1000", "", 5);
        System.out.println(filterList(t5)); // [0, 99, 1000, 5]
    }
}