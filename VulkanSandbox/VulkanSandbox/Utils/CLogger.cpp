#include "CLogger.h"
#include <iostream>

using namespace std;
const std::size_t CLogger::MAX_LINES = 128;
std::vector<CLogger::Line> CLogger::_lineBuf(MAX_LINES);
int CLogger::_lineEnd = 0;
size_t CLogger::_numLines = 0;
constexpr bool DIRECT_TO_STDOUT = true;

string FormatForStream(const string_view msg, vector<CLogPair> &tags, source_location &loc)
{
	string ret;

	ret.reserve(128);

	ret = loc.file_name();
	ret += "(";
	ret += to_string(loc.line());
	ret += "): ";
	ret += msg;

	for (int i = 0; i < tags.size(); ++i)
	{
		ret += " (";
		ret += tags[i].key;
		ret += ", ";
		ret += tags[i].value.To_String();
		ret += ")";
	}

	return ret;
}

void Log(string_view msg, vector<CLogPair> &&tags, source_location loc)
{
	if constexpr (DIRECT_TO_STDOUT)
	{
		const string line(move(FormatForStream(msg, tags, loc)));

		cout << line << std::endl;
	}

	CLogger::AddLine(CLogger::Line{ msg, move(tags), loc });
}

void Error(std::string_view msg, std::vector<CLogPair> &&tags, std::source_location loc)
{
	if constexpr (DIRECT_TO_STDOUT)
	{
		const string line(move(FormatForStream(msg, tags, loc)));

		cerr << "ERROR " << line << std::endl;
	}

	CLogger::AddLine(CLogger::Line{ msg, move(tags), loc });
}

void Assert(bool condition, std::string_view msg, std::vector<CLogPair> &&tags, std::source_location loc)
{
	if (!condition)
	{
		if constexpr (DIRECT_TO_STDOUT)
		{
			const string line(move(FormatForStream(msg, tags, loc)));

			cerr << "ASSERTION FAILED " << line << std::endl;
		}

		CLogger::AddLine(CLogger::Line{ msg, move(tags), loc });

		abort();
	}
}

void CLogger::AddLine(Line&& line)
{
	_lineBuf[_lineEnd] = move(line);

	_lineEnd = (_lineEnd + 1) % _lineBuf.size();

	_numLines++;
}

const CLogger::Line& CLogger::GetLine(std::size_t idx)
{
	Assert(idx < MAX_LINES, "idx was past the end of the line array");

	size_t offset = _numLines > MAX_LINES ? _lineEnd : 0;

	return _lineBuf[(offset + idx) % _lineBuf.size()];
}

const size_t CLogger::LineBufferLen()
{
	return min(_numLines, MAX_LINES);
}

const size_t CLogger::NumLines()
{
	return _numLines;
}
