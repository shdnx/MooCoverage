#ifndef LIBMOOCOV_SOURCELOCATION_H
#define LIBMOOCOV_SOURCELOCATION_H

#include <tuple>

namespace libmoocov {

using line_t = std::size_t;
using column_t = std::size_t;

/// \brief Represents a (line, column) file position.
struct SourceLocation {
	line_t line;
	column_t column;

	/*implicit*/ SourceLocation() = default;
	/*implicit*/ SourceLocation(line_t line_, column_t column_)
		: line{line_}, column{column_} {}

	bool isValid() const { return line && column; }
	bool isInvalid() const { return !isValid(); }

	std::pair<line_t, column_t> data() const {
		return { line, column };
	}

	bool operator==(const SourceLocation& rhs) const {
		return line == rhs.line && column == rhs.column;
	}

	bool operator!=(const SourceLocation& rhs) const {
		return !operator==(rhs);
	}

	bool operator<(const SourceLocation& rhs) const {
		return line < rhs.line || (line == rhs.line && column < rhs.column);
	}

	bool operator>(const SourceLocation& rhs) const {
		return line > rhs.line || (line == rhs.line && column > rhs.column);
	}

	bool operator<=(const SourceLocation& rhs) const {
		return operator<(rhs) || operator==(rhs);
	}

	bool operator>=(const SourceLocation& rhs) const {
		return operator>(rhs) || operator==(rhs);
	}
};

/// \brief Represents a range of SourceLocation-s.
///
/// Ordering:
///  - The range that begins earlier comes first.
///  - If they begin at the exact same SourceLocation, then the longer range comes first.
struct SourceRange {
	SourceLocation begin, end;

	/*implicit*/ SourceRange() = default;

	/*implicit*/ SourceRange(SourceLocation begin_, SourceLocation end_)
		: begin{begin_}, end{end_} {}

	bool isValid() const { return begin.isValid() && end.isValid(); }
	bool isInvalid() const { return !isValid(); }

	bool contains(const SourceLocation& loc) const {
		return begin <= loc && loc <= end;
	}

	bool contains(line_t line, column_t column) const {
		return contains({line, column});
	}

	bool contains(const SourceRange& range) const {
		return begin <= range.begin && range.end <= end;
	}

	bool containsLine(line_t line) const {
		return begin.line <= line && line <= end.line;
	}

	bool startsBefore(const SourceLocation& loc) const {
		return begin < loc;
	}

	bool startsBefore(line_t line, column_t column) const {
		return startsBefore({line, column});
	}

	bool startsBeforeLine(line_t line) const {
		return begin.line < line;
	}

	bool endsBefore(const SourceLocation& loc) const {
		return end < loc;
	}

	bool endsBefore(line_t line, column_t column) const {
		return endsBefore({line, column});
	}

	bool endsBeforeLine(line_t line) const {
		return end.line < line;
	}

	bool startsAfter(const SourceLocation& loc) const {
		return loc < end;
	}

	bool startsAfter(line_t line, column_t column) const {
		return startsAfter({line, column});
	}

	bool startsAfterLine(line_t line) const {
		return line < end.line;
	}

	bool endsAfter(const SourceLocation& loc) const {
		return loc < end;
	}

	bool endsAfter(line_t line, column_t column) const {
		return endsAfter({line, column});
	}

	bool endsAfterLine(line_t line) const {
		return line < end.line;
	}

	bool operator==(const SourceRange& rhs) const {
		return begin == rhs.begin && end == rhs.end;
	}

	bool operator!=(const SourceRange& rhs) const {
		return !operator==(rhs);
	}

	bool operator<(const SourceRange& rhs) const {
		return begin < rhs.begin
			|| (begin == rhs.begin && end > rhs.end);
	}

	bool operator<=(const SourceRange& rhs) const {
		return operator<(rhs) || operator==(rhs);
	}

	bool operator>(const SourceRange& rhs) const {
		return rhs < *this;
	}

	bool operator>=(const SourceRange& rhs) const {
		return rhs <= *this;
	}
};

} // end namespace libmoocov

#endif // LIBMOOCOV_SOURCELOCATION_H
