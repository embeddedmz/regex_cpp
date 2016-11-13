#include <string>
#include <cstring>
#include <regex>
#include <iterator>
#include <iostream>
#include <chrono>
#include <cassert>
#include <memory>

#include <boost/regex.hpp>

using namespace std;

// *** The Winner - 1st place *** on Windows
const char* ProcessSpecialChar(const char* pszInputString)
{
   char* pszOutputString = NULL;
   if (pszInputString != NULL && *pszInputString)
   {
      // Dynamically allocate a char array which size is the double of the input string (very sufficient) and + 1 for the zero
      pszOutputString = new char[(strlen(pszInputString) * 2) + 1];

      char* pszOutputChar = pszOutputString;
      char* pszInputChar = const_cast<char*>(pszInputString);
      for (; *pszInputChar != '\0'; ++pszInputChar, ++pszOutputChar)
      {
         switch (*pszInputChar)
         {
            case '+':
            case '-':
            case '!':
            case '(':
            case ')':
            case '{':
            case '}':
            case '[':
            case ']':
            case '^':
            case '\"':
            case '~':
            case '*':
            case '?':
            case ':':
            case '\\':
               *pszOutputChar++ = '\\';
               break;

            case '&':
            case '|':
               if (*(pszInputChar + 1) == *pszInputChar)
               {
                  *pszOutputChar++ = '\\';
                  *pszOutputChar++ = *pszInputChar++;
               }
               break;

            default:
               break;
         }
         *pszOutputChar = *pszInputChar;
      }
      *pszOutputChar = '\0';
   }
   return pszOutputString;
}

#define LUCENE_SPEC_CHARS_SORTED "!\"&()*+-:?[\\]^{|}~"
const static std::string g_strEscapeChars = LUCENE_SPEC_CHARS_SORTED;
void StringProcessSpecialCharInSitu(std::string& strString)
{
   if (!strString.empty())
   {
      #ifdef _DEBUG
      assert(std::is_sorted(g_strEscapeChars.begin(), g_strEscapeChars.end()));
      #endif
      
      size_t uLength = strString.length();
      for (size_t uIndexChar = 0; uIndexChar < uLength; ++uIndexChar)
      {
         if (std::binary_search(g_strEscapeChars.begin(), g_strEscapeChars.end(), strString[uIndexChar]))
         {
            if (strString[uIndexChar] == '&' || strString[uIndexChar] == '|')
            {
               if (strString[uIndexChar] == strString[uIndexChar + 1])
               {
                  strString.insert(uIndexChar++, "\\");
                  ++uIndexChar;
               }
               else
                  continue;
            }
            else
               strString.insert(uIndexChar++, "\\");

            ++uLength;
         }
      }
   }
}

// not faster than ProcessSpecialChar although we are using a binary search... maybe that's due to
// a compiler optimization
// ** 2nd place **  on Windows
const char* ProcessSpecialCharOptimized(const char* pszInputString)
{
   char* pszOutputString = nullptr;
   if (pszInputString != nullptr && *pszInputString)
   {
      // Dynamically allocate a char array which size is the double of the input string (very sufficient) and + 1 for the zero
      pszOutputString = new char[(strlen(pszInputString) * 2) + 1];

      char* pszOutputChar = pszOutputString;
      char* pszInputChar = const_cast<char*>(pszInputString);
      for (; *pszInputChar != '\0'; ++pszInputChar, ++pszOutputChar)
      {
         if (std::binary_search(g_strEscapeChars.cbegin(), g_strEscapeChars.cend(), *pszInputChar))
         {
            if (*pszInputChar == '&' || *pszInputChar == '|')
            {
               if (*pszInputChar == *(pszInputChar + 1))
               {
                  *pszOutputChar++ = '\\';
                  *pszOutputChar++ = *pszInputChar++;
               }
            }
            else
               *pszOutputChar++ = '\\';
         }
         *pszOutputChar = *pszInputChar;
      }
      *pszOutputChar = '\0';
   }
   return pszOutputString;
}

// 4th place  on Windows (big string) - * 3rd place * (small one)
std::string ProcessSpecialCharWithSTLRegex(const char* pszInputString)
{
   // revo solution
   return std::regex_replace(pszInputString, std::regex("[-+!\\\\\"\\[\\](){}^~*?:]|&&|\\|\\|"), "\\$&");
   // Wiktor Stribizew solution
   //return std::regex_replace(pszInputString, std::regex(R"([-+!\\\"\[\](){}^~*?:]|&&|\|\|)"), "\\$&");
}

// * 3rd place *  on Windows (big string) - 4th place (small one)
std::string ProcessSpecialCharWithBoostRegex(const char* pszInputString)
{
   //return boost::regex_replace(std::string(pszInputString), boost::regex("[-+!\\\\\"\\[\\](){}^~*?:]|&&|\\|\\|"), std::string("\\$0"));
   return boost::regex_replace(std::string(pszInputString), boost::regex(R"([-+!\\\"\[\](){}^~*?:]|&&|\|\|)"), std::string("\\\\&"),
      boost::match_default | boost::format_sed);
}


int main()
{

   typedef std::chrono::high_resolution_clock clock;
   typedef std::chrono::duration<float, std::milli> mil;

   //#define ProcessSpecialChar_II
   //#define SMALL_TEST_STRING
   #ifndef SMALL_TEST_STRING
   const char* pszTestString = "ENDRESS+HAUSER*ST-DELL!HP||BESTMATCH&&GOOOOOD\\ABCD[1234567891]"
      "0111216521584515151151515151{MATHS}(ASTRONOMY)RXP^_^****&&~100?100:\\\"TRUMP\"AAAAAAAAAA"
      "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
      "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
      "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
      "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA";
   const char* pszExpectedString = "ENDRESS\\+HAUSER\\*ST\\-DELL\\!HP\\||BESTMATCH\\&&GOOOOOD\\\\ABCD\\[1234567891\\]"
      "0111216521584515151151515151\\{MATHS\\}\\(ASTRONOMY\\)RXP\\^_\\^\\*\\*\\*\\*\\&&\\~100\\?100\\:\\\\\\\"TRUMP\\\"AAAAAAAAAA"
      "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
      "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
      "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
      "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA";
   #else
   const char* pszTestString = "(1+1):2";
   const char* pszExpectedString = "\\(1\\+1\\)\\:2";
   #endif

   
   auto t0 = clock::now();
   
   //#define BOOSTREGEXP
   //#define STLREGEXP
   #ifdef STLREGEXP
   for (int i = 0; i < 1000; ++i)
   {
      std::string strRes = ProcessSpecialCharWithSTLRegex(pszTestString);
      #ifdef _DEBUG
      assert(strRes.compare(pszExpectedString) == 0);
      #endif
   }
   #elif defined BOOSTREGEXP
   for (int i = 0; i < 1000; ++i)
   {
      std::string strRes = ProcessSpecialCharWithBoostRegex(pszTestString);
      #ifdef _DEBUG
      assert(strRes.compare(pszExpectedString) == 0);
      #endif
   }
   #elif defined ProcessSpecialChar_II
   for (int i = 0; i < 1000; ++i)
   {
      #ifdef ProcessInSitu
      std::string strTest(pszTestString);
      StringProcessSpecialCharInSitu(strTest);
      #ifdef _DEBUG
      assert(strTest.compare(pszExpectedString) == 0);
      #endif
      #else
      std::unique_ptr<char[]> pszRes(const_cast<char*>(ProcessSpecialCharOptimized(pszTestString)));
      #ifdef _DEBUG
      assert(strcmp(pszRes.get(), pszExpectedString) == 0);
      #endif
      #endif
   }
   #else
   for (int i = 0; i < 1000; ++i)
   {
      #define SMART_PTR
      #ifdef SMART_PTR
      std::unique_ptr<char[]> pszRes(const_cast<char*>(ProcessSpecialChar(pszTestString)));
      #else
      const char* pszRes = ProcessSpecialChar(pszTestString); // MUST be freed !
      #endif
      
      #ifdef _DEBUG
      #ifdef SMART_PTR
      assert(strcmp(pszRes.get(), pszExpectedString) == 0);
      #else
      assert(strcmp(pszRes, pszExpectedString) == 0);
      #endif
      #endif

      #ifndef SMART_PTR
      delete[] pszRes;
      #endif
   }
   #endif
   auto t1 = clock::now();
   std::cout << mil(t1 - t0).count() << "ms\n";

   return 0;
}