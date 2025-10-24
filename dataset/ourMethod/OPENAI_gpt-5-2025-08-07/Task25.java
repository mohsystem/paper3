import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

public final class Task25 {
    private Task25() {
        // Prevent instantiation
    }

    public static List<String> numberLines(final List<String> lines) {
        if (lines == null) {
            throw new IllegalArgumentException("Input list must not be null.");
        }
        final int size = lines.size();
        final List<String> result = new ArrayList<>(size);
        for (int i = 0; i < size; i++) {
            final String s = lines.get(i);
            if (s == null) {
                throw new IllegalArgumentException("Input element at index " + i + " must not be null.");
            }
            result.add((i + 1) + ": " + s);
        }
        return result;
    }

    public static void main(String[] args) {
        // 5 test cases
        List<List<String>> tests = Arrays.asList(
            Arrays.asList(),
            Arrays.asList("a", "b", "c"),
            Arrays.asList("", "x"),
            Arrays.asList("one line with spaces", "two:with:colons"),
            Arrays.asList("emoji 😀", "multibyte 字", "punctuation!?", "tabs\tand\nnewlines") // note: newlines shown as literal new line
        );

        int t = 1;
        for (List<String> test : tests) {
            List<String> numbered = numberLines(test);
            System.out.println("Test " + (t++) + ": " + numbered);
        }
    }
}