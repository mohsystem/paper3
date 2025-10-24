
import javax.net.ssl.*;
import java.io.*;
import java.security.cert.X509Certificate;
import java.security.SecureRandom;

public class Task105 {
    
    /**
     * Establishes a secure SSL/TLS connection with a remote server
     * @param hostname The remote server hostname
     * @param port The remote server port
     * @param message The message to send (optional)
     * @return Response from the server or error message
     */
    public static String establishSecureConnection(String hostname, int port, String message) {
        if (hostname == null || hostname.trim().isEmpty()) {
            return "Error: Invalid hostname";
        }
        
        if (port < 1 || port > 65535) {
            return "Error: Invalid port number";
        }
        
        SSLSocketFactory factory = null;
        SSLSocket socket = null;
        BufferedReader reader = null;
        PrintWriter writer = null;
        StringBuilder response = new StringBuilder();
        
        try {
            // Create SSL context with secure protocols
            SSLContext sslContext = SSLContext.getInstance("TLSv1.3");
            sslContext.init(null, null, new SecureRandom());
            
            factory = sslContext.getSocketFactory();
            socket = (SSLSocket) factory.createSocket(hostname, port);
            
            // Enable only secure protocols
            String[] enabledProtocols = {"TLSv1.3", "TLSv1.2"};
            socket.setEnabledProtocols(enabledProtocols);
            
            // Enable only secure cipher suites
            String[] secureCiphers = {
                "TLS_AES_256_GCM_SHA384",
                "TLS_AES_128_GCM_SHA256",
                "TLS_ECDHE_RSA_WITH_AES_256_GCM_SHA384",
                "TLS_ECDHE_RSA_WITH_AES_128_GCM_SHA256"
            };
            socket.setEnabledCipherSuites(secureCiphers);
            
            // Start handshake
            socket.startHandshake();
            
            // Verify hostname
            SSLSession session = socket.getSession();
            if (!verifyHostname(hostname, session)) {
                return "Error: Hostname verification failed";
            }
            
            // Send message if provided
            if (message != null && !message.isEmpty()) {
                writer = new PrintWriter(new OutputStreamWriter(socket.getOutputStream()), true);
                writer.println(message);
            }
            
            // Read response
            reader = new BufferedReader(new InputStreamReader(socket.getInputStream()));
            String line;
            int maxLines = 100; // Prevent infinite read
            int lineCount = 0;
            
            while ((line = reader.readLine()) != null && lineCount < maxLines) {
                response.append(line).append("\\n");
                lineCount++;
            }
            
            return "Connection successful. Protocol: " + session.getProtocol() + 
                   ", Cipher: " + session.getCipherSuite() + "\\nResponse: " + response.toString();
            
        } catch (Exception e) {
            return "Error: " + e.getMessage();
        } finally {
            // Clean up resources
            try {
                if (reader != null) reader.close();
                if (writer != null) writer.close();
                if (socket != null) socket.close();
            } catch (IOException e) {
                // Log error but don't expose details\n            }\n        }\n    }\n    \n    /**\n     * Verifies hostname against SSL session\n     */\n    private static boolean verifyHostname(String hostname, SSLSession session) {\n        try {\n            HostnameVerifier verifier = HttpsURLConnection.getDefaultHostnameVerifier();\n            return verifier.verify(hostname, session);\n        } catch (Exception e) {\n            return false;\n        }\n    }\n    \n    /**\n     * Simpler method for basic HTTPS connection\n     */\n    public static String establishHttpsConnection(String urlString) {\n        if (urlString == null || urlString.trim().isEmpty()) {\n            return "Error: Invalid URL";\n        }\n        \n        HttpsURLConnection connection = null;\n        BufferedReader reader = null;\n        StringBuilder response = new StringBuilder();\n        \n        try {\n            // Validate URL\n            if (!urlString.startsWith("https://")) {\n                return "Error: Only HTTPS URLs are allowed";\n            }\n            \n            java.net.URL url = new java.net.URL(urlString);\n            \n            // Create SSL context with secure settings\n            SSLContext sslContext = SSLContext.getInstance("TLSv1.3");\n            sslContext.init(null, null, new SecureRandom());\n            \n            connection = (HttpsURLConnection) url.openConnection();\n            connection.setSSLSocketFactory(sslContext.getSocketFactory());\n            \n            // Set secure connection properties\n            connection.setRequestMethod("GET");\n            connection.setConnectTimeout(10000); // 10 seconds\n            connection.setReadTimeout(10000);\n            connection.setHostnameVerifier(HttpsURLConnection.getDefaultHostnameVerifier());\n            \n            // Read response\n            int responseCode = connection.getResponseCode();\n            if (responseCode == 200) {\n                reader = new BufferedReader(new InputStreamReader(connection.getInputStream()));\n                String line;\n                int maxLines = 100;\n                int lineCount = 0;\n                \n                while ((line = reader.readLine()) != null && lineCount < maxLines) {\n                    response.append(line).append("\\n");\n                    lineCount++;\n                }\n                return "HTTPS connection successful. Response code: " + responseCode + "\\n" + response.toString();\n            } else {\n                return "Error: HTTP response code " + responseCode;\n            }\n            \n        } catch (Exception e) {\n            return "Error: " + e.getMessage();\n        } finally {\n            try {\n                if (reader != null) reader.close();\n                if (connection != null) connection.disconnect();\n            } catch (IOException e) {\n                // Log error but don't expose details
            }
        }
    }
    
    public static void main(String[] args) {
        System.out.println("=== SSL/TLS Secure Connection Test Cases ===\\n");
        
        // Test Case 1: Valid HTTPS connection
        System.out.println("Test Case 1: Valid HTTPS URL");
        String result1 = establishHttpsConnection("https://www.google.com");
        System.out.println(result1.substring(0, Math.min(200, result1.length())) + "...\\n");
        
        // Test Case 2: Invalid URL (non-HTTPS)
        System.out.println("Test Case 2: Non-HTTPS URL");
        String result2 = establishHttpsConnection("http://www.example.com");
        System.out.println(result2 + "\\n");
        
        // Test Case 3: Empty hostname
        System.out.println("Test Case 3: Empty hostname");
        String result3 = establishSecureConnection("", 443, "GET / HTTP/1.1");
        System.out.println(result3 + "\\n");
        
        // Test Case 4: Invalid port
        System.out.println("Test Case 4: Invalid port");
        String result4 = establishSecureConnection("www.google.com", 99999, "GET / HTTP/1.1");
        System.out.println(result4 + "\\n");
        
        // Test Case 5: Null URL
        System.out.println("Test Case 5: Null URL");
        String result5 = establishHttpsConnection(null);
        System.out.println(result5 + "\\n");
    }
}
