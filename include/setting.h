/*
 * Copyright (c) 2009, Jianing Yang<jianingy.yang@gmail.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * The names of its contributors may not be used to endorse or promote
 *       products derived from this software without specific prior written
 *       permission.
 *
 * THIS SOFTWARE IS PROVIDED BY detrox@gmail.com ''AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL detrox@gmail.com BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#ifndef SETTING_H_
#define SETTING_H_

#include <ctype.h>

#include <map>
#include <string>
#include <fstream>
#include <cstdlib>
#include <stdexcept>
#include <vector>

#define BEGIN_SETTING_NAMESPACE namespace dutil {
#define END_SETTING_NAMESPACE }

#ifndef DISALLOW_COPY_AND_ASSIGN
#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
      TypeName(const TypeName&);           \
      void operator=(const TypeName&)
#endif

/**
 * @mainpage libsetting
 *
 * @section Introduction
 *
 * libsetting is a lightweight configuration parser with limited syntax
 * but enough in practice.
 *
 * @section Syntax
 *
 * One line for each configuration item. If a line is started with a '#',
 * it will be treated as a comment line and simplely be ignored.
 *
 * Each configuration line separated by a '='. The string before the '='
 * is treated as a key while the one after '=' is its value.
 *
 * Notice that all white-spaces before a key or around the '=' are ignored.
 *
 * For example,
 *
 * @code
 * core.alpha = 0.05
 * core.beta = 0.2
 * core.id = HU7321
 * core.start = 12:30
 * @endcode
 */


BEGIN_SETTING_NAMESPACE

/** @addtogroup setting_api libsetting API
 *
 *  @{ The libsetting's API
 */

/// Setting parser.
class setting {
  public:
    /**
     * Constructs a setting.
     *
     * @param level Maximum recusion times for parsing variable
     */
    explicit setting(size_t level = 3): recursion_level_(level) {}

    /**
     * Constructs a setting with a configuration file.
     *
     * @param s        The filename.
     * @param level    Maximum recusion time for parsing variable.
     */
    explicit setting(const char *s, size_t level = 3)
        :recursion_level_(level)
    {
        read_from_file(s);
    }

    /**
     * Adds a extra line.
     *
     * @param s Text according to @see Syntax.
     * @return A instance of setting.
     */
    setting& operator<< (const char *s)
    {
        insert(std::string(s));
        return *this;
    }

    /**
     * Adds an extra line.
     *
     * @param str Text according to @see Syntax.
     * @return A instance of setting.
     */
    setting& operator<< (const std::string &str)
    {
        insert(str);
        return *this;
    }

    /**
     * Gets a value using key and convert it to integer.
     *
     * @param   key      The Key.
     * @param   defval   Default value to be returned if key doesn't exist.
     * @return The value of given key or defval if key doesn't exist.
     */
    int get_int(const std::string &key, int defval = 0) const
    {
        return (get_value(key))?atoi(reserve_.c_str()):defval;
    }

    /**
     * Gets a value using key and convert it to long integer.
     *
     * @param   key      The Key.
     * @param   defval   Default value to be returned if key doesn't exist.
     * @return The value of given key or defval if key doesn't exist.
     */
    long get_long(const std::string &key, long defval = 0) const
    {
        return (get_value(key))?atol(reserve_.c_str()):defval;
    }

    /**
     * Gets a value using key and convert it to long long integer.
     *
     * @param   key      The Key.
     * @param   defval   Default value to be returned if key doesn't exist.
     * @return The value of given key or defval if key doesn't exist.
     */
    long get_longlong(const std::string &key, long long defval = 0) const
    {
        return (get_value(key))?atoll(reserve_.c_str()):defval;
    }

    /**
     * Gets a value using key and convert it to double.
     *
     * @param   key      The Key.
     * @param   defval   Default value to be returned if key doesn't exist.
     * @return The value of given key or defval if key doesn't exist.
     */
    long get_double(const std::string &key, double defval = 0.0) const
    {
        return (get_value(key))?strtod(reserve_.c_str(), NULL):defval;
    }

    /**
     * Gets a value using key and conver it to c-style string. You
     * need to copy the value immediately. It may changed after
     * next get_* call.
     *
     * @param   key      The Key.
     * @param   defval   Default value to be returned if key doesn't exist.
     * @return The value of given key or defval if key doesn't exist.
     */
    const char* get_cstr(const std::string &key,
                         const char *defval = NULL) const
    {
        return (get_value(key))?reserve_.c_str():defval;
    }

    /** Gets a value using key and splitted it into a vector by comma.
      *
      * @param   key      The Key.
      * @param   out      Pointer to a std::vector<std::string> object
      *                   used to store the outputs.
      * @return true if key exists, otherwise false.
      */
    bool get_vector(const std::string &key, std::vector<std::string> *out)
    {
        std::string::size_type break_pos, pos;
        std::string s;

        if (get_value(key) == false)
            return false;
        for (pos = 0; pos < reserve_.npos; pos = break_pos + 1) {
            break_pos = reserve_.find(',', pos);
            if (break_pos == reserve_.npos)
                break_pos = reserve_.npos - 1;
            trim(reserve_.substr(pos, break_pos - pos), &s);
            if (!s.empty())
                out->push_back(s);
        }
        return true;
    }

    /**
     * Dumps configuration text.
     *
     * @param    out    Pointer to a std::string object used to store
     *                  the outputs.
     */
    void dump(std::string *out) const
    {
        std::map<std::string, std::string>::const_iterator it;
        out->clear();

        for (it = map_.begin(); it != map_.end(); it++) {
            out->append(it->first).append(" = ");
            out->append(it->second).append("\n");
        }
    }

    /**
     * Loads a configuration.
     *
     * @param filename Filename of the configuration.
     */
    void read_from_file(const char *filename)
    {
        std::ifstream ifs(filename);
        if (ifs.is_open()) {
            std::string line, trimmed_line;
            while (!(std::getline(ifs, line).eof())) {
                trim(line, &trimmed_line);
                if (!trimmed_line.empty() && trimmed_line[0] != '#')
                    insert(trimmed_line);
            }
        } else {
            throw std::runtime_error(
                    std::string("can not open configuration file ") +
                    std::string(filename) + std::string("."));
        }
    }

  protected:
    /** Internal Key-Value type. */
    typedef std::map<std::string, std::string> item_type;
    /** Maximum Recursion Level. */
    size_t recursion_level_;
    /** Internal Key-Value Map. */
    item_type map_;
    /** Temporary string. */
    mutable std::string reserve_;

    /**
     * Trims a string, removes its heading nand trailing white-spaces.
     *
     * @param  s     The string.
     * @param  out   Pointer to a std::string object to hold the trimmed
     *               string.
     */
    static void trim(const std::string &s, std::string *out)
    {
        static const char       whitespace[] = " \t\r\n";
        std::string::size_type  begin;
        std::string::size_type  end;

        out->clear();
        begin = s.find_first_not_of(whitespace);
        if (begin == s.npos)
            return;
        end = s.find_last_not_of(whitespace);
        *out = s.substr(begin, end + 1);
    }

    /**
     * Tests if ch is an identifier.
     *
     * @param ch The char to be tested.
     * @return true if it is an identifier.
     */
    static bool check_identifier(const char ch)
    {
        return (isalnum(ch) || (ch) == '_');
    }

    /**
     * Gets a value into reserve_ using key.
     *
     * @param    key    The Key.
     * @return   true if key exists, otherwise false.
     */
    bool get_value(const std::string &key) const
    {
        item_type::const_iterator found = map_.find(key);
        if (found != map_.end()) {
            parse_recursive(found->second, &reserve_);
            return true;
        }
        return false;
    }

    /**
     * Adds an extra line to setting.
     *
     * @param s Text according to @see Syntax.
     * @return A instance of setting.
     */
    void insert(const std::string &s)
    {
        int break_pos = s.find("=");
        std::string key;
        std::string value;

        trim(s.substr(0, break_pos), &key);
        trim(s.substr(break_pos + 1), &value);

        if (!key.empty())
            map_[key] = value;
    }

    /**
     * Parses a string.
     *
     * @param     str   The string.
     * @param     out   Pointer to a std::string object used to hold the
     *                  output.
     */
    void parse_once(const std::string &str, std::string *out) const
    {
        enum {
            PS_UNKNOW = 0,
            PS_ESCAPE,
            PS_DOLLAR,
            PS_FETCHKEY,
            PS_REPLACE,
            PS_FINISH,
            PS_REPLACE_FINISH,
        } state = PS_UNKNOW;

        bool brace_open = false;
        std::string key;

        out->clear();
        for (const char *pch = str.c_str(); /* Forever */ ; pch++) {
            if (*pch == '\\')
                state = PS_ESCAPE;
            else if (*pch == '\0')
                state = (state == PS_FETCHKEY)?PS_REPLACE_FINISH:PS_FINISH;
            else if (*pch == '$' && state != PS_ESCAPE)
                state = PS_DOLLAR;
            else if (state == PS_DOLLAR)
                state = PS_FETCHKEY;
            else if (state == PS_FETCHKEY &&
                    !check_identifier(*pch) && *pch != '{')
                state = PS_REPLACE;
            else if (state != PS_FETCHKEY)
                state = PS_UNKNOW;

            if (state == PS_UNKNOW) {
                out->append(pch, 1);
            } else if (state == PS_FETCHKEY) {
                if (check_identifier(*pch))
                    key.append(pch, 1);
                if (*pch == '{')
                    brace_open = true;
            } else if (state == PS_REPLACE || state == PS_REPLACE_FINISH) {
                item_type::const_iterator found = map_.find(key);
                if (found != map_.end()) {
                    out->append(found->second);
                    key.clear();
                }
                if (!(brace_open && *pch == '}'))
                    pch--;
            }
            if (state == PS_FINISH || state == PS_REPLACE_FINISH)
                break;
        }
    }

    /**
     * Parses a string recursively.
     *
     * @param    str   The string.
     * @param    out   Pointer to a std::string object used to hold the
     *                 outputs.
     */
    void parse_recursive(const std::string &str, std::string *out) const
    {
        std::string lhs(str), rhs;

        for (size_t i = 0;
             i < recursion_level_ && str.find('$') < str.npos;
             i++) {
            parse_once(lhs, &rhs);
            lhs = rhs;
        }
        *out = lhs;
    }

  private:
    DISALLOW_COPY_AND_ASSIGN(setting);
};

/** @} */

END_SETTING_NAMESPACE

#endif  // SETTING_H_

// vim: ts=4 sw=4 et ai cindent
