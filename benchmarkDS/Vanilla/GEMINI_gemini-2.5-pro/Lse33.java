public class Lse33 {

    /**
     * Checks if the URL's domain is "example.com".
     *
     * @param urlString The URL to check.
     * @return A redirection message if the domain is valid, otherwise an error message.
     */
    public static String redirectInvalidUrl(String urlString) {
        try {
            // Use java.net.URL for robust parsing.
            java.net.URL url = new java.net.URL(urlString);
            String host = url.getHost();

            if ("example.com".equals(host)) {
                return "Redirecting to " + urlString;
            } else {
                return "Error: Invalid domain. Only example.com is allowed. Domain was: " + host;
            }
        } catch (java.net.MalformedURLException e) {
            // This catches URLs with incorrect protocols or general malformations.
            return "Error: Malformed URL provided.";
        }
    }

    public static void main(String[] args) {
        String[] testUrls = {
            "http://example.com/path/to/page",
            "https://www.google.com",
            "http://test.example.com",
            "ftp://example.com/resource",
            "htp:/invalid-url" // Malformed URL
        };

        System.out.println("--- Running Java Test Cases ---");
        for (String url : testUrls) {
            System.out.println("Input : " + url);
            System.out.println("Output: " + redirectInvalidUrl(url));
            System.out.println("---------------------------------");
        }
    }
}