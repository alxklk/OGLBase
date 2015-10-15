#pragma once

#include <windows.h>

class CFIMM
{
public:
	HANDLE af;
	unsigned long afsize;
	HANDLE afmapping;
	void* afmap;
	CFIMM():af(INVALID_HANDLE_VALUE),afmapping(INVALID_HANDLE_VALUE),afsize(0),afmap(0){};
	~CFIMM(){Close();};

	unsigned int GetSize()const{return afsize;};

	const void* Open(const char* name)
	{
		af=::CreateFile(name,GENERIC_READ,0,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);
		if(af==INVALID_HANDLE_VALUE)
			return 0;
		else
		{
			afsize=::GetFileSize(af,0);
			afmapping=CreateFileMapping(af,0,PAGE_READONLY,0,0,0);
			if(afmapping==INVALID_HANDLE_VALUE)
			{
				::CloseHandle(af);
				af=INVALID_HANDLE_VALUE;
				return 0;
			}
			afmap=::MapViewOfFile(afmapping,FILE_MAP_READ,0,0,0);
			return afmap;
		}
	};

	const void* Addr()const
	{
		if(af==INVALID_HANDLE_VALUE)
			return 0;
		return afmap;
	};

	const char* CAddr()const
	{
		return (const char*)Addr();
	};

	void Close()
	{
		if(af!=INVALID_HANDLE_VALUE)
		{
			if(afmap)
			{
				::UnmapViewOfFile(afmap);
			}
			if(afmapping!=INVALID_HANDLE_VALUE)
			{
				::CloseHandle(afmapping);
			}
			::CloseHandle(af);
		}
		af=INVALID_HANDLE_VALUE;
	};
};
