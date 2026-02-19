#include "../../valve/buf.h"

#include "../../leychan.h"
#include "svc_createstringtable.h"
#include <fstream>
#include <cctype>
#include <set>

bool svc_createstringtable::Register(leychan* chan)
{
	leychan::netcallbackfn fn = reinterpret_cast<leychan::netcallbackfn>(&svc_createstringtable::ParseMessage);

	return chan->RegisterMessageHandler(this->GetMsgType(), this, fn);
}

bool svc_createstringtable::ParseMessage(leychan* chan, svc_createstringtable* thisptr, bf_read& msg)
{
	char name[500];
	msg.ReadString(name, sizeof(name));

	short maxentries = msg.ReadWord();

	unsigned int size = (int)(log2(maxentries) + 1);
	int entries = msg.ReadUBitLong(size);

	int bits = msg.ReadVarInt32();
	int userdata = msg.ReadOneBit();

	if (userdata == 1)
	{
		int userdatasize = msg.ReadUBitLong(12);

		int userdatabits = msg.ReadUBitLong(4);
	}

	int compressed = msg.ReadOneBit();

	if (bits < 1)
		return true;

	// bits = number of bits read into the buffer. allocate in bytes
	int nbytes = (bits + 7) / 8;
	char* data = new char[nbytes];

	// ReadBits takes number of bits
	msg.ReadBits(data, bits);
	
	std::string filename = name;
	filename += ".bin";

	std::ofstream file(filename, std::ios::binary);
	if(!file) {
		printf("Can't open file %s", filename);
		delete[] data;
		return false;
	}
	else {
		printf("writing data (%d bytes)\n", nbytes);
		file.write(data, nbytes);
		file.close();
	}
	printf("Received svc_CreateStringTable, name: %s | maxentries: %i | size: %d | entries: %i | compressed: %i\n", name, maxentries, size, entries, compressed);


	// Если нужно распарсить сам stringtable, можно создать bf_read
	// поверх считанных битов и попробовать прочитать строки.
	// Пример (попытка распечатать первые N строк, формат таблицы может отличаться):
	{
		// используем bf_read из valve/buf.h
		bf_read tablebits(data, nbytes, bits);

		// entries — уже прочитанное количество записей (см. выше)
		int toprint = entries;
		if (toprint > 32) toprint = 32;

		char s[1024];
		for (int i = 0; i < toprint; ++i)
		{
			// Попробуем считать C-строку из текущей позиции.
			// В зависимости от формата stringtable это может не сработать напрямую.
			if (tablebits.ReadString(s, sizeof(s))) {
				std::ofstream sf(filename + ".strings" + std::to_string(i) + ".txt", std::ios::binary);
				if (sf) {
					sf.write(s, sizeof(s));
					sf.close();
				}
				// printf("entry %d: %s\n", i, s);
			} else {
				// Если не получилось, прервём — вероятно формат другой
				break;
			}
		}
	}

	// Эвристический ASCII-дамп: ищем печатные подпоследовательности длиной >= 4
	{
		const int minlen = 4;
		int i = 0;
		while (i < nbytes)
		{
			if (std::isprint((unsigned char)data[i]))
			{
				int j = i;
				while (j < nbytes && std::isprint((unsigned char)data[j])) j++;
				int len = j - i;
				if (len >= minlen)
				{
					std::string s(&data[i], &data[i] + len);
					printf("ASCII candidate at %d (len %d): %s\\n", i, len, s.c_str());
				}
				i = j;
			}
			else i++;
		}
	}

	// Попробуем детектировать LZSS-упаковку и распаковать, если есть
	{
		char* lzstart = nullptr;
		int lzoffset = 0;
		unsigned int uDecompSize = 0;
		// Проверяем на наличие LZSS прямо в начале или со смещением 4 (частый случай)
		if (nbytes >= 8 && leychan::NET_GetDecompressedBufferSize(data) != 0)
		{
			lzstart = data;
			lzoffset = 0;
			uDecompSize = leychan::NET_GetDecompressedBufferSize(data);
		}
		else if (nbytes > 4 && leychan::NET_GetDecompressedBufferSize(data + 4) != 0)
		{
			lzstart = data + 4;
			lzoffset = 4;
			uDecompSize = leychan::NET_GetDecompressedBufferSize(data + 4);
		}

		if (lzstart)
		{
			printf("Detected LZSS compressed block (offset %d) -> uncompressed size %u\n", lzoffset, uDecompSize);
			char* out = new char[uDecompSize + 16];
			unsigned int destLen = uDecompSize;
			unsigned int sourceLen = nbytes - lzoffset;
			if (leychan::NET_BufferToBufferDecompress(out, destLen, lzstart, sourceLen))
			{
				std::string decname = filename + ".decompressed.bin";
				std::ofstream df(decname, std::ios::binary);
				if (df)
				{
					df.write(out, destLen);
					df.close();
					printf("Wrote decompressed data to %s (%u bytes)\n", decname.c_str(), destLen);
				}
				// Соберём кандидаты-строки из распакованного буфера
				{
					const int minlen = 4;
					std::set<std::string> uniq;
					int i = 0;
					while (i < (int)destLen)
					{
						if (std::isprint((unsigned char)out[i]))
						{
							int j = i;
							while (j < (int)destLen && std::isprint((unsigned char)out[j])) j++;
							int len = j - i;
							if (len >= minlen)
							{
								std::string s(&out[i], &out[i] + len);
								// Простая эвристика: содержат путь или расширение
								if (s.find('/') != std::string::npos || s.find('.') != std::string::npos)
									uniq.insert(s);
							}
							i = j;
						}
						else i++;
					}

					if (!uniq.empty())
					{
						std::string listname = filename + ".decompressed.strings.txt";
						std::ofstream sf(listname);
						if (sf)
						{
							for (auto &st : uniq) sf << st << "\n";
							sf.close();
							printf("Wrote %zu candidate strings to %s\n", uniq.size(), listname.c_str());
						}
					}
				}
			}
			else
			{
				printf("LZSS decompression failed\n");
			}
			delete[] out;
		}
	}

	delete[] data;

	return true;
}