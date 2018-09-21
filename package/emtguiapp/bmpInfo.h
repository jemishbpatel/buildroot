#include <stdint.h>

typedef struct _fileHeader {
	uint16_t bfType; /* The characters "BM" */
	uint32_t bfSize; /* The size of the file in bytes */
	uint16_t bfReserved1; /* Unused - must be zero */
	uint16_t bfReserved2; /* Unused - must be zero */
	uint32_t bfOffBits; /* Offset to start of Pixel Data */
} __attribute__((packed)) fileHeader;

typedef struct _imageHeader {
	uint32_t biSize; /* Header Size - Must be at least 40 */
	uint32_t biWidth; /* Image width in pixels */
	uint32_t biHeight; /* Image height in pixels */
	uint16_t biPlanes; /* Must be 1 */
	uint16_t biBitCount; /* Bits per pixel - 1, 4, 8, 16, 24, or 32 */
	uint32_t biCompression; /* Compression type (0 = uncompressed) */
	uint32_t biSizeImage; /* Image Size - may be zero for uncompressed images */
	uint32_t biXPelsPerMeter; /* Preferred resolution in pixels per meter */
	uint32_t biYPelsPerMeter; /* Preferred resolution in pixels per meter */
	uint32_t biClrUsed; /* Number Color Map entries that are actually used */
	uint32_t biClrImportant; /* Number of significant colors */
} __attribute__((packed)) imageHeader;

/* For future : For 24 bit BMP and For 32 bit BMP */
typedef struct _rgbQuad {
	uint8_t rgbBlue; /* Red component value */
	uint8_t rgbGreen; /* Green component value */
	uint8_t rgbRed; /* Red component value */
	uint8_t rgbReserved; /* Reserved */
} __attribute__((packed)) rgbQuad;

/* For Monochrome BMP */
typedef struct _gBmpFile {
	fileHeader bmFileHeader; /* BMP File Header */
	imageHeader bmInfoHeader; /* BMP Information Header */
	uint8_t *bmPixelData; /* Pixel Data */
} __attribute__((packed)) gBmpFile;

/* For Color BMP */
typedef struct _cBmpFile {
	fileHeader bmFileHeader; /* BMP File Header */
	imageHeader bmInfoHeader; /* BMP Information Header */
	rgbQuad *colorTable;
	uint8_t *bmPixelData; /* Pixel Data */
} __attribute__((packed)) cBmpFile;
