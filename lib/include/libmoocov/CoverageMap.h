#ifndef LIBMOOCOV_COVERAGEMAP_H
#define LIBMOOCOV_COVERAGEMAP_H

#include <tuple>
#include <string>
#include <set>
#include <cassert>

#include "llvm/ADT/DenseMap.h"

#include "libmoocov/Core.h"
#include "libmoocov/SourceLocation.h"

namespace libmoocov {

/// \brief Maps a (file ID, signal ID) to a source range.
struct SignalMapping {
	FileID fileID;
	signalid_t id;
	SourceRange sourceRange;

	bool operator==(const SignalMapping& rhs) const {
		return std::tie(fileID, id) == std::tie(rhs.fileID, rhs.id);
	}

	bool operator!=(const SignalMapping& rhs) const {
		return !operator==(rhs);
	}

	/// \brief Defines an ordering of SignalMappings based on the source ranges they cover.
	///
	/// SignalMappings that begin earlier come first.
	/// If two SignalMappings begin at the same SourceLocation, then whichever covers the longer range comes first.
	/// If two SignalMappings cover the exact same source range, they'll evaluate to equal.
	struct CompareByUniqueRange {
		bool operator()(const SignalMapping& lhs, const SignalMapping& rhs) const {
			return lhs.sourceRange < rhs.sourceRange;
		}
	};

	/// The same as CompareByRange, except if two SignalMappings cover the exact same range, they won't evaluate as being equal (but their exact ordering is undefined).
	///
	/// This is useful so as to not lose data with e.g. header files that are instrumented differently because of the different preprocessor contexts at their include points).
	struct CompareByRange {
		bool operator()(const SignalMapping& lhs, const SignalMapping& rhs) const {
			if(lhs.sourceRange < rhs.sourceRange) return true;
			else if(lhs.sourceRange == rhs.sourceRange) return lhs.fileID < rhs.fileID;
			return false;
		}
	};
};

/// \brief Represents an ordered set of SignalMapping-s, potentially from different FileIDs (and potentially having signals that cover the exact same source range).
using SignalSet = std::set<SignalMapping, SignalMapping::CompareByRange>;

/// \brief A simple structure representing the mapping information of a single FileID.
struct SignalMap {
	FileID fileID;
	std::string sourceFilePath;
	llvm::DenseMap<signalid_t, SignalMapping> signals;

	/*implicit*/ SignalMap() = default;

	explicit SignalMap(FileID fileID_, const std::string& sourceFilePath_)
		: fileID{fileID_}, sourceFilePath{sourceFilePath_} {}

	bool isValid() const { return fileID.isValid() && !sourceFilePath.empty(); }

	void add(const SignalMapping& signal) {
		signals.insert(std::make_pair(signal.id, signal));
	}

	bool read(const std::string& filePath);

	void clear() {
		fileID = FileID{};
		sourceFilePath = "";
		signals.clear();
	}
};

/// \brief Represents a complete mapping of a single (original, uninstrumented) source file.
/// This can contain SignalMapping-s from multiple FileID-s (e.g. headers that are included multiple times in one or more translation units).
class SourceFileMap {
public:
	/*implicit*/ SourceFileMap() = default;

	explicit SourceFileMap(const std::string& sourcePath)
		: m_path{sourcePath} {}

	bool isValid() const { return !m_path.empty(); }
	bool isEmpty() const { return m_signals.empty(); }

	const std::string& getSourcePath() const { return m_path; }
	const std::set<FileID>& getFiles() const { return m_files; }
	const SignalSet& getSignals() const { return m_signals; }

	bool hasFileID(FileID id) const {
		return m_files.find(id) != m_files.end();
	}

	const SignalMapping* getSignal(FileID fileID, signalid_t signalID) const {
		for(const SignalMapping& signal : m_signals) {
			if(signal.id == signalID && signal.fileID == fileID) {
				return &signal;
			}
		}

		return nullptr;
	}

	void addSignalMap(const SignalMap& map) {
		if(m_path.empty()) m_path = map.sourceFilePath;
		assert(m_path == map.sourceFilePath);

		m_files.insert(map.fileID);

		for(const auto& pair : map.signals) {
			m_signals.insert(pair.second);
		}
	}

private:
	std::string m_path;
	std::set<FileID> m_files;
	SignalSet m_signals;
};

} // end namespace libmoocov

#endif // LIBMOOCOV_COVERAGEMAP_H
