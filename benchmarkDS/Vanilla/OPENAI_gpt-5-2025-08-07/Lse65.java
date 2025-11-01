import java.io.*;
import java.net.HttpURLConnection;
import java.net.URL;
import java.nio.charset.StandardCharsets;

public class Lse65 {

    public static int uploadPdf(String filePath, String urlStr) {
        String boundary = "----LSE65Boundary" + System.currentTimeMillis();
        String CRLF = "\r\n";
        File file = new File(filePath);
        if (!file.exists() || !file.isFile()) {
            System.out.println("File not found: " + filePath);
            return -1;
        }

        String destPart =
                "--" + boundary + CRLF +
                "Content-Disposition: form-data; name=\"destination\"" + CRLF + CRLF +
                "uploads" + CRLF;

        String filePartHeader =
                "--" + boundary + CRLF +
                "Content-Disposition: form-data; name=\"file\"; filename=\"" + file.getName() + "\"" + CRLF +
                "Content-Type: application/pdf" + CRLF + CRLF;

        String closing = CRLF + "--" + boundary + "--" + CRLF;

        byte[] destBytes = destPart.getBytes(StandardCharsets.UTF_8);
        byte[] fileHeaderBytes = filePartHeader.getBytes(StandardCharsets.UTF_8);
        byte[] closingBytes = closing.getBytes(StandardCharsets.UTF_8);

        long fileLen = file.length();
        long contentLength = destBytes.length + fileHeaderBytes.length + fileLen + closingBytes.length;

        HttpURLConnection conn = null;
        OutputStream out = null;
        InputStream in = null;

        try {
            URL url = new URL(urlStr);
            conn = (HttpURLConnection) url.openConnection();
            conn.setUseCaches(false);
            conn.setDoOutput(true);
            conn.setDoInput(true);
            conn.setRequestMethod("POST");
            conn.setRequestProperty("Content-Type", "multipart/form-data; boundary=" + boundary);
            if (contentLength <= Integer.MAX_VALUE) {
                conn.setFixedLengthStreamingMode((int) contentLength);
            } else {
                conn.setFixedLengthStreamingMode(contentLength);
            }
            conn.setRequestProperty("Connection", "close");

            out = new BufferedOutputStream(conn.getOutputStream());

            out.write(destBytes);
            out.write(fileHeaderBytes);

            try (InputStream fileIn = new BufferedInputStream(new FileInputStream(file))) {
                byte[] buffer = new byte[8192];
                int read;
                while ((read = fileIn.read(buffer)) != -1) {
                    out.write(buffer, 0, read);
                }
            }

            out.write(closingBytes);
            out.flush();

            int code = conn.getResponseCode();

            // Optionally read response (not required to return)
            try {
                in = (code >= 200 && code < 400) ? conn.getInputStream() : conn.getErrorStream();
                if (in != null) {
                    ByteArrayOutputStream baos = new ByteArrayOutputStream();
                    byte[] buf = new byte[4096];
                    int r;
                    while ((r = in.read(buf)) != -1) {
                        baos.write(buf, 0, r);
                    }
                    // You can use baos.toString("UTF-8") if needed
                }
            } catch (Exception ignore) {}

            return code;
        } catch (Exception e) {
            System.out.println("Error uploading " + filePath + ": " + e.getMessage());
            return -1;
        } finally {
            try { if (out != null) out.close(); } catch (Exception ignore) {}
            try { if (in != null) in.close(); } catch (Exception ignore) {}
            if (conn != null) conn.disconnect();
        }
    }

    public static void main(String[] args) {
        String url = "http://127.0.0.1:8080/upload";
        String[] testFiles = {
            "./sample1.pdf",
            "./sample2.pdf",
            "./docs/test.pdf",
            "./data/report.pdf",
            "./dummy.pdf"
        };

        for (String filePath : testFiles) {
            int status = uploadPdf(filePath, url);
            System.out.println("Upload: " + filePath + " -> HTTP Status: " + status);
        }
    }
}