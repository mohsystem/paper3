import java.util.*;

public class Task25 {
    public static List<String> number(List<String> lines) {
        List<String> res = new ArrayList<>();
        if (lines == null) return res;
        for (int i = 0; i < lines.size(); i++) {
            res.add((i + 1) + ": " + lines.get(i));
        }
        return res;
    }

    private static void printList(List<String> list) {
        System.out.println(list);
    }

    public static void main(String[] args) {
        List<String> t1 = Collections.emptyList();
        List<String> t2 = Arrays.asList("a", "b", "c");
        List<String> t3 = Arrays.asList("", "hello");
        List<String> t4 = Arrays.asList("foo bar", "baz");
        List<String> t5 = Arrays.asList("x");

        printList(number(t1));
        printList(number(t2));
        printList(number(t3));
        printList(number(t4));
        printList(number(t5));
    }
}