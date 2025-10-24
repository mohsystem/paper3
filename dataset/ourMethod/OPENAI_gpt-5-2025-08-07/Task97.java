import javax.xml.XMLConstants;
import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.ParserConfigurationException;
import javax.xml.xpath.*;
import java.io.IOException;
import java.io.InputStream;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;
import java.util.ArrayList;
import java.util.List;
import java.util.regex.Pattern;

import org.w3c.dom.Document;
import org.w3c.dom.NodeList;
import org.xml.sax.SAXException;

public class Task97 {

    private static final long MAX_FILE_SIZE_BYTES = 5_000_000L; // 5 MB
    private static final Pattern ID_PATTERN = Pattern.compile("^[A-Za-z0-9._:-]{1,128}$");

    public static List<String> executeXPath(String xmlPathParam, String idParam) throws Exception {
        return executeXPathWithBaseDir(Paths.get(".").toAbsolutePath().normalize(), xmlPathParam, idParam);
    }

    public static List<String> executeXPathWithBaseDir(Path baseDir, String xmlPathParam, String idParam) throws Exception {
        if (idParam == null || !ID_PATTERN.matcher(idParam).matches()) {
            throw new IllegalArgumentException("Invalid id parameter. Allowed: 1-128 of [A-Za-z0-9._:-]");
        }
        if (xmlPathParam == null || xmlPathParam.length() == 0 || xmlPathParam.length() > 4096) {
            throw new IllegalArgumentException("Invalid xml path parameter.");
        }

        Path baseCanonical = baseDir.toRealPath(LinkOption.NOFOLLOW_LINKS);
        Path requested = baseCanonical.resolve(xmlPathParam).normalize();

        if (!requested.startsWith(baseCanonical)) {
            throw new SecurityException("Resolved path escapes base directory.");
        }
        if (Files.isSymbolicLink(requested)) {
            throw new SecurityException("Refusing to process symlinked file.");
        }
        if (!Files.isRegularFile(requested, LinkOption.NOFOLLOW_LINKS)) {
            throw new IOException("Provided path is not a regular file.");
        }
        long size = Files.size(requested);
        if (size <= 0 || size > MAX_FILE_SIZE_BYTES) {
            throw new IOException("File size is invalid or exceeds limit.");
        }

        byte[] data;
        try (InputStream in = Files.newInputStream(requested, StandardOpenOption.READ)) {
            data = in.readAllBytes();
        }
        String content = new String(data, StandardCharsets.UTF_8);
        String upper = content.toUpperCase();
        if (upper.contains("<!DOCTYPE") || upper.contains("<!ENTITY")) {
            throw new SecurityException("XML with DOCTYPE or ENTITY is not allowed.");
        }

        DocumentBuilderFactory dbf = DocumentBuilderFactory.newInstance();
        secureDBF(dbf);
        DocumentBuilder db = dbf.newDocumentBuilder();
        Document doc;
        try (InputStream in = Files.newInputStream(requested, StandardOpenOption.READ)) {
            doc = db.parse(in);
        }
        doc.getDocumentElement().normalize();

        XPathFactory xpf = XPathFactory.newInstance();
        XPath xp = xpf.newXPath();
        String expr = "/tag[@id='" + idParam + "']";
        XPathExpression xpe = xp.compile(expr);
        NodeList nodes = (NodeList) xpe.evaluate(doc, XPathConstants.NODESET);

        List<String> results = new ArrayList<>();
        for (int i = 0; i < nodes.getLength(); i++) {
            String text = nodes.item(i).getTextContent();
            results.add(text == null ? "" : text);
        }
        return results;
    }

    private static void secureDBF(DocumentBuilderFactory dbf) throws ParserConfigurationException {
        dbf.setNamespaceAware(false);
        dbf.setXIncludeAware(false);
        dbf.setExpandEntityReferences(false);
        dbf.setFeature(XMLConstants.FEATURE_SECURE_PROCESSING, true);
        try {
            dbf.setFeature("http://apache.org/xml/features/disallow-doctype-decl", true);
        } catch (ParserConfigurationException ignored) {}
        try {
            dbf.setFeature("http://xml.org/sax/features/external-general-entities", false);
        } catch (ParserConfigurationException ignored) {}
        try {
            dbf.setFeature("http://xml.org/sax/features/external-parameter-entities", false);
        } catch (ParserConfigurationException ignored) {}
        try {
            dbf.setAttribute(XMLConstants.ACCESS_EXTERNAL_DTD, "");
        } catch (IllegalArgumentException ignored) {}
        try {
            dbf.setAttribute(XMLConstants.ACCESS_EXTERNAL_SCHEMA, "");
        } catch (IllegalArgumentException ignored) {}
    }

    private static void writeSampleFile(Path path, String content) throws IOException {
        Path tmp = path.resolveSibling(path.getFileName().toString() + ".tmp");
        byte[] data = content.getBytes(StandardCharsets.UTF_8);
        Files.write(tmp, data, StandardOpenOption.CREATE, StandardOpenOption.TRUNCATE_EXISTING, StandardOpenOption.WRITE);
        Files.move(tmp, path, StandardCopyOption.REPLACE_EXISTING, StandardCopyOption.ATOMIC_MOVE);
    }

    public static void main(String[] args) {
        try {
            if (args.length == 2) {
                String id = args[0];
                String xmlFile = args[1];
                List<String> res = executeXPath(xmlFile, id);
                for (String s : res) {
                    System.out.println(s);
                }
                return;
            }
            // Self test with 5 cases
            String xml = ""
                    + "<root>\n"
                    + "  <tag id=\"a\">Alpha</tag>\n"
                    + "  <tag id=\"b\">Beta</tag>\n"
                    + "  <tag id=\"c\">Gamma</tag>\n"
                    + "  <group>\n"
                    + "    <tag id=\"d\">Delta</tag>\n"
                    + "  </group>\n"
                    + "  <tag id=\"a-2\">Alpha2</tag>\n"
                    + "</root>\n";
            Path sample = Paths.get("sample97.xml").toAbsolutePath().normalize();
            writeSampleFile(sample, xml);

            String[] testIds = new String[]{"a", "b", "x", "a-2", "d"};
            for (String id : testIds) {
                System.out.println("Query id=" + id + ":");
                List<String> out = executeXPath(sample.toString(), id);
                if (out.isEmpty()) {
                    System.out.println("(no results)");
                } else {
                    for (String s : out) System.out.println(s);
                }
                System.out.println("---");
            }
        } catch (Exception e) {
            System.err.println("Error: " + e.getMessage());
            System.exit(1);
        }
    }
}