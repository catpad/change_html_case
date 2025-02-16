import os
from pathlib import Path
import re
from typing import Iterator, Optional
from bs4 import BeautifulSoup


def find_html_files(directory: Path) -> Iterator[Path]:
    """
    Recursively find all HTML files in the given directory.

    Args:
        directory (Path): Starting directory for search

    Yields:
        Path: Path objects for each HTML file found
    """
    for path in directory.rglob('*'):
        if path.is_file() and path.suffix.lower() in ('.html', '.htm'):
            yield path


def get_actual_path(path: Path) -> Optional[Path]:
    """
    Find the actual case-sensitive path for a given path.

    Args:
        path (Path): Path to check (may have incorrect case)

    Returns:
        Optional[Path]: Actual path with correct case, or None if not found
    """
    try:
        # Start from the root of the path
        current = Path(path.parts[0])
        remaining_parts = list(path.parts[1:])

        # Traverse each part of the path
        for part in remaining_parts:
            if not current.exists() or not current.is_dir():
                return None

            # Get all entries in the current directory
            try:
                entries = list(current.iterdir())
                # Find matching entry ignoring case
                matching_entry = next(
                    (entry for entry in entries
                     if entry.name.lower() == part.lower()),
                    None
                )

                if matching_entry is None:
                    return None

                current = matching_entry

            except (PermissionError, OSError):
                return None

        return current if current.exists() else None

    except Exception:
        return None


def correct_file_references(content: str, html_file: Path) -> str:
    """
    Correct case sensitivity in file references within HTML content.

    Args:
        content (str): HTML content to process
        html_file (Path): Path to the HTML file being processed

    Returns:
        str: Corrected HTML content
    """
    soup = BeautifulSoup(content, 'html.parser')

    # Process all tags with src or href attributes
    for tag in soup.find_all():
        # Get original attribute names to preserve case
        orig_attrs = set()
        for attr in tag.attrs:
            if attr.lower() in ('src', 'href'):
                orig_attrs.add(attr)

        # Process each attribute
        for attr in orig_attrs:
            if tag.get(attr):
                ref_path = tag[attr]
                # Convert to Path object relative to HTML file location
                full_path = html_file.parent / ref_path

                # Find actual path with correct case
                actual_path = get_actual_path(full_path)
                if actual_path:
                    # Convert back to relative path and update attribute
                    try:
                        relative_path = actual_path.relative_to(html_file.parent)
                        tag[attr] = str(relative_path)
                    except ValueError:
                        # Path is not relative, keep original
                        pass

    # Use minimal HTML output
    return str(soup).replace('/>', '>')


def process_directory(start_dir: Path) -> None:
    """
    Process all HTML files in a directory tree and correct case sensitivity.

    Args:
        start_dir (Path): Starting directory for processing
    """
    for html_file in find_html_files(start_dir):
        try:
            # Read the file with UTF-8 encoding
            content = html_file.read_text(encoding='utf-8')

            # Correct references
            corrected_content = correct_file_references(content, html_file)

            # Write back only if changes were made
            if content != corrected_content:
                html_file.write_text(corrected_content, encoding='utf-8')

        except (PermissionError, UnicodeDecodeError) as e:
            # Re-raise permission errors, but maybe log other errors
            if isinstance(e, PermissionError):
                raise
            print(f"Error processing {html_file}: {e}")


def main():
    """Main entry point for the script."""
    import argparse

    parser = argparse.ArgumentParser(
        description='Correct case sensitivity in HTML file references'
    )
    parser.add_argument(
        'directory',
        nargs='?',
        default='.',
        help='Starting directory (default: current directory)'
    )

    args = parser.parse_args()
    start_dir = Path(args.directory)

    if not start_dir.is_dir():
        print(f"Error: {start_dir} is not a directory")
        return 1

    try:
        process_directory(start_dir)
        return 0
    except Exception as e:
        print(f"Error: {e}")
        return 1


if __name__ == '__main__':
    exit(main())

