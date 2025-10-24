import java.nio.charset.StandardCharsets;

public class Task59 {

    /**
     * Transforms a string based on character position.
     * Example: accum("abcd") -> "A-Bb-Ccc-Dddd"
     *
     * @param s The input string, containing only letters from a..z and A..Z.
     * @return The transformed string.
     */
    public static String accum(String s) {
        if (s == null || s.isEmpty()) {
            return "";
        }

        StringBuilder resultBuilder = new StringBuilder();
        // Using getBytes to iterate over characters is not ideal,
        // but it's one way to process without complex char-to-byte logic.
        // A simple for loop with charAt is more idiomatic and safer.
        for (int i = 0; i < s.length(); i++) {
            if (i > 0) {
                resultBuilder.append('-');
            }
            char currentChar = s.charAt(i);
            resultBuilder.append(Character.toUpperCase(currentChar));
            for (int j = 0; j < i; j++) {
                resultBuilder.append(Character.toLowerCase(currentChar));
            }
        }
        return resultBuilder.toString();
    }

    public static void main(String[] args) {
        String[] testCases = {
            "abcd",
            "RqaEzty",
            "cwAt",
            "ZpglnRxqenU",
            ""
        };
        String[] expectedResults = {
            "A-Bb-Ccc-Dddd",
            "R-Qq-Aaa-Eeee-Zzzzz-Tttttt-Yyyyyyy",
            "C-Ww-Aaa-Tttt",
            "Z-Pp-Ggg-Llll-Nnnnn-Rrrrrr-Xxxxxxx-Qqqqqqqq-Eeeeeeeee-Nnnnnnnnnn-Uuuuuuuuuuu",
            ""
        };

        for (int i = 0; i < testCases.length; i++) {
            String result = accum(testCases[i]);
            System.out.println("Test case " + (i + 1) + ": accum(\"" + testCases[i] + "\")");
            System.out.println("Expected: " + expectedResults[i]);
            System.out.println("Actual  : " + result);
            System.out.println("Result: " + (expectedResults[i].equals(result) ? "PASS" : "FAIL"));
            System.out.println();
        }
    }
}