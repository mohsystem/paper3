#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <sys/stat.h>

bool createExecutableScript(const std::string& directory, const std::string& contentInput) {
    try {
        std::filesystem::path dir =
            (directory.empty() || directory == ".") ? std::filesystem::path(".") : std::filesystem::path(directory);
        std::filesystem::path scriptPath = dir / "script.sh";

        if (scriptPath.has_parent_path()) {
            std::filesystem::create_directories(scriptPath.parent_path());
        }

        std::string content = contentInput.empty()
                              ? "#!/bin/bash\necho \"Hello from script.sh\"\n"
                              : contentInput;

        std::ofstream ofs(scriptPath, std::ios::binary);
        if (!ofs) return false;
        ofs << content;
        ofs.close();

        // Set mode to 0755
        if (chmod(scriptPath.string().c_str(), 0755) != 0) {
            // On some systems (e.g., Windows), chmod may not work as expected; still return true if file exists.
            return std::filesystem::exists(scriptPath);
        }
        return true;
    } catch (...) {
        return false;
    }
}

int main() {
    std::pair<std::string, std::string> tests[5] = {
        {".", "#!/bin/bash\necho \"C++ test 1\""},
        {"cpp_out1", "#!/bin/bash\necho \"C++ test 2\""},
        {"cpp_out2/nested", "#!/bin/bash\necho \"C++ test 3\""},
        {"cpp out3/spaced", "#!/bin/bash\necho \"C++ test 4\""},
        {"cpp_out4/a/b", "#!/bin/bash\necho \"C++ test 5\""}
    };
    for (int i = 0; i < 5; ++i) {
        bool ok = createExecutableScript(tests[i].first, tests[i].second);
        std::cout << "Test " << (i + 1) << " (" << tests[i].first << "): " << (ok ? "success" : "failed") << std::endl;
    }
    return 0;
}