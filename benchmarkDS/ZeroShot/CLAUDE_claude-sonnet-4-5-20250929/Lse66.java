
import java.io.*;
import java.nio.file.*;
import java.security.MessageDigest;
import java.util.*;
import javax.servlet.http.*;
import javax.servlet.annotation.*;

@WebServlet("/upload")
@MultipartConfig(
    maxFileSize = 5 * 1024 * 1024, // 5MB
    maxRequestSize = 10 * 1024 * 1024 // 10MB
)
public class Lse66 extends HttpServlet {
    
    private static final String UPLOAD_DIR = "uploads";
    private static final Set<String> ALLOWED_EXTENSIONS = new HashSet<>(Arrays.asList("pdf"));
    private static final long MAX_FILE_SIZE = 5 * 1024 * 1024; // 5MB
    
    @Override
    protected void doPost(HttpServletRequest request, HttpServletResponse response) 
            throws IOException {
        try {
            Part filePart = request.getPart("file");
            String fileName = getFileName(filePart);
            
            if (isValidPdfFile(filePart, fileName)) {
                String sanitizedFileName = sanitizeFileName(fileName);
                String uniqueFileName = generateUniqueFileName(sanitizedFileName);
                Path uploadPath = Paths.get(UPLOAD_DIR);
                
                if (!Files.exists(uploadPath)) {
                    Files.createDirectories(uploadPath);
                }
                
                Path filePath = uploadPath.resolve(uniqueFileName);
                Files.copy(filePart.getInputStream(), filePath, StandardCopyOption.REPLACE_EXISTING);
                
                response.getWriter().write("File uploaded successfully: " + uniqueFileName);
            } else {
                response.setStatus(HttpServletResponse.SC_BAD_REQUEST);
                response.getWriter().write("Invalid file. Only PDF files are allowed.");
            }
        } catch (Exception e) {
            response.setStatus(HttpServletResponse.SC_INTERNAL_SERVER_ERROR);
            response.getWriter().write("Error uploading file: " + e.getMessage());
        }
    }
    
    private String getFileName(Part part) {
        String contentDisposition = part.getHeader("content-disposition");
        for (String content : contentDisposition.split(";")) {
            if (content.trim().startsWith("filename")) {
                return content.substring(content.indexOf('=') + 1).trim().replace("\\"", "");
            }
        }
        return null;
    }
    
    private boolean isValidPdfFile(Part filePart, String fileName) throws IOException {
        if (fileName == null || filePart.getSize() > MAX_FILE_SIZE) {
            return false;
        }
        
        String extension = getFileExtension(fileName).toLowerCase();
        if (!ALLOWED_EXTENSIONS.contains(extension)) {
            return false;
        }
        
        // Check PDF magic number
        byte[] header = new byte[4];
        InputStream is = filePart.getInputStream();
        is.read(header);
        is.close();
        
        return header[0] == 0x25 && header[1] == 0x50 && header[2] == 0x44 && header[3] == 0x46; // %PDF
    }
    
    private String getFileExtension(String fileName) {
        int lastIndexOf = fileName.lastIndexOf(".");
        if (lastIndexOf == -1) {
            return "";
        }
        return fileName.substring(lastIndexOf + 1);
    }
    
    private String sanitizeFileName(String fileName) {
        return fileName.replaceAll("[^a-zA-Z0-9.-]", "_");
    }
    
    private String generateUniqueFileName(String fileName) {
        String timestamp = String.valueOf(System.currentTimeMillis());
        String nameWithoutExt = fileName.substring(0, fileName.lastIndexOf('.'));
        String ext = fileName.substring(fileName.lastIndexOf('.'));
        return nameWithoutExt + "_" + timestamp + ext;
    }
    
    public static void main(String[] args) {
        System.out.println("PDF Upload Servlet - Test Cases");
        System.out.println("1. Valid PDF file: document.pdf");
        System.out.println("2. Invalid extension: document.txt");
        System.out.println("3. File too large: > 5MB");
        System.out.println("4. Invalid PDF content");
        System.out.println("5. Special characters in filename: doc@#$%.pdf");
    }
}
