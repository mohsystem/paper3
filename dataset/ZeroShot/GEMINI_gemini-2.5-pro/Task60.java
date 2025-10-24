import java.util.ArrayList;
import java.util.List;

public class Task60 {
    /**
     * Converts a positive number into its expanded form string.
     *
     * @param num The number to convert, must be greater than 0.
     * @return The expanded form of the number as a string.
     */
    public static String expandedForm(long num) {
        // According to the problem description, num will be > 0.
        // Adding a check for robustness.
        if (num <= 0) {
            return "";
        }

        String s = String.valueOf(num);
        List<String> parts = new ArrayList<>();
        int len = s.length();

        for (int i = 0; i < len; i++) {
            char digitChar = s.charAt(i);
            if (digitChar != '0') {
                StringBuilder part = new StringBuilder();
                part.append(digitChar);
                for (int j = 0; j < len - 1 - i; j++) {
                    part.append('0');
                }
                parts.add(part.toString());
            }
        }
        return String.join(" + ", parts);
    }

    public static void main(String[] args) {
        // Test Cases
        System.out.println("12 -> " + expandedForm(12)); // Expected: 10 + 2
        System.out.println("42 -> " + expandedForm(42)); // Expected: 40 + 2
        System.out.println("70304 -> " + expandedForm(70304)); // Expected: 70000 + 300 + 4
        System.out.println("9000000 -> " + expandedForm(9000000)); // Expected: 9000000
        System.out.println("1 -> " + expandedForm(1)); // Expected: 1
    }
}