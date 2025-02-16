// html_case_corrector.h
#ifndef HTML_CASE_CORRECTOR_H
#define HTML_CASE_CORRECTOR_H

#include <string>
#include <vector>
#include <filesystem>
#include <optional>
#include <regex>
#include <fstream>
#include <sstream>
#include <locale>
#include <algorithm>
#include <unordered_set>
#include "gumbo.h" // HTML parser library

namespace fs = std::filesystem;

class HtmlCaseCorrector {
public:
    // Main function to process a directory
    void processDirectory(const fs::path& startDir);

    // Get actual case-sensitive path
    std::optional<fs::path> getActualPath(const fs::path& path) const;

    // Process single HTML file
    void processFile(const fs::path& htmlFile);

    // Make public for testing
    std::vector<fs::path> findHtmlFiles(const fs::path& directory) const;
    std::string readFile(const fs::path& path) const;

private:
    // Correct file references in HTML content
    std::string correctFileReferences(const std::string& content, const fs::path& htmlFile);

    // Process HTML node recursively
    void processNode(GumboNode* node, const fs::path& htmlFile, std::string& content);

    // Update attribute with correct case
    void updateAttribute(GumboAttribute* attr, const fs::path& htmlFile, std::string& content);

    // Helper functions
    bool comparePathsIgnoreCase(const fs::path& a, const fs::path& b) const;
    void replaceInContent(std::string& content, const std::string& oldStr, const std::string& newStr);
    void writeFile(const fs::path& path, const std::string& content) const;
};

