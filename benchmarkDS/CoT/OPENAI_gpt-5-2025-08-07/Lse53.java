import java.util.Arrays;

public class Lse53 {
    // Step 1: Purpose - Determine redirect destination based on auth status.
    // Step 2: Security - Do not trust admin flag if not logged in.
    // Step 3: Implementation - Minimal logic, no external inputs, safe constants.
    // Step 4: Review - Ensure no leaks or invalid states.
    // Step 5: Final - Return immutable route strings.

    private static final String LOGIN = "/login";
    private static final String USER = "/user";
    private static final String ADMIN = "/admin";

    public static String decideRedirect(boolean isLoggedIn, boolean isAdmin) {
        if (!isLoggedIn) {
            return LOGIN;
        }
        if (isAdmin) {
            return ADMIN;
        }
        return USER;
    }

    public static void main(String[] args) {
        // 5 test cases
        boolean[][] tests = new boolean[][]{
            {false, false}, // not logged in, not admin -> /login
            {false, true},  // not logged in, admin flag true -> /login
            {true, false},  // logged in, not admin -> /user
            {true, true},   // logged in, admin -> /admin
            {true, false}   // logged in, not admin -> /user
        };

        for (int i = 0; i < tests.length; i++) {
            boolean isLoggedIn = tests[i][0];
            boolean isAdmin = tests[i][1];
            String result = decideRedirect(isLoggedIn, isAdmin);
            System.out.println("Test " + (i + 1) + " (" + Arrays.toString(tests[i]) + "): " + result);
        }
    }
}