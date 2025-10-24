import java.io.*;
import java.net.*;
import java.nio.charset.Charset;
import java.util.regex.*;
import java.util.zip.GZIPInputStream;

public class Task76 {

    public static String fetchTitle(String urlStr) {
        try {
            return fetchTitleWithRedirects(urlStr, 5);
        } catch (Exception e) {
            return "";
        }
    }

    private static String fetchTitleWithRedirects(String urlStr, int maxRedirects) throws IOException {
        URL url = new URL(urlStr);
        for (int i = 0; i <= maxRedirects; i++) {
            HttpURLConnection conn = (HttpURLConnection) url.openConnection();
            if (conn instanceof HttpsURLConnection) {
                // default SSL verification
            }
            conn.setInstanceFollowRedirects(false);
            conn.setConnectTimeout(15000);
            conn.setReadTimeout(20000);
            conn.setRequestMethod("GET");
            conn.setRequestProperty("User-Agent", "Task76-Java/1.0 (+https://example.com)");
            conn.setRequestProperty("Accept", "text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8");
            conn.setRequestProperty("Accept-Encoding", "gzip");
            conn.setRequestProperty("Connection", "close");

            int status = conn.getResponseCode();
            if (status >= 300 && status < 400) {
                String loc = conn.getHeaderField("Location");
                if (loc == null) throw new IOException("Redirect without Location");
                url = new URL(url, loc);
                conn.disconnect();
                continue;
            }

            InputStream is;
            try {
                is = conn.getInputStream();
            } catch (IOException ex) {
                is = conn.getErrorStream();
                if (is == null) throw ex;
            }

            String contentEncoding = conn.getHeaderField("Content-Encoding");
            if (contentEncoding != null && contentEncoding.toLowerCase().contains("gzip")) {
                is = new GZIPInputStream(is);
            }

            byte[] bytes = readAllBytes(is);
            conn.disconnect();

            String charset = getCharsetFromContentType(conn.getContentType());
            if (charset == null || charset.isEmpty()) charset = "UTF-8";
            String html = new String(bytes, Charset.forName(charset));

            String title = extractTitle(html);
            return title == null ? "" : title;
        }
        throw new IOException("Too many redirects");
    }

    private static String extractTitle(String html) {
        Pattern p = Pattern.compile("(?is)<title[^>]*>(.*?)</title>");
        Matcher m = p.matcher(html);
        if (m.find()) {
            return m.group(1).replaceAll("\\s+", " ").trim();
        }
        return null;
    }

    private static String getCharsetFromContentType(String contentType) {
        if (contentType == null) return null;
        String[] parts = contentType.split(";");
        for (String part : parts) {
            String s = part.trim().toLowerCase();
            if (s.startsWith("charset=")) {
                return s.substring(8).trim().replace("\"", "");
            }
        }
        return null;
    }

    private static byte[] readAllBytes(InputStream is) throws IOException {
        ByteArrayOutputStream bos = new ByteArrayOutputStream(64 * 1024);
        byte[] buf = new byte[8192];
        int r;
        while ((r = is.read(buf)) != -1) {
            bos.write(buf, 0, r);
        }
        return bos.toByteArray();
    }

    public static void main(String[] args) {
        String[] urls;
        if (args != null && args.length > 0) {
            urls = args;
        } else {
            urls = new String[] {
                "https://www.example.com/",
                "https://www.wikipedia.org/",
                "https://www.iana.org/domains/reserved",
                "https://www.github.com/",
                "https://www.stackoverflow.com/"
            };
        }
        for (String u : urls) {
            String title = fetchTitle(u);
            System.out.println(u + " => " + title);
        }
    }
}