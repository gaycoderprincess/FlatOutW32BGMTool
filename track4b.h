struct BITMAPHEADER {
	uint16_t identifier = 0x4D42;
	uint32_t fileSize;
	uint32_t unused = 0;
	uint32_t dataOffset = 0;
} __attribute__((packed, aligned(1)));
static_assert(sizeof(BITMAPHEADER) == 14);

struct BITMAPINFOHEADER {
	uint32_t biSize;
	uint32_t biWidth;
	uint32_t biHeight;
	uint16_t biPlanes;
	uint16_t biBitCount;
	uint32_t biCompression;
	uint32_t biSizeImage;
	uint32_t biXPelsPerMeter;
	uint32_t biYPelsPerMeter;
	uint32_t biClrUsed;
	uint32_t biClrImportant;
};
static_assert(sizeof(BITMAPINFOHEADER) == 40);

uint8_t* a4BData = nullptr;
size_t n4BDataSize = 0;

bool Parse4B() {
	WriteConsole("Parsing 4B map...", LOG_ALWAYS);

	if (sFileName.extension() != ".4b") {
		return false;
	}

	std::ifstream fin(sFileName, std::ios::in | std::ios::binary );
	if (!fin.is_open()) return false;

	fin.seekg(0, std::ios::end);
	n4BDataSize = fin.tellg();
	fin.seekg(0, std::ios::beg);

	a4BData = new uint8_t[n4BDataSize];
	fin.read((char*)a4BData, n4BDataSize);
	return true;
}

bool Write4BToBMP() {
	WriteConsole("Exporting 4B map to .bmp...", LOG_ALWAYS);

	std::ofstream fout(sFileNameNoExt.string() + "_out.bmp", std::ios::out | std::ios::binary );
	if (!fout.is_open()) return false;

	uint32_t bmpFileSize = n4BDataSize * 6;

	BITMAPHEADER baseHeader;
	baseHeader.fileSize = bmpFileSize + sizeof(BITMAPHEADER) + sizeof(BITMAPINFOHEADER);
	baseHeader.dataOffset = sizeof(BITMAPHEADER) + sizeof(BITMAPINFOHEADER);

	BITMAPINFOHEADER header;
	header.biSize = sizeof(header);
	header.biWidth = 256;
	header.biHeight = 256;
	header.biPlanes = 1;
	header.biBitCount = 24;
	header.biCompression = 0;
	header.biSizeImage = bmpFileSize;
	header.biXPelsPerMeter = 256;
	header.biYPelsPerMeter = 256;
	header.biClrUsed = 0;
	header.biClrImportant = 0;
	fout.write((char*)&baseHeader, sizeof(baseHeader));
	fout.write((char*)&header, sizeof(header));
	for (int i = 0; i < n4BDataSize; i++) {
		// duplicate data 3 times for 24 bit, and twice afterwards to make it square
		for (int j = 0; j < 6; j++) {
			fout.write((char*)&a4BData[i], 1);
		}
	}

	return true;
}