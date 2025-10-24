public class Task59 {

    /**
     * Transforms a string based on the specified accumulation rule.
     * Example: accum("abcd") -> "A-Bb-Ccc-Dddd"
     *
     * @param s The input string, containing only letters from a..z and A..Z.
     * @return The transformed string.
     */
    public static String accum(String s) {
        if (s == null) {
            return "";
        }
        StringBuilder result = new StringBuilder();
        for (int i = 0; i < s.length(); i++) {
            if (i > 0) {
                result.append("-");
            }
            char c = s.charAt(i);
            result.append(Character.toUpperCase(c));
            for (int j = 0; j < i; j++) {
                result.append(Character.toLowerCase(c));
            }
        }
        return result.toString();
    }

    public static void main(String[] args) {
        String[] testCases = {
            "abcd",
            "RqaEzty",
            "cwAt",
            "Z",
            ""
        };

        for (String testCase : testCases) {
            System.out.println("Input: \"" + testCase + "\" -> Output: \"" + accum(testCase) + "\"");
        }
    }
}