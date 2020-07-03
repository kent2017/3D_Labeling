#include <string>
#include <vector>
#include <experimental/filesystem>


class FileContainer {
public:
	FileContainer(const std::string& mesh_format, const std::string& label_format) { meshFormat = mesh_format; labelFormat = label_format; }
	~FileContainer() {}

	void Init(const std::vector<std::string>& dirsOrFiles);
	void Clear() { filepaths.clear(); }

	int size() const { return filepaths.size(); }

	std::string Pop();

private:
	std::string meshFormat;
	std::string labelFormat;
	std::vector<std::string> filepaths;
};