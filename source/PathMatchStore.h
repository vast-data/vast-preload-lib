#ifndef PATHMATCHSTORE_H_
#define PATHMATCHSTORE_H_

#include <boost/algorithm/string.hpp>
#include <fstream>
#include <mutex>
#include <regex>
#include <set>
#include <string>
#include "Common.h"
#include "Logger.h"

class PathMatchStoreElem
{
	public:
		PathMatchStoreElem(std::regex regex, std::string regexStr) :
			regex(regex), regexStr(regexStr) {}

		std::regex regex;
		std::string regexStr;


	bool operator< (const PathMatchStoreElem& rhs) const
	{
		return this->regexStr < rhs.regexStr;
	}
};

typedef std::set<PathMatchStoreElem> PathMatchMap;
typedef PathMatchMap::iterator PathMatchMapIter;

/**
 * Store paths with wildcards ("*", "?") and check if other paths match against these.
 */
class PathMatchStore
{
	public:
		bool loadPathFile(std::string path);

	private:
		PathMatchMap pathMap;

		// inliners
	public:

		bool checkPathMatch(std::string path)
		{
			for(const PathMatchStoreElem& currentMapElem : pathMap)
			{
				bool isMatch = std::regex_match(path, currentMapElem.regex);

				if(isMatch)
					return true;
			}

			return false; // no match found
		}

		int size()
		{
			return pathMap.size();
		}

	private:

		/**
		 * Convert given string with wildcards ("*", "?") to std::regex.
		 *
		 * See https://stackoverflow.com/a/65851545/14419626
		 */
		std::regex wildcardStrToRegex(std::string wildcardStr, bool caseSensitive = true)
		{
			std::string regexStr(wildcardStr);

			// escape all regex special chars

			regexStr = std::regex_replace(regexStr, std::regex("\\\\"), "\\\\");
			regexStr = std::regex_replace(regexStr, std::regex("\\^"), "\\^");
			regexStr = std::regex_replace(regexStr, std::regex("\\."), "\\.");
			regexStr = std::regex_replace(regexStr, std::regex("\\$"), "\\$");
			regexStr = std::regex_replace(regexStr, std::regex("\\|"), "\\|");
			regexStr = std::regex_replace(regexStr, std::regex("\\("), "\\(");
			regexStr = std::regex_replace(regexStr, std::regex("\\)"), "\\)");
			regexStr = std::regex_replace(regexStr, std::regex("\\{"), "\\{");
			regexStr = std::regex_replace(regexStr, std::regex("\\{"), "\\}");
			regexStr = std::regex_replace(regexStr, std::regex("\\["), "\\[");
			regexStr = std::regex_replace(regexStr, std::regex("\\]"), "\\]");
			regexStr = std::regex_replace(regexStr, std::regex("\\+"), "\\+");
			regexStr = std::regex_replace(regexStr, std::regex("\\/"), "\\/");

			// convert wildcard specific chars ("*", "?") to their regex equivalents

			regexStr = std::regex_replace(regexStr, std::regex("\\?"), ".");
			regexStr = std::regex_replace(regexStr, std::regex("\\*"), ".*");

			return std::regex(regexStr,
				caseSensitive ? std::regex_constants::ECMAScript : std::regex_constants::icase);
		}

};

extern PathMatchStore* pathMatchStore;

#endif /* PATHMATCHSTORE_H_ */
