
import java.io.*;
import java.nio.file.*;
import javax.servlet.http.*;
import javax.servlet.annotation.*;

@WebServlet("/images/*")
public class Lse47 extends HttpServlet {
    private static final String IMAGES_FOLDER = "images";
    
    @Override
    protected void doGet(HttpServletRequest request, HttpServletResponse response) 
            throws IOException {
        serveImage(request, response);
    }
    
    public void serveImage(HttpServletRequest request, HttpServletResponse response) 
            throws IOException {
        // Get the requested file path
        String pathInfo = request.getPathInfo();
        if (pathInfo == null || pathInfo.equals("/")) {
            response.sendError(HttpServletResponse.SC_BAD_REQUEST, "No file specified");
            return;
        }
        
        // Remove leading slash
        String fileName = pathInfo.substring(1);
        
        // Prevent directory traversal attacks
        if (fileName.contains("..") || fileName.contains("/") || fileName.contains("\\\\")) {\n            response.sendError(HttpServletResponse.SC_FORBIDDEN, "Invalid file path");\n            return;\n        }\n        \n        // Build the file path\n        Path filePath = Paths.get(IMAGES_FOLDER, fileName);\n        File file = filePath.toFile();\n        \n        // Check if file exists and is a file (not directory)\n        if (!file.exists() || !file.isFile()) {\n            response.sendError(HttpServletResponse.SC_NOT_FOUND, "File not found");\n            return;\n        }\n        \n        // Set content type based on file extension\n        String contentType = getServletContext().getMimeType(fileName);\n        if (contentType == null) {\n            contentType = "application/octet-stream";\n        }\n        response.setContentType(contentType);\n        response.setContentLength((int) file.length());\n        \n        // Send file to browser\n        try (FileInputStream fis = new FileInputStream(file);\n             OutputStream out = response.getOutputStream()) {\n            byte[] buffer = new byte[4096];\n            int bytesRead;\n            while ((bytesRead = fis.read(buffer)) != -1) {\n                out.write(buffer, 0, bytesRead);\n            }\n        }\n    }\n    \n    // Main method for testing\n    public static void main(String[] args) {\n        System.out.println("This is a servlet class. Deploy it to a servlet container like Tomcat.");\n        System.out.println("Test URLs:");\n        System.out.println("1. http://localhost:8080/images/test.jpg");\n        System.out.println("2. http://localhost:8080/images/logo.png");\n        System.out.println("3. http://localhost:8080/images/banner.gif");\n        System.out.println("4. http://localhost:8080/images/photo.bmp");\n        System.out.println("5. http://localhost:8080/images/icon.ico");
    }
}
