#pragma once

#include <string>
#include <locale>
#include <codecvt>


namespace vanilla {

    template <typename T>
    std::string to_utf8(const std::basic_string<T, std::char_traits<T>, std::allocator<T>>& source)
    {
        std::wstring_convert<std::codecvt_utf8_utf16<T>, T> convertor;
        std::string result = convertor.to_bytes(source);
        return result;
    }

    template <typename T>
    void from_utf8(const std::string& source, std::basic_string<T, std::char_traits<T>, std::allocator<T>>& result)
    {
        std::wstring_convert<std::codecvt_utf8_utf16<T>, T> convertor;
        result = convertor.from_bytes(source);
    }

    class unicode_string {
    public:
        using  iterator = std::u32string::iterator ;
        using  const_iterator = std::u32string::const_iterator ;

        unicode_string()
        {}

        explicit unicode_string(const std::string &source) {
            from_utf8(source, data_);
        }

        unicode_string(const unicode_string &other)
            : data_(other.data_)
        {}

        unicode_string(unicode_string &&other) noexcept
        {
            *this = std::move(other);
        }

        unicode_string &operator=(const unicode_string &other)
        {
            std::swap(unicode_string(other).data_, data_);
            return *this;
        }

        unicode_string &operator=(unicode_string &&other) noexcept
        {
            data_ = std::move(other.data_);
            return *this;
        }

        unicode_string &operator=(const std::string &other) {
            from_utf8(other, data_);
            return *this;
        }

        bool empty() const {
            return data_.empty();
        }

        iterator begin() {
            return data_.begin();
        }

        const_iterator begin() const {
            return data_.begin();
        }

        iterator end() {
            return data_.end();
        }

        const_iterator end() const {
            return data_.end();
        }

        const std::u32string &data() const {
            return data_;
        }

        std::string to_utf8()  {
            return vanilla::to_utf8(data_);
        }

        const char32_t *c_str() const {
            return data_.c_str();
        }

        const size_t length() const {
            return data_.length();
        }


        unicode_string &operator+=(const std::string &other) {
            std::u32string u32other;
            from_utf8(other, u32other);
            data_ += u32other;
            return *this;
        }

        unicode_string &operator+=(const unicode_string &other) {
            data_ += other.data_;
            return *this;
        }

        bool operator==(const unicode_string &other) const {
            return data_ == other.data_;
        }

        bool operator!=(const unicode_string &other) const {
            return !operator==(other);
        }

        bool operator<(const unicode_string &other) const {
            return data_ < other.data_;
        }

    private:
        std::u32string data_;
    };

}

