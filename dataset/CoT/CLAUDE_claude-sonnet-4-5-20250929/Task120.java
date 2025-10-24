
import java.io.*;
import javax.servlet.*;
import javax.servlet.http.*;
import org.apache.commons.text.StringEscapeUtils;

public class Task120 extends HttpServlet {
    
    // Method to safely encode user input for HTML display
    public static String encodeForHTML(String input) {
        if (input == null) {
            return "";
        }
        // Manual HTML encoding to prevent XSS
        return input.replace("&", "&amp;")
                   .replace("<", "&lt;")
                   .replace(">", "&gt;")
                   .replace("\\"", "&quot;")
                   .replace("'", "&#x27;")
                   .replace("/", "&#x2F;");
    }
    
    // Method to display user input safely
    public static String displayUserInput(String userInput) {
        String safeInput = encodeForHTML(userInput);
        return "<html><body><h1>User Input Display</h1><p>You entered: " + 
               safeInput + "</p></body></html>";
    }
    
    protected void doGet(HttpServletRequest request, HttpServletResponse response) 
            throws ServletException, IOException {
        response.setContentType("text/html;charset=UTF-8");
        String userInput = request.getParameter("input");
        
        PrintWriter out = response.getWriter();
        out.println(displayUserInput(userInput != null ? userInput : ""));
    }
    
    // Main method with test cases for demonstration
    public static void main(String[] args) {
        System.out.println("Test Case 1 - Normal input:");
        System.out.println(displayUserInput("Hello World"));
        System.out.println();
        
        System.out.println("Test Case 2 - Input with HTML tags:");
        System.out.println(displayUserInput("<script>alert('XSS')</script>"));
        System.out.println();
        
        System.out.println("Test Case 3 - Input with special characters:");
        System.out.println(displayUserInput("Tom & Jerry's \\"Adventure\\""));
        System.out.println();
        
        System.out.println("Test Case 4 - Input with single quotes:");
        System.out.println(displayUserInput("It's a beautiful day"));
        System.out.println();
        
        System.out.println("Test Case 5 - Input with mixed content:");
        System.out.println(displayUserInput("<div>Hello</div> & <span>World</span>"));
    }
}
