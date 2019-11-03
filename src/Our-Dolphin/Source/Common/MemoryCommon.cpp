#include "MemoryCommon.h"

#include <bitset>
#include <cstring>
#include <iomanip>
#include <sstream>
#include <vector>

#include "../Common/CommonTypes.h"
#include "../Common/CommonUtils.h"

#include <iostream>

namespace Common
{
size_t getSizeForType(const MemType type, const size_t length)
{
  switch (type)
  {
  case MemType::type_byte:
    return sizeof(u8);
  case MemType::type_halfword:
    return sizeof(u16);
  case MemType::type_word:
    return sizeof(u32);
  case MemType::type_float:
    return sizeof(float);
  case MemType::type_double:
    return sizeof(double);
  case MemType::type_string:
    return length;
  case MemType::type_byteArray:
    return length;
  default:
    return 0;
  }
}

bool shouldBeBSwappedForType(const MemType type)
{
  switch (type)
  {
  case MemType::type_byte:
    return false;
  case MemType::type_halfword:
    return true;
  case MemType::type_word:
    return true;
  case MemType::type_float:
    return true;
  case MemType::type_double:
    return true;
  case MemType::type_string:
    return false;
  case MemType::type_byteArray:
    return false;
  default:
    return false;
  }
}

int getNbrBytesAlignementForType(const MemType type)
{
  switch (type)
  {
  case MemType::type_byte:
    return 1;
  case MemType::type_halfword:
    return 2;
  case MemType::type_word:
    return 4;
  case MemType::type_float:
    return 4;
  case MemType::type_double:
    return 4;
  case MemType::type_string:
    return 1;
  case MemType::type_byteArray:
    return 1;
  default:
    return 1;
  }
}

char* formatStringToMemory(MemOperationReturnCode& returnCode, size_t& actualLength,
                           const std::string inputString, const MemBase base, const MemType type,
                           const size_t length)
{
  if (inputString.length() == 0)
  {
    returnCode = MemOperationReturnCode::invalidInput;
    return nullptr;
  }

  std::stringstream ss(inputString);
  switch (base)
  {
  case MemBase::base_octal:
    ss >> std::oct;
    break;
  case MemBase::base_decimal:
    ss >> std::dec;
    break;
  case MemBase::base_hexadecimal:
    ss >> std::hex;
    break;
  }

  size_t size = getSizeForType(type, length);
  char* buffer = new char[size];

  switch (type)
  {
  case MemType::type_byte:
  {
    u8 theByte = 0;
    if (base == MemBase::base_binary)
    {
      unsigned long long input = 0;
      try
      {
        input = std::bitset<sizeof(u8) * 8>(inputString).to_ullong();
      }
      catch (std::invalid_argument)
      {
        delete[] buffer;
        buffer = nullptr;
        returnCode = MemOperationReturnCode::invalidInput;
        return buffer;
      }
      theByte = static_cast<u8>(input);
    }
    else
    {
      int theByteInt = 0;
      ss >> theByteInt;
      if (ss.fail())
      {
        delete[] buffer;
        buffer = nullptr;
        returnCode = MemOperationReturnCode::invalidInput;
        return buffer;
      }
      theByte = static_cast<u8>(theByteInt);
    }

    std::memcpy(buffer, &theByte, size);
    actualLength = sizeof(u8);
    break;
  }

  case MemType::type_halfword:
  {
    u16 theHalfword = 0;
    if (base == MemBase::base_binary)
    {
      unsigned long long input = 0;
      try
      {
        input = std::bitset<sizeof(u16) * 8>(inputString).to_ullong();
      }
      catch (std::invalid_argument)
      {
        delete[] buffer;
        buffer = nullptr;
        returnCode = MemOperationReturnCode::invalidInput;
        return buffer;
      }
      theHalfword = static_cast<u16>(input);
    }
    else
    {
      ss >> theHalfword;
      if (ss.fail())
      {
        delete[] buffer;
        buffer = nullptr;
        returnCode = MemOperationReturnCode::invalidInput;
        return buffer;
      }
    }

    std::memcpy(buffer, &theHalfword, size);
    actualLength = sizeof(u16);
    break;
  }

  case MemType::type_word:
  {
    u32 theWord = 0;
    if (base == MemBase::base_binary)
    {
      unsigned long long input = 0;
      try
      {
        input = std::bitset<sizeof(u32) * 8>(inputString).to_ullong();
      }
      catch (std::invalid_argument)
      {
        delete[] buffer;
        buffer = nullptr;
        returnCode = MemOperationReturnCode::invalidInput;
        return buffer;
      }
      theWord = static_cast<u32>(input);
    }
    else
    {
      ss >> theWord;
      if (ss.fail())
      {
        delete[] buffer;
        buffer = nullptr;
        returnCode = MemOperationReturnCode::invalidInput;
        return buffer;
      }
    }

    std::memcpy(buffer, &theWord, size);
    actualLength = sizeof(u32);
    break;
  }

  case MemType::type_float:
  {
    float theFloat = 0.0f;
    // 9 digits is the max number of digits in a flaot that can recover any binary format
    ss >> std::setprecision(9) >> theFloat;
    if (ss.fail())
    {
      delete[] buffer;
      buffer = nullptr;
      returnCode = MemOperationReturnCode::invalidInput;
      return buffer;
    }
    std::memcpy(buffer, &theFloat, size);
    actualLength = sizeof(float);
    break;
  }

  case MemType::type_double:
  {
    double theDouble = 0.0;
    // 17 digits is the max number of digits in a double that can recover any binary format
    ss >> std::setprecision(17) >> theDouble;
    if (ss.fail())
    {
      delete[] buffer;
      buffer = nullptr;
      returnCode = MemOperationReturnCode::invalidInput;
      return buffer;
    }
    std::memcpy(buffer, &theDouble, size);
    actualLength = sizeof(double);
    break;
  }

  case MemType::type_string:
  {
    if (inputString.length() > length)
    {
      delete[] buffer;
      buffer = nullptr;
      returnCode = MemOperationReturnCode::inputTooLong;
      return buffer;
    }
    std::memcpy(buffer, inputString.c_str(), length);
    actualLength = length;
    break;
  }

  case MemType::type_byteArray:
  {
    std::vector<std::string> bytes;
    std::string next;
    for (auto i : inputString)
    {
      if (i == ' ')
      {
        if (!next.empty())
        {
          bytes.push_back(next);
          next.clear();
        }
      }
      else
      {
        next += i;
      }
    }
    if (!next.empty())
    {
      bytes.push_back(next);
      next.clear();
    }

    if (bytes.size() > length)
    {
      delete[] buffer;
      buffer = nullptr;
      returnCode = MemOperationReturnCode::inputTooLong;
      return buffer;
    }

    int index = 0;
    for (const auto& i : bytes)
    {
      std::stringstream byteStream(i);
      ss >> std::hex;
      u8 theByte = 0;
      int theByteInt = 0;
      ss >> theByteInt;
      if (ss.fail())
      {
        delete[] buffer;
        buffer = nullptr;
        returnCode = MemOperationReturnCode::invalidInput;
        return buffer;
      }
      theByte = static_cast<u8>(theByteInt);
      std::memcpy(&(buffer[index]), &theByte, sizeof(u8));
      index++;
    }
    actualLength = bytes.size();
  }
  }
  return buffer;
}

std::string formatMemoryToString(const char* memory, const MemType type, const size_t length,
                                 const MemBase base, const bool isUnsigned, const bool withBSwap)
{


  // std::cout << "fromating" << std::endl;
  // std::cout<< "length: "<< length<<std::endl;
  // std::cout<< "Bswap: "<<withBSwap << std::endl;
  // std::cout << "Is unsinged" << isUnsigned << std::endl;

  std::stringstream ss;
  switch (base)
  {
  case Common::MemBase::base_octal:
    ss << std::oct;
    break;
  case Common::MemBase::base_decimal:
    ss << std::dec;
    break;
  case Common::MemBase::base_hexadecimal:
    ss << std::hex << std::uppercase;
    break;
  }

  switch (type)
  {
  case Common::MemType::type_byte:
  {
    if (isUnsigned || base == Common::MemBase::base_binary)
    {
      u8 unsignedByte = 0;
      std::memcpy(&unsignedByte, memory, sizeof(u8));
      if (base == Common::MemBase::base_binary)
        return std::bitset<sizeof(u8) * 8>(unsignedByte).to_string();
      // This has to be converted to an integer type because printing a uint8_t would resolve to a
      // char and print a single character.
      ss << static_cast<unsigned int>(unsignedByte);
      return ss.str();
    }
    else
    {
      s8 aByte = 0;
      std::memcpy(&aByte, memory, sizeof(s8));
      // This has to be converted to an integer type because printing a uint8_t would resolve to a
      // char and print a single character.  Additionaly, casting a signed type to a larger signed
      // type will extend the sign to match the size of the destination type, this is required for
      // signed values in decimal, but must be bypassed for other bases, this is solved by first
      // casting to u8 then to signed int.
      if (base == Common::MemBase::base_decimal)
        ss << static_cast<int>(aByte);
      else
        ss << static_cast<int>(static_cast<u8>(aByte));
      return ss.str();
    }
  }
  case Common::MemType::type_halfword:
  {
    char* memoryCopy = new char[sizeof(u16)];
    std::memcpy(memoryCopy, memory, sizeof(u16));
    if (withBSwap)
    {
      u16 halfword = 0;
      std::memcpy(&halfword, memoryCopy, sizeof(u16));
      halfword = Common::bSwap16(halfword);
      std::memcpy(memoryCopy, &halfword, sizeof(u16));
    }

    if (isUnsigned || base == Common::MemBase::base_binary)
    {
      u16 unsignedHalfword = 0;
      std::memcpy(&unsignedHalfword, memoryCopy, sizeof(u16));
      if (base == Common::MemBase::base_binary)
      {
        delete[] memoryCopy;
        return std::bitset<sizeof(u16) * 8>(unsignedHalfword).to_string();
      }
      ss << unsignedHalfword;
      delete[] memoryCopy;
      return ss.str();
    }
    s16 aHalfword = 0;
    std::memcpy(&aHalfword, memoryCopy, sizeof(s16));
    ss << aHalfword;
    delete[] memoryCopy;
    return ss.str();
  }
  case Common::MemType::type_word:
  {
    char* memoryCopy = new char[sizeof(u32)];
    std::memcpy(memoryCopy, memory, sizeof(u32));
    if (withBSwap)
    {
      u32 word = 0;
      std::memcpy(&word, memoryCopy, sizeof(u32));
      word = Common::bSwap32(word);
      std::memcpy(memoryCopy, &word, sizeof(u32));
    }

    if (isUnsigned || base == Common::MemBase::base_binary)
    {
      u32 unsignedWord = 0;
      std::memcpy(&unsignedWord, memoryCopy, sizeof(u32));
      if (base == Common::MemBase::base_binary)
      {
        delete[] memoryCopy;
        return std::bitset<sizeof(u32) * 8>(unsignedWord).to_string();
      }
      ss << unsignedWord;
      delete[] memoryCopy;
      return ss.str();
    }
    s32 aWord = 0;
    std::memcpy(&aWord, memoryCopy, sizeof(s32));
    ss << aWord;
    delete[] memoryCopy;
    return ss.str();
  }
  case Common::MemType::type_float:
  {
    char* memoryCopy = new char[sizeof(u32)];
    std::memcpy(memoryCopy, memory, sizeof(u32));
    if (withBSwap)
    {
      u32 word = 0;
      std::memcpy(&word, memoryCopy, sizeof(u32));
      word = Common::bSwap32(word);
      std::memcpy(memoryCopy, &word, sizeof(u32));
    }

    float aFloat = 0.0f;
    std::memcpy(&aFloat, memoryCopy, sizeof(float));
    // With 9 digits of precision, it is possible to convert a float back and forth to its binary
    // representation without any loss
    ss << std::setprecision(9) << aFloat;
    delete[] memoryCopy;
    return ss.str();
  }
  case Common::MemType::type_double:
  {
    char* memoryCopy = new char[sizeof(u64)];
    std::memcpy(memoryCopy, memory, sizeof(u64));
    if (withBSwap)
    {
      u64 doubleword = 0;
      std::memcpy(&doubleword, memoryCopy, sizeof(u64));
      doubleword = Common::bSwap64(doubleword);
      std::memcpy(memoryCopy, &doubleword, sizeof(u64));
    }

    double aDouble = 0.0;
    std::memcpy(&aDouble, memoryCopy, sizeof(double));
    // With 17 digits of precision, it is possible to convert a double back and forth to its binary
    // representation without any loss
    ss << std::setprecision(17) << aDouble;
    delete[] memoryCopy;
    return ss.str();
  }
  case Common::MemType::type_string:
  {
    int actualLength = 0;
    for (actualLength; actualLength < length; ++actualLength)
    {
      if (*(memory + actualLength) == 0x00)
        break;
    }
    return std::string(memory, actualLength);
  }
  case Common::MemType::type_byteArray:
  {
    // Force Hexadecimal, no matter the base
    ss << std::hex << std::uppercase;
    for (int i = 0; i < length; ++i)
    {
      u8 aByte = 0;
      std::memcpy(&aByte, memory + i, sizeof(u8));
      ss << std::setfill('0') << std::setw(2) << static_cast<int>(aByte) << " ";
    }
    std::string str = ss.str();
    // Remove the space at the end
    str.pop_back();
    return str;
  }
  default:
    return "";
    break;
  }
}
} // namespace Common