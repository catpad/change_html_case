#include "HtmlCaseCorrector.h"

// html_case_corrector.cpp
void HtmlCaseCorrector::processDirectory(const fs::path& startDir) {
    for (const auto& htmlFile : findHtmlFiles(startDir)) {
        try {
            processFile(htmlFile);
        } catch (const std::exception& e) {
            std::cerr << "Error processing " << htmlFile << ": " << e.what() << std::endl;
        }
    }
}

std::optional<fs::path> HtmlCaseCorrector::getActualPath(const fs::path& path) const {
    if (!fs::exists(path.parent_path())) {
        return std::nullopt;
    }

    try {
        for (const auto& entry : fs::directory_iterator(path.parent_path())) {
            if (comparePathsIgnoreCase(entry.path().filename(), path.filename())) {
                return entry.path();
            }
        }
    } catch (const fs::filesystem_error&) {
        return std::nullopt;
    }

    return std::nullopt;
}

void HtmlCaseCorrector::processFile(const fs::path& htmlFile) {
    std::string content = readFile(htmlFile);
    std::string corrected = correctFileReferences(content, htmlFile);
    
    if (content != corrected) {
        writeFile(htmlFile, corrected);
    }
}

std::vector<fs::path> HtmlCaseCorrector::findHtmlFiles(const fs::path& directory) const {
    std::vector<fs::path> htmlFiles;
    const std::unordered_set<std::string> validExtensions = {".html", ".htm"};

    try {
        for (const auto& entry : fs::recursive_directory_iterator(directory)) {
            if (entry.is_regular_file()) {
                std::string ext = entry.path().extension().string();
                std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
                if (validExtensions.count(ext) > 0) {
                    htmlFiles.push_back(entry.path());
                }
            }
        }
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Error accessing directory: " << e.what() << std::endl;
    }

    return htmlFiles;
}

std::string HtmlCaseCorrector::correctFileReferences(const std::string& content, const fs::path& htmlFile) {
    GumboOutput* output = gumbo_parse(content.c_str());
    std::string result = content;

    if (output) {
        processNode(output->root, htmlFile, result);
        gumbo_destroy_output(&kGumboDefaultOptions, output);
    }

    return result;
}

void HtmlCaseCorrector::processNode(GumboNode* node, const fs::path& htmlFile, std::string& content) {
    if (node->type != GUMBO_NODE_ELEMENT) {
        return;
    }

    GumboAttribute* src = gumbo_get_attribute(&node->v.element.attributes, "src");
    GumboAttribute* href = gumbo_get_attribute(&node->v.element.attributes, "href");

    if (src) {
        updateAttribute(src, htmlFile, content);
    }
    if (href) {
        updateAttribute(href, htmlFile, content);
    }

    GumboVector* children = &node->v.element.children;
    for (unsigned int i = 0; i < children->length; ++i) {
        processNode(static_cast<GumboNode*>(children->data[i]), htmlFile, content);
    }
}

void HtmlCaseCorrector::updateAttribute(GumboAttribute* attr, const fs::path& htmlFile, std::string& content) {
    fs::path refPath = fs::path(attr->value);
    fs::path fullPath = htmlFile.parent_path() / refPath;

    auto actualPath = getActualPath(fullPath);
    if (actualPath) {
        fs::path relativePath = fs::relative(*actualPath, htmlFile.parent_path());
        replaceInContent(content, attr->value, relativePath.string());
    }
}

bool HtmlCaseCorrector::comparePathsIgnoreCase(const fs::path& a, const fs::path& b) const {
    std::string aStr = a.string();
    std::string bStr = b.string();
    return std::equal(aStr.begin(), aStr.end(), bStr.begin(), bStr.end(),
                     [](char a, char b) { return std::tolower(a) == std::tolower(b); });
}

void HtmlCaseCorrector::replaceInContent(std::string& content, const std::string& oldStr, const std::string& newStr) {
    size_t pos = 0;
    while ((pos = content.find(oldStr, pos)) != std::string::npos) {
        content.replace(pos, oldStr.length(), newStr);
        pos += newStr.length();
    }
}

std::string HtmlCaseCorrector::readFile(const fs::path& path) const {
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Cannot open file: " + path.string());
    }
    return std::string(std::istreambuf_iterator<char>(file),
                      std::istreambuf_iterator<char>());
}

void HtmlCaseCorrector::writeFile(const fs::path& path, const std::string& content) const {
    std::ofstream file(path, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Cannot write file: " + path.string());
    }
    file << content;
}



#endif // HTML_CASE_CORRECTOR_H
