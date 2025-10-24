import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.io.InputStream;
import java.net.HttpURLConnection;
import java.net.URL;
import java.nio.charset.StandardCharsets;
import java.sql.*;
import java.time.Instant;
import java.util.ArrayList;
import java.util.List;
import java.util.regex.*;

public class Task47 {

    public static int scrapeAndStore(String url, String dbPath) throws Exception {
        String html = fetchUrl(url);
        String title = extractTitle(html);
        List<Link> links = extractLinks(html);

        int insertedLinks = storeToSqlite(dbPath, url, title, links);
        return insertedLinks;
    }

    private static String fetchUrl(String urlStr) throws Exception {
        HttpURLConnection conn = null;
        InputStream in = null;
        try {
            URL url = new URL(urlStr);
            conn = (HttpURLConnection) url.openConnection();
            conn.setInstanceFollowRedirects(true);
            conn.setRequestProperty("User-Agent", "Task47Bot/1.0 (+https://example.org)");
            conn.setConnectTimeout(15000);
            conn.setReadTimeout(20000);
            int code = conn.getResponseCode();
            in = (code >= 200 && code < 400) ? conn.getInputStream() : conn.getErrorStream();
            BufferedReader br = new BufferedReader(new InputStreamReader(in, StandardCharsets.UTF_8));
            StringBuilder sb = new StringBuilder();
            char[] buf = new char[8192];
            int n;
            while ((n = br.read(buf)) != -1) {
                sb.append(buf, 0, n);
            }
            return sb.toString();
        } finally {
            if (in != null) try { in.close(); } catch (Exception ignored) {}
            if (conn != null) conn.disconnect();
        }
    }

    private static String extractTitle(String html) {
        Pattern p = Pattern.compile("(?is)<title\\b[^>]*>(.*?)</title>");
        Matcher m = p.matcher(html);
        if (m.find()) {
            return m.group(1).replaceAll("\\s+", " ").trim();
        }
        return "";
    }

    private static class Link {
        String href;
        String text;
        Link(String h, String t) { href = h; text = t; }
    }

    private static List<Link> extractLinks(String html) {
        List<Link> out = new ArrayList<>();
        Pattern p = Pattern.compile("(?is)<a\\b[^>]*href\\s*=\\s*(['\"])(.*?)\\1[^>]*>(.*?)</a>");
        Matcher m = p.matcher(html);
        while (m.find()) {
            String href = m.group(2).trim();
            String text = m.group(3).replaceAll("<[^>]+>", "").replaceAll("\\s+", " ").trim();
            out.add(new Link(href, text));
        }
        return out;
    }

    private static int storeToSqlite(String dbPath, String url, String title, List<Link> links) throws Exception {
        try { Class.forName("org.sqlite.JDBC"); } catch (Throwable ignored) {}
        String jdbcUrl = "jdbc:sqlite:" + dbPath;
        try (Connection conn = DriverManager.getConnection(jdbcUrl)) {
            conn.setAutoCommit(false);
            try (Statement st = conn.createStatement()) {
                st.execute("CREATE TABLE IF NOT EXISTS pages (id INTEGER PRIMARY KEY AUTOINCREMENT, url TEXT, title TEXT, fetched_at TEXT)");
                st.execute("CREATE TABLE IF NOT EXISTS links (id INTEGER PRIMARY KEY AUTOINCREMENT, page_id INTEGER, href TEXT, text TEXT, FOREIGN KEY(page_id) REFERENCES pages(id))");
            }

            long pageId;
            String fetchedAt = Instant.now().toString();
            try (PreparedStatement ps = conn.prepareStatement(
                    "INSERT INTO pages(url, title, fetched_at) VALUES(?,?,?)",
                    Statement.RETURN_GENERATED_KEYS)) {
                ps.setString(1, url);
                ps.setString(2, title);
                ps.setString(3, fetchedAt);
                ps.executeUpdate();
                try (ResultSet rs = ps.getGeneratedKeys()) {
                    if (rs.next()) pageId = rs.getLong(1);
                    else throw new SQLException("Failed to get page id");
                }
            }

            int count = 0;
            try (PreparedStatement ps = conn.prepareStatement(
                    "INSERT INTO links(page_id, href, text) VALUES(?,?,?)")) {
                for (Link l : links) {
                    ps.setLong(1, pageId);
                    ps.setString(2, l.href);
                    ps.setString(3, l.text);
                    ps.addBatch();
                    count++;
                }
                ps.executeBatch();
            }
            conn.commit();
            return count;
        }
    }

    public static void main(String[] args) {
        String dbPath = "scrape_java.db";
        String[] testUrls = new String[] {
                "https://example.com",
                "https://www.wikipedia.org",
                "https://www.python.org",
                "https://www.gnu.org",
                "https://www.rfc-editor.org"
        };
        for (String url : testUrls) {
            try {
                int inserted = scrapeAndStore(url, dbPath);
                System.out.println("Java: Inserted " + inserted + " links from " + url + " into " + dbPath);
            } catch (Exception e) {
                System.out.println("Java: Failed for " + url + " -> " + e.getMessage());
            }
        }
    }
}