import java.io.FileWriter;
import java.io.IOException;
import java.io.PrintWriter;

public class Task120 {

    /**
     * Creates an HTML file with the given content.
     *
     * @param content  The user-supplied string to display on the web page.
     * @param filename The name of the HTML file to create (e.g., "index.html").
     */
    public static void createWebPage(String content, String filename) {
        String htmlContent = "<!DOCTYPE html>\n" +
                             "<html lang=\"en\">\n" +
                             "<head>\n" +
                             "    <meta charset=\"UTF-8\">\n" +
                             "    <title>User Input Display</title>\n" +
                             "</head>\n" +
                             "<body>\n" +
                             "    <h1>User-Supplied Content:</h1>\n" +
                             "    <p>" + content + "</p>\n" +
                             "</body>\n" +
                             "</html>";

        try (PrintWriter out = new PrintWriter(new FileWriter(filename))) {
            out.println(htmlContent);
            System.out.println("Successfully created " + filename);
        } catch (IOException e) {
            System.err.println("An error occurred while writing to the file: " + e.getMessage());
        }
    }

    public static void main(String[] args) {
        // 5 Test Cases
        createWebPage("Hello, World!", "java_test1.html");
        createWebPage("This is a test from Java.", "java_test2.html");
        createWebPage("12345", "java_test3.html");
        createWebPage("Special characters: <script>alert('xss')</script>", "java_test4.html");
        createWebPage("A longer sentence to see how it wraps inside the paragraph tag on the web page.", "java_test5.html");
    }
}