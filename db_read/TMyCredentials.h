#pragma once
#include <tuple>
#include <string>
#include <tchar.h>
#include <format>

template<typename String=std::string,typename StringView=std::string_view>
class TMyCredentials {

private:
	std::tuple<String, String> data;
public:

	constexpr TMyCredentials(StringView const& usr, StringView const& pwd) 
		: data{ usr,pwd } 
	{};
	constexpr TMyCredentials()
		: TMyCredentials(_T(""),_T(""))
	{}

	TMyCredentials(TMyCredentials const& ref)
		: data{ref.data }
	{}
	TMyCredentials(TMyCredentials&& ref) noexcept
	{	
		swap(ref);	
	}

	virtual ~TMyCredentials() = default;

	TMyCredentials& operator=(TMyCredentials const& ref)
	{
		data = ref.data;
		return *this;
	}

	TMyCredentials& operator=(TMyCredentials&& ref) noexcept
	{
		swap(ref);
		return *this;
	}

#ifdef __cpp_impl_three_way_comparison
	auto operator<=>(TMyCredentials const& ref) const = default;
#endif

	void swap(TMyCredentials& ref) noexcept { std::swap(data, ref.data); }

	constexpr String const& User(void) const { return std::get<0>(data); }
	constexpr String const& Pwd(void) const { return std::get<1>(data); }

	template<bool bSecure=false>
	constexpr String GetCredential() const
	{
		if constexpr (bSecure) {
			return std::format(_T("User: {} Pwd: {}"), User(), Pwd() ); }
		else {
			return std::format(_T("User: {} Pwd: ***"), User());
		}
	}
};