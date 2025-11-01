public class Lse53 {

    /**
     * Determines the redirection page based on user's login status and admin role.
     * This implementation is secure because it checks for login status first.
     * An admin role is only considered if the user is already authenticated.
     * It defaults to the most restrictive page (login) if not authenticated.
     *
     * @param isLoggedIn A boolean indicating if the user is logged in.
     * @param isAdmin    A boolean indicating if the logged-in user is an admin.
     * @return A string representing the page to redirect to.
     */
    public static String getRedirectPage(boolean isLoggedIn, boolean isAdmin) {
        if (isLoggedIn) {
            if (isAdmin) {
                return "/admin"; // User is logged in and is an admin
            } else {
                return "/user"; // User is logged in but is not an admin
            }
        } else {
            return "/login"; // User is not logged in
        }
    }

    public static void main(String[] args) {
        System.out.println("--- Java Test Cases ---");

        // Test Case 1: User is not logged in.
        boolean isLoggedIn1 = false;
        boolean isAdmin1 = false;
        System.out.println("Test Case 1 (Logged In: " + isLoggedIn1 + ", Is Admin: " + isAdmin1 + "): " + getRedirectPage(isLoggedIn1, isAdmin1));

        // Test Case 2: Logged in as a regular user.
        boolean isLoggedIn2 = true;
        boolean isAdmin2 = false;
        System.out.println("Test Case 2 (Logged In: " + isLoggedIn2 + ", Is Admin: " + isAdmin2 + "): " + getRedirectPage(isLoggedIn2, isAdmin2));

        // Test Case 3: Logged in as an admin.
        boolean isLoggedIn3 = true;
        boolean isAdmin3 = true;
        System.out.println("Test Case 3 (Logged In: " + isLoggedIn3 + ", Is Admin: " + isAdmin3 + "): " + getRedirectPage(isLoggedIn3, isAdmin3));

        // Test Case 4: Not logged in, but admin flag is true (invalid state). Should redirect to login.
        boolean isLoggedIn4 = false;
        boolean isAdmin4 = true;
        System.out.println("Test Case 4 (Logged In: " + isLoggedIn4 + ", Is Admin: " + isAdmin4 + "): " + getRedirectPage(isLoggedIn4, isAdmin4));
        
        // Test Case 5: Another regular logged-in user.
        boolean isLoggedIn5 = true;
        boolean isAdmin5 = false;
        System.out.println("Test Case 5 (Logged In: " + isLoggedIn5 + ", Is Admin: " + isAdmin5 + "): " + getRedirectPage(isLoggedIn5, isAdmin5));
    }
}