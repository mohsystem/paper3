import java.io.BufferedInputStream;
import java.io.BufferedWriter;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.net.URI;
import java.net.URISyntaxException;
import java.net.http.HttpClient;
import java.net.http.HttpRequest;
import java.net.http.HttpResponse;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.StandardOpenOption;
import java.time.Instant;
import java.util.ArrayList;
import java.util.List;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

public final class Task47 {

    public static final class ScrapeResult {
        public final String url;
        public final String title;
        public final List<String> links;
        public final String timestamp;

        public ScrapeResult(String url, String title, List<String> links, String timestamp) {
            this.url = url;
            this.title = title;
            this.links = List.copyOf(links);
            this.timestamp = timestamp;
        }
    }

    public static final class LocalDatabase implements AutoCloseable {
        private final Path dbPath;

        public LocalDatabase(String filePath) {
            if (filePath == null || filePath.isBlank()) {
                throw new IllegalArgumentException("Database file path must be non-empty.");
            }
            this.dbPath = Path.of(filePath);
        }

        public boolean insert(ScrapeResult r) {
            if (r == null) return false;
            String json = toJson(r);
            try {
                if (!Files.exists(dbPath)) {
                    Files.createFile(dbPath);
                }
                try (BufferedWriter w = Files.newBufferedWriter(
                        dbPath,
                        StandardCharsets.UTF_8,
                        StandardOpenOption.APPEND)) {
                    w.write(json);
                    w.write(System.lineSeparator());
                }
                return true;
            } catch (IOException e) {
                return false;
            }
        }

        private static String toJson(ScrapeResult r) {
            StringBuilder sb = new StringBuilder(256 + r.links.size() * 64);
            sb.append('{');
            sb.append("\"url\":\"").append(jsonEscape(r.url)).append("\",");
            sb.append("\"title\":\"").append(jsonEscape(r.title)).append("\",");
            sb.append("\"links\":[");
            for (int i = 0; i < r.links.size(); i++) {
                if (i > 0) sb.append(',');
                sb.append("\"").append(jsonEscape(r.links.get(i))).append("\"");
            }
            sb.append("],");
            sb.append("\"timestamp\":\"").append(jsonEscape(r.timestamp)).append("\"");
            sb.append('}');
            return sb.toString();
        }

        private static String jsonEscape(String s) {
            StringBuilder sb = new StringBuilder(s.length() + 16);
            for (int i = 0; i < s.length(); i++) {
                char c = s.charAt(i);
                switch (c) {
                    case '"': sb.append("\\\""); break;
                    case '\\': sb.append("\\\\"); break;
                    case '\b': sb.append("\\b"); break;
                    case '\f': sb.append("\\f"); break;
                    case '\n': sb.append("\\n"); break;
                    case '\r': sb.append("\\r"); break;
                    case '\t': sb.append("\\t"); break;
                    default:
                        if (c < 0x20) {
                            sb.append(String.format("\\u%04x", (int) c));
                        } else {
                            sb.append(c);
                        }
                }
            }
            return sb.toString();
        }

        @Override
        public void close() {
            // No resources to close; placeholder to satisfy AutoCloseable for symmetry.
        }
    }

    public static ScrapeResult scrapeToDb(String url, String dbFile) throws IOException, InterruptedException {
        String html = fetchHtml(url);
        Parsed parsed = parseHtml(html);
        String ts = Instant.now().toString();
        ScrapeResult result = new ScrapeResult(url, parsed.title, parsed.links, ts);
        try (LocalDatabase db = new LocalDatabase(dbFile)) {
            db.insert(result);
        }
        return result;
    }

    private static final class Parsed {
        final String title;
        final List<String> links;
        Parsed(String title, List<String> links) {
            this.title = title;
            this.links = links;
        }
    }

    private static String fetchHtml(String url) throws IOException, InterruptedException {
        URI uri = validateHttpsUrl(url);
        HttpClient client = HttpClient.newBuilder()
                .followRedirects(HttpClient.Redirect.NORMAL)
                .connectTimeout(java.time.Duration.ofSeconds(10))
                .build();

        HttpRequest req = HttpRequest.newBuilder()
                .uri(uri)
                .timeout(java.time.Duration.ofSeconds(20))
                .header("User-Agent", "Task47Scraper/1.0 (+https://example.local)")
                .GET()
                .build();

        HttpResponse<InputStream> resp = client.send(req, HttpResponse.BodyHandlers.ofInputStream());
        int code = resp.statusCode();
        if (code < 200 || code >= 300) {
            throw new IOException("HTTP error: " + code);
        }

        String contentType = resp.headers().firstValue("Content-Type").orElse("application/octet-stream");
        if (!contentType.toLowerCase().contains("text/html")) {
            // still read but be cautious
        }

        final int MAX_BYTES = 1_048_576; // 1 MiB limit to avoid excessive memory
        try (InputStream is = new BufferedInputStream(resp.body())) {
            ByteArrayOutputStream bos = new ByteArrayOutputStream(Math.min(64_000, MAX_BYTES));
            byte[] buf = new byte[8192];
            int total = 0;
            int r;
            while ((r = is.read(buf)) != -1) {
                total += r;
                if (total > MAX_BYTES) {
                    // truncate
                    bos.write(buf, 0, r - (total - MAX_BYTES));
                    break;
                } else {
                    bos.write(buf, 0, r);
                }
            }
            return bos.toString(StandardCharsets.UTF_8);
        }
    }

    private static URI validateHttpsUrl(String url) {
        if (url == null) {
            throw new IllegalArgumentException("URL must not be null.");
        }
        if (url.length() > 2048) {
            throw new IllegalArgumentException("URL too long.");
        }
        try {
            URI uri = new URI(url);
            String scheme = uri.getScheme();
            if (scheme == null || !scheme.equalsIgnoreCase("https")) {
                throw new IllegalArgumentException("Only HTTPS URLs are allowed.");
            }
            String host = uri.getHost();
            if (host == null || host.isBlank()) {
                throw new IllegalArgumentException("URL must have a valid host.");
            }
            return uri.normalize();
        } catch (URISyntaxException e) {
            throw new IllegalArgumentException("Invalid URL syntax.", e);
        }
    }

    private static Parsed parseHtml(String html) {
        String limited = html == null ? "" : html;
        if (limited.length() > 1_200_000) {
            limited = limited.substring(0, 1_200_000);
        }

        String title = extractTitle(limited);
        List<String> links = extractLinks(limited, 1000);
        return new Parsed(title, links);
        }

    private static String extractTitle(String html) {
        Pattern p = Pattern.compile("(?is)<title[^>]*>(.*?)</title>");
        Matcher m = p.matcher(html);
        if (m.find()) {
            String t = m.group(1).replaceAll("(?is)<[^>]+>", "");
            t = t.trim();
            if (t.length() > 512) t = t.substring(0, 512);
            return t;
        }
        return "";
    }

    private static List<String> extractLinks(String html, int maxCount) {
        List<String> links = new ArrayList<>();
        Pattern p = Pattern.compile("(?is)<a\\s+[^>]*?href\\s*=\\s*(?:\"([^\"]*)\"|'([^']*)')", Pattern.CASE_INSENSITIVE | Pattern.DOTALL);
        Matcher m = p.matcher(html);
        while (m.find() && links.size() < maxCount) {
            String href = m.group(1) != null ? m.group(1) : m.group(2);
            if (href == null) continue;
            href = href.trim();
            if (href.startsWith("http://") || href.startsWith("https://")) {
                if (href.length() > 2048) continue;
                links.add(href);
            }
        }
        return links;
    }

    public static void main(String[] args) {
        String dbFile = "java_scrape_db.jsonl";
        String[] testUrls = new String[] {
                "https://example.com/",
                "https://www.iana.org/domains/reserved",
                "https://www.wikipedia.org/",
                "https://www.gnu.org/",
                "https://httpbin.org/html"
        };
        for (String u : testUrls) {
            try {
                ScrapeResult r = scrapeToDb(u, dbFile);
                System.out.println("Scraped: " + r.url + " | title=" + r.title + " | links=" + r.links.size());
            } catch (Exception e) {
                System.out.println("Failed for " + u + " -> " + e.getMessage());
            }
        }
    }
}