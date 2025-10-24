public class Task59 {

    public static String accum(String s) {
        if (s == null || s.isEmpty()) {
            return "";
        }
        StringBuilder result = new StringBuilder();
        for (int i = 0; i < s.length(); i++) {
            char c = s.charAt(i);
            // Append the first character, capitalized
            result.append(Character.toUpperCase(c));
            // Append the rest of the characters, in lowercase, i times
            for (int j = 0; j < i; j++) {
                result.append(Character.toLowerCase(c));
            }
            // Append a hyphen if it's not the last part
            if (i < s.length() - 1) {
                result.append("-");
            }
        }
        return result.toString();
    }

    public static void main(String[] args) {
        String[] testCases = {"abcd", "RqaEzty", "cwAt", "Z", ""};
        for (String testCase : testCases) {
            System.out.println(String.format("\"%s\" -> \"%s\"", testCase, accum(testCase)));
        }
    }
}