#ifndef INCLUDE_GIOVANNI_DICANIO_WINREG_HPP
#define INCLUDE_GIOVANNI_DICANIO_WINREG_HPP


////////////////////////////////////////////////////////////////////////////////
//
//      *** Modern C++ Wrappers Around Windows Registry C API ***
// 
//               Copyright (C) by Giovanni Dicanio 
//  
// First version: 2017, January 22nd
// Last update: 2017, August 9th
// 
// E-mail: <giovanni.dicanio AT gmail.com>
// 
// Registry key handles are safely and conveniently wrapped 
// in the RegKey resource manager C++ class.
// 
// Errors are signaled throwing exceptions of class RegException.
// 
// Unicode UTF-16 strings are represented using the tstring class; 
// ATL's CString is not used, to avoid dependencies from ATL or MFC.
// 
// This is a header-only self-contained reusable module.
//
// Compiler: Visual Studio 2015
// Code compiles clean at /W4 on both 32-bit and 64-bit builds.
// 
// ===========================================================================
//
// The MIT License(MIT)
//
// Copyright(c) 2017 Giovanni Dicanio
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files(the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
////////////////////////////////////////////////////////////////////////////////


#include <Windows.h>        // Windows Platform SDK
#include <crtdbg.h>         // _ASSERTE
#include <tchar.h>			// TCHAR

#include <memory>           // std::unique_ptr
#include <stdexcept>        // std::runtime_error
#include <string>           // std::wstring, std::string
#include <utility>          // std::swap, std::pair
#include <vector>           // std::vector


namespace winreg
{
#ifdef UNICODE
	using tstring = std::wstring;
	const auto tstrlen = wcslen;
#else
	using tstring = std::string;
	const auto tstrlen = strlen;
#endif // UNICODE


	//------------------------------------------------------------------------------
	// Safe, efficient and convenient C++ wrapper around HKEY registry key handles.
	//
	// This class is movable but not copyable.
	//
	// This class is designed to be very *efficient* and low-overhead, for example: 
	// non-throwing operations are carefully marked as noexcept, so the C++ compiler 
	// can emit optimized code.
	// 
	// Moreover, this class just wraps a raw HKEY handle, without any 
	// shared-ownership overhead like in std::shared_ptr; you can think of this 
	// class kind of like a std::unique_ptr for HKEYs.
	//
	// The class is also swappable (defines a custom non-member swap); 
	// relational operators are properly overloaded as well.
	//------------------------------------------------------------------------------
	class RegKey
	{
	public:

		//
		// Construction/Destruction
		// 

		// Initialize as an empty key handle
		RegKey() noexcept = default;

		// Take ownership of the input key handle
		explicit RegKey(HKEY hKey) noexcept;

		// Open the given registry key if it exists, else create a new key.
		// Uses default KEY_READ|KEY_WRITE access.
		// For finer grained control, call the Create() method overloads.
		// Throw RegException on failure.
		RegKey(HKEY hKeyParent, const tstring& subKey);

		// Open the given registry key if it exists, else create a new key.
		// Allow the caller to specify the desired access to the key (e.g. KEY_READ
		// for read-only access).
		// For finer grained control, call the Create() method overloads.
		// Throw RegException on failure.
		RegKey(HKEY hKeyParent, const tstring& subKey, REGSAM desiredAccess);


		// Take ownership of the input key handle.
		// The input key handle wrapper is reset to an empty state.
		RegKey(RegKey&& other) noexcept;

		// Move-assign from the input key handle.
		// Properly check against self-move-assign (which is safe and does nothing).
		RegKey& operator=(RegKey&& other) noexcept;

		// Ban copy
		RegKey(const RegKey&) = delete;
		RegKey& operator=(const RegKey&) = delete;

		// Safely close the wrapped key handle (if any)
		~RegKey() noexcept;


		//
		// Properties
		// 

		// Access the wrapped raw HKEY handle
		HKEY Get() const noexcept;

		// Is the wrapped HKEY handle valid?
		bool IsValid() const noexcept;

		// Same as IsValid(), but allow a short "if (regKey)" syntax
		explicit operator bool() const noexcept;

		// Is the wrapped handle a predefined handle (e.g.HKEY_CURRENT_USER) ?
		bool IsPredefined() const noexcept;


		//
		// Operations
		// 

		// Close current HKEY handle.
		// If there's no valid handle, do nothing.
		// This method doesn't close predefined HKEY handles (e.g. HKEY_CURRENT_USER).
		void Close() noexcept;

		// Transfer ownership of current HKEY to the caller.
		// Note that the caller is responsible for closing the key handle!
		HKEY Detach() noexcept;

		// Take ownership of the input HKEY handle.
		// Safely close any previously open handle.
		// Input key handle can be nullptr.
		void Attach(HKEY hKey) noexcept;

		// Non-throwing swap;
		// Note: There's also a non-member swap overload
		void SwapWith(RegKey& other) noexcept;


		//
		// Wrappers around Windows Registry APIs.
		// See the official MSDN documentation for these APIs for detailed explanations
		// of the wrapper method parameters.
		// 

		// Wrapper around RegCreateKeyEx, that allows you to specify desired access
		void Create(
			HKEY hKeyParent,
			const tstring& subKey,
			REGSAM desiredAccess = KEY_READ | KEY_WRITE
		);

		// Wrapper around RegCreateKeyEx
		void Create(
			HKEY hKeyParent,
			const tstring& subKey,
			REGSAM desiredAccess,
			DWORD options,
			SECURITY_ATTRIBUTES* securityAttributes,
			DWORD* disposition
		);

		// Wrapper around RegOpenKeyEx
		void Open(
			HKEY hKeyParent,
			const tstring& subKey,
			REGSAM desiredAccess = KEY_READ | KEY_WRITE
		);


		//
		// Registry Value Setters
		// 

		void SetDwordValue(const tstring& valueName, DWORD data);
		void SetQwordValue(const tstring& valueName, const ULONGLONG& data);
		void SetStringValue(const tstring& valueName, const tstring& data);
		void SetExpandStringValue(const tstring& valueName, const tstring& data);
		void SetMultiStringValue(const tstring& valueName, const std::vector<tstring>& data);
		void SetBinaryValue(const tstring& valueName, const std::vector<BYTE>& data);
		void SetBinaryValue(const tstring& valueName, const void* data, DWORD dataSize);


		//
		// Registry Value Getters
		// 

		DWORD GetDwordValue(const tstring& valueName);
		ULONGLONG GetQwordValue(const tstring& valueName);
		tstring GetStringValue(const tstring& valueName);
		bool FindStringValue(const tstring& valueName);
		bool FindDwordValue(const tstring& valueName);

		enum class ExpandStringOption
		{
			DontExpand,
			Expand
		};

		tstring GetExpandStringValue(
			const tstring& valueName,
			ExpandStringOption expandOption = ExpandStringOption::DontExpand
		);

		std::vector<tstring> GetMultiStringValue(const tstring& valueName);
		std::vector<BYTE> GetBinaryValue(const tstring& valueName);


		//
		// Query Operations
		// 

		void QueryInfoKey(DWORD& subKeys, DWORD& values, FILETIME& lastWriteTime);

		// Return the DWORD type ID for the input registry value
		DWORD QueryValueType(const tstring& valueName);

		// Enumerate the subkeys of the registry key, using RegEnumKeyEx
		std::vector<tstring> EnumSubKeys();

		// Enumerate the values under the registry key, using RegEnumValue.
		// Returns a vector of pairs: In each pair, the wstring is the value name, 
		// the DWORD is the value type.
		std::vector<std::pair<tstring, DWORD>> EnumValues();


		//
		// Misc Registry API Wrappers
		// 

		void DeleteValue(const tstring& valueName);
		void DeleteKey(const tstring& subKey, REGSAM desiredAccess);
		void FlushKey();
		void LoadKey(const tstring& subKey, const tstring& filename);
		void SaveKey(const tstring& filename, SECURITY_ATTRIBUTES* securityAttributes);
		void EnableReflectionKey();
		void DisableReflectionKey();
		bool QueryReflectionKey();
		void ConnectRegistry(const tstring& machineName, HKEY hKeyPredefined);


		// Return a string representation of Windows registry types
		static tstring RegTypeToString(DWORD regType);

		//
		// Relational comparison operators are overloaded as non-members
		// ==, !=, <, <=, >, >=
		// 


	private:
		// The wrapped registry key handle
		HKEY m_hKey{ nullptr };
	};


	//------------------------------------------------------------------------------
	// An exception representing an error with the registry operations
	//------------------------------------------------------------------------------
	class RegException
		: public std::runtime_error
	{
	public:
		RegException(const char* message, LONG errorCode)
			: std::runtime_error{ message }
			, m_errorCode{ errorCode }
		{}

		RegException(const std::string& message, LONG errorCode)
			: std::runtime_error{ message }
			, m_errorCode{ errorCode }
		{}

		// Get the error code returned by Windows registry APIs
		LONG ErrorCode() const noexcept
		{
			return m_errorCode;
		}

	private:
		// Error code, as returned by Windows registry APIs
		LONG m_errorCode;
	};


	//------------------------------------------------------------------------------
	//          Overloads of relational comparison operators for RegKey
	//------------------------------------------------------------------------------

	inline bool operator==(const RegKey& a, const RegKey& b) noexcept
	{
		return a.Get() == b.Get();
	}

	inline bool operator!=(const RegKey& a, const RegKey& b) noexcept
	{
		return a.Get() != b.Get();
	}

	inline bool operator<(const RegKey& a, const RegKey& b) noexcept
	{
		return a.Get() < b.Get();
	}

	inline bool operator<=(const RegKey& a, const RegKey& b) noexcept
	{
		return a.Get() <= b.Get();
	}

	inline bool operator>(const RegKey& a, const RegKey& b) noexcept
	{
		return a.Get() > b.Get();
	}

	inline bool operator>=(const RegKey& a, const RegKey& b) noexcept
	{
		return a.Get() >= b.Get();
	}


	//------------------------------------------------------------------------------
	//                          RegKey Inline Methods
	//------------------------------------------------------------------------------

	inline RegKey::RegKey(const HKEY hKey) noexcept
		: m_hKey{ hKey }
	{}


	inline RegKey::RegKey(const HKEY hKeyParent, const tstring& subKey)
	{
		Create(hKeyParent, subKey);
	}


	inline RegKey::RegKey(const HKEY hKeyParent, const tstring& subKey, REGSAM desiredAccess)
	{
		Create(hKeyParent, subKey, desiredAccess);
	}


	inline RegKey::RegKey(RegKey&& other) noexcept
		: m_hKey{ other.m_hKey }
	{
		// Other doesn't own the handle anymore
		other.m_hKey = nullptr;
	}


	inline RegKey& RegKey::operator=(RegKey&& other) noexcept
	{
		// Prevent self-move-assign
		if ((this != &other) && (m_hKey != other.m_hKey))
		{
			// Close current
			Close();

			// Move from other (i.e. take ownership of other's raw handle)
			m_hKey = other.m_hKey;
			other.m_hKey = nullptr;
		}
		return *this;
	}


	inline RegKey::~RegKey() noexcept
	{
		// Release the owned handle (if any)
		Close();
	}


	inline HKEY RegKey::Get() const noexcept
	{
		return m_hKey;
	}


	inline void RegKey::Close() noexcept
	{
		if (IsValid())
		{
			// Do not call RegCloseKey on predefined keys
			if (!IsPredefined())
			{
				::RegCloseKey(m_hKey);
			}

			// Avoid dangling references
			m_hKey = nullptr;
		}
	}


	inline bool RegKey::IsValid() const noexcept
	{
		return m_hKey != nullptr;
	}


	inline RegKey::operator bool() const noexcept
	{
		return IsValid();
	}


	inline bool RegKey::IsPredefined() const noexcept
	{
		// Predefined keys
		// https://msdn.microsoft.com/en-us/library/windows/desktop/ms724836(v=vs.85).aspx

		if ((m_hKey == HKEY_CURRENT_USER)
			|| (m_hKey == HKEY_LOCAL_MACHINE)
			|| (m_hKey == HKEY_CLASSES_ROOT)
			|| (m_hKey == HKEY_CURRENT_CONFIG)
			|| (m_hKey == HKEY_CURRENT_USER_LOCAL_SETTINGS)
			|| (m_hKey == HKEY_PERFORMANCE_DATA)
			|| (m_hKey == HKEY_PERFORMANCE_NLSTEXT)
			|| (m_hKey == HKEY_PERFORMANCE_TEXT)
			|| (m_hKey == HKEY_USERS))
		{
			return true;
		}

		return false;
	}


	inline HKEY RegKey::Detach() noexcept
	{
		HKEY hKey{ m_hKey };

		// We don't own the HKEY handle anymore
		m_hKey = nullptr;

		// Transfer ownership to the caller
		return hKey;
	}


	inline void RegKey::Attach(const HKEY hKey) noexcept
	{
		// Prevent self-attach
		if (m_hKey != hKey)
		{
			// Close any open registry handle
			Close();

			// Take ownership of the input hKey
			m_hKey = hKey;
		}
	}


	inline void RegKey::SwapWith(RegKey& other) noexcept
	{
		// Enable ADL (not necessary in this case, but good practice)
		using std::swap;

		// Swap the raw handle members
		swap(m_hKey, other.m_hKey);
	}


	inline void swap(RegKey& a, RegKey& b) noexcept
	{
		a.SwapWith(b);
	}


	inline void RegKey::Create(
		const HKEY                  hKeyParent,
		const tstring& subKey,
		const REGSAM                desiredAccess
	)
	{
		constexpr DWORD kDefaultOptions = REG_OPTION_NON_VOLATILE;

		Create(hKeyParent, subKey, desiredAccess, kDefaultOptions,
			nullptr, // no security attributes,
			nullptr  // no disposition 
		);
	}


	inline void RegKey::Create(
		const HKEY                  hKeyParent,
		const tstring& subKey,
		const REGSAM                desiredAccess,
		const DWORD                 options,
		SECURITY_ATTRIBUTES* const  securityAttributes,
		DWORD* const                disposition
	)
	{
		HKEY hKey{ nullptr };
		LONG retCode = ::RegCreateKeyEx(
			hKeyParent,
			subKey.c_str(),
			0,          // reserved
			REG_NONE,   // user-defined class type parameter not supported
			options,
			desiredAccess,
			securityAttributes,
			&hKey,
			disposition
		);
		if (retCode != ERROR_SUCCESS)
		{
			throw RegException{ xorstr_("RegCreateKeyEx failed."), retCode };
		}

		// Safely close any previously opened key
		Close();

		// Take ownership of the newly created key
		m_hKey = hKey;
	}


	inline void RegKey::Open(
		const HKEY              hKeyParent,
		const tstring& subKey,
		const REGSAM            desiredAccess
	)
	{
		HKEY hKey{ nullptr };
		LONG retCode = ::RegOpenKeyEx(
			hKeyParent,
			subKey.c_str(),
			REG_NONE,           // default options
			desiredAccess,
			&hKey
		);
		if (retCode != ERROR_SUCCESS)
		{
			throw RegException{ xorstr_("RegOpenKeyEx failed."), retCode };
		}

		// Safely close any previously opened key
		Close();

		// Take ownership of the newly created key
		m_hKey = hKey;
	}


	inline void RegKey::SetDwordValue(const tstring& valueName, const DWORD data)
	{
		_ASSERTE(IsValid());

		LONG retCode = ::RegSetValueEx(
			m_hKey,
			valueName.c_str(),
			0, // reserved
			REG_DWORD,
			reinterpret_cast<const BYTE*>(&data),
			sizeof(data)
		);
		if (retCode != ERROR_SUCCESS)
		{
			throw RegException{ xorstr_("Cannot write DWORD value: RegSetValueEx failed."), retCode };
		}
	}


	inline void RegKey::SetQwordValue(const tstring& valueName, const ULONGLONG& data)
	{
		_ASSERTE(IsValid());

		LONG retCode = ::RegSetValueEx(
			m_hKey,
			valueName.c_str(),
			0, // reserved
			REG_QWORD,
			reinterpret_cast<const BYTE*>(&data),
			sizeof(data)
		);
		if (retCode != ERROR_SUCCESS)
		{
			throw RegException{ xorstr_("Cannot write QWORD value: RegSetValueEx failed."), retCode };
		}
	}


	inline void RegKey::SetStringValue(const tstring& valueName, const tstring& data)
	{
		_ASSERTE(IsValid());

		// String size including the terminating NUL, in bytes
		const DWORD dataSize = static_cast<DWORD>((data.length() + 1) * sizeof(TCHAR));

		LONG retCode = ::RegSetValueEx(
			m_hKey,
			valueName.c_str(),
			0, // reserved
			REG_SZ,
			reinterpret_cast<const BYTE*>(data.c_str()),
			dataSize
		);
		if (retCode != ERROR_SUCCESS)
		{
			throw RegException{ xorstr_("Cannot write string value: RegSetValueEx failed."), retCode };
		}
	}


	inline void RegKey::SetExpandStringValue(const tstring& valueName, const tstring& data)
	{
		_ASSERTE(IsValid());

		// String size including the terminating NUL, in bytes
		const DWORD dataSize = static_cast<DWORD>((data.length() + 1) * sizeof(TCHAR));

		LONG retCode = ::RegSetValueEx(
			m_hKey,
			valueName.c_str(),
			0, // reserved
			REG_EXPAND_SZ,
			reinterpret_cast<const BYTE*>(data.c_str()),
			dataSize
		);
		if (retCode != ERROR_SUCCESS)
		{
			throw RegException{ xorstr_("Cannot write expand string value: RegSetValueEx failed."), retCode };
		}
	}


	namespace details
	{

		// Helper function to build a multi-string from a vector<wstring>
		inline std::vector<TCHAR> BuildMultiString(const std::vector<tstring>& data)
		{
			// Special case of the empty multi-string
			if (data.empty())
			{
				// Build a vector containing just two NULs
				return std::vector<TCHAR>(2, L'\0');
			}

			// Get the total length in TCHARs of the multi-string
			size_t totalLen = 0;
			for (const auto& s : data)
			{
				// Add one to current string's length for the terminating NUL
				totalLen += (s.length() + 1);
			}

			// Add one for the last NUL terminator (making the whole structure double-NUL terminated)
			totalLen++;

			// Allocate a buffer to store the multi-string
			std::vector<TCHAR> multiString;
			multiString.reserve(totalLen);

			// Copy the single strings into the multi-string
			for (const auto& s : data)
			{
				multiString.insert(multiString.end(), s.begin(), s.end());

				// Don't forget to NUL-terminate the current string
				multiString.push_back(_T('\0'));
			}

			// Add the last NUL-terminator
			multiString.push_back(_T('\0'));

			return multiString;
		}

	} // namespace details


	inline void RegKey::SetMultiStringValue(
		const tstring& valueName,
		const std::vector<tstring>& data
	)
	{
		_ASSERTE(IsValid());

		// First, we have to build a double-NUL-terminated multi-string from the input data
		const std::vector<TCHAR> multiString = details::BuildMultiString(data);

		// Total size, in bytes, of the whole multi-string structure
		const DWORD dataSize = static_cast<DWORD>(multiString.size() * sizeof(TCHAR));

		LONG retCode = ::RegSetValueEx(
			m_hKey,
			valueName.c_str(),
			0, // reserved
			REG_MULTI_SZ,
			reinterpret_cast<const BYTE*>(&multiString[0]),
			dataSize
		);
		if (retCode != ERROR_SUCCESS)
		{
			throw RegException{ xorstr_("Cannot write multi-string value: RegSetValueEx failed."), retCode };
		}
	}


	inline void RegKey::SetBinaryValue(const tstring& valueName, const std::vector<BYTE>& data)
	{
		_ASSERTE(IsValid());

		// Total data size, in bytes
		const DWORD dataSize = static_cast<DWORD>(data.size());

		LONG retCode = ::RegSetValueEx(
			m_hKey,
			valueName.c_str(),
			0, // reserved
			REG_BINARY,
			&data[0],
			dataSize
		);
		if (retCode != ERROR_SUCCESS)
		{
			throw RegException{ xorstr_("Cannot write binary data value: RegSetValueEx failed."), retCode };
		}
	}


	inline void RegKey::SetBinaryValue(
		const tstring& valueName,
		const void* const data,
		const DWORD dataSize
	)
	{
		_ASSERTE(IsValid());

		LONG retCode = ::RegSetValueEx(
			m_hKey,
			valueName.c_str(),
			0, // reserved
			REG_BINARY,
			static_cast<const BYTE*>(data),
			dataSize
		);
		if (retCode != ERROR_SUCCESS)
		{
			throw RegException{ xorstr_("Cannot write binary data value: RegSetValueEx failed."), retCode };
		}
	}


	inline DWORD RegKey::GetDwordValue(const tstring& valueName)
	{
		_ASSERTE(IsValid());

		DWORD data{};                   // to be read from the registry
		DWORD dataSize = sizeof(data);  // size of data, in bytes

		const DWORD flags = RRF_RT_REG_DWORD;
		LONG retCode = ::RegGetValue(
			m_hKey,
			nullptr, // no subkey
			valueName.c_str(),
			flags,
			nullptr, // type not required
			&data,
			&dataSize
		);
		if (retCode != ERROR_SUCCESS)
		{
			throw RegException{ xorstr_("Cannot get DWORD value: RegGetValue failed."), retCode };
		}

		return data;
	}


	inline ULONGLONG RegKey::GetQwordValue(const tstring& valueName)
	{
		_ASSERTE(IsValid());

		ULONGLONG data{};               // to be read from the registry
		DWORD dataSize = sizeof(data);  // size of data, in bytes

		const DWORD flags = RRF_RT_REG_QWORD;
		LONG retCode = ::RegGetValue(
			m_hKey,
			nullptr, // no subkey
			valueName.c_str(),
			flags,
			nullptr, // type not required
			&data,
			&dataSize
		);
		if (retCode != ERROR_SUCCESS)
		{
			throw RegException{ xorstr_("Cannot get QWORD value: RegGetValue failed."), retCode };
		}

		return data;
	}

	inline bool RegKey::FindStringValue(const tstring& valueName)
	{
		_ASSERTE(IsValid());

		// Get the size of the result string
		DWORD dataSize = 0; // size of data, in bytes
		const DWORD flags = RRF_RT_REG_SZ;
		LONG retCode = ::RegGetValue(
			m_hKey,
			nullptr, // no subkey
			valueName.c_str(),
			flags,
			nullptr, // type not required
			nullptr, // output buffer not needed now
			&dataSize
		);
		return retCode == ERROR_SUCCESS;
	}

	inline bool RegKey::FindDwordValue(const tstring& valueName)
	{
		_ASSERTE(IsValid());

		DWORD data{};                   // to be read from the registry
		DWORD dataSize = sizeof(data);  // size of data, in bytes

		const DWORD flags = RRF_RT_REG_DWORD;
		LONG retCode = ::RegGetValue(
			m_hKey,
			nullptr, // no subkey
			valueName.c_str(),
			flags,
			nullptr, // type not required
			&data,
			&dataSize
		);
		return retCode == ERROR_SUCCESS;
	}


	inline tstring RegKey::GetStringValue(const tstring& valueName)
	{
		_ASSERTE(IsValid());

		// Get the size of the result string
		DWORD dataSize = 0; // size of data, in bytes
		const DWORD flags = RRF_RT_REG_SZ;
		LONG retCode = ::RegGetValue(
			m_hKey,
			nullptr, // no subkey
			valueName.c_str(),
			flags,
			nullptr, // type not required
			nullptr, // output buffer not needed now
			&dataSize
		);
		if (retCode != ERROR_SUCCESS)
		{
			throw RegException{ xorstr_("Cannot get size of string value: RegGetValue failed."), retCode };
		}

		// Allocate a string of proper size.
		// Note that dataSize is in bytes and includes the terminating NUL;
		// we have to convert the size from bytes to TCHARs for wstring::resize.
		tstring result;
		result.resize(dataSize / sizeof(TCHAR));

		// Call RegGetValue for the second time to read the string's content
		retCode = ::RegGetValue(
			m_hKey,
			nullptr,    // no subkey
			valueName.c_str(),
			flags,
			nullptr,    // type not required
			&result[0], // output buffer
			&dataSize
		);
		if (retCode != ERROR_SUCCESS)
		{
			throw RegException{ xorstr_("Cannot get string value: RegGetValue failed."), retCode };
		}

		// Remove the NUL terminator scribbled by RegGetValue from the wstring
		result.resize((dataSize / sizeof(TCHAR)) - 1);

		return result;
	}


	inline tstring RegKey::GetExpandStringValue(
		const tstring& valueName,
		const ExpandStringOption expandOption
	)
	{
		_ASSERTE(IsValid());

		DWORD flags = RRF_RT_REG_EXPAND_SZ;

		// Adjust the flag for RegGetValue considering the expand string option specified by the caller   
		if (expandOption == ExpandStringOption::DontExpand)
		{
			flags |= RRF_NOEXPAND;
		}

		// Get the size of the result string
		DWORD dataSize = 0; // size of data, in bytes
		LONG retCode = ::RegGetValue(
			m_hKey,
			nullptr,    // no subkey
			valueName.c_str(),
			flags,
			nullptr,    // type not required
			nullptr,    // output buffer not needed now
			&dataSize
		);
		if (retCode != ERROR_SUCCESS)
		{
			throw RegException{ xorstr_("Cannot get size of expand string value: RegGetValue failed."), retCode };
		}

		// Allocate a string of proper size.
		// Note that dataSize is in bytes and includes the terminating NUL.
		// We must convert from bytes to TCHARs for wstring::resize.
		tstring result;
		result.resize(dataSize / sizeof(TCHAR));

		// Call RegGetValue for the second time to read the string's content
		retCode = ::RegGetValue(
			m_hKey,
			nullptr,    // no subkey
			valueName.c_str(),
			flags,
			nullptr,    // type not required
			&result[0], // output buffer
			&dataSize
		);
		if (retCode != ERROR_SUCCESS)
		{
			throw RegException{ xorstr_("Cannot get expand string value: RegGetValue failed."), retCode };
		}

		// Remove the NUL terminator scribbled by RegGetValue from the wstring
		result.resize((dataSize / sizeof(TCHAR)) - 1);

		return result;
	}


	inline std::vector<tstring> RegKey::GetMultiStringValue(const tstring& valueName)
	{
		_ASSERTE(IsValid());

		// Request the size of the multi-string, in bytes
		DWORD dataSize = 0;
		const DWORD flags = RRF_RT_REG_MULTI_SZ;
		LONG retCode = ::RegGetValue(
			m_hKey,
			nullptr,    // no subkey
			valueName.c_str(),
			flags,
			nullptr,    // type not required
			nullptr,    // output buffer not needed now
			&dataSize
		);
		if (retCode != ERROR_SUCCESS)
		{
			throw RegException{ xorstr_("Cannot get size of multi-string value: RegGetValue failed."), retCode };
		}

		// Allocate room for the result multi-string.
		// Note that dataSize is in bytes, but our vector<TCHAR>::resize method requires size 
		// to be expressed in TCHARs.
		std::vector<TCHAR> data;
		data.resize(dataSize / sizeof(TCHAR));

		// Read the multi-string from the registry into the vector object
		retCode = ::RegGetValue(
			m_hKey,
			nullptr,    // no subkey
			valueName.c_str(),
			flags,
			nullptr,    // no type required
			&data[0],   // output buffer
			&dataSize
		);
		if (retCode != ERROR_SUCCESS)
		{
			throw RegException{ xorstr_("Cannot get multi-string value: RegGetValue failed."), retCode };
		}

		// Resize vector to the actual size returned by GetRegValue.
		// Note that the vector is a vector of TCHARs, instead the size returned by GetRegValue
		// is in bytes, so we have to scale from bytes to TCHAR count.
		data.resize(dataSize / sizeof(TCHAR));

		// Parse the double-NUL-terminated string into a vector<wstring>, 
		// which will be returned to the caller
		std::vector<tstring> result;
		const TCHAR* currStringPtr = &data[0];
		while (*currStringPtr != _T('\0'))
		{
			// Current string is NUL-terminated, so get its length calling wcslen
			const size_t currStringLength = tstrlen(currStringPtr);

			// Add current string to the result vector
			result.push_back(tstring{ currStringPtr, currStringLength });

			// Move to the next string
			currStringPtr += currStringLength + 1;
		}

		return result;
	}


	inline std::vector<BYTE> RegKey::GetBinaryValue(const tstring& valueName)
	{
		_ASSERTE(IsValid());

		// Get the size of the binary data
		DWORD dataSize = 0; // size of data, in bytes
		const DWORD flags = RRF_RT_REG_BINARY;
		LONG retCode = ::RegGetValue(
			m_hKey,
			nullptr,    // no subkey
			valueName.c_str(),
			flags,
			nullptr,    // type not required
			nullptr,    // output buffer not needed now
			&dataSize
		);
		if (retCode != ERROR_SUCCESS)
		{
			throw RegException{ xorstr_("Cannot get size of binary data: RegGetValue failed."), retCode };
		}

		// Allocate a buffer of proper size to store the binary data
		std::vector<BYTE> data(dataSize);

		// Call RegGetValue for the second time to read the data content
		retCode = ::RegGetValue(
			m_hKey,
			nullptr,    // no subkey
			valueName.c_str(),
			flags,
			nullptr,    // type not required
			&data[0],   // output buffer
			&dataSize
		);
		if (retCode != ERROR_SUCCESS)
		{
			throw RegException{ xorstr_("Cannot get binary data: RegGetValue failed."), retCode };
		}

		return data;
	}


	inline DWORD RegKey::QueryValueType(const tstring& valueName)
	{
		_ASSERTE(IsValid());

		DWORD typeId{};     // will be returned by RegQueryValueEx

		LONG retCode = ::RegQueryValueEx(
			m_hKey,
			valueName.c_str(),
			nullptr,    // reserved
			&typeId,
			nullptr,    // not interested
			nullptr     // not interested
		);

		if (retCode != ERROR_SUCCESS)
		{
			throw RegException{ xorstr_("Cannot get the value type: RegQueryValueEx failed."), retCode };
		}

		return typeId;
	}


	inline void RegKey::QueryInfoKey(DWORD& subKeys, DWORD& values, FILETIME& lastWriteTime)
	{
		_ASSERTE(IsValid());

		LONG retCode = ::RegQueryInfoKey(
			m_hKey,
			nullptr,
			nullptr,
			nullptr,
			&subKeys,
			nullptr,
			nullptr,
			&values,
			nullptr,
			nullptr,
			nullptr,
			&lastWriteTime
		);
		if (retCode != ERROR_SUCCESS)
		{
			throw RegException{ xorstr_("RegQueryInfoKey failed."), retCode };
		}
	}


	inline std::vector<tstring> RegKey::EnumSubKeys()
	{
		_ASSERTE(IsValid());

		// Get some useful enumeration info, like the total number of subkeys
		// and the maximum length of the subkey names
		DWORD subKeyCount{};
		DWORD maxSubKeyNameLen{};
		LONG retCode = ::RegQueryInfoKey(
			m_hKey,
			nullptr,    // no user-defined class
			nullptr,    // no user-defined class size
			nullptr,    // reserved
			&subKeyCount,
			&maxSubKeyNameLen,
			nullptr,    // no subkey class length
			nullptr,    // no value count
			nullptr,    // no value name max length
			nullptr,    // no max value length
			nullptr,    // no security descriptor
			nullptr     // no last write time
		);
		if (retCode != ERROR_SUCCESS)
		{
			throw RegException{ xorstr_("RegQueryInfoKey failed while preparing for subkey enumeration."), retCode };
		}

		// NOTE: According to the MSDN documentation, the size returned for subkey name max length
		// does *not* include the terminating NUL, so let's add +1 to take it into account
		// when I allocate the buffer for reading subkey names.
		maxSubKeyNameLen++;

		// Preallocate a buffer for the subkey names
		auto nameBuffer = std::make_unique<TCHAR[]>(maxSubKeyNameLen);

		// The result subkey names will be stored here
		std::vector<tstring> subkeyNames;

		// Enumerate all the subkeys
		for (DWORD index = 0; index < subKeyCount; index++)
		{
			// Get the name of the current subkey
			DWORD subKeyNameLen = maxSubKeyNameLen;
			retCode = ::RegEnumKeyEx(
				m_hKey,
				index,
				nameBuffer.get(),
				&subKeyNameLen,
				nullptr, // reserved
				nullptr, // no class
				nullptr, // no class
				nullptr  // no last write time
			);
			if (retCode != ERROR_SUCCESS)
			{
				throw RegException{ xorstr_("Cannot enumerate subkeys: RegEnumKeyEx failed."), retCode };
			}

			// On success, the ::RegEnumKeyEx API writes the length of the
			// subkey name in the subKeyNameLen output parameter 
			// (not including the terminating NUL).
			// So I can build a wstring based on that length.
			subkeyNames.push_back(tstring{ nameBuffer.get(), subKeyNameLen });
		}

		return subkeyNames;
	}


	inline std::vector<std::pair<tstring, DWORD>> RegKey::EnumValues()
	{
		_ASSERTE(IsValid());

		// Get useful enumeration info, like the total number of values
		// and the maximum length of the value names
		DWORD valueCount{};
		DWORD maxValueNameLen{};
		LONG retCode = ::RegQueryInfoKey(
			m_hKey,
			nullptr,    // no user-defined class
			nullptr,    // no user-defined class size
			nullptr,    // reserved
			nullptr,    // no subkey count
			nullptr,    // no subkey max length
			nullptr,    // no subkey class length
			&valueCount,
			&maxValueNameLen,
			nullptr,    // no max value length
			nullptr,    // no security descriptor
			nullptr     // no last write time
		);
		if (retCode != ERROR_SUCCESS)
		{
			throw RegException{
				xorstr_("RegQueryInfoKey failed while preparing for value enumeration."),
				retCode
			};
		}

		// NOTE: According to the MSDN documentation, the size returned for value name max length
		// does *not* include the terminating NUL, so let's add +1 to take it into account
		// when I allocate the buffer for reading value names.
		maxValueNameLen++;

		// Preallocate a buffer for the value names
		auto nameBuffer = std::make_unique<TCHAR[]>(maxValueNameLen);

		// The value names and types will be stored here
		std::vector<std::pair<tstring, DWORD>> valueInfo;

		// Enumerate all the values
		for (DWORD index = 0; index < valueCount; index++)
		{
			// Get the name and the type of the current value
			DWORD valueNameLen = maxValueNameLen;
			DWORD valueType{};
			retCode = ::RegEnumValue(
				m_hKey,
				index,
				nameBuffer.get(),
				&valueNameLen,
				nullptr,    // reserved
				&valueType,
				nullptr,    // no data
				nullptr     // no data size
			);
			if (retCode != ERROR_SUCCESS)
			{
				throw RegException{ xorstr_("Cannot enumerate values: RegEnumValue failed."), retCode };
			}

			// On success, the RegEnumValue API writes the length of the
			// value name in the valueNameLen output parameter 
			// (not including the terminating NUL).
			// So we can build a wstring based on that.
			valueInfo.push_back(
				std::make_pair(tstring{ nameBuffer.get(), valueNameLen }, valueType)
			);
		}

		return valueInfo;
	}


	inline void RegKey::DeleteValue(const tstring& valueName)
	{
		_ASSERTE(IsValid());

		LONG retCode = ::RegDeleteValue(m_hKey, valueName.c_str());
		if (retCode != ERROR_SUCCESS)
		{
			throw RegException{ xorstr_("RegDeleteValue failed."), retCode };
		}
	}


	inline void RegKey::DeleteKey(const tstring& subKey, const REGSAM desiredAccess)
	{
		_ASSERTE(IsValid());

		LONG retCode = ::RegDeleteKeyEx(m_hKey, subKey.c_str(), desiredAccess, 0);
		if (retCode != ERROR_SUCCESS)
		{
			throw RegException{ xorstr_("RegDeleteKeyEx failed."), retCode };
		}
	}


	inline void RegKey::FlushKey()
	{
		_ASSERTE(IsValid());

		LONG retCode = ::RegFlushKey(m_hKey);
		if (retCode != ERROR_SUCCESS)
		{
			throw RegException{ xorstr_("RegFlushKey failed."), retCode };
		}
	}


	inline void RegKey::LoadKey(const tstring& subKey, const tstring& filename)
	{
		Close();

		LONG retCode = ::RegLoadKey(m_hKey, subKey.c_str(), filename.c_str());
		if (retCode != ERROR_SUCCESS)
		{
			throw RegException{ xorstr_("RegLoadKey failed."), retCode };
		}
	}


	inline void RegKey::SaveKey(
		const tstring& filename,
		SECURITY_ATTRIBUTES* const securityAttributes
	)
	{
		_ASSERTE(IsValid());

		LONG retCode = ::RegSaveKey(m_hKey, filename.c_str(), securityAttributes);
		if (retCode != ERROR_SUCCESS)
		{
			throw RegException{ xorstr_("RegSaveKey failed."), retCode };
		}
	}


	inline void RegKey::EnableReflectionKey()
	{
		LONG retCode = ::RegEnableReflectionKey(m_hKey);
		if (retCode != ERROR_SUCCESS)
		{
			throw RegException{ xorstr_("RegEnableReflectionKey failed."), retCode };
		}
	}


	inline void RegKey::DisableReflectionKey()
	{
		LONG retCode = ::RegDisableReflectionKey(m_hKey);
		if (retCode != ERROR_SUCCESS)
		{
			throw RegException{ xorstr_("RegDisableReflectionKey failed."), retCode };
		}
	}


	inline bool RegKey::QueryReflectionKey()
	{
		BOOL isReflectionDisabled = FALSE;
		LONG retCode = ::RegQueryReflectionKey(m_hKey, &isReflectionDisabled);
		if (retCode != ERROR_SUCCESS)
		{
			throw RegException{ xorstr_("RegQueryReflectionKey failed."), retCode };
		}

		return (isReflectionDisabled ? true : false);
	}


	inline void RegKey::ConnectRegistry(const tstring& machineName, const HKEY hKeyPredefined)
	{
		// Safely close any previously opened key
		Close();

		HKEY hKeyResult{ nullptr };
		LONG retCode = ::RegConnectRegistry(machineName.c_str(), hKeyPredefined, &hKeyResult);
		if (retCode != ERROR_SUCCESS)
		{
			throw RegException{ xorstr_("RegConnectRegistry failed."), retCode };
		}

		// Take ownership of the result key
		m_hKey = hKeyResult;
	}


	inline tstring RegKey::RegTypeToString(const DWORD regType)
	{
		switch (regType)
		{
		case REG_SZ:        return _T("REG_SZ");
		case REG_EXPAND_SZ: return _T("REG_EXPAND_SZ");
		case REG_MULTI_SZ:  return _T("REG_MULTI_SZ");
		case REG_DWORD:     return _T("REG_DWORD");
		case REG_QWORD:     return _T("REG_QWORD");
		case REG_BINARY:    return _T("REG_BINARY");

		default:            return _T("Unknown/unsupported registry type");
		}
	}


} // namespace winreg


#endif // INCLUDE_GIOVANNI_DICANIO_WINREG_HPP