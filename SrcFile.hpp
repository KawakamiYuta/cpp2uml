#ifndef SRCFILE_HPP
#define SRCFILE_HPP
#include <memory>
#include <cstdio>
#include <string>
#include <vector>

using FilePath = std::string;

class SrcList
{
	private:
		std::vector<FilePath> mSrcList;
	public:
		SrcList() = default;
		~SrcList() = default;
		SrcList(const std::vector<FilePath>& aFilePath) {
			add(aFilePath);
		}
		void add(const std::vector<FilePath>& aFilePath) {
			std::unique_ptr<const char[]> cwd(::get_current_dir_name());
			for (auto&& fp : aFilePath) {
				mSrcList.push_back(cwd.get() + ("/" + fp));
				::printf("SRCFILE:%s\n",mSrcList.back().c_str());
			}
		}
		bool contain(const FilePath& aFilePath) {
			auto itr = find(mSrcList.begin(),mSrcList.end(),aFilePath);
			return itr != mSrcList.end();
		}
};

static inline std::string basename(const std::string& path)
{
	return path.substr(path.find_last_of('/') + 1);
}

static inline std::vector<std::string> split(const std::string& str,char delm)
{
	std::vector<std::string> elems;
	std::stringstream ss(str);
	std::string item;
	while (getline(ss,item,delm)) {
		if (!item.empty()) {
			elems.push_back(item);
		}
	}
	return elems;
}

#endif
