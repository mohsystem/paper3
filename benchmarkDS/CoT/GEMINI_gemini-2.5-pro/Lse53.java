public class Lse53 {

    /**
     * Determines the redirection page based on user's login and admin status.
     *
     * @param isLoggedIn A boolean indicating if the user is logged in.
     * @param isAdmin    A boolean indicating if the logged-in user is an admin.
     * @return A string message indicating the redirection destination.
     */
    public static String redirectUser(boolean isLoggedIn, boolean isAdmin) {
        if (!isLoggedIn) {
            return "Redirecting to Login Page";
        } else {
            if (isAdmin) {
                return "Redirecting to Admin Page";
            } else {
                return "Redirecting to User Page";
            }
        }
    }

    public static void main(String[] args) {
        // Test Case 1: Not logged in
        System.out.println("Test Case 1 (isLoggedIn: false, isAdmin: false): " + redirectUser(false, false));

        // Test Case 2: Not logged in, but admin flag is true (should still go to login)
        System.out.println("Test Case 2 (isLoggedIn: false, isAdmin: true): " + redirectUser(false, true));

        // Test Case 3: Logged in, but not an admin
        System.out.println("Test Case 3 (isLoggedIn: true, isAdmin: false): " + redirectUser(true, false));

        // Test Case 4: Logged in and is an admin
        System.out.println("Test Case 4 (isLoggedIn: true, isAdmin: true): " + redirectUser(true, true));
        
        // Test Case 5: Another test for a regular logged-in user
        System.out.println("Test Case 5 (isLoggedIn: true, isAdmin: false): " + redirectUser(true, false));
    }
}