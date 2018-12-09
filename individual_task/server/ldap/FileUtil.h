#include <string>

class FileUtil {
public:
	static bool createAndWriteToFile(std::string filename, std::string data);
	static std::string readFile(std::string filename);
	static std::string findFile(std::string folder, std::string name);
	static bool deleteFile(std::string filename);
};
