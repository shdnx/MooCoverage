#ifndef MOOCOV_DELAYEDSTRING_H
#define MOOCOV_DELAYEDSTRING_H

#include <functional>
#include <string>
#include <utility>

#include "llvm/ADT/StringRef.h"
#include "llvm/Support/raw_ostream.h"

class DelayedString {
public:
	using action = std::function<void(llvm::StringRef)>;

private:
	std::string m_str;
	llvm::raw_string_ostream m_os;

	action m_action;
	bool m_isActive = true;

public:
	DelayedString() : m_os{m_str}, m_isActive{false} {}

	/*implicit*/ DelayedString(action act)
		: m_os{m_str}, m_action{act} {}

	template<typename TFunc>
	/*implicit*/ DelayedString(TFunc&& act)
		: DelayedString{action{std::forward<TFunc>(act)}} {}

	DelayedString(const DelayedString&) = delete;
	DelayedString(DelayedString&& other) : m_str{std::move((other.m_os.flush(), other.m_str))}, m_os{m_str}, m_action{std::move(other.m_action)} {
		other.m_isActive = false;
	}

	~DelayedString() {
		invokeAction();
	}

	DelayedString& operator=(const DelayedString&) = delete;
	DelayedString& operator=(DelayedString&& rhs) {
		rhs.m_os.flush();
		m_str = std::move(rhs.m_str);
		m_action = std::move(rhs.m_action);
		m_isActive = rhs.m_isActive;

		rhs.m_isActive = false;
		return *this;
	}

	bool hasAction() const { return m_isActive; }

	void invokeAction() {
		if(m_isActive)
			m_action(m_os.str());
	}

	template<typename T>
	DelayedString& operator<<(T&& value) & {
		m_os << std::forward<T>(value);
		return *this;
	}

	template<typename T>
	DelayedString&& operator<<(T&& value) && {
		m_os << std::forward<T>(value);
		return std::move(*this);
	}

	std::string str() { return m_os.str(); }
	std::string str() const { return m_str; }

	operator llvm::StringRef() { return m_os.str(); }
	operator llvm::StringRef() const { return m_str; }
};

#endif // MOOCOV_DELAYEDSTRING_H
