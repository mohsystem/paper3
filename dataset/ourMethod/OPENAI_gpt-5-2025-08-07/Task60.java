import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.List;

public class Task60 {
    public static String expandedForm(long n) {
        if (n <= 0) {
            throw new IllegalArgumentException("Input must be a whole number greater than 0.");
        }
        final String s = Long.toString(n);
        if (s.length() > 19) {
            throw new IllegalArgumentException("Input too large.");
        }
        StringBuilder sb = new StringBuilder();
        boolean first = true;
        for (int i = 0; i < s.length(); i++) {
            char c = s.charAt(i);
            if (c != '0') {
                int zeros = s.length() - 1 - i;
                if (!first) {
                    sb.append(" + ");
                }
                sb.append(c);
                for (int z = 0; z < zeros; z++) {
                    sb.append('0');
                }
                first = false;
            }
        }
        return sb.toString();
    }

    public static void main(String[] args) {
        long[] tests = new long[] {12, 42, 70304, 9000000, 105};
        for (long t : tests) {
            try {
                String result = expandedForm(t);
                System.out.println(t + " -> " + result);
            } catch (IllegalArgumentException ex) {
                System.out.println("Error for input " + t + ": " + ex.getMessage());
            }
        }
    }
}