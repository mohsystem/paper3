#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>

static std::string transpose(const std::string& content) {
    // Check empty (after normalizing CR)
    bool anyNonWhitespace = false;
    for (char ch : content) {
        if (ch != ' ' && ch != '\n' && ch != '\r' && ch != '\t') { anyNonWhitespace = true; break; }
    }
    if (!anyNonWhitespace) return std::string();

    // Split lines, remove trailing CR
    std::vector<std::string> lines;
    {
        std::string line;
        line.reserve(content.size());
        for (size_t i = 0; i < content.size(); ++i) {
            char ch = content[i];
            if (ch == '\n') {
                if (!line.empty() && line.back() == '\r') line.pop_back();
                lines.push_back(line);
                line.clear();
            } else {
                line.push_back(ch);
            }
        }
        if (!line.empty() && line.back() == '\r') line.pop_back();
        lines.push_back(line);
    }

    // Determine first and last non-empty lines
    int firstNonEmpty = -1, lastNonEmpty = -1;
    for (int i = 0; i < (int)lines.size(); ++i) {
        if (!lines[i].empty()) { firstNonEmpty = i; break; }
    }
    for (int i = (int)lines.size() - 1; i >= 0; --i) {
        if (!lines[i].empty()) { lastNonEmpty = i; break; }
    }
    if (firstNonEmpty == -1) return std::string();

    for (int i = firstNonEmpty; i <= lastNonEmpty; ++i) {
        if (lines[i].empty()) throw std::invalid_argument("Empty line detected within data block");
    }

    std::vector<std::vector<std::string>> rows;
    int cols = -1;

    for (int i = firstNonEmpty; i <= lastNonEmpty; ++i) {
        const std::string& ln = lines[i];
        if (!ln.empty()) {
            if (ln.front() == ' ' || ln.back() == ' ') throw std::invalid_argument("Invalid spacing: leading/trailing spaces");
            for (size_t k = 1; k < ln.size(); ++k) {
                if (ln[k] == ' ' && ln[k-1] == ' ') throw std::invalid_argument("Invalid spacing: multiple consecutive spaces");
            }
        }
        std::vector<std::string> tokens;
        if (!ln.empty()) {
            size_t start = 0;
            for (size_t pos = 0; pos <= ln.size(); ++pos) {
                if (pos == ln.size() || ln[pos] == ' ') {
                    tokens.emplace_back(ln.substr(start, pos - start));
                    start = pos + 1;
                }
            }
        }
        if (cols == -1) cols = (int)tokens.size();
        else if ((int)tokens.size() != cols) throw std::invalid_argument("Inconsistent column counts");
        rows.push_back(std::move(tokens));
    }

    if (rows.empty() || cols == 0) return std::string();

    // Build output
    std::string out;
    // Precompute length for reserve
    size_t sumTokenLen = 0;
    for (const auto& r : rows) for (const auto& t : r) sumTokenLen += t.size();
    size_t spaces = (size_t)cols * (rows.size() > 0 ? (rows.size() - 1) : 0);
    size_t newlines = (cols > 0 ? (cols - 1) : 0);
    out.reserve(sumTokenLen + spaces + newlines);

    for (int c = 0; c < cols; ++c) {
        for (size_t r = 0; r < rows.size(); ++r) {
            out.append(rows[r][c]);
            if (r + 1 < rows.size()) out.push_back(' ');
        }
        if (c + 1 < cols) out.push_back('\n');
    }
    return out;
}

int main() {
    std::vector<std::string> tests = {
        "name age\nalice 21\nryan 30",
        "a b c",
        "a\nb\nc",
        "a b\nc d\n",
        "a b\nc"
    };
    for (size_t i = 0; i < tests.size(); ++i) {
        std::cout << "Test " << (i + 1) << ":\n";
        try {
            std::string res = transpose(tests[i]);
            std::cout << res << "\n";
        } catch (const std::exception& ex) {
            std::cout << "ERROR: " << ex.what() << "\n";
        }
        std::cout << "----\n";
    }
    return 0;
}