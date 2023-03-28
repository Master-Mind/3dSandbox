#pragma once
#include <string>
#include <variant>
#include <compare>

class PrimativeVal
{
public:
	PrimativeVal()
	{
		
	}

	PrimativeVal(const char *str) : _value(str)
	{

	}

	template<typename T>
	PrimativeVal(T val) : _value(val)
	{

	}

	auto operator<=>(const PrimativeVal& rhs) const
	{
		return _value <=> rhs._value;
	}

	//I don't know why but I can't use the spaceship operator for equality
	bool operator==(const PrimativeVal& rhs) const
	{
		return _value == rhs._value;
	}

	//compare to char pointer (for ease of use in the debug window)
	auto operator<=>(const char *rhs) const
	{
		std::partial_ordering ret;
		//-1 for invalid, 0 for strong, 1 for partial
		bool isValid = true;

		std::visit([&ret, &isValid, rhs](auto& arg)
			{
				using T = std::decay_t<decltype(arg)>;
				char* end;
				if constexpr (std::is_same_v<T, std::string>)
				{
					ret = arg <=> rhs;
				}
				else if constexpr (std::is_same_v<T, bool>)
				{
					if (rhs && (rhs[0] == '1' || (rhs[0] == 't') || (rhs[0] == 'T')))
					{
						ret = arg <=> true;
					}
					else
					{
						ret = arg <=> false;
					}
				}
				else if constexpr (std::is_unsigned_v<T>)
				{
					ret = arg <=> strtoull(rhs, &end, 10);
					isValid = !(*end);
				}
				else if constexpr (std::is_integral_v<T>)
				{
					ret = arg <=> strtoll(rhs, &end, 10);
					isValid = !(*end);
				}
				else if constexpr (std::is_arithmetic_v<T>)
				{
					ret = arg <=> strtold(rhs, &end);
					isValid = !(*end);
				}
				else
				{
					abort();
				}
			}, _value);

		if (isValid)
		{
			return ret;
		}
		else
		{
			//return unordered, basically always return false
			return NAN <=> NAN;
		}
	}

	//I don't know why but I can't use the spaceship operator for equality
	bool operator==(const char* rhs) const
	{
		bool ret;

		std::visit([&ret, rhs](auto& arg)
			{
				using T = std::decay_t<decltype(arg)>;
				char* end;
				if constexpr (std::is_same_v<T, std::string>)
				{
					ret = arg == rhs;
				}
				else if constexpr (std::is_same_v<T, bool>)
				{
					ret = rhs && (rhs[0] == '1' || (rhs[0] == 't') || (rhs[0] == 'T'));
				}
				else if constexpr (std::is_unsigned_v<T>)
				{
					ret = arg == strtoull(rhs, &end, 10);
					ret &= !(*end);
				}
				else if constexpr (std::is_integral_v<T>)
				{
					ret = arg == strtoll(rhs, &end, 10);
					ret &= !(*end);
				}
				else if constexpr (std::is_arithmetic_v<T>)
				{
					ret = arg == strtold(rhs, &end);
					ret &= !(*end);
				}
				else
				{
					abort();
				}
			}, _value);

		return ret;
	}

	std::string To_String() const
	{
		std::string ret;

		std::visit([&ret](auto& arg)
			{
				using T = std::decay_t<decltype(arg)>;

				if constexpr (std::is_same_v<T, std::string>)
				{
					ret = arg;
				}
				else if constexpr (std::is_arithmetic_v<T>)
				{
					ret = std::to_string(arg);
				}
				else
				{
					abort();
				}
			}, _value);

		return ret;
	}

	operator bool() const
	{
		bool ret = false;

		std::visit([&ret](auto& arg)
			{
				using T = std::decay_t<decltype(arg)>;

				if constexpr (std::is_same_v<T, std::string>)
				{
					ret = !arg.empty();
				}
				else if constexpr (std::is_integral_v<T>)
				{
					ret = arg;
				}
				else if constexpr (std::is_arithmetic_v<T>)
				{
					ret = arg < 0 ? arg > -0.01 : arg < 0.01;
				}
				else
				{
					abort();
				}
			}, _value);

		return ret;
	}

private:

	std::variant<std::string,
		bool,
		char,
		unsigned char,
		wchar_t,
		short,
		unsigned short,
		int,
		unsigned,
		float,
		double,
		long,
		unsigned long,
		long long,
		unsigned long long,
		long double> _value;
};

