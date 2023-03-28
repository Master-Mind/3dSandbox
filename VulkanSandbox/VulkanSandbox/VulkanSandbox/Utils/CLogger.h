#pragma once
#include <source_location>
#include <vector>
#include <string>
#include <variant>
#include <typeinfo>

#include "PrimativeVal.h"

class CLogPair
{
public:

	template<typename T>
	CLogPair(const char *_key, const T &_value) : key(_key)
	{
		value = _value;
	}

	std::string_view key;
	PrimativeVal value;
};

class CLogger
{
public:
	class Line
	{
	public:
		std::string_view msg;
		std::vector<CLogPair> tags;
		std::source_location loc;

		Line()
		{
			
		}

		Line(const std::string_view _msg, std::vector<CLogPair> &&_tags, std::source_location _loc) : msg(_msg), tags(_tags), loc(_loc)
		{
			
		}

		Line(Line &&rhs) noexcept : msg(rhs.msg), tags(std::move(rhs.tags)), loc(std::move(rhs.loc))
		{
		}

		Line &operator=(Line && rhs) noexcept
		{
			msg = rhs.msg;
			tags = std::move(rhs.tags);
			loc = std::move(rhs.loc);

			return *this;
		}
	};

	static void AddLine(Line &&line);
	static const Line &GetLine(std::size_t idx);
	static const size_t LineBufferLen();
	static const size_t NumLines();
	static const size_t MAX_LINES;
private:
	static std::vector<Line> _lineBuf;
	static int _lineEnd;
	static size_t _numLines;
};

void Log(std::string_view msg, std::vector<CLogPair>&& tags = {}, std::source_location loc = std::source_location::current());
void Error(std::string_view msg, std::vector<CLogPair> &&tags = {}, std::source_location loc = std::source_location::current());
void Assert(bool condition, std::string_view msg, std::vector<CLogPair> &&tags = {}, std::source_location loc = std::source_location::current());