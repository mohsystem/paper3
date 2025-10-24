import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

public class Task60 {

    /**
     * Converts a number to its expanded form string.
     * For example: 70304 becomes "70000 + 300 + 4"
     * @param num The positive integer to convert.
     * @return The string representing the expanded form.
     */
    public static String expandedForm(int num) {
        String s = String.valueOf(num);
        StringBuilder result = new StringBuilder();

        for (int i = 0; i < s.length(); i++) {
            char digitChar = s.charAt(i);
            if (digitChar != '0') {
                if (result.length() > 0) {
                    result.append(" + ");
                }
                result.append(digitChar);
                for (int j = 0; j < s.length() - 1 - i; j++) {
                    result.append('0');
                }
            }
        }
        return result.toString();
    }

    public static void main(String[] args) {
        int[] testCases = {12, 42, 70304, 9000000, 806};
        for (int testCase : testCases) {
            System.out.println("Input: " + testCase + ", Output: " + expandedForm(testCase));
        }
    }
}