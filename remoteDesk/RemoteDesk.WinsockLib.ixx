#include "pch.h"
#include <stdexcept>
#include <system_error>

export module RemoteDesk.WinsockLib;

namespace RemoteDesk
{
	export class WinsockLib final
	{
	public:
		explicit WinsockLib()
		{
			const int result = WSAStartup(WINSOCK_VERSION, &mWSAData);
			if (result != 0)
			{
				throw std::system_error(result, std::system_category());
			}
			didInit = true;
		}

		~WinsockLib() noexcept
		{
			if (didInit)
			{
				std::ignore = WSACleanup();
			}
		}
		
		// prevent copy
		WinsockLib(WinsockLib const&) = delete;
		WinsockLib(WinsockLib&) = delete;
		WinsockLib& operator=(WinsockLib const&) = delete;
		WinsockLib& operator=(WinsockLib&) = delete;

		//allow move
		WinsockLib(WinsockLib&& rhs) noexcept
		{
			didInit=std::exchange(rhs.didInit, false);
			std::swap(mWSAData, rhs.mWSAData);
		}

		WinsockLib& operator=(WinsockLib&& rhs) noexcept
		{
			didInit = std::exchange(rhs.didInit, false);
			std::swap(mWSAData, rhs.mWSAData);
			return *this;
		}

		const WSADATA& GetWSAData() const noexcept
		{
			return this->mWSAData;
		}
	private:
		bool didInit = false;
		WSADATA mWSAData{};
	};
}