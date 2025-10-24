import java.net.URI;
import java.net.http.HttpClient;
import java.net.http.HttpRequest;
import java.net.http.HttpResponse;
import java.time.Duration;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

public class Task76 {

    /**
     * Fetches the content of a URL and extracts the page title from the HTML.
     *
     * @param urlString The HTTPS URL to fetch.
     * @return The extracted page title, or an error/not found message.
     */
    public static String getPageTitle(String urlString) {
        try {
            HttpClient client = HttpClient.newBuilder()
                    .followRedirects(HttpClient.Redirect.ALWAYS)
                    .connectTimeout(Duration.ofSeconds(10))
                    .build();

            HttpRequest request = HttpRequest.newBuilder()
                    .uri(URI.create(urlString))
                    .header("User-Agent", "Mozilla/5.0 (Java HttpClient)")
                    .build();

            HttpResponse<String> response = client.send(request, HttpResponse.BodyHandlers.ofString());
            String html = response.body();

            // Use regex to find the title tag. Case-insensitive and dotall mode.
            Pattern pattern = Pattern.compile("<title.*?>(.*?)</title>", Pattern.CASE_INSENSITIVE | Pattern.DOTALL);
            Matcher matcher = pattern.matcher(html);

            if (matcher.find()) {
                // The first capturing group contains the title text.
                return matcher.group(1).trim();
            } else {
                return "Title not found.";
            }
        } catch (Exception e) {
            return "Error fetching URL: " + e.getMessage();
        }
    }

    public static void main(String[] args) {
        if (args.length > 0) {
            System.out.println("Processing URL from command line: " + args[0]);
            String title = getPageTitle(args[0]);
            System.out.println("Title: " + title);
            System.out.println("\n--- Running built-in test cases ---");
        }

        String[] testUrls = {
            "https://www.google.com",
            "https://github.com",
            "https://www.oracle.com/java/",
            "https://www.python.org",
            "https://isocpp.org"
        };

        for (String url : testUrls) {
            System.out.println("Fetching title for: " + url);
            String title = getPageTitle(url);
            System.out.println("-> Title: " + title);
            System.out.println();
        }
    }
}