// Chain-of-Through secure implementation
// 1) Problem understanding:
//    Build a function that fetches HTML from a given URL, extracts hyperlinks, and stores them into a local SQLite database.
//    Inputs: url (String), dbPath (String). Output: number of links stored (int).
// 2) Security requirements:
//    - Only allow http/https URLs; reject others.
//    - Set timeouts and follow limited redirects.
//    - Use safe User-Agent.
//    - Enforce maximum download size (2 MB).
//    - Use parameterized/PreparedStatements for DB writes.
//    - Handle errors gracefully and close resources.
// 3) Secure coding generation: Implement with validation, limits, prepared statements, and try-with-resources.
// 4) Code review: Ensured no SQL injection, validated URL scheme, applied timeouts and size limits, avoided dangerous parsing.
// 5) Secure code output: Final code below mitigates identified risks.

import java.net.URI;
import java.net.http.HttpClient;
import java.net.http.HttpTimeoutException;
import java.net.http.HttpRequest;
import java.net.http.HttpResponse;
import java.net.http.HttpClient.Redirect;
import java.net.http.HttpClient.Version;
import java.net.http.HttpHeaders;
import java.net.URISyntaxException;
import java.nio.charset.StandardCharsets;
import java.sql.*;
import java.time.Instant;
import java.time.ZoneOffset;
import java.time.format.DateTimeFormatter;
import java.time.Duration;
import java.util.*;
import java.util.regex.*;

public class Task47 {

    // Extract links using a basic regex for anchors. This is simplistic and intended for demo purposes.
    private static List<String[]> extractLinks(String html) {
        List<String[]> out = new ArrayList<>();
        if (html == null || html.isEmpty()) return out;
        Pattern p = Pattern.compile("(?i)<a\\s+[^>]*href\\s*=\\s*\"([^\"]+)\"[^>]*>(.*?)</a>", Pattern.DOTALL);
        Matcher m = p.matcher(html);
        while (m.find()) {
            String href = m.group(1).trim();
            String text = m.group(2).replaceAll("<[^>]+>", "").trim();
            out.add(new String[]{href, text});
        }
        return out;
    }

    private static boolean isHttpUrl(String url) {
        try {
            URI uri = new URI(url);
            String scheme = uri.getScheme();
            return scheme != null && (scheme.equalsIgnoreCase("http") || scheme.equalsIgnoreCase("https")) && uri.getHost() != null;
        } catch (URISyntaxException e) {
            return false;
        }
    }

    private static String nowIso() {
        return DateTimeFormatter.ISO_INSTANT.format(Instant.now().atOffset(ZoneOffset.UTC));
    }

    private static void ensureSchema(Connection conn) throws SQLException {
        String sql = "CREATE TABLE IF NOT EXISTS links (" +
                "id INTEGER PRIMARY KEY AUTOINCREMENT," +
                "page_url TEXT NOT NULL," +
                "link_url TEXT NOT NULL," +
                "link_text TEXT," +
                "scraped_at TEXT NOT NULL" +
                ")";
        try (Statement st = conn.createStatement()) {
            st.execute(sql);
        }
        try (Statement st = conn.createStatement()) {
            st.execute("CREATE INDEX IF NOT EXISTS idx_links_page ON links(page_url)");
        }
    }

    private static byte[] fetchLimited(String url, int maxBytes) throws Exception {
        HttpClient client = HttpClient.newBuilder()
                .version(Version.HTTP_2)
                .followRedirects(Redirect.NORMAL)
                .connectTimeout(Duration.ofSeconds(10))
                .build();
        HttpRequest req = HttpRequest.newBuilder()
                .uri(URI.create(url))
                .timeout(Duration.ofSeconds(20))
                .header("User-Agent", "Task47Bot/1.0 (+https://example.local/bot)")
                .GET()
                .build();

        HttpResponse<byte[]> resp = client.send(req, HttpResponse.BodyHandlers.ofByteArray());
        int status = resp.statusCode();
        if (status < 200 || status >= 300) throw new RuntimeException("HTTP status: " + status);

        HttpHeaders headers = resp.headers();
        OptionalLong cl = headers.firstValueAsLong("Content-Length");
        if (cl.isPresent() && cl.getAsLong() > maxBytes) {
            throw new RuntimeException("Content too large");
        }

        byte[] body = resp.body();
        if (body == null) body = new byte[0];
        if (body.length > maxBytes) {
            // Truncate safely
            return Arrays.copyOf(body, maxBytes);
        }
        return body;
    }

    public static int scrapeAndStore(String url, String dbPath) throws Exception {
        if (url == null || dbPath == null) throw new IllegalArgumentException("Null arguments");
        if (!isHttpUrl(url)) throw new IllegalArgumentException("Only http/https URLs allowed");

        final int MAX_BYTES = 2_000_000;
        byte[] body = fetchLimited(url, MAX_BYTES);
        String html = new String(body, StandardCharsets.UTF_8);

        List<String[]> links = extractLinks(html);

        String when = nowIso();
        String jdbc = "jdbc:sqlite:" + dbPath;
        int inserted = 0;
        try (Connection conn = DriverManager.getConnection(jdbc)) {
            conn.setAutoCommit(false);
            ensureSchema(conn);
            String ins = "INSERT INTO links(page_url, link_url, link_text, scraped_at) VALUES(?,?,?,?)";
            try (PreparedStatement ps = conn.prepareStatement(ins)) {
                for (String[] link : links) {
                    ps.setString(1, url);
                    ps.setString(2, link[0]);
                    ps.setString(3, link[1]);
                    ps.setString(4, when);
                    ps.addBatch();
                }
                int[] res = ps.executeBatch();
                for (int r : res) {
                    if (r == Statement.SUCCESS_NO_INFO || r > 0) inserted++;
                }
            }
            conn.commit();
        }
        return inserted;
    }

    // Main with 5 test cases
    public static void main(String[] args) {
        String db = "scrape_demo_java.db";
        String[] tests = new String[] {
                "https://example.com",
                "https://www.iana.org/domains/reserved",
                "https://httpbin.org/html",
                "https://www.wikipedia.org",
                "https://www.rfc-editor.org/rfc/rfc2616"
        };
        for (String t : tests) {
            try {
                int n = scrapeAndStore(t, db);
                System.out.println("Scraped " + t + " -> stored " + n + " links.");
            } catch (HttpTimeoutException te) {
                System.out.println("Timeout: " + t);
            } catch (Exception e) {
                System.out.println("Error scraping " + t + ": " + e.getMessage());
            }
        }
    }
}