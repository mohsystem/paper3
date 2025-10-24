import java.util.ArrayList;
import java.util.List;
import java.util.stream.Collectors;

public class Task60 {

    /**
     * Converts a positive integer into its expanded form string.
     * For example: 70304 becomes "70000 + 300 + 4".
     *
     * @param num The positive integer to expand. Must be greater than 0.
     * @return The expanded form of the number as a String.
     */
    public static String expandedForm(int num) {
        if (num <= 0) {
            // As per prompt, numbers are > 0. Handle defensively.
            return "";
        }

        String s = String.valueOf(num);
        List<String> parts = new ArrayList<>();

        for (int i = 0; i < s.length(); i++) {
            char digitChar = s.charAt(i);
            if (digitChar != '0') {
                StringBuilder part = new StringBuilder();
                part.append(digitChar);
                for (int j = 0; j < s.length() - 1 - i; j++) {
                    part.append('0');
                }
                parts.add(part.toString());
            }
        }

        return String.join(" + ", parts);
    }

    public static void main(String[] args) {
        int[] testCases = {12, 42, 70304, 9000000, 1};

        for (int testCase : testCases) {
            System.out.printf("%d -> \"%s\"\n", testCase, expandedForm(testCase));
        }
    }
}