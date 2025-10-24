public class Task126 {

    /**
     * Redirects a user based on a page string.
     *
     * @param page The string representing the desired page (e.g., "home", "about").
     * @return A string message indicating the redirection action.
     */
    public static String redirectUser(String page) {
        String message;
        switch (page.toLowerCase()) {
            case "home":
                message = "Redirecting to Home Page...";
                break;
            case "about":
                message = "Redirecting to About Us Page...";
                break;
            case "contact":
                message = "Redirecting to Contact Page...";
                break;
            case "products":
                message = "Redirecting to Products Page...";
                break;
            default:
                message = "Page not found. Redirecting to 404 Error Page...";
                break;
        }
        return message;
    }

    public static void main(String[] args) {
        String[] testCases = {"home", "about", "contact", "products", "login"};

        System.out.println("--- Java Test Cases ---");
        for (String testCase : testCases) {
            String result = redirectUser(testCase);
            System.out.println("Input: \"" + testCase + "\" -> Output: " + result);
        }
    }
}