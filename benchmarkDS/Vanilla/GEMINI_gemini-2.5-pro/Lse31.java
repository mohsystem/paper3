public class Lse31 {

    /**
     * Parses a URL to check if its domain is "example.com".
     *
     * @param urlString The URL to check.
     * @return "redirected" if the domain is "example.com", otherwise "404 error".
     */
    public static String redirectUrl(String urlString) {
        try {
            // In Java, use the java.net.URL class for parsing.
            java.net.URL url = new java.net.URL(urlString);
            String domain = url.getHost();

            // Check if the domain (host) is exactly "example.com"
            if ("example.com".equals(domain)) {
                return "redirected";
            } else {
                return "404 error";
            }
        } catch (java.net.MalformedURLException e) {
            // If the URL is not valid, it's considered a "not found" case.
            return "404 error";
        }
    }

    public static void main(String[] args) {
        String[] testUrls = {
            "https://example.com/home",
            "http://www.example.com/test",
            "ftp://google.com/files",
            "https://example.com:8080/path?a=1",
            "not_a_valid_url"
        };

        System.out.println("Java Test Cases:");
        for (String url : testUrls) {
            System.out.println("URL: " + url + " -> " + redirectUrl(url));
        }
    }
}