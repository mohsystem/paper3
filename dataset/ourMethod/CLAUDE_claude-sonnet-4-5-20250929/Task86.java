
// WARNING: This code demonstrates a dangerous pattern and should NOT be used in production.
// Downloading and executing arbitrary executables is a critical security vulnerability.

import javax.net.ssl.*;
import java.io.*;
import java.net.URI;
import java.net.URL;
import java.nio.file.*;
import java.security.SecureRandom;
import java.util.HexFormat;

public class Task86 {
    
    private static final long MAX_DOWNLOAD_SIZE = 500L * 1024 * 1024; // 500MB
    
    /**
     * SECURITY WARNING: This method downloads and executes arbitrary code.
     * In production, you MUST:
     * 1. Verify cryptographic signatures using the vendor's public key
     * 2. Validate file hashes against known-good values
     * 3. Implement user consent with security warnings
     * 4. Use sandboxing for execution
     * 5. Maintain audit logs
     */
    public static boolean downloadAndExecute(String urlString) {
        // Input validation per Rules #5
        if (urlString == null || urlString.isEmpty()) {
            System.err.println("Error: Invalid URL provided");
            return false;
        }
        
        URI uri;
        try {
            uri = new URI(urlString);
            if (!"https".equalsIgnoreCase(uri.getScheme())) {
                System.err.println("Error: Only HTTPS URLs are permitted");
                return false;
            }
            if (uri.getHost() == null || uri.getHost().isEmpty()) {
                System.err.println("Error: Invalid URL format");
                return false;
            }
        } catch (Exception e) {
            System.err.println("Error: Malformed URL - " + e.getMessage());
            return false;
        }
        
        Path tempFile = null;
        
        try {
            // Create temporary file with restrictive permissions per Rules #1
            SecureRandom secureRandom = new SecureRandom();
            byte[] randomBytes = new byte[16];
            secureRandom.nextBytes(randomBytes);
            String randomSuffix = HexFormat.of().formatHex(randomBytes);
            
            tempFile = Files.createTempFile("download_" + randomSuffix, ".exe");
            
            // Set restrictive permissions (owner only) per Rules #1
            if (tempFile.getFileSystem().supportedFileAttributeViews().contains("posix")) {
                Files.setPosixFilePermissions(tempFile, 
                    java.util.Set.of(
                        java.nio.file.attribute.PosixFilePermission.OWNER_READ,
                        java.nio.file.attribute.PosixFilePermission.OWNER_WRITE
                    )
                );
            }
            
            // Configure secure TLS context per Rules #3, #4, #13
            SSLContext sslContext = SSLContext.getInstance("TLS");
            sslContext.init(null, null, secureRandom);
            
            SSLSocketFactory sslSocketFactory = sslContext.getSocketFactory();
            
            URL url = uri.toURL();
            HttpsURLConnection connection = (HttpsURLConnection) url.openConnection();
            
            // Enforce TLS 1.2+ and certificate validation per Rules #13
            connection.setSSLSocketFactory(sslSocketFactory);
            connection.setHostnameVerifier(HttpsURLConnection.getDefaultHostnameVerifier());
            connection.setConnectTimeout(30000);
            connection.setReadTimeout(30000);
            connection.setRequestProperty("User-Agent", "SecureDownloader/1.0");
            
            System.out.println("Downloading from: " + urlString);
            System.out.println("WARNING: Executing downloaded executables is dangerous!");
            
            long downloaded = 0;
            
            // Download with size limit per Rules #6
            try (InputStream in = connection.getInputStream();
                 OutputStream out = Files.newOutputStream(tempFile, 
                     StandardOpenOption.WRITE, StandardOpenOption.TRUNCATE_EXISTING)) {
                
                byte[] buffer = new byte[8192];
                int bytesRead;
                
                while ((bytesRead = in.read(buffer)) != -1) {
                    downloaded += bytesRead;
                    if (downloaded > MAX_DOWNLOAD_SIZE) {
                        System.err.println("Error: File exceeds maximum size of " + MAX_DOWNLOAD_SIZE + " bytes");
                        return false;
                    }
                    out.write(buffer, 0, bytesRead);
                }
            }
            
            System.out.println("Download complete: " + downloaded + " bytes");
            System.out.println("Saved to: " + tempFile);
            
            // CRITICAL SECURITY ISSUE: No signature verification!
            System.out.println("\\nWARNING: No signature verification performed!");
            System.out.println("This executable could be malicious!");
            
            // Execute the downloaded file (SECURITY ISSUE: Running untrusted code)
            System.out.println("\\nExecuting: " + tempFile);
            
            ProcessBuilder processBuilder = new ProcessBuilder(tempFile.toString());
            processBuilder.redirectErrorStream(true);
            
            Process process = processBuilder.start();
            
            // Wait with timeout
            boolean finished = process.waitFor(300, java.util.concurrent.TimeUnit.SECONDS);
            
            if (!finished) {
                process.destroyForcibly();
                System.err.println("Error: Execution timeout");
                return false;
            }
            
            System.out.println("Exit code: " + process.exitValue());
            
            return true;
            
        } catch (Exception e) {
            System.err.println("Error: " + e.getMessage());
            return false;
        } finally {
            // Cleanup temporary file
            if (tempFile != null) {
                try {
                    Files.deleteIfExists(tempFile);
                } catch (IOException e) {
                    // Ignore cleanup errors
                }
            }
        }
    }
    
    public static void main(String[] args) {
        System.out.println("=".repeat(70));
        System.out.println("SECURITY WARNING: This program downloads and executes code!");
        System.out.println("This is a DANGEROUS operation and should only be used for demonstration.");
        System.out.println("=".repeat(70));
        System.out.println();
        
        // Test cases (DO NOT RUN with real executables)
        String[] testUrls = {
            "https://example.com/test.exe",      // Would fail - not a real file
            "http://insecure.com/test.exe",      // Rejected - not HTTPS
            "",                                   // Rejected - empty URL
            "https://",                          // Rejected - invalid format
            "https://example.com/large.exe"      // Would test size limits
        };
        
        System.out.println("Test case validation (not executing):");
        for (int i = 0; i < testUrls.length; i++) {
            String testUrl = testUrls[i];
            System.out.println("\\nTest " + (i + 1) + ": " + (testUrl.isEmpty() ? "(empty)" : testUrl));
            
            if (testUrl.isEmpty() || !testUrl.startsWith("https://") || testUrl.equals("https://")) {
                System.out.println("  -> Would be rejected by validation");
            }
        }
    }
}
