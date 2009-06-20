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

#ifndef TEXT_CONFIG_H_
#define TEXT_CONFIG_H_

#include <ctype.h>

#include <map>
#include <string>
#include <fstream>
#include <cstdlib>
#include <stdexcept>
#include <vector>

#ifndef DISALLOW_COPY_AND_ASSIGN
#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
      TypeName(const TypeName&);           \
      void operator=(const TypeName&)
#endif

/**
 * @mainpage TextConfig
 * @section Introduction
 *
 * TextConfig is a lightweight configuration parser with limited syntax
 * but enough for practice.
 *
 * @section Syntax
 *
 * One line for each configuration item. If a line is started by a '#', it will
 * be treated as a comment line and simplely ignored.
 *
 * Each configuration line separated by a '='. The string before the '='
 * is treated as a key while the one after '=' is its value.
 *
 * Notice that all white-spaces before a key or before the '=' are ignored.
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

/**
 * @brief Configuration Parser
 */

class TextConfig {
  public:
    /** Default Constructor, create a TextConfig object
     *  @param     level    Maximum recusion time for parsing variable
     */
    explicit TextConfig(size_t level = 3): recursion_level_(level) {}

    /** Parse a given configuration file
     *  @param     s        The filename
     *  @param     level    Maximum recusion time for parsing variable
     */
    explicit TextConfig(const char *s, size_t level = 3)
        :recursion_level_(level)
    {
        read_from_file(s);
    }

    /** Add a new line to TextConfig according to its syntax
      * @param   s      configuration string
      * @return  The reference of this object
      */
    TextConfig& operator<< (const char *s)
    {
        insert(std::string(s));
        return *this;
    }

    /** Add a new line to TextConfig according to its syntax
      * @param   str    configuration string
      * @return  The reference of this object
      */
    TextConfig& operator<< (const std::string &str)
    {
        insert(str);
        return *this;
    }

    /** Get a value from parser by given key and convert it to integer
      * @param   key      The Key
      * @param   defval   Default value to be returned if key doesn't exist
      * @return The value of given key or defval if key doesn't exist
      */
    int get_int(const std::string &key, int defval = 0) const
    {
        return (get_value(key))?atoi(reserve_.c_str()):defval;
    }

    /** Get a value from parser by given key and convert it to long integer
      * @param   key      The Key
      * @param   defval   Default value to be returned if key doesn't exist
      * @return The value of given key or defval if key doesn't exist
      */
    long get_long(const std::string &key, long defval = 0) const
    {
        return (get_value(key))?atol(reserve_.c_str()):defval;
    }

    /** Get a value from parser by given key and convert it to
      * long long integer
      * @param   key      The Key
      * @param   defval   Default value to be returned if key doesn't exist
      * @return The value of given key or defval if key doesn't exist
      */
    long get_longlong(const std::string &key, long long defval = 0) const
    {
        return (get_value(key))?atoll(reserve_.c_str()):defval;
    }

    /** Get a value from parser by given key and convert it to double
      * @param   key      The Key
      * @param   defval   Default value to be returned if key doesn't exist
      * @return The value of given key or defval if key doesn't exist
      */
    long get_double(const std::string &key, double defval = 0.0) const
    {
        return (get_value(key))?strtod(reserve_.c_str(), NULL):defval;
    }

    /** Get value from parser by given key as const char *. You
      * need to copy the value immediately. It may changed after
      * next get_* call.
      * @param   key      The Key
      * @param   defval   Default value to be returned if key doesn't exist
      * @return The value of given key or defval if key doesn't exist
      */
    const char* get_cstr(const std::string &key,
                         const char *defval = NULL) const
    {
        return (get_value(key))?reserve_.c_str():defval;
    }

    /** Get a value from parser by given key and cut it into a vector
      * by comma.
      * @param   key      The Key
      * @param   out      Pointer to a std::vector<std::string> object
      *                   used to store the outputs.
      * @return true if key exists, otherwise false.
      */
    void get_vector(const std::string &key, std::vector<std::string> *out)
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

    /** Dump configuration into a std::string
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

    /** Load a configuration by given filename
      * @param    filename    Filename of the configuration
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
    /** Internal Key-Value type */
    typedef std::map<std::string, std::string> item_type;
    /** Maximum Recursion Level */
    size_t recursion_level_;
    /** Internal Key-Value Map */
    item_type map_;
    /** Temporary string */
    mutable std::string reserve_;

    /** Trim a string
      * @param  s     The string
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

    /** check if a char is an identifier
      * @param ch The char to be tested
      * @return true if it is an identifier
      */
    static bool check_identifier(const char ch)
    {
        return (isalnum(ch) || (ch) == '_');
    }

    /** Save the value of given key into reserve_
      * @param    key    The Key
      * @return   true if key exists, otherwise false
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

    /** Insert a new line to TextConfig. Only in memory and will not be
     *  flushed to disk file
     *  @param    s    The line according to syntax
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

    /** Parse a given line
      * @param     str   The line to be parsed
      * @param     out   Pointer to a std::string object used to hold the
      *                  output
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

    /** Parse a given line recursively
      * @param    str   The given line
      * @param    out   Pointer to a std::string object used to hold the
      *                 outputs
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
    DISALLOW_COPY_AND_ASSIGN(TextConfig);
};

#endif  // TEXT_CONFIG_H_

// vim: ts=4 sw=4 et ai cindent
