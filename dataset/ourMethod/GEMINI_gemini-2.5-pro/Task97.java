import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.ParserConfigurationException;
import javax.xml.xpath.XPath;
import javax.xml.xpath.XPathConstants;
import javax.xml.xpath.XPathExpression;
import javax.xml.xpath.XPathExpressionException;
import javax.xml.xpath.XPathFactory;
import javax.xml.xpath.XPathVariableResolver;
import org.w3c.dom.Document;
import org.w3c.dom.NodeList;
import org.xml.sax.SAXException;
import java.io.File;
import java.io.IOException;
import java.io.FileWriter;
import java.nio.file.InvalidPathException;
import java.nio.file.Paths;
import java.util.ArrayList;
import java.util.List;
import java.util.regex.Pattern;
import javax.xml.namespace.QName;
import javax.xml.XMLConstants;

public class Task97 {

    // Regex for validating the ID to be alphanumeric + underscore
    private static final Pattern ID_PATTERN = Pattern.compile("^[a-zA-Z0-9_]+$");

    /**
     * Executes an XPath query to find nodes by ID.
     *
     * @param id The ID to search for. Must be a simple alphanumeric string.
     * @param xmlFilePath The path to the XML file. Must be a simple filename without path components.
     * @return A list of string contents of the found nodes.
     * @throws IllegalArgumentException for invalid inputs.
     * @throws ParserConfigurationException, SAXException, IOException, XPathExpressionException on XML/XPath processing errors.
     */
    public static List<String> executeXPathQuery(final String id, final String xmlFilePath) 
            throws ParserConfigurationException, SAXException, IOException, XPathExpressionException {
        
        // Rule #1 & #5: Validate inputs
        if (id == null || !ID_PATTERN.matcher(id).matches()) {
            throw new IllegalArgumentException("Invalid ID format. Only alphanumeric characters and underscores are allowed.");
        }
        
        if (xmlFilePath == null || xmlFilePath.trim().isEmpty()) {
            throw new IllegalArgumentException("XML file path cannot be null or empty.");
        }

        // Rule #5: Prevent path traversal
        try {
            if (!Paths.get(xmlFilePath).getFileName().toString().equals(xmlFilePath)) {
                throw new IllegalArgumentException("Invalid file path. Only simple filenames are allowed.");
            }
        } catch (InvalidPathException e) {
            throw new IllegalArgumentException("Invalid file path format.", e);
        }

        File xmlFile = new File(xmlFilePath);
        if (!xmlFile.exists() || !xmlFile.isFile()) {
            throw new IOException("XML file does not exist or is not a regular file: " + xmlFilePath);
        }

        // Rule #4: Securely configure the XML parser to prevent XXE
        DocumentBuilderFactory dbf = DocumentBuilderFactory.newInstance();
        dbf.setFeature(XMLConstants.FEATURE_SECURE_PROCESSING, true);
        dbf.setFeature("http://apache.org/xml/features/disallow-doctype-decl", true);
        dbf.setFeature("http://xml.org/sax/features/external-general-entities", false);
        dbf.setFeature("http://xml.org/sax/features/external-parameter-entities", false);
        dbf.setExpandEntityReferences(false);
        
        DocumentBuilder db = dbf.newDocumentBuilder();
        Document doc = db.parse(xmlFile);

        XPathFactory xpf = XPathFactory.newInstance();
        XPath xpath = xpf.newXPath();

        // Use a variable resolver to prevent XPath injection
        xpath.setXPathVariableResolver(new XPathVariableResolver() {
            @Override
            public Object resolveVariable(QName variableName) {
                if (variableName.getLocalPart().equals("idVar")) {
                    return id;
                }
                return null;
            }
        });

        // The format is /tag[@id={}], here we use `*` for any tag
        String expression = "//*[@id=$idVar]";
        XPathExpression expr = xpath.compile(expression);

        NodeList nodeList = (NodeList) expr.evaluate(doc, XPathConstants.NODESET);
        List<String> results = new ArrayList<>();
        for (int i = 0; i < nodeList.getLength(); i++) {
            results.add(nodeList.item(i).getTextContent());
        }

        return results;
    }

    private static void setupTestFile(String filename) throws IOException {
        try (FileWriter writer = new FileWriter(filename)) {
            writer.write("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
            writer.write("<root>\n");
            writer.write("    <item id=\"item1\">Content 1</item>\n");
            writer.write("    <data>\n");
            writer.write("        <item id=\"item2\">Content 2</item>\n");
            writer.write("    </data>\n");
            writer.write("    <item id=\"item3\">Content 3</item>\n");
            writer.write("    <item id=\"another_id_4\">More Content</item>\n");
            writer.write("</root>");
        }
    }

    private static void cleanupTestFile(String filename) {
        new File(filename).delete();
    }

    public static void main(String[] args) {
        if (args.length != 2) {
            System.err.println("Usage: java Task97 <id> <xml_file>");
            System.err.println("Running test cases instead...");
            runTests();
            return;
        }

        try {
            List<String> results = executeXPathQuery(args[0], args[1]);
            System.out.println("Query Result:");
            if (results.isEmpty()) {
                System.out.println("No nodes found.");
            } else {
                results.forEach(System.out::println);
            }
        } catch (Exception e) {
            System.err.println("An error occurred: " + e.getMessage());
        }
    }
    
    private static void runTests() {
        String testFile = "test97.xml";
        try {
            setupTestFile(testFile);
            System.out.println("--- Running 5 Test Cases ---");

            // Test Case 1: Valid ID, existing item
            System.out.println("\n[Test 1] ID: 'item1', File: '" + testFile + "'");
            try {
                System.out.println("Result: " + executeXPathQuery("item1", testFile));
            } catch (Exception e) {
                System.err.println("Error: " + e.getMessage());
            }

            // Test Case 2: Valid ID, nested item
            System.out.println("\n[Test 2] ID: 'item2', File: '" + testFile + "'");
            try {
                System.out.println("Result: " + executeXPathQuery("item2", testFile));
            } catch (Exception e) {
                System.err.println("Error: " + e.getMessage());
            }

            // Test Case 3: Valid ID, non-existent item
            System.out.println("\n[Test 3] ID: 'nonexistent', File: '" + testFile + "'");
            try {
                System.out.println("Result: " + executeXPathQuery("nonexistent", testFile));
            } catch (Exception e) {
                System.err.println("Error: " + e.getMessage());
            }

            // Test Case 4: Invalid ID (potential injection)
            System.out.println("\n[Test 4] ID: \"' or '1'='1'\", File: '" + testFile + "'");
            try {
                executeXPathQuery("' or '1'='1'", testFile);
            } catch (Exception e) {
                System.err.println("Caught expected error: " + e.getMessage());
            }

            // Test Case 5: Invalid filename (path traversal)
            System.out.println("\n[Test 5] ID: 'item1', File: '../test97.xml'");
            try {
                executeXPathQuery("item1", "../test97.xml");
            } catch (Exception e) {
                System.err.println("Caught expected error: " + e.getMessage());
            }
            
            System.out.println("\n--- Test Cases Finished ---");

        } catch (IOException e) {
            System.err.println("Failed to set up or run tests: " + e.getMessage());
        } finally {
            cleanupTestFile(testFile);
        }
    }
}