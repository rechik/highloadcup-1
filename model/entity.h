#ifndef HIGHLOAD_ENTITY_H
#define HIGHLOAD_ENTITY_H

#include <string>
#include <codecvt>
#include <locale>

#include "../utility.h"
#include "../json.h"

using namespace std;

class Entity
{
public:
    virtual bool parse(const json::Object &json, bool require_all) = 0;

    char *json = NULL;

protected:
    static void _fix_unicode(char* str) // HACK for replacing \u sequences
    {
        char* res = str;

        for(int i = 0; str[i] != 0; i++)
        {
            if (str[i] == '\\' && str[i + 1] == 'u')
            {
                auto wchr = wchar_t(_hex_to_dec(str[i + 2]) << 12
                                    | _hex_to_dec(str[i + 3]) << 8
                                    | _hex_to_dec(str[i + 4]) << 4
                                    | _hex_to_dec(str[i + 5]));

                std::wstring_convert<std::codecvt_utf8<wchar_t>> utf8_conv;
                auto s = utf8_conv.to_bytes(wchr);
                *res++ = s[0];
                *res++ = s[1];
                i += 5;
            }
            else
            {
                *res++ = str[i];
            }
        }
        *res = 0;
    }

    static int _hex_to_dec(char chr)
    {
        return isdigit(chr) != 0 ? chr - '0' : (chr - 'a') + 10;
    }
};

#define TRY_PARSE_COMPARE_FIELD(field)\
        if (memcmp(key, #field, sizeof(#field)) == 0)

#define TRY_PARSE_SET_HAS(field)\
        has_ ## field = true;\
        props_count++;\

#define TRY_PARSE_INT_VAL(field)\
        bool has_ ## field = false;\
        TRY_PARSE_COMPARE_FIELD(field) {\
            if (val.GetType() != json::IntVal) return false;\
            field = (int) val;\
            TRY_PARSE_SET_HAS(field);\
        }

#define TRY_PARSE_STR_VAL(field)\
        bool has_ ## field = false;\
        TRY_PARSE_COMPARE_FIELD(field) {\
            if (val.GetType() != json::StringVal) return false;  \
            field = (char*) val;\
            TRY_PARSE_SET_HAS(field)\
        }

#define TRY_PARSE_GENDER_VAL(field)\
        bool has_ ## field = false;\
        TRY_PARSE_COMPARE_FIELD(field) {\
            if (val.GetType() != json::StringVal) return false;  \
            auto _str = (const char*) val;\
            if (_str[0] == 0 || _str[1] != 0 || _str[0] != 'm' && _str[0] != 'f') return false;\
            field = _str[0] == 'm' ? Male : Female;\
            TRY_PARSE_SET_HAS(field)\
        }

#define TRY_PARSE_COPY_JSON(json, buf)\
        auto json_len = strlen(buf);\
        /*delete json; memory leak*/\
        json = new char[json_len + 1];\
        memcpy(json, buf, json_len);\
        json[json_len] = 0;

#endif //HIGHLOAD_ENTITY_H
