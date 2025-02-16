# HTML Case Sensitivity Corrector

A Python utility to fix case sensitivity issues in HTML file references when migrating from Windows to Linux systems.

## Problem Statement

When migrating web content from Windows to Linux systems, HTML files often break due to case sensitivity differences:
- Windows file systems store case in filenames but are case-insensitive
- Linux file systems are case-sensitive
- This leads to broken links and images when file references in HTML don't match the actual case on disk

## Features

- Recursively processes HTML files in a directory structure
- Corrects case sensitivity in:
  - Image source attributes (`src`)
  - Hyperlink references (`href`)
- Preserves:
  - Original HTML structure
  - Attribute order
  - UTF-8 encoding and special characters
  - Relative paths
- Handles:
  - Both .html and .htm files
  - Symbolic links
  - Mixed case in directory names
  - Non-ASCII characters in filenames

## Installation

1. Clone the repository:
```bash
git clone https://github.com/yourusername/html-case-corrector.git
cd html-case-corrector
```

2. Create and activate a virtual environment (recommended):
```bash
python -m venv venv
source venv/bin/activate  # On Windows: venv\Scripts\activate
```

3. Install dependencies:
```bash
pip install beautifulsoup4 pytest
```

## Usage

### Command Line

Basic usage:
```bash
python html_case_corrector.py [directory_path]
```

If no directory is specified, the current directory will be processed.

### As a Module

```python
from pathlib import Path
from html_case_corrector import process_directory

# Process a specific directory
process_directory(Path("/path/to/your/web/files"))
```

## Example

Before:
```html
<img src="images/logo.PNG" alt="Logo">
<a href="PAGES/About.HTML">About Us</a>
```

After (assuming actual files are "Images/Logo.png" and "pages/about.html"):
```html
<img src="Images/Logo.png" alt="Logo">
<a href="pages/about.html">About Us</a>
```

## Testing

The project includes comprehensive test coverage using pytest:

```bash
# Run all tests
pytest

# Run specific test file
pytest test_html_filename_change.py

# Run with verbosity
pytest -v
```

## API Reference

### Main Functions

#### `process_directory(start_dir: Path) -> None`
Process all HTML files in a directory tree and correct case sensitivity.

#### `find_html_files(directory: Path) -> Iterator[Path]`
Recursively find all HTML files in the given directory.

#### `correct_file_references(content: str, html_file: Path) -> str`
Correct case sensitivity in file references within HTML content.

#### `get_actual_path(path: Path) -> Optional[Path]`
Find the actual case-sensitive path for a given path.

## Error Handling

The script handles various error conditions:
- Permission errors (unreadable files/directories)
- UTF-8 encoding issues
- Non-existent files
- Invalid HTML
- Circular symbolic links

## Technical Details

- Uses BeautifulSoup4 for robust HTML parsing
- Preserves original HTML structure and formatting
- Handles both Windows and Unix-style paths
- Supports complex directory structures
- Thread-safe implementation

## Limitations

- Only processes .html and .htm files
- External URLs are not affected
- Requires read/write permissions for all files
- Does not handle server-side includes or dynamic content

## Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Run the test suite
5. Submit a pull request

## License

This project is licensed under the MIT License - see the LICENSE file for details.

## Authors

Claude AI

## Acknowledgments

- BeautifulSoup4 team for HTML parsing
- Python Pathlib contributors
- Testing framework provided by pytest
- 
