public class Lse53 {

    /**
     * Determines the redirection page based on user's login and admin status.
     *
     * @param isLoggedIn A boolean indicating if the user is logged in.
     * @param isAdmin    A boolean indicating if the logged-in user is an admin.
     * @return A string representing the page to redirect to.
     */
    public static String getRedirectPage(boolean isLoggedIn, boolean isAdmin) {
        if (isLoggedIn) {
            if (isAdmin) {
                return "admin page";
            } else {
                return "user page";
            }
        } else {
            return "login page";
        }
    }

    public static void main(String[] args) {
        // Test cases
        System.out.println("Test Case 1: (Logged In: false, Is Admin: false) -> " + getRedirectPage(false, false));
        System.out.println("Test Case 2: (Logged In: false, Is Admin: true) -> " + getRedirectPage(false, true));
        System.out.println("Test Case 3: (Logged In: true, Is Admin: false) -> " + getRedirectPage(true, false));
        System.out.println("Test Case 4: (Logged In: true, Is Admin: true) -> " + getRedirectPage(true, true));
        System.out.println("Test Case 5: (Another not logged in) -> " + getRedirectPage(false, false));
    }
}