public class Lse125 {

    public static String trimTrailing(String str) {
        if (str == null || str.isEmpty()) {
            return str;
        }
        int i = str.length() - 1;
        while (i >= 0 && Character.isWhitespace(str.charAt(i))) {
            i--;
        }
        return str.substring(0, i + 1);
    }

    public static void main(String[] args) {
        if (args.length > 0) {
            System.out.println(trimTrailing(args[0]));
        } else {
            System.out.println("--- Running Test Cases ---");
            String[] testCases = {
                "Hello World   ",
                "  leading and trailing  ",
                "no_whitespace",
                "   \t\n",
                "Trailing tab\t"
            };

            for (String test : testCases) {
                System.out.println("Original: [" + test + "]");
                System.out.println("Trimmed:  [" + trimTrailing(test) + "]");
                System.out.println();
            }
        }
    }
}