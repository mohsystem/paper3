public class Task126 {

    /**
     * Redirects a user based on a string choice.
     * The comparison is case-insensitive.
     *
     * @param choice The user's page selection (e.g., "home", "about").
     * @return A string indicating the redirection URL or an error message.
     */
    public static String redirectUser(String choice) {
        if (choice == null || choice.trim().isEmpty()) {
            return "Error: Page not found (404)";
        }

        // Convert choice to lowercase for case-insensitive matching
        String lowerCaseChoice = choice.toLowerCase();

        switch (lowerCaseChoice) {
            case "home":
                return "Redirecting to /home.html";
            case "about":
                return "Redirecting to /about.html";
            case "contact":
                return "Redirecting to /contact.html";
            case "products":
                return "Redirecting to /products.html";
            default:
                return "Error: Page not found (404)";
        }
    }

    public static void main(String[] args) {
        String[] testCases = {"home", "About", "products", "login", ""};
        
        System.out.println("--- Java Test Cases ---");
        for (String test : testCases) {
            System.out.println("Input: \"" + test + "\" -> Output: " + redirectUser(test));
        }
    }
}