import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import org.w3c.dom.Document;
import org.xml.sax.InputSource;
import java.io.StringReader;

public class Task61 {

    /**
     * Parses an XML string and returns the name of the root element.
     * This implementation uses Java's built-in JAXP, configured to be secure against XXE attacks.
     *
     * @param xmlString The string containing the XML document.
     * @return The name of the root element, or an error message if parsing fails.
     */
    public static String getRootElement(String xmlString) {
        if (xmlString == null || xmlString.trim().isEmpty()) {
            return "Error: XML string is null or empty.";
        }
        try {
            DocumentBuilderFactory dbf = DocumentBuilderFactory.newInstance();
            
            // Secure processing: Disable DTDs and external entities to prevent XXE attacks.
            dbf.setFeature("http://apache.org/xml/features/disallow-doctype-decl", true);
            dbf.setFeature("http://xml.org/sax/features/external-general-entities", false);
            dbf.setFeature("http://xml.org/sax/features/external-parameter-entities", false);
            dbf.setXIncludeAware(false);
            dbf.setExpandEntityReferences(false);

            DocumentBuilder db = dbf.newDocumentBuilder();
            InputSource is = new InputSource(new StringReader(xmlString));
            Document doc = db.parse(is);
            
            return doc.getDocumentElement().getNodeName();
        } catch (Exception e) {
            // Catches ParserConfigurationException, SAXException, IOException, etc.
            return "Error parsing XML: " + e.getMessage();
        }
    }

    public static void main(String[] args) {
        String[] testCases = {
            "<root><child>data</child></root>",
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?><bookstore><book category=\"cooking\"><title lang=\"en\">Everyday Italian</title></book></bookstore>",
            "<!-- This is a comment --><project name=\"xml-parser\"><version>1.0</version></project>",
            "<note><to>Tove</to><from>Jani</from><heading>Reminder</heading></note>",
            "This is not a valid XML string."
        };

        System.out.println("Running Java Test Cases:");
        for (int i = 0; i < testCases.length; i++) {
            System.out.println("Test Case " + (i + 1) + ": \"" + testCases[i] + "\"");
            System.out.println("Root Element: " + getRootElement(testCases[i]));
            System.out.println();
        }
    }
}