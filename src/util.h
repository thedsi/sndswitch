#pragma once


class WideString // light string wrapper
{
private:
	LPCWSTR str = nullptr;

public:
	WideString() = default;

	WideString(LPCWSTR src)
	{
		Assign(src);
	}

	WideString(const WideString &other)
	{
		Assign(other);
	}

	WideString(WideString&& other)
	{
		str = other.str;
		other.str = nullptr;
	}

	void operator=(const WideString& other)
	{
		Assign(other);
	}

	void operator=(WideString&& other)
	{
		str = other.str;
		other.str = nullptr;
	}

	~WideString()
	{
		Free();
	}

	void Assign(LPCWSTR other)
	{
		Free();
		str = StrDup(other);
	}

	void Free()
	{
		if (str)
		{
			LocalFree((HLOCAL) str);
			str = nullptr;
		}
	}

	bool operator==(LPCWSTR other) const
	{
		return !lstrcmpW(str, other);
	}

	LPCWSTR GetString() const
	{
		return str;
	}

	operator LPCWSTR() const
	{
		return str;
	}

	operator bool() const
	{
		return str != nullptr;
	}
};
