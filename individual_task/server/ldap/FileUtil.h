class FileUtil {
public:
	static bool createAndWriteToFile(const char* filename, const char* data);
	static char* readFile(const char* filename);
	static char* findFile(const char* folder, const char* name);
	static bool deleteFile(const char* filename);
};
