public class Lse125 {

    public static String trimTrailingWhitespace(String input) {
        if (input == null) {
            return null;
        }
        int i = input.length();
        while (i > 0 && Character.isWhitespace(input.charAt(i - 1))) {
            i--;
        }
        return input.substring(0, i);
    }

    public static void main(String[] args) {
        // Command-line behavior
        if (args != null && args.length > 0) {
            String result = trimTrailingWhitespace(args[0]);
            System.out.println(result == null ? "" : result);
        }

        // 5 test cases
        String[] tests = new String[] {
            "hello   ",
            " no-trim",
            "tabs\t\t",
            "mixed spaces\t ",
            ""
        };

        for (String t : tests) {
            String res = trimTrailingWhitespace(t);
            System.out.println(res == null ? "" : res);
        }
    }
}