import java.nio.ByteBuffer;
import java.nio.charset.CharsetDecoder;
import java.nio.charset.CodingErrorAction;
import java.nio.charset.StandardCharsets;
import java.util.Base64;

public class Task63 {

    // Process request: validate base64, decode, validate UTF-8, basic format checks, return decoded text
    public static String processRequest(String rawDataB64, String dataType) {
        if (rawDataB64 == null || dataType == null) {
            return "ERROR: Null input";
        }

        // Rule #1: Validate input format and length
        String trimmedType = dataType.trim().toLowerCase();
        if (!trimmedType.equals("json") && !trimmedType.equals("xml")) {
            return "ERROR: Unsupported data type";
        }

        if (!isAscii(rawDataB64)) {
            return "ERROR: raw_data must be ASCII/Base64 characters";
        }

        String sanitized = stripWhitespace(rawDataB64);
        if (!isBase64Chars(sanitized)) {
            return "ERROR: Invalid Base64 characters";
        }

        final int maxDecodedBytes = 1024 * 1024; // 1 MiB
        // Maximum base64 length that can decode within maxDecodedBytes
        long maxB64Len = ((long) maxDecodedBytes + 2) / 3 * 4;
        if (sanitized.length() > maxB64Len) {
            return "ERROR: Input too large";
        }

        byte[] decodedBytes;
        try {
            decodedBytes = Base64.getDecoder().decode(sanitized);
        } catch (IllegalArgumentException e) {
            return "ERROR: Base64 decoding failed";
        }

        if (decodedBytes.length > maxDecodedBytes) {
            return "ERROR: Decoded data too large";
        }

        // Validate UTF-8 strictly
        String decodedText = decodeStrictUtf8(decodedBytes);
        if (decodedText == null) {
            return "ERROR: Decoded bytes are not valid UTF-8";
        }

        String content = decodedText.trim();

        if ("json".equals(trimmedType)) {
            if (!(content.startsWith("{") || content.startsWith("["))) {
                return "ERROR: JSON must start with '{' or '['";
            }
            // Basic sanity check: disallow unescaped control characters (except whitespace)
            if (containsDisallowedControlChars(content)) {
                return "ERROR: JSON contains disallowed control characters";
            }
            return decodedText;
        } else { // xml
            if (!content.startsWith("<")) {
                return "ERROR: XML must start with '<'";
            }
            // Rule #4: Disallow external entities / DOCTYPE
            String lower = content.toLowerCase();
            if (lower.contains("<!doctype") || lower.contains("<!entity")) {
                return "ERROR: XML contains disallowed DOCTYPE or ENTITY";
            }
            // Attempt a safe parse to validate well-formedness and disable XXE
            if (!safeXmlParse(content)) {
                return "ERROR: XML parsing/validation failed";
            }
            return decodedText;
        }
    }

    private static boolean isAscii(String s) {
        for (int i = 0; i < s.length(); i++) {
            if (s.charAt(i) > 0x7F) return false;
        }
        return true;
    }

    private static String stripWhitespace(String s) {
        StringBuilder sb = new StringBuilder(s.length());
        for (int i = 0; i < s.length(); i++) {
            char c = s.charAt(i);
            if (!(c == ' ' || c == '\n' || c == '\r' || c == '\t')) {
                sb.append(c);
            }
        }
        return sb.toString();
    }

    private static boolean isBase64Chars(String s) {
        for (int i = 0; i < s.length(); i++) {
            char c = s.charAt(i);
            boolean ok = (c >= 'A' && c <= 'Z') ||
                         (c >= 'a' && c <= 'z') ||
                         (c >= '0' && c <= '9') ||
                         c == '+' || c == '/' || c == '=';
            if (!ok) return false;
        }
        return true;
    }

    private static String decodeStrictUtf8(byte[] bytes) {
        try {
            CharsetDecoder dec = StandardCharsets.UTF_8.newDecoder();
            dec.onMalformedInput(CodingErrorAction.REPORT);
            dec.onUnmappableCharacter(CodingErrorAction.REPORT);
            return dec.decode(ByteBuffer.wrap(bytes)).toString();
        } catch (Exception e) {
            return null;
        }
    }

    private static boolean containsDisallowedControlChars(String s) {
        for (int i = 0; i < s.length(); i++) {
            char ch = s.charAt(i);
            // allow tab, LF, CR; disallow other C0 controls
            if (ch >= 0x00 && ch <= 0x1F && ch != 0x09 && ch != 0x0A && ch != 0x0D) {
                return true;
            }
        }
        return false;
    }

    private static boolean safeXmlParse(String xml) {
        try {
            javax.xml.parsers.DocumentBuilderFactory dbf = javax.xml.parsers.DocumentBuilderFactory.newInstance();
            // Rule #4: Disable DTDs and external entities
            dbf.setFeature("http://apache.org/xml/features/disallow-doctype-decl", true);
            dbf.setFeature("http://xml.org/sax/features/external-general-entities", false);
            dbf.setFeature("http://xml.org/sax/features/external-parameter-entities", false);
            dbf.setFeature("http://apache.org/xml/features/nonvalidating/load-external-dtd", false);
            dbf.setXIncludeAware(false);
            dbf.setExpandEntityReferences(false);
            dbf.setNamespaceAware(true);
            javax.xml.parsers.DocumentBuilder builder = dbf.newDocumentBuilder();
            try (java.io.ByteArrayInputStream bais = new java.io.ByteArrayInputStream(xml.getBytes(StandardCharsets.UTF_8))) {
                builder.parse(bais);
            }
            return true;
        } catch (Exception ex) {
            return false;
        }
    }

    // Helper for tests
    private static String toBase64(String s) {
        return Base64.getEncoder().encodeToString(s.getBytes(StandardCharsets.UTF_8));
    }

    public static void main(String[] args) {
        // Test cases
        String json1 = "{\"msg\":\"hello\",\"n\":5}";
        String json2 = "[\"hello\",2,true]";
        String xml1  = "<root><a>1</a><b>x</b></root>";
        String invalidB64 = "!!!not_base64!!!";
        String xmlWithDoctype = "<?xml version=\"1.0\"?><!DOCTYPE foo [<!ENTITY xxe SYSTEM \"file:///etc/passwd\">]><foo>&xxe;</foo>";

        String b64json1 = toBase64(json1);
        String b64json2 = toBase64(json2);
        String b64xml1  = toBase64(xml1);
        String b64xmlDoctype = toBase64(xmlWithDoctype);

        System.out.println(processRequest(b64json1, "json"));
        System.out.println(processRequest(b64json2, "json"));
        System.out.println(processRequest(b64xml1,  "xml"));
        System.out.println(processRequest(invalidB64, "json"));
        System.out.println(processRequest(b64xmlDoctype, "xml"));
    }
}