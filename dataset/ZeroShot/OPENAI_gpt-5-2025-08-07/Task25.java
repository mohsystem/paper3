import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

public class Task25 {
    public static List<String> number(List<String> lines) {
        List<String> result = new ArrayList<>();
        if (lines == null) {
            return result;
        }
        long idx = 1L;
        for (String s : lines) {
            String safe = (s == null) ? "" : s;
            result.add(Long.toString(idx) + ": " + safe);
            if (idx == Long.MAX_VALUE) {
                break;
            }
            idx++;
        }
        return result;
    }

    private static void printList(List<String> l) {
        System.out.println(l);
    }

    public static void main(String[] args) {
        // Test case 1: empty list
        printList(number(Arrays.asList()));
        // Test case 2: ["a", "b", "c"]
        printList(number(Arrays.asList("a", "b", "c")));
        // Test case 3: [null, "x", ""]
        printList(number(Arrays.asList(null, "x", "")));
        // Test case 4: ["only one"]
        printList(number(Arrays.asList("only one")));
        // Test case 5: multiple lines
        List<String> many = new ArrayList<>();
        for (int i = 0; i < 5; i++) {
            many.add("line " + i);
        }
        printList(number(many));
    }
}