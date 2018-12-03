class FileUtil {
public:
	static bool createAndWriteToFile(const char* filename, const char* data);
	static char* readFile(const char* filename);
	static bool deleteFile(const char* filename);
};
