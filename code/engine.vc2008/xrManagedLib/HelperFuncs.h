#pragma once


// char* ConvertWidecharToAscii(const wchar_t* pData, unsigned int size);
// void ConvertWidecharToAscii(const wchar_t* pData, unsigned int size, string1024& OutStr);
// void ConvertWidecharToAscii(const wchar_t* pData, unsigned int size, string512& OutStr);
// void ConvertWidecharToAscii(const wchar_t* pData, unsigned int size, string256& OutStr);
// 
// template <int Size>
// void ConvertWidecharToAscii(const wchar_t* pData, unsigned int size, char(&OutStr)[Size]);

template <int Size>
void ConvertWidecharToAscii(const wchar_t* pData, unsigned int size, char(&OutStr)[Size])
{
	WideCharToMultiByte(CP_ACP, 0, pData, size, OutStr, Size, nullptr, nullptr);
}

template <int Size>
void ConvertDotNetStringToAscii(::System::String^ InStr, char(&OutStr)[Size])
{
	pin_ptr<const wchar_t> pinString = PtrToStringChars(InStr);
	ConvertWidecharToAscii(pinString, InStr->Length, OutStr);
}

#define CONVERT_FVECTOR(vector) Utils::FromFvector(IntPtr((void*)&vector))
#define CONVERT_FMATRIX(matrix) Utils::FromXForm(IntPtr((void*)&matrix))