public class Lse53 {
    public static String getRedirect(boolean isLoggedIn, boolean isAdmin) {
        if (!isLoggedIn) return "login";
        if (isAdmin) return "admin";
        return "user";
    }

    public static void main(String[] args) {
        System.out.println(getRedirect(false, false)); // Expected: login
        System.out.println(getRedirect(false, true));  // Expected: login
        System.out.println(getRedirect(true, false));  // Expected: user
        System.out.println(getRedirect(true, true));   // Expected: admin
        System.out.println(getRedirect(true, false));  // Expected: user
    }
}