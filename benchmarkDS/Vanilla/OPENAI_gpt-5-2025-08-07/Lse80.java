import java.util.*;

public class Lse80 {
    public static class Result {
        public String text;
        public int count;
        public int consumed;
        public Result(String text, int count, int consumed) {
            this.text = text;
            this.count = count;
            this.consumed = consumed;
        }
        @Override
        public String toString() {
            return "text=\"" + text + "\", count=" + count + ", consumed=" + consumed;
        }
    }

    // Reads up to 1023 characters from input until a newline or end-of-input.
    // The newline (if present) is consumed but not stored.
    // Returns Result containing: stored text, count of stored chars, and total consumed chars.
    public static Result scan(String input) {
        if (input == null) input = "";
        StringBuilder sb = new StringBuilder(1023);
        int count = 0;
        int consumed = 0;
        int n = input.length();
        for (int i = 0; i < n; i++) {
            char ch = input.charAt(i);
            if (ch == '\n') {
                consumed++;
                break;
            }
            if (count < 1023) {
                sb.append(ch);
                count++;
            }
            consumed++;
        }
        return new Result(sb.toString(), count, consumed);
    }

    // 5 test cases
    public static void main(String[] args) {
        List<String> tests = new ArrayList<>();
        tests.add("hello\nworld");                  // normal line with newline
        tests.add("\nstart");                       // newline first
        char[] arr = new char[1100];
        Arrays.fill(arr, 'a');
        tests.add(new String(arr) + "\nNEXT");      // longer than 1023, then newline
        tests.add("no newline here");               // no newline
        tests.add("");                              // empty input

        for (int i = 0; i < tests.size(); i++) {
            Result r = scan(tests.get(i));
            System.out.println("Test " + (i+1) + ": " + r.toString());
        }
    }
}