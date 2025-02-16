#include "html_case_corrector.h"
#include <iostream>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <directory>" << std::endl;
        return 1;
    }

    try {
        fs::path startDir = argv[1];
        if (!fs::exists(startDir) || !fs::is_directory(startDir)) {
            std::cerr << "Error: " << startDir << " is not a valid directory" << std::endl;
            return 1;
        }

        HtmlCaseCorrector corrector;
        corrector.processDirectory(startDir);
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}

