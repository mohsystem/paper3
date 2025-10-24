import java.io.*;
import java.net.*;
import java.nio.charset.Charset;
import java.util.*;

public class Task129 {

    public static String fetchFile(String baseUrl, String pathPrefix, String fileName, int timeoutMs) throws IOException {
        String path = normalizePath(pathPrefix, fileName);
        URL url = new URL(new URL(baseUrl), path);
        HttpURLConnection conn = (HttpURLConnection) url.openConnection();
        conn.setRequestMethod("GET");
        conn.setConnectTimeout(timeoutMs);
        conn.setReadTimeout(timeoutMs);
        conn.setRequestProperty("User-Agent", "Task129/1.0");
        conn.setRequestProperty("Accept-Encoding", "identity");
        conn.setRequestProperty("Connection", "close");

        int code = conn.getResponseCode();
        InputStream is = code >= 400 ? conn.getErrorStream() : conn.getInputStream();
        if (is == null) return "";
        byte[] data = readAll(is);
        String contentType = conn.getHeaderField("Content-Type");
        Charset cs = charsetFromContentType(contentType);
        return new String(data, cs);
    }

    private static String normalizePath(String prefix, String fileName) {
        if (prefix == null) prefix = "/";
        if (!prefix.startsWith("/")) prefix = "/" + prefix;
        if (prefix.length() > 1 && prefix.endsWith("/")) {
            // keep as-is
        }
        String encoded = percentEncodePathSegment(fileName == null ? "" : fileName);
        if (encoded.isEmpty()) return prefix.equals("/") ? "/" : prefix;
        if (prefix.endsWith("/")) return prefix + encoded;
        return prefix + "/" + encoded;
    }

    private static byte[] readAll(InputStream is) throws IOException {
        ByteArrayOutputStream bos = new ByteArrayOutputStream();
        byte[] buf = new byte[8192];
        int n;
        while ((n = is.read(buf)) != -1) {
            bos.write(buf, 0, n);
        }
        return bos.toByteArray();
    }

    private static Charset charsetFromContentType(String ct) {
        if (ct != null) {
            String[] parts = ct.split(";");
            for (String p : parts) {
                p = p.trim();
                if (p.toLowerCase(Locale.ROOT).startsWith("charset=")) {
                    String name = p.substring(8).trim().replace("\"", "");
                    try {
                        return Charset.forName(name);
                    } catch (Exception ignored) {}
                }
            }
        }
        return Charset.forName("UTF-8");
    }

    private static String percentEncodePathSegment(String s) {
        if (s == null) return "";
        StringBuilder sb = new StringBuilder();
        try {
            byte[] bytes = s.getBytes("UTF-8");
            for (byte b : bytes) {
                int c = b & 0xFF;
                if ((c >= 'A' && c <= 'Z') ||
                    (c >= 'a' && c <= 'z') ||
                    (c >= '0' && c <= '9') ||
                    c == '-' || c == '_' || c == '.' || c == '~') {
                    sb.append((char) c);
                } else {
                    sb.append('%');
                    String hex = Integer.toHexString(c).toUpperCase(Locale.ROOT);
                    if (hex.length() < 2) sb.append('0');
                    sb.append(hex);
                }
            }
        } catch (UnsupportedEncodingException e) {
            // UTF-8 always supported
        }
        return sb.toString();
    }

    public static void main(String[] args) {
        String baseUrl = "http://example.com";
        String prefix = "/";
        String[] files = new String[] {
            "", "robots.txt", "favicon.ico", "nonexisting.file", "index.html"
        };
        for (int i = 0; i < files.length; i++) {
            try {
                String content = fetchFile(baseUrl, prefix, files[i], 10000);
                System.out.println("Test " + (i + 1) + " (" + files[i] + "): length=" + content.length());
                System.out.println(content.substring(0, Math.min(content.length(), 200)));
                System.out.println("-----");
            } catch (Exception e) {
                System.out.println("Test " + (i + 1) + " (" + files[i] + "): error=" + e.getMessage());
                System.out.println("-----");
            }
        }
    }
}