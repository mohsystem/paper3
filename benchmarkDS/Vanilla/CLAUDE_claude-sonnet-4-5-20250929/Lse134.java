
import java.io.IOException;
import java.io.PrintWriter;
import java.net.URLDecoder;
import java.nio.charset.StandardCharsets;
import javax.servlet.ServletException;
import javax.servlet.annotation.WebServlet;
import javax.servlet.http.HttpServlet;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import org.apache.commons.text.StringEscapeUtils;

public class Lse134 {
    
    public static String hello(String username) {
        String escapedUsername = escapeHtml(username);
        return "Hello " + escapedUsername;
    }
    
    public static String escapeHtml(String input) {
        if (input == null) {
            return null;
        }
        StringBuilder escaped = new StringBuilder();
        for (char c : input.toCharArray()) {
            switch (c) {
                case '<':
                    escaped.append("&lt;");
                    break;
                case '>':
                    escaped.append("&gt;");
                    break;
                case '&':
                    escaped.append("&amp;");
                    break;
                case '"':
                    escaped.append("&quot;");
                    break;
                case '\\'':
                    escaped.append("&#x27;");
                    break;
                case '/':
                    escaped.append("&#x2F;");
                    break;
                default:
                    escaped.append(c);
            }
        }
        return escaped.toString();
    }
    
    public static void main(String[] args) {
        // Test cases
        System.out.println(hello("John"));
        System.out.println(hello("Alice"));
        System.out.println(hello("<script>alert('XSS')</script>"));
        System.out.println(hello("User&Name"));
        System.out.println(hello("Test'User\\"123"));
    }
}
