#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <cctype>
#include <set>

#include "../src/valve/clzss.h"

static std::vector<char> readFile(const std::string &path)
{
    std::ifstream f(path, std::ios::binary);
    if (!f) return {};
    f.seekg(0, std::ios::end);
    size_t sz = f.tellg();
    f.seekg(0);
    std::vector<char> buf(sz);
    f.read(buf.data(), sz);
    return buf;
}

static void writeFile(const std::string &path, const char* data, size_t len)
{
    std::ofstream f(path, std::ios::binary);
    if (!f) return;
    f.write(data, len);
}

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        printf("Usage: %s <path/to/file1.bin> [file2.bin ...]\n", argv[0]);
        return 1;
    }

    for (int i = 1; i < argc; ++i)
    {
        std::string path = argv[i];
        auto buf = readFile(path);
        if (buf.empty())
        {
            printf("Failed to read %s\n", path.c_str());
            continue;
        }

        printf("Processing %s (%zu bytes)\n", path.c_str(), buf.size());

        // Detect LZSS at offset 0 or 4
        CLZSS lz;
        unsigned char* data0 = (unsigned char*)buf.data();
        unsigned char* data4 = buf.size() > 4 ? (unsigned char*)(buf.data() + 4) : nullptr;
        unsigned int actualSize = 0;
        unsigned char* source = nullptr;
        size_t sourceLen = 0;
        int offset = 0;
        if (lz.IsCompressed(data0))
        {
            actualSize = lz.GetActualSize(data0);
            source = data0;
            sourceLen = buf.size();
            offset = 0;
        }
        else if (data4 && lz.IsCompressed(data4))
        {
            actualSize = lz.GetActualSize(data4);
            source = data4;
            sourceLen = buf.size() - 4;
            offset = 4;
        }

        if (source)
        {
            printf("Detected LZSS at offset %d, uncompressed size %u\n", offset, actualSize);
            std::vector<char> out(actualSize + 16);
            unsigned int res = lz.Uncompress(source, (unsigned char*)out.data());
            if (res == 0)
            {
                printf("Decompression failed for %s\n", path.c_str());
            }
            else
            {
                std::string decpath = path + std::string(".decompressed.bin");
                writeFile(decpath, out.data(), res);
                printf("Wrote %s (%u bytes)\n", decpath.c_str(), res);

                // collect candidate strings using improved heuristics
                std::set<std::string> uniq;
                const int global_minlen = 5;
                const int minlen_with_symbol = 3; // if contains '/' or '.' allow shorter
                int p = 0;
                auto is_reasonable = [&](const std::string &s)->bool {
                    if (s.empty()) return false;
                    // normalize backslashes to slashes for checks
                    int alnum = 0;
                    int punct = 0;
                    for (unsigned char c : s)
                    {
                        if (std::isalnum(c)) alnum++;
                        else if (std::ispunct(c)) punct++;
                    }
                    if (alnum == 0) return false;
                    double alnum_ratio = (double)alnum / (double)s.size();
                    // known extensions allow short strings
                    static const char* exts[] = {".lua",".txt",".vmt",".vtf",".mdl",".wav",".mp3",".png",".jpg",".jpeg",".res",".cfg",".dat",".lua",".pap",".scn"};
                    for (auto e : exts)
                    {
                        if (s.size() > strlen(e) && strcasecmp(s.c_str() + s.size() - strlen(e), e) == 0) return true;
                    }
                    // if contains path or dot, accept shorter minimal length
                    if (s.find('/') != std::string::npos || s.find('.') != std::string::npos)
                    {
                        if ((int)s.size() >= minlen_with_symbol && alnum_ratio >= 0.25) return true;
                    }
                    // otherwise require longer and sufficiently alnum
                    if ((int)s.size() >= global_minlen && alnum_ratio >= 0.6) return true;
                    return false;
                };

                while (p < (int)res)
                {
                    if (std::isprint((unsigned char)out[p]))
                    {
                        int q = p;
                        while (q < (int)res && std::isprint((unsigned char)out[q])) q++;
                        int len = q - p;
                        if (len >= 1)
                        {
                            std::string s(&out[p], &out[p] + len);
                            // trim non-alnum from ends but keep / . _ -
                            auto trim = [](std::string &t){
                                int a=0,b=(int)t.size()-1;
                                while (a<=b && !(std::isalnum((unsigned char)t[a]) || t[a]=='/'||t[a]=='.'||t[a]=='_'||t[a]=='-' )) a++;
                                while (b>=a && !(std::isalnum((unsigned char)t[b]) || t[b]=='/'||t[b]=='.'||t[b]=='_'||t[b]=='-' )) b--;
                                if (a==0 && b==(int)t.size()-1) return;
                                if (a>b) { t.clear(); return; }
                                t = t.substr(a, b-a+1);
                            };
                            trim(s);
                            // normalize backslashes to forward slashes
                            for (auto &c : s) if (c=='\\') c = '/';
                            if (is_reasonable(s)) uniq.insert(s);
                        }
                        p = q;
                    }
                    else p++;
                }

                if (!uniq.empty())
                {
                    std::string listname = path + std::string(".decompressed.strings.txt");
                    std::string cleanname = path + std::string(".decompressed.clean.strings.txt");
                    std::ofstream sf(listname);
                    std::ofstream cf(cleanname);
                    for (auto &st : uniq) { if (sf) sf << st << "\n"; if (cf) cf << st << "\n"; }
                    if (sf) printf("Wrote %zu candidate strings to %s\n", uniq.size(), listname.c_str());
                    if (cf) printf("Wrote cleaned %zu candidate strings to %s\n", uniq.size(), cleanname.c_str());
                }
            }
        }
        else
        {
            // No LZSS: try to extract printable candidates from raw buffer
            std::set<std::string> uniq;
            const int minlen = 4;
            int p = 0;
            while (p < (int)buf.size())
            {
                if (std::isprint((unsigned char)buf[p]))
                {
                    int q = p;
                    while (q < (int)buf.size() && std::isprint((unsigned char)buf[q])) q++;
                    int len = q - p;
                    if (len >= minlen)
                    {
                        std::string s(&buf[p], &buf[p] + len);
                        if (s.find('/') != std::string::npos || s.find('.') != std::string::npos)
                            uniq.insert(s);
                    }
                    p = q;
                }
                else p++;
            }
            if (!uniq.empty())
            {
                std::string listpath = path + std::string(".strings.txt");
                std::ofstream sf(listpath);
                for (auto &st : uniq) sf << st << "\n";
                printf("Wrote %zu candidate strings to %s\n", uniq.size(), listpath.c_str());
            }
            else
            {
                printf("No obvious candidate strings found in raw buffer for %s\n", path.c_str());
            }
        }
    }

    return 0;
}
