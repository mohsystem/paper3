
import javax.net.ssl.HttpsURLConnection;
import javax.net.ssl.SSLContext;
import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.net.URL;
import java.nio.charset.StandardCharsets;
import java.security.SecureRandom;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

public class Task76 {
    private static final int MAX_CONTENT_LENGTH = 10485760; // 10MB
    private static final int TIMEOUT_MS = 10000;
    private static final Pattern TITLE_PATTERN = Pattern.compile("<title[^>]*>([^<]*)</title>", Pattern.CASE_INSENSITIVE);

    public static String extractTitle(String urlString) {
        if (urlString == null || urlString.trim().isEmpty()) {
            throw new IllegalArgumentException("URL cannot be null or empty");
        }

        urlString = urlString.trim();
        
        if (!urlString.toLowerCase().startsWith("https://")) {
            throw new IllegalArgumentException("Only HTTPS URLs are allowed");
        }

        if (urlString.length() > 2048) {
            throw new IllegalArgumentException("URL exceeds maximum allowed length");
        }

        HttpsURLConnection connection = null;
        try {
            URL url = new URL(urlString);
            
            if (!"https".equalsIgnoreCase(url.getProtocol())) {
                throw new IllegalArgumentException("Only HTTPS protocol is allowed");
            }

            SSLContext sslContext = SSLContext.getInstance("TLSv1.3");
            sslContext.init(null, null, new SecureRandom());

            connection = (HttpsURLConnection) url.openConnection();
            connection.setSSLSocketFactory(sslContext.getSocketFactory());
            connection.setRequestMethod("GET");
            connection.setConnectTimeout(TIMEOUT_MS);
            connection.setReadTimeout(TIMEOUT_MS);
            connection.setInstanceFollowRedirects(false);
            connection.setRequestProperty("User-Agent", "TitleExtractor/1.0");

            int responseCode = connection.getResponseCode();
            if (responseCode != 200) {
                throw new RuntimeException("HTTP request failed with code: " + responseCode);
            }

            String contentType = connection.getContentType();
            if (contentType != null && !contentType.toLowerCase().contains("text/html")) {
                throw new RuntimeException("Response is not HTML content");
            }

            StringBuilder content = new StringBuilder();
            try (BufferedReader reader = new BufferedReader(
                    new InputStreamReader(connection.getInputStream(), StandardCharsets.UTF_8))) {
                String line;
                int totalRead = 0;
                while ((line = reader.readLine()) != null) {
                    totalRead += line.length();
                    if (totalRead > MAX_CONTENT_LENGTH) {
                        throw new RuntimeException("Content exceeds maximum allowed size");
                    }
                    content.append(line).append("\\n");
                }
            }

            Matcher matcher = TITLE_PATTERN.matcher(content.toString());
            if (matcher.find()) {
                String title = matcher.group(1).trim();
                return title.isEmpty() ? "No title found" : title;
            }

            return "No title found";

        } catch (IllegalArgumentException e) {
            throw e;
        } catch (Exception e) {
            throw new RuntimeException("Failed to extract title: " + e.getMessage());
        } finally {
            if (connection != null) {
                connection.disconnect();
            }
        }
    }

    public static void main(String[] args) {
        if (args.length == 0) {
            System.err.println("Usage: java Task76 <https-url>");
            System.exit(1);
        }

        String[] testUrls = {
            args[0],
            "https://www.example.com",
            "https://www.wikipedia.org",
            "https://www.github.com",
            "https://www.google.com"
        };

        for (String url : testUrls) {
            try {
                String title = extractTitle(url);
                System.out.println("URL: " + url);
                System.out.println("Title: " + title);
                System.out.println();
            } catch (Exception e) {
                System.err.println("Error for URL " + url + ": " + e.getMessage());
                System.err.println();
            }
        }
    }
}
