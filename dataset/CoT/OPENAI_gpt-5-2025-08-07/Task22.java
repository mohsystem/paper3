import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

public class Task22 {
    public static List<Integer> filterList(List<?> list) {
        List<Integer> result = new ArrayList<>();
        if (list == null) return result;
        for (Object o : list) {
            if (o instanceof Integer) {
                int v = (Integer) o;
                if (v >= 0) {
                    result.add(v);
                }
            }
        }
        return result;
    }

    private static String listToString(List<Integer> list) {
        StringBuilder sb = new StringBuilder();
        sb.append("[");
        for (int i = 0; i < list.size(); i++) {
            if (i > 0) sb.append(", ");
            sb.append(list.get(i));
        }
        sb.append("]");
        return sb.toString();
    }

    public static void main(String[] args) {
        List<?> t1 = Arrays.asList(1, 2, "a", "b");
        List<?> t2 = Arrays.asList(1, "a", "b", 0, 15);
        List<?> t3 = Arrays.asList(1, 2, "a", "b", "aasf", "1", "123", 123);
        List<?> t4 = Arrays.asList(0, "zero", 99, "99", 100, -5);
        List<?> t5 = Arrays.asList("only", "strings", "here");

        List<List<?>> tests = Arrays.asList(t1, t2, t3, t4, t5);

        for (int i = 0; i < tests.size(); i++) {
            List<Integer> filtered = filterList(tests.get(i));
            System.out.println("Test " + (i + 1) + ": " + listToString(filtered));
        }
    }
}