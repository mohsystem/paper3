import java.net.URI;
import java.net.http.HttpClient;
import java.net.http.HttpRequest;
import java.net.http.HttpResponse;
import java.nio.charset.StandardCharsets;
import java.time.Duration;

public class Task160 {
    public static String fetchUrl(String url) {
        try {
            HttpClient client = HttpClient.newBuilder()
                    .followRedirects(HttpClient.Redirect.NORMAL)
                    .connectTimeout(Duration.ofSeconds(10))
                    .build();
            HttpRequest request = HttpRequest.newBuilder()
                    .uri(URI.create(url))
                    .timeout(Duration.ofSeconds(20))
                    .header("User-Agent", "Task160/1.0")
                    .GET()
                    .build();
            HttpResponse<byte[]> response = client.send(request, HttpResponse.BodyHandlers.ofByteArray());
            String body = new String(response.body(), StandardCharsets.UTF_8);
            return "HTTP " + response.statusCode() + "\n" + body;
        } catch (Exception e) {
            return "ERROR: " + e.getClass().getSimpleName() + ": " + e.getMessage();
        }
    }

    public static void main(String[] args) {
        String[] testUrls = new String[] {
                "https://www.example.com/",
                "https://httpbin.org/get",
                "https://jsonplaceholder.typicode.com/todos/1",
                "http://httpstat.us/200",
                "http://httpstat.us/404"
        };
        for (int i = 0; i < testUrls.length; i++) {
            String url = testUrls[i];
            String result = fetchUrl(url);
            System.out.println("=== Test " + (i + 1) + ": " + url + " ===");
            if (result.length() > 500) {
                System.out.println(result.substring(0, 500) + "...");
            } else {
                System.out.println(result);
            }
            System.out.println();
        }
    }
}