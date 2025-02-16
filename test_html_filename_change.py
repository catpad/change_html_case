import os
import pytest
from pathlib import Path
from bs4 import BeautifulSoup
from html_filename_change import (
    find_html_files,
    correct_file_references,
    process_directory,
    get_actual_path
)


@pytest.fixture
def tmp_html_structure(tmp_path):
    """Create a temporary directory structure with HTML files for testing."""
    # Create main directory structure
    img_dir = tmp_path / "Images"
    sub_dir = tmp_path / "SubDir"
    img_dir.mkdir()
    sub_dir.mkdir()

    # Create test files with mixed case
    (img_dir / "Test_Image.jpg").write_text("")
    (img_dir / "icon.PNG").write_text("")
    (sub_dir / "Page2.html").write_text("")

    # Create main HTML file with incorrect case references
    main_html_content = """
    <html>
    <body>
        <img src="images/test_image.jpg" alt="Test">
        <img SRC="IMAGES/ICON.png" alt="Icon">
        <a href="subdir/page2.HTML">Link</a>
    </body>
    </html>
    """
    (tmp_path / "index.html").write_text(main_html_content)

    return tmp_path


def test_find_html_files(tmp_html_structure):
    """Test finding HTML files in directory structure."""
    html_files = list(find_html_files(tmp_html_structure))

    assert len(html_files) == 2
    assert any(file.name == "index.html" for file in html_files)
    assert any(file.name == "Page2.html" for file in html_files)


def test_get_actual_path(tmp_html_structure):
    """Test finding actual case-sensitive path."""
    # Test existing file with incorrect case
    test_path = tmp_html_structure / "images" / "test_image.jpg"
    actual_path = get_actual_path(test_path)
    assert actual_path.name == "Test_Image.jpg"

    # Test non-existent file
    non_existent = tmp_html_structure / "nonexistent.txt"
    assert get_actual_path(non_existent) is None


def test_correct_file_references(tmp_html_structure):
    """Test correction of file references in HTML content."""
    html_content = """
    <img src="images/test_image.jpg">
    <a href="SUBDIR/PAGE2.HTML">
    """

    corrected = correct_file_references(
        html_content,
        tmp_html_structure / "index.html"
    )

    assert 'Images/Test_Image.jpg' in corrected
    assert 'SubDir/Page2.html' in corrected


def test_process_directory(tmp_html_structure):
    """Test complete directory processing."""
    process_directory(tmp_html_structure)
    main_html = (tmp_html_structure / "index.html").read_text()

    assert 'Images/Test_Image.jpg' in main_html
    assert 'Images/icon.PNG' in main_html
    assert 'SubDir/Page2.html' in main_html


@pytest.mark.parametrize("html_content,expected_path", [
    (
            '<img src="test.jpg">',
            'Test.jpg'
    ),
    (
            '<img SRC="TEST.JPG">',
            'Test.jpg'
    ),
    (
            '<a href="subdir/page.html">',
            'SubDir/Page.html'
    ),
    (
            '<img src="test.jpg" href="page.html">',
            'Test.jpg'
    ),
])
def test_correct_file_references_parametrized(tmp_html_structure, html_content, expected_path):
    """Test HTML pattern corrections with various inputs.

    Args:
        tmp_html_structure: Temporary directory for test files
        html_content: Input HTML content to test
        expected_path: Expected path after case correction
    """
    # Create test files with correct case
    (tmp_html_structure / "Test.jpg").write_text("")
    (tmp_html_structure / "SubDir").mkdir(exist_ok=True)
    (tmp_html_structure / "SubDir" / "Page.html").write_text("")

    result = correct_file_references(
        html_content,
        tmp_html_structure / "index.html"
    )

    # Parse the result to check if the correct path is present
    soup = BeautifulSoup(result, 'html.parser')
    found = False
    for tag in soup.find_all():
        for attr in ('src', 'href'):
            if tag.get(attr):
                if tag[attr] == expected_path:
                    found = True
                    break
    assert found, f"Expected path {expected_path} not found in result: {result}"


def test_process_directory_with_symlinks(tmp_path):
    """Test processing with symbolic links."""
    real_dir = tmp_path / "RealDir"
    real_dir.mkdir()
    symlink_dir = tmp_path / "SymlinkDir"

    (real_dir / "Test.html").write_text('<img src="image.jpg">')
    (real_dir / "Image.jpg").write_text("")

    try:
        os.symlink(real_dir, symlink_dir)
    except (OSError, NotImplementedError):
        pytest.skip("Symlinks not supported on this platform")

    process_directory(symlink_dir)
    assert 'Image.jpg' in (real_dir / "Test.html").read_text()


def test_error_handling(tmp_html_structure):
    """Test error handling for various scenarios."""
    test_file = tmp_html_structure / "unreadable.html"
    test_file.write_text('<img src="test.jpg">')
    test_file.chmod(0o000)

    try:
        process_directory(tmp_html_structure)
        test_file.chmod(0o666)
        pytest.fail("Should raise PermissionError")
    except PermissionError:
        test_file.chmod(0o666)
        pass


def test_utf8_encoding(tmp_html_structure):
    """Test handling of UTF-8 encoded files with special characters."""
    html_content = """
    <html>
    <body>
        <img src="имя файла.jpg" alt="тест">
        <a href="подпапка/страница.html">Ссылка</a>
    </body>
    </html>
    """.encode('utf-8')

    (tmp_html_structure / "Имя файла.jpg").write_bytes(b"")
    subdir = tmp_html_structure / "Подпапка"
    subdir.mkdir()
    (subdir / "Страница.html").write_bytes(b"")

    test_file = tmp_html_structure / "test_utf8.html"
    test_file.write_bytes(html_content)

    process_directory(tmp_html_structure)

    corrected_content = test_file.read_text('utf-8')
    assert 'src="Имя файла.jpg"' in corrected_content
    assert 'href="Подпапка/Страница.html"' in corrected_content

