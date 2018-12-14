#include <string>

class FileUtil {
public:
	static void createAndWriteToFile(std::string filename, std::string data);
	static std::string readFile(std::string filename);
	static std::string findFile(std::string folder, std::string name);
	static void deleteFile(std::string filename);
};
