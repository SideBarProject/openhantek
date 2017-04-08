#include "stdStringSplit.h"

std::string section(const std::string& data, int start) {
    std::vector<std::string> dataParts = split(data, " ");
    int validSection = 0;
    for(std::string d: dataParts) {
        d.erase(0, d.find_first_not_of(" \n\r\t"));
        d.erase(d.find_last_not_of(" \n\r\t")+1);
        if (!d.size()) continue;
        if (validSection++ == start)
            return d;
    }
    return std::string();
}

/// \brief Returns the hex dump for the given data.
/// \param dump The string with the hex dump of the data.
/// \param data Pointer to the address where the data bytes should be saved.
/// \param length The maximum length of the data array in bytes.
/// \return The length of the saved data.
unsigned int hexParse(std::string dumpString, unsigned char *data, unsigned length) {
    dumpString.erase(std::remove(dumpString.begin(), dumpString.end(), ' '), dumpString.end());
    length = std::max(length, (unsigned)dumpString.size()/2);

    unsigned index = 0;
    for(; index < length; ++index) {
        char acTmp[3];
        sscanf(dumpString.c_str() + (index << 1), "%2s", acTmp);
        data[index] = (char)strtol(acTmp, NULL, 16);
    }

    return index;
}

std::string hexDump(const unsigned char *data, unsigned len)
{
    static const char* const lut = "0123456789ABCDEF";

    std::string output;
    output.reserve(2 * len);
    for (size_t i = 0; i < len; ++i)
    {
        const unsigned char c = data[i];
        output.push_back(lut[c >> 4]);
        output.push_back(lut[c & 15]);
    }
    return output;
}
