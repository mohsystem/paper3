
import java.io.*;
import java.nio.file.*;
import javax.servlet.http.*;
import javax.servlet.*;

public class Lse72 extends HttpServlet {
    private static final String UPLOAD_DIR = "uploads";
    
    public void uploadFile(HttpServletRequest request, HttpServletResponse response) 
            throws ServletException, IOException {
        
        // Create uploads directory if it doesn't exist\n        String uploadPath = getServletContext().getRealPath("") + File.separator + UPLOAD_DIR;\n        File uploadDir = new File(uploadPath);\n        if (!uploadDir.exists()) {\n            uploadDir.mkdir();\n        }\n        \n        try {\n            Part filePart = request.getPart("file");\n            String fileName = getFileName(filePart);\n            \n            if (fileName != null && !fileName.isEmpty()) {\n                String filePath = uploadPath + File.separator + fileName;\n                filePart.write(filePath);\n                response.getWriter().println("File uploaded successfully: " + fileName);\n            }\n        } catch (Exception e) {\n            response.getWriter().println("Error uploading file: " + e.getMessage());\n        }\n    }\n    \n    private String getFileName(Part part) {\n        String contentDisp = part.getHeader("content-disposition");\n        String[] tokens = contentDisp.split(";");\n        for (String token : tokens) {\n            if (token.trim().startsWith("filename")) {\n                return token.substring(token.indexOf("=") + 2, token.length() - 1);\n            }\n        }\n        return null;\n    }\n    \n    public static void main(String[] args) {\n        // This is a servlet-based implementation that requires a web container to run\n        // For demonstration, here's a standalone file upload simulation
        
        String uploadDir = "uploads";
        File dir = new File(uploadDir);
        if (!dir.exists()) {
            dir.mkdir();
        }
        
        System.out.println("Test Case 1: Upload directory created");
        System.out.println("Directory exists: " + dir.exists());
        
        System.out.println("\\nTest Case 2: Simulate file save");
        try {
            String testFile = "test1.txt";
            String content = "Test content 1";
            Files.write(Paths.get(uploadDir, testFile), content.getBytes());
            System.out.println("File saved: " + testFile);
        } catch (IOException e) {
            System.out.println("Error: " + e.getMessage());
        }
        
        System.out.println("\\nTest Case 3: Simulate another file save");
        try {
            String testFile = "test2.txt";
            String content = "Test content 2";
            Files.write(Paths.get(uploadDir, testFile), content.getBytes());
            System.out.println("File saved: " + testFile);
        } catch (IOException e) {
            System.out.println("Error: " + e.getMessage());
        }
        
        System.out.println("\\nTest Case 4: List uploaded files");
        File[] files = dir.listFiles();
        if (files != null) {
            for (File file : files) {
                System.out.println("Found file: " + file.getName());
            }
        }
        
        System.out.println("\\nTest Case 5: Verify file content");
        try {
            String content = new String(Files.readAllBytes(Paths.get(uploadDir, "test1.txt")));
            System.out.println("Content of test1.txt: " + content);
        } catch (IOException e) {
            System.out.println("Error: " + e.getMessage());
        }
    }
}
