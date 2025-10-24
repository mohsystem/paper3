import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.ParserConfigurationException;
import javax.xml.xpath.XPath;
import javax.xml.xpath.XPathConstants;
import javax.xml.xpath.XPathFactory;
import javax.xml.xpath.XPathVariableResolver;
import javax.xml.namespace.QName;
import org.w3c.dom.Document;
import org.w3c.dom.Node;
import org.xml.sax.SAXException;
import javax.xml.XMLConstants;
import java.io.File;
import java.io.IOException;
import java.io.PrintWriter;

public class Task97 {

    /**
     * Executes a secure XPath query on a local XML file.
     *
     * @param id The ID value to be used in the XPath query.
     * @param filename The name of the XML file to query. Assumed to be in the current directory.
     * @return The text content of the first matching node, or "Not found" if no match is found.
     */
    public static String performXpathQuery(String id, String filename) {
        // 1. Secure Filename Handling: Prevent path traversal
        if (filename.contains("..") || filename.contains("/") || filename.contains("\\")) {
            return "Error: Invalid filename.";
        }
        
        File xmlFile = new File(filename);
        if (!xmlFile.exists()) {
            return "Error: File not found.";
        }

        try {
            // 2. Secure XML Parsing: Disable DTDs and external entities to prevent XXE
            DocumentBuilderFactory dbf = DocumentBuilderFactory.newInstance();
            dbf.setFeature(XMLConstants.FEATURE_SECURE_PROCESSING, true);
            dbf.setFeature("http://apache.org/xml/features/disallow-doctype-decl", true);
            dbf.setFeature("http://xml.org/sax/features/external-general-entities", false);
            dbf.setFeature("http://xml.org/sax/features/external-parameter-entities", false);
            dbf.setExpandEntityReferences(false);

            DocumentBuilder db = dbf.newDocumentBuilder();
            Document doc = db.parse(xmlFile);

            // 3. Secure XPath Execution: Use a variable resolver to prevent XPath injection
            XPathFactory xpf = XPathFactory.newInstance();
            XPath xpath = xpf.newXPath();

            XPathVariableResolver variableResolver = new XPathVariableResolver() {
                public Object resolveVariable(QName variableName) {
                    if (variableName.getLocalPart().equals("idVar")) {
                        return id;
                    }
                    return null;
                }
            };
            xpath.setXPathVariableResolver(variableResolver);
            
            // The query uses a variable ($idVar) instead of concatenating user input
            String expression = "//tag[@id=$idVar]";
            Node node = (Node) xpath.evaluate(expression, doc, XPathConstants.NODE);

            if (node != null) {
                return node.getTextContent();
            } else {
                return "Not found";
            }

        } catch (ParserConfigurationException | SAXException | IOException | javax.xml.xpath.XPathExpressionException e) {
            // e.printStackTrace();
            return "Error: " + e.getMessage();
        }
    }

    public static void main(String[] args) {
        // Create a dummy XML file for testing
        String testXmlFilename = "test97.xml";
        String xmlContent = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n" +
                            "<root>\n" +
                            "    <data>\n" +
                            "        <tag id=\"1\">First item</tag>\n" +
                            "        <tag id=\"2\">Second item</tag>\n" +
                            "        <tag id=\"admin\">Admin item</tag>\n" +
                            "    </data>\n" +
                            "    <other>\n" +
                            "        <tag id=\"100\">Other item</tag>\n" +
                            "    </other>\n" +
                            "</root>";
        try (PrintWriter out = new PrintWriter(testXmlFilename)) {
            out.println(xmlContent);
        } catch (IOException e) {
            System.err.println("Failed to create test file: " + e.getMessage());
            return;
        }

        // --- Test Cases ---
        String[][] testCases = {
            {"1", "First item"},
            {"100", "Other item"},
            {"999", "Not found"},
            {"' or '1'='1", "Not found"}, // Malicious input for injection test
            {"admin", "Admin item"}
        };

        System.out.println("Running Java Test Cases...");
        for (int i = 0; i < testCases.length; i++) {
            String id = testCases[i][0];
            String expected = testCases[i][1];
            String result = performXpathQuery(id, testXmlFilename);
            System.out.printf("Test %d: id='%s'\n  - Expected: %s\n  - Got: %s\n  - Status: %s\n\n",
                    i + 1, id, expected, result, expected.equals(result) ? "PASSED" : "FAILED");
        }

        // Cleanup the test file
        new File(testXmlFilename).delete();
    }
}