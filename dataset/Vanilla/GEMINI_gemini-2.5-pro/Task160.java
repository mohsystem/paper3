import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.io.IOException;
import java.net.HttpURLConnection;
import java.net.URL;
import java.net.MalformedURLException;

public class Task160 {

    /**
     * Makes an HTTP GET request to the specified URL.
     *
     * @param urlString The URL to make the request to.
     * @return A string containing the HTTP status and the response body or an error message.
     */
    public String makeHttpRequest(String urlString) {
        StringBuilder response = new StringBuilder();
        HttpURLConnection conn = null;
        try {
            URL url = new URL(urlString);
            conn = (HttpURLConnection) url.openConnection();
            conn.setRequestMethod("GET");
            conn.setConnectTimeout(5000); // 5 seconds connection timeout
            conn.setReadTimeout(5000);    // 5 seconds read timeout
            conn.setInstanceFollowRedirects(true); // Automatically follow redirects

            int status = conn.getResponseCode();
            
            // Use the appropriate stream based on the response code
            BufferedReader in;
            if (status >= 200 && status < 300) {
                in = new BufferedReader(new InputStreamReader(conn.getInputStream()));
            } else {
                in = new BufferedReader(new InputStreamReader(conn.getErrorStream()));
            }

            String inputLine;
            while ((inputLine = in.readLine()) != null) {
                response.append(inputLine);
                response.append("\n");
            }
            in.close();

            return "Status: " + status + "\nResponse:\n" + response.toString();

        } catch (MalformedURLException e) {
            return "Error: Invalid URL. " + e.getMessage();
        } catch (IOException e) {
            return "Error: Could not connect to the server or read response. " + e.getMessage();
        } catch (Exception e) {
            return "An unexpected error occurred: " + e.getMessage();
        } finally {
            if (conn != null) {
                conn.disconnect();
            }
        }
    }

    public static void main(String[] args) {
        Task160 requester = new Task160();
        String[] testUrls = {
            "https://jsonplaceholder.typicode.com/todos/1",
            "https://httpbin.org/get",
            "https://api.ipify.org?format=json",
            "https://www.google.com",
            "http://thissitedoesnotexist.invalidtld/"
        };

        for (String url : testUrls) {
            System.out.println("Testing URL: " + url);
            String result = requester.makeHttpRequest(url);
            
            // Truncate long responses for better readability in the console
            if (result.length() > 300) {
                System.out.println("Result (first 300 chars):\n" + result.substring(0, 300) + "...\n");
            } else {
                System.out.println("Result:\n" + result + "\n");
            }
            System.out.println("----------------------------------------");
        }
    }
}