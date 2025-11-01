public class Lse30 {

    /**
     * Determines the redirect URL based on whether the target string matches a specific regex.
     * The regex matches a string that starts with one or more alphanumeric characters or hyphens,
     * followed by a period.
     *
     * @param target The target query parameter string.
     * @return A string indicating redirection to the target if it matches, or to the root ("/") otherwise.
     */
    public static String getRedirectUrl(String target) {
        if (target == null) {
            return "Redirecting to: /";
        }
        // Regex: starts with one or more alphanumeric/hyphen chars, followed by a period.
        String regex = "^[a-zA-Z0-9-]+\\..*";

        if (target.matches(regex)) {
            return "Redirecting to: " + target;
        } else {
            return "Redirecting to: /";
        }
    }

    public static void main(String[] args) {
        String[] testCases = {
            "example.com",
            "sub-domain.example.org/path?query=1",
            "123-valid.net",
            ".invalid-start.com",
            "/local/path/only"
        };

        System.out.println("Java Test Cases:");
        for (String testCase : testCases) {
            System.out.println("Input: \"" + testCase + "\" -> Output: " + getRedirectUrl(testCase));
        }
    }
}