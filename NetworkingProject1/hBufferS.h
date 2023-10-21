#pragma once
#include <vector>
#include <string>

class Buffer
{
public:
	//● Initialize with size N
	Buffer(int size)
	{
		m_BufferData.resize(size);
		m_WriteIndex = 0;
		m_ReadIndex = 0;
	}
	~Buffer() { }

	//short
	uint16_t ReadUInt16LE()
	{
		uint16_t value = 0;

		value |= m_BufferData[m_ReadIndex++];
		value |= m_BufferData[m_ReadIndex++] << 4;
		value |= m_BufferData[m_ReadIndex++] << 8;
		value |= m_BufferData[m_ReadIndex++] << 12;

		return value;
	}

	void WriteUInt16LE(uint16_t value)
	{
		m_BufferData[m_WriteIndex++] = value;
		m_BufferData[m_WriteIndex++] = value >> 4;
		m_BufferData[m_WriteIndex++] = value >> 8;
		m_BufferData[m_WriteIndex++] = value >> 12;
	}
	//int
	uint32_t ReadUInt32LE()
	{
		uint32_t value = 0;

		value |= m_BufferData[m_ReadIndex++];
		value |= m_BufferData[m_ReadIndex++] << 8;
		value |= m_BufferData[m_ReadIndex++] << 16;
		value |= m_BufferData[m_ReadIndex++] << 24;

		return value;
	}

	void WriteUInt32LE(uint32_t value)
	{
		m_BufferData[m_WriteIndex++] = value;
		m_BufferData[m_WriteIndex++] = value >> 8;
		m_BufferData[m_WriteIndex++] = value >> 16;
		m_BufferData[m_WriteIndex++] = value >> 24;
	}

	//string
	void WriteString(const std::string& str)
	{
		int strLength = str.length();
		for (int i = 0; i < strLength; i++)
		{
			m_BufferData[m_WriteIndex++] = str[i];
		}
	}

	std::string ReadString(uint32_t length)
	{
		std::string str;
		for (int i = 0; i < length; i++)
		{
			str.push_back(m_BufferData[m_ReadIndex++]);
		}
		return str;
	}

	std::vector<uint8_t> m_BufferData;
	int m_WriteIndex;
	int m_ReadIndex;
};

///*cBuffer.cpp
//In this class we want to:

//● Grow when serializing past the write index
//● Serialize, Deserialize unsigned int (32 bit)
//● Serialize, Deserialize unsigned short (16 bit)
//● Serialize, Deserialize string
//*/
//#include <iostream>
//#include <vector>
//#include <cassert>
//typedef unsigned int uint32;
//class cBuffer
//{
//	cBuffer(size_t size)
//	{
//		m_Buffer.resize(size, 0);
//	}
//	~cBuffer()
//	{
//		m_Buffer.clear();
//	}
//	void WriteUInt32LE(size_t index, uint32 value)
//	{
//		// Implement this
//		assert(false, "Function not implemented");
//	}
//
//	void WriteUInt32LE(uint32 value)
//	{
//		// Implement this
//		assert(false, "Function not implemented");
//	}
//
//	uint32 ReadUInt32LE(size_t index)
//	{
//		// Implement this
//		assert(false, "Function not implemented");
//		return 0;
//	}
//
//	uint32 ReadUInt32LE()
//	{
//		// Implement this
//		assert(false, "Function not implemented");
//		return 0;
//	}
//
//private: 
//	std::vector<uint8_t> m_Buffer;
//	uint32 m_ReadIndex;
//	uint32 m_WriteIndex;
//
//};