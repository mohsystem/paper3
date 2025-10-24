import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.net.HttpURLConnection;
import java.net.URL;
import java.nio.charset.StandardCharsets;
import javax.net.ssl.HttpsURLConnection;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

public class Task76 {

    /**
     * Fetches content from a URL and extracts the page title.
     *
     * @param urlString The HTTPS URL to fetch.
     * @return The page title, or an empty string if not found or an error occurs.
     */
    public static String extractPageTitle(String urlString) {
        if (urlString == null || !urlString.toLowerCase().startsWith("https://")) {
            return "";
        }

        StringBuilder content = new StringBuilder();
        try {
            URL url = new URL(urlString);
            HttpURLConnection connection = (HttpURLConnection) url.openConnection();
            
            if (!(connection instanceof HttpsURLConnection)) {
                return "";
            }

            connection.setRequestMethod("GET");
            connection.setConnectTimeout(5000);
            connection.setReadTimeout(5000);
            connection.setInstanceFollowRedirects(true);
            connection.setRequestProperty("User-Agent", "Mozilla/5.0");


            int responseCode = connection.getResponseCode();
            if (responseCode != HttpURLConnection.HTTP_OK) {
                return "";
            }

            try (BufferedReader in = new BufferedReader(
                    new InputStreamReader(connection.getInputStream(), StandardCharsets.UTF_8))) {
                String inputLine;
                // Read only a reasonable amount of data to find the title
                int charsRead = 0;
                int maxChars = 8192; // 8KB is usually enough for the <head> section
                while ((inputLine = in.readLine()) != null && charsRead < maxChars) {
                    content.append(inputLine).append("\n");
                    charsRead += inputLine.length();
                }
            }
        } catch (Exception e) {
            return "";
        }

        // Use regex to find the title, case-insensitively, allowing for attributes and newlines.
        Pattern pattern = Pattern.compile("<title.*?>(.*?)</title>", Pattern.CASE_INSENSITIVE | Pattern.DOTALL);
        Matcher matcher = pattern.matcher(content.toString());

        if (matcher.find()) {
            // Group 1 contains the text between the tags
            return matcher.group(1).trim();
        }

        return "";
    }

    public static void main(String[] args) {
        if (args.length > 0) {
            String url = args[0];
            System.out.println("Extracting title from URL provided via command line: " + url);
            String title = extractPageTitle(url);
            if (!title.isEmpty()) {
                System.out.println("Page Title: " + title);
            } else {
                System.out.println("Could not extract page title.");
            }
            System.out.println("---");
        }
        
        System.out.println("Running test cases...");
        String[] testUrls = {
            "https://www.google.com",
            "https://example.com",
            "https://httpbin.org/html",
            "invalid-url",
            "https://thishostshouldnotexist12345.com"
        };

        for (int i = 0; i < testUrls.length; i++) {
            System.out.println("Test Case " + (i + 1) + ": " + testUrls[i]);
            String title = extractPageTitle(testUrls[i]);
            if (!title.isEmpty()) {
                System.out.println("  -> Title: " + title);
            } else {
                System.out.println("  -> Could not extract title (as expected for invalid cases).");
            }
        }
    }
}