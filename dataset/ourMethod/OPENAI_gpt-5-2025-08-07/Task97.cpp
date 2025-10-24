#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <regex>
#include <filesystem>

#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>

static const size_t MAX_FILE_SIZE_BYTES = 5 * 1024 * 1024; // 5MB

static bool isValidId(const std::string& id) {
    static const std::regex re("^[A-Za-z0-9._:-]{1,128}$");
    return std::regex_match(id, re);
}

static bool containsForbiddenXML(const std::string& content) {
    std::string upper;
    upper.reserve(content.size());
    for (char c : content) {
        upper.push_back(static_cast<char>(::toupper(static_cast<unsigned char>(c))));
    }
    return (upper.find("<!DOCTYPE") != std::string::npos) || (upper.find("<!ENTITY") != std::string::npos);
}

static xmlParserInputPtr blocked_entity_loader(const char* /*URL*/, const char* /*ID*/, xmlParserCtxtPtr /*ctxt*/) {
    return nullptr;
}

static std::vector<std::string> executeXPath(const std::string& xmlPathParam, const std::string& idParam) {
    std::vector<std::string> results;
    if (!isValidId(idParam)) {
        throw std::runtime_error("Invalid id parameter. Allowed: 1-128 of [A-Za-z0-9._:-]");
    }
    if (xmlPathParam.empty() || xmlPathParam.size() > 4096) {
        throw std::runtime_error("Invalid xml path parameter.");
    }

    namespace fs = std::filesystem;
    fs::path base = fs::current_path();
    fs::path candidate = (base / fs::path(xmlPathParam)).lexically_normal();

    // Reject symlink and require regular file
    std::error_code ec;
    if (fs::is_symlink(candidate, ec)) {
        throw std::runtime_error("Refusing to process symlinked file.");
    }
    if (!fs::exists(candidate, ec) || !fs::is_regular_file(candidate, ec)) {
        throw std::runtime_error("Provided path is not a regular file.");
    }

    // Ensure candidate resolves within base
    fs::path baseCanon = fs::canonical(base, ec);
    fs::path candCanon = fs::canonical(candidate, ec);
    if (ec || candCanon.string().rfind(baseCanon.string(), 0) != 0) {
        throw std::runtime_error("Resolved path escapes base directory.");
    }

    std::ifstream in(candCanon, std::ios::binary);
    if (!in) {
        throw std::runtime_error("Failed to open file.");
    }
    in.seekg(0, std::ios::end);
    std::streamoff len = in.tellg();
    if (len <= 0 || static_cast<size_t>(len) > MAX_FILE_SIZE_BYTES) {
        throw std::runtime_error("File size is invalid or exceeds limit.");
    }
    in.seekg(0, std::ios::beg);
    std::string content;
    content.resize(static_cast<size_t>(len));
    in.read(&content[0], len);
    if (!in) {
        throw std::runtime_error("Failed to read file.");
    }
    if (containsForbiddenXML(content)) {
        throw std::runtime_error("XML with DOCTYPE or ENTITY is not allowed.");
    }

    // Initialize libxml2 and disable external entities
    xmlInitParser();
    xmlExternalEntityLoader old_loader = xmlGetExternalEntityLoader();
    xmlSetExternalEntityLoader(blocked_entity_loader);

    int parseOpts = XML_PARSE_NONET | XML_PARSE_NOCDATA | XML_PARSE_COMPACT;
    xmlDocPtr doc = xmlReadMemory(content.c_str(), static_cast<int>(content.size()), "inmem.xml", nullptr, parseOpts);
    if (doc == nullptr) {
        xmlSetExternalEntityLoader(old_loader);
        xmlCleanupParser();
        throw std::runtime_error("Failed to parse XML.");
    }

    xmlXPathContextPtr ctxt = xmlXPathNewContext(doc);
    if (ctxt == nullptr) {
        xmlFreeDoc(doc);
        xmlSetExternalEntityLoader(old_loader);
        xmlCleanupParser();
        throw std::runtime_error("Failed to create XPath context.");
    }

    std::string expr = "/tag[@id='" + idParam + "']";
    xmlXPathObjectPtr obj = xmlXPathEvalExpression(reinterpret_cast<const xmlChar*>(expr.c_str()), ctxt);
    if (obj == nullptr) {
        xmlXPathFreeContext(ctxt);
        xmlFreeDoc(doc);
        xmlSetExternalEntityLoader(old_loader);
        xmlCleanupParser();
        throw std::runtime_error("XPath evaluation failed.");
    }

    if (obj->type == XPATH_NODESET && obj->nodesetval != nullptr) {
        xmlNodeSetPtr ns = obj->nodesetval;
        for (int i = 0; i < ns->nodeNr; ++i) {
            xmlNodePtr node = ns->nodeTab[i];
            xmlChar* txt = xmlNodeGetContent(node);
            if (txt) {
                results.emplace_back(reinterpret_cast<const char*>(txt));
                xmlFree(txt);
            } else {
                results.emplace_back("");
            }
        }
    }

    xmlXPathFreeObject(obj);
    xmlXPathFreeContext(ctxt);
    xmlFreeDoc(doc);
    xmlSetExternalEntityLoader(old_loader);
    xmlCleanupParser();

    return results;
}

static void writeSample(const std::filesystem::path& path, const std::string& content) {
    namespace fs = std::filesystem;
    fs::path tmp = path;
    tmp += ".tmp";
    std::ofstream out(tmp, std::ios::binary | std::ios::trunc);
    out.write(content.data(), static_cast<std::streamsize>(content.size()));
    out.close();
    fs::rename(tmp, path);
}

int main(int argc, char* argv[]) {
    try {
        if (argc == 3) {
            std::string id = argv[1];
            std::string xmlFile = argv[2];
            auto res = executeXPath(xmlFile, id);
            for (const auto& s : res) {
                std::cout << s << "\n";
            }
            return 0;
        }

        // Self test with 5 cases
        std::string xml =
            "<root>\n"
            "  <tag id=\"a\">Alpha</tag>\n"
            "  <tag id=\"b\">Beta</tag>\n"
            "  <tag id=\"c\">Gamma</tag>\n"
            "  <group>\n"
            "    <tag id=\"d\">Delta</tag>\n"
            "  </group>\n"
            "  <tag id=\"a-2\">Alpha2</tag>\n"
            "</root>\n";
        std::filesystem::path sample = std::filesystem::current_path() / "sample97.xml";
        writeSample(sample, xml);

        std::vector<std::string> tests = {"a", "b", "x", "a-2", "d"};
        for (const auto& t : tests) {
            std::cout << "Query id=" << t << ":\n";
            try {
                auto out = executeXPath(sample.string(), t);
                if (out.empty()) {
                    std::cout << "(no results)\n";
                } else {
                    for (const auto& line : out) std::cout << line << "\n";
                }
            } catch (const std::exception& e) {
                std::cout << "Error: " << e.what() << "\n";
            }
            std::cout << "---\n";
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
    return 0;
}