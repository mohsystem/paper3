import javax.script.ScriptEngine;
import javax.script.ScriptEngineManager;
import javax.script.ScriptException;
import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.net.HttpURLConnection;
import java.net.URL;
import java.net.MalformedURLException;
import java.nio.charset.StandardCharsets;

public class Task64 {

    private static final int CONNECT_TIMEOUT = 5000; // 5 seconds
    private static final int READ_TIMEOUT = 5000;    // 5 seconds
    private static final long MAX_RESPONSE_SIZE = 1024 * 1024; // 1 MB

    /**
     * Fetches content from a URL, parses it as JSON, and returns the result.
     *
     * @param urlString The URL to fetch the JSON from.
     * @return A Java object representing the JSON (e.g., Map, List).
     * @throws IOException      if a network error occurs.
     * @throws SecurityException if the URL protocol is not HTTP or HTTPS.
     * @throws ScriptException  if the content cannot be parsed as JSON.
     */
    public static Object fetchAndParseJson(String urlString) throws IOException, SecurityException, ScriptException {
        // 1. Security: Validate URL protocol to prevent Server-Side Request Forgery (SSRF)
        URL url;
        try {
            url = new URL(urlString);
            String protocol = url.getProtocol().toLowerCase();
            if (!protocol.equals("http") && !protocol.equals("https")) {
                throw new SecurityException("Invalid protocol: " + protocol + ". Only HTTP and HTTPS are allowed.");
            }
        } catch (MalformedURLException e) {
            throw new IOException("Malformed URL: " + urlString, e);
        }

        HttpURLConnection connection = null;
        try {
            connection = (HttpURLConnection) url.openConnection();

            // 2. Security: Set timeouts to prevent Denial-of-Service (DoS) attacks from slow servers
            connection.setConnectTimeout(CONNECT_TIMEOUT);
            connection.setReadTimeout(READ_TIMEOUT);
            connection.setRequestMethod("GET");
            // Set a user-agent to appear as a standard browser
            connection.setRequestProperty("User-Agent", "Mozilla/5.0");


            int responseCode = connection.getResponseCode();
            if (responseCode != HttpURLConnection.HTTP_OK) {
                throw new IOException("HTTP request failed with response code: " + responseCode);
            }

            // 3. Security: Limit response size to prevent OutOfMemoryError (DoS vector)
            StringBuilder content = new StringBuilder();
            try (BufferedReader in = new BufferedReader(new InputStreamReader(connection.getInputStream(), StandardCharsets.UTF_8))) {
                char[] buffer = new char[1024];
                int bytesRead;
                long totalBytesRead = 0;
                while ((bytesRead = in.read(buffer, 0, buffer.length)) != -1) {
                    totalBytesRead += bytesRead;
                    if (totalBytesRead > MAX_RESPONSE_SIZE) {
                        throw new IOException("Response size exceeds the limit of " + MAX_RESPONSE_SIZE + " bytes.");
                    }
                    content.append(buffer, 0, bytesRead);
                }
            }

            String jsonString = content.toString();

            // 4. Parse JSON using a built-in script engine to avoid external dependencies.
            // Note: Nashorn JS engine is deprecated in JDK 11 and removed in JDK 15.
            // This works well on JDK 8-14. For newer JDKs, GraalVM JS might be needed.
            ScriptEngineManager manager = new ScriptEngineManager();
            ScriptEngine engine = manager.getEngineByName("JavaScript");
            if (engine == null) {
                throw new RuntimeException("JavaScript engine not found. This method requires a JS script engine for JSON parsing.");
            }
            // Pass the string as a variable to the engine to prevent script injection vulnerabilities.
            engine.put("jsonString", jsonString);
            return engine.eval("JSON.parse(jsonString)");
        } finally {
            if (connection != null) {
                connection.disconnect();
            }
        }
    }

    public static void main(String[] args) {
        String[] testUrls = {
            "https://jsonplaceholder.typicode.com/posts/1",      // Test 1: Success, single object
            "https://jsonplaceholder.typicode.com/posts",         // Test 2: Success, array of objects
            "https://jsonplaceholder.typicode.com/posts/999999",  // Test 3: Client Error (404)
            "https://nonexistent-domain-for-testing123.com",     // Test 4: Network Error
            "https://www.google.com",                             // Test 5: JSON Parse Error
            "file:///etc/hosts"                                   // Test 6: Security Error (Invalid Protocol)
        };

        for (int i = 0; i < testUrls.length; i++) {
            System.out.println("--- Test Case " + (i + 1) + ": " + testUrls[i] + " ---");
            try {
                Object jsonObject = fetchAndParseJson(testUrls[i]);
                System.out.println("Success! Parsed JSON object.");
                // Note: The returned object is a ScriptObjectMirror which may not have a pretty toString().
                // For demonstration, we just print the object itself.
                // In a real application, you would cast and navigate it like a Map.
                System.out.println(jsonObject.toString().substring(0, Math.min(200, jsonObject.toString().length())) + "...");
            } catch (Exception e) {
                System.err.println("Failed: " + e.getClass().getSimpleName() + " - " + e.getMessage());
            }
            System.out.println();
        }
    }
}