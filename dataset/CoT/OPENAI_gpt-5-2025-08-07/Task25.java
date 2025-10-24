import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.List;

public class Task25 {
    public static List<String> number(List<String> lines) {
        if (lines == null) return new ArrayList<>();
        List<String> result = new ArrayList<>(lines.size());
        for (int i = 0; i < lines.size(); i++) {
            String s = String.valueOf(lines.get(i));
            result.add((i + 1) + ": " + s);
        }
        return result;
    }

    public static void main(String[] args) {
        // Test case 1: Empty list
        List<String> t1 = Collections.emptyList();
        System.out.println(number(t1));

        // Test case 2: ["a", "b", "c"]
        List<String> t2 = Arrays.asList("a", "b", "c");
        System.out.println(number(t2));

        // Test case 3: ["", "foo", "bar"]
        List<String> t3 = Arrays.asList("", "foo", "bar");
        System.out.println(number(t3));

        // Test case 4: ["hello world", "Line 2"]
        List<String> t4 = Arrays.asList("hello world", "Line 2");
        System.out.println(number(t4));

        // Test case 5: ["multiple", "lines", "with", "numbers", "10"]
        List<String> t5 = Arrays.asList("multiple", "lines", "with", "numbers", "10");
        System.out.println(number(t5));
    }
}