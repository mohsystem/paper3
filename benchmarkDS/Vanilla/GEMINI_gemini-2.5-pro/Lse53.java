public class Lse53 {

    /**
     * Determines the redirection page based on user's login and admin status.
     *
     * @param isLoggedIn A boolean indicating if the user is logged in.
     * @param isAdmin    A boolean indicating if the logged-in user is an admin.
     * @return A String representing the page to redirect to.
     */
    public static String redirectUser(boolean isLoggedIn, boolean isAdmin) {
        if (isLoggedIn) {
            if (isAdmin) {
                return "Admin Page";
            } else {
                return "User Page";
            }
        } else {
            return "Login Page";
        }
    }

    public static void main(String[] args) {
        // Test Cases
        System.out.println("Test Case 1 (Not logged in, not admin): " + redirectUser(false, false));
        System.out.println("Test Case 2 (Not logged in, is admin): " + redirectUser(false, true));
        System.out.println("Test Case 3 (Logged in, not admin): " + redirectUser(true, false));
        System.out.println("Test Case 4 (Logged in, is admin): " + redirectUser(true, true));
        System.out.println("Test Case 5 (Another logged in user): " + redirectUser(true, false));
    }
}