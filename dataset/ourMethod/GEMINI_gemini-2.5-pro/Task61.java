import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.ParserConfigurationException;
import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.xml.sax.InputSource;
import org.xml.sax.SAXException;
import java.io.IOException;
import java.io.StringReader;

public class Task61 {

    /**
     * Parses an XML string and returns the name of the root element.
     * This method is configured to be secure against XXE (XML External Entity) attacks
     * by disabling DTDs and external entity processing.
     *
     * @param xmlString The XML string to parse.
     * @return The name of the root element, or an empty string if parsing fails,
     *         the input is null/empty, or no root element is found.
     */
    public static String getRootElementName(String xmlString) {
        if (xmlString == null || xmlString.trim().isEmpty()) {
            return "";
        }

        try {
            DocumentBuilderFactory factory = DocumentBuilderFactory.newInstance();
            
            // Secure configuration to prevent XXE attacks
            factory.setFeature("http://apache.org/xml/features/disallow-doctype-decl", true);
            factory.setFeature("http://xml.org/sax/features/external-general-entities", false);
            factory.setFeature("http://xml.org/sax/features/external-parameter-entities", false);
            factory.setFeature("http://apache.org/xml/features/nonvalidating/load-external-dtd", false);
            factory.setXIncludeAware(false);
            factory.setExpandEntityReferences(false);
            
            DocumentBuilder builder = factory.newDocumentBuilder();
            
            // The try-with-resources statement ensures that the reader is closed
            try (StringReader reader = new StringReader(xmlString)) {
                InputSource source = new InputSource(reader);
                Document doc = builder.parse(source);
                doc.getDocumentElement().normalize();
                Element root = doc.getDocumentElement();
                if (root != null) {
                    return root.getTagName();
                }
            }

        } catch (ParserConfigurationException | SAXException | IOException e) {
            // In a real application, logging the exception is recommended.
            // e.g., Logger.getLogger(Task61.class.getName()).log(Level.SEVERE, "XML parsing failed", e);
            return ""; // Return empty string on any parsing error
        }
        
        return "";
    }

    public static void main(String[] args) {
        String[] testCases = {
            // 1. Simple valid XML
            "<root><child>data</child></root>",
            // 2. XML with attributes and a different root name
            "<document id='123'><item>value</item></document>",
            // 3. Malformed XML (unclosed tag)
            "<notes><note>text</notes>",
            // 4. Empty string input
            "",
            // 5. XML with a DOCTYPE declaration (should be safely rejected)
            "<?xml version=\"1.0\"?><!DOCTYPE root [<!ENTITY xxe SYSTEM \"file:///etc/passwd\">]><root>&xxe;</root>"
        };

        for (int i = 0; i < testCases.length; i++) {
            String xml = testCases[i];
            String rootElement = getRootElementName(xml);
            System.out.printf("Test Case %d:\n", i + 1);
            System.out.printf("Input XML: \"%s\"\n", xml);
            System.out.printf("Root Element: \"%s\"\n\n", rootElement);
        }
    }
}