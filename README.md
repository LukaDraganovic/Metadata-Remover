# Metadata-Remover
This project removes all non-essential metadata (EXIF, XMP, IPTC) from JPEG images while ensuring output files remain valid, readable, and secure. The tool prioritizes correctness and safety over fragile marker-level manipulation by using a full decode → re-encode pipeline, guaranteeing that metadata cannot survive the process.
# JPEG Metadata Remover

A command-line tool that removes EXIF, XMP, and IPTC metadata from JPEG images by re-encoding them through libjpeg-turbo.

## Why This Exists

JPEG files often contain metadata you might not want to share: GPS coordinates, camera models, timestamps, software tags, and more. This tool strips all of that out while keeping the actual image intact.

## How It Works

Rather than trying to surgically remove metadata markers (which is fragile and error-prone), this tool takes a different approach:

1. Decode the JPEG completely using libjpeg-turbo
2. Re-encode it with default settings and no metadata
3. Write the clean output

This guarantees no metadata survives, handles malformed JPEGs safely, and produces valid output that works in any viewer. It's slower than marker manipulation but far more reliable.

## Usage

```bash
metadata-remover.exe input.jpg output.jpg
```

That's it. The output will be a clean JPEG with all metadata removed.

## Building

**Requirements:**
- Windows 10 or later
- Visual Studio 2022 (x64)
- libjpeg-turbo installed via vcpkg

**Steps:**
```bash
vcpkg install libjpeg-turbo:x64-windows
```

Open the solution in Visual Studio and build in x64 Release mode.

## What Gets Removed

- EXIF data (camera settings, GPS, timestamps)
- XMP metadata (Adobe/editing software data)
- IPTC tags (copyright, captions, keywords)
- Comment markers

The image data itself is preserved. Quality settings default to libjpeg-turbo's standard compression.

## Technical Notes

- Uses `setjmp`/`longjmp` for libjpeg error handling
- Full decode/encode cycle ensures no metadata leakage
- Handles edge cases that marker-based tools miss

## Limitations

- Windows only (for now)
- JPEG only - no PNG, WebP, HEIC support
- No batch processing
- Re-encoding may slightly change file size/quality

## Potential Improvements

- Cross-platform support (Linux/macOS with standard libjpeg)
- Batch mode for processing directories
- Quality control options
- Support for other formats
- Preserve vs strip mode
- User frendly GUI

## License

MIT License - see LICENSE file

## Why Not Just Use Existing Tools?

You can. ExifTool, ImageMagick, and others work fine. This project exists because:
- I wanted to understand JPEG processing at a low level
- It's a practical exercise in C, external libraries, and binary file handling
- Sometimes you want a simple single-purpose tool
