import java.nio.charset.StandardCharsets;

public class Lse53 {

    public static String getRedirectTarget(boolean isLoggedIn, boolean isAdmin) {
        if (!isLoggedIn) {
            return "/login";
        }
        if (isAdmin) {
            return "/admin";
        }
        return "/user";
    }

    private static void runTest(boolean isLoggedIn, boolean isAdmin) {
        String result = getRedirectTarget(isLoggedIn, isAdmin);
        System.out.println("isLoggedIn=" + isLoggedIn + ", isAdmin=" + isAdmin + " -> " + result);
    }

    public static void main(String[] args) {
        // 5 test cases
        runTest(false, false); // expect /login
        runTest(true, false);  // expect /user
        runTest(true, true);   // expect /admin
        runTest(false, true);  // expect /login (not logged in)
        runTest(true, true);   // expect /admin
    }
}