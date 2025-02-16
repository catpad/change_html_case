#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "html_case_corrector.h"
#include <fstream>

class HtmlCaseCorrectorTest : public ::testing::Test {
protected:
    void SetUp() override {
        tempDir = fs::temp_directory_path() / "html_case_test";
        fs::create_directories(tempDir);
    }

    void TearDown() override {
        fs::remove_all(tempDir);
    }

    void createFile(const fs::path& path, const std::string& content) {
        fs::create_directories(path.parent_path());
        std::ofstream file(path);
        file << content;
    }

    fs::path tempDir;
    HtmlCaseCorrector corrector;
};

TEST_F(HtmlCaseCorrectorTest, FindsHtmlFiles) {
    // Create test files
    createFile(tempDir / "test.html", "<html></html>");
    createFile(tempDir / "test.htm", "<html></html>");
    createFile(tempDir / "test.txt", "text file");
    createFile(tempDir / "subdir" / "nested.html", "<html></html>");

    auto htmlFiles = corrector.findHtmlFiles(tempDir);
    ASSERT_EQ(htmlFiles.size(), 3);
    
    std::vector<std::string> fileNames;
    for (const auto& file : htmlFiles) {
        fileNames.push_back(file.filename().string());
    }
    
    EXPECT_THAT(fileNames, testing::UnorderedElementsAre(
        "test.html", "test.htm", "nested.html"
    ));
}

TEST_F(HtmlCaseCorrectorTest, GetActualPathFindsCorrectCase) {
    // Create files with specific case
    createFile(tempDir / "Test.jpg", "");
    createFile(tempDir / "SubDir" / "Page.html", "");

    // Test with incorrect case
    auto actualPath1 = corrector.getActualPath(tempDir / "test.jpg");
    ASSERT_TRUE(actualPath1.has_value());
    EXPECT_EQ(actualPath1->filename(), "Test.jpg");

    auto actualPath2 = corrector.getActualPath(tempDir / "subdir" / "page.html");
    ASSERT_TRUE(actualPath2.has_value());
    EXPECT_EQ(actualPath2->filename(), "Page.html");

    // Test non-existent file
    auto actualPath3 = corrector.getActualPath(tempDir / "nonexistent.txt");
    EXPECT_FALSE(actualPath3.has_value());
}

TEST_F(HtmlCaseCorrectorTest, CorrectFileReferencesFixesCase) {
    // Create test files with specific case
    createFile(tempDir / "Images" / "Test.jpg", "");
    createFile(tempDir / "SubDir" / "Page.html", "");

    // Create HTML file with incorrect case
    std::string htmlContent = R"(
        <html>
        <body>
            <img src="images/test.jpg">
            <a href="subdir/page.html">Link</a>
        </body>
        </html>
    )";
    
    fs::path htmlFile = tempDir / "index.html";
    createFile(htmlFile, htmlContent);

    // Process the file
    corrector.processFile(htmlFile);

    // Read and verify corrected content
    std::string corrected = corrector.readFile(htmlFile);
    EXPECT_THAT(corrected, testing::AllOf(
        testing::HasSubstr("Images/Test.jpg"),
        testing::HasSubstr("SubDir/Page.html")
    ));
}

TEST_F(HtmlCaseCorrectorTest, HandlesSpecialCharacters) {
    // Create test files with UTF-8 names
    createFile(tempDir / "Изображение.jpg", "");
    createFile(tempDir / "Документы" / "Страница.html", "");

    std::string htmlContent = R"(
        <html>
        <body>
            <img src="изображение.jpg">
            <a href="документы/страница.html">Link</a>
        </body>
        </html>
    )";
    
    fs::path htmlFile = tempDir / "index.html";
    createFile(htmlFile, htmlContent);

    corrector.processFile(htmlFile);

    std::string corrected = corrector.readFile(htmlFile);
    EXPECT_THAT(corrected, testing::AllOf(
        testing::HasSubstr("Изображение.jpg"),
        testing::HasSubstr("Документы/Страница.html")
    ));
}

TEST_F(HtmlCaseCorrectorTest, HandlesSymlinks) {
    // Create real directory and symlink
    fs::path realDir = tempDir / "RealDir";
    fs::path symlinkDir = tempDir / "SymlinkDir";
    
    createFile(realDir / "Test.html", R"(<img src="image.jpg">)");
    createFile(realDir / "Image.jpg", "");

    try {
        fs::create_directory_symlink(realDir, symlinkDir);
    } catch (const fs::filesystem_error&) {
        GTEST_SKIP() << "Symlinks not supported on this platform";
    }

    corrector.processDirectory(symlinkDir);

    std::string corrected = corrector.readFile(realDir / "Test.html");
    EXPECT_THAT(corrected, testing::HasSubstr("Image.jpg"));
}

TEST_F(HtmlCaseCorrectorTest, HandlesPermissionErrors) {
    // Create test file
    fs::path testFile = tempDir / "test.html";
    createFile(testFile, "<html></html>");

    // Remove read permissions
    fs::permissions(testFile, fs::perms::none);

    EXPECT_THROW(corrector.processFile(testFile), std::runtime_error);

    // Restore permissions for cleanup
    fs::permissions(testFile, fs::perms::owner_all);
}

// Parameterized test for different HTML patterns
class HtmlPatternTest : public HtmlCaseCorrectorTest,
                       public testing::WithParamInterface<std::tuple<std::string, std::string>> {
};

TEST_P(HtmlPatternTest, CorrectsDifferentPatterns) {
    auto [input, expected] = GetParam();
    
    // Create necessary files
    createFile(tempDir / "Test.jpg", "");
    createFile(tempDir / "SubDir" / "Page.html", "");

    fs::path htmlFile = tempDir / "test.html";
    createFile(htmlFile, input);

    corrector.processFile(htmlFile);

    std::string corrected = corrector.readFile(htmlFile);
    EXPECT_THAT(corrected, testing::HasSubstr(expected));
}

INSTANTIATE_TEST_SUITE_P(
    HtmlPatterns,
    HtmlPatternTest,
    testing::Values(
        std::make_tuple("<img src='test.jpg'>", "Test.jpg"),
        std::make_tuple("<img SRC='TEST.JPG'>", "Test.jpg"),
        std::make_tuple("<a href='subdir/page.html'>", "SubDir/Page.html"),
        std::make_tuple("<img src='test.jpg' href='page.html'>", "Test.jpg")
    )
);

int main(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
