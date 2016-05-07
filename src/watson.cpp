/*!
 \file watson/watson.cpp
 \brief WatSON format implementation.
 \author Jason Watson

 Copyright (c) 2015, Jason Watson
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright notice,
 this list of conditions and the following disclaimer.

 * Redistributions in binary form must reproduce the above copyright notice,
 this list of conditions and the following disclaimer in the documentation
 and/or other materials provided with the distribution.

 * Neither the name of the LogJammin nor the names of its contributors
 may be used to endorse or promote products derived from this software
 without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.
 */

#include "watson.h"
#include "snappy.h"
#include <cassert>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <utility>

namespace watson
{
    // ----------------------------------------------------------------
    // Ngrdnt Class
    // ----------------------------------------------------------------

    Ngrdnt::Ngrdnt() :
            ptr_(new uint8_t[1]),
            data_(ptr_.get()),
            parent_(nullptr)
    {
        ptr_[0] = ::watson::type_marker(Size_type::k_zero,
                Ngrdnt_type::k_null);
    }

    Ngrdnt::Ngrdnt(const Ngrdnt& o) :
            ptr_(new uint8_t[o.size()]),
            data_(ptr_.get()),
            parent_(o.parent_)
    {
        std::memcpy(ptr_.get(), o.data(), o.size());
    }

    Ngrdnt::Ngrdnt(const std::uint8_t* d) :
            ptr_(nullptr),
            data_(d),
            parent_(nullptr)
    {
    }

    Ngrdnt::Ngrdnt(std::unique_ptr<uint8_t[]>&& bytes,
            const Ngrdnt::Ptr& p) :
            ptr_(std::move(bytes)),
            data_(ptr_.get()),
            parent_(p)
    {
    }

    Ngrdnt& Ngrdnt::operator=(const Ngrdnt& rhs)
    {
        // Implemented via copy constructor
        (*this) = Ngrdnt(rhs);
        return *this;
    }

    Ngrdnt& Ngrdnt::operator=(Ngrdnt&& rhs)
    {
        std::swap(ptr_, rhs.ptr_);
        std::swap(parent_, rhs.parent_);
        data_ = rhs.data_;
        return *this;
    };

    uint64_t Ngrdnt::size() const
    {
        uint64_t sz;
        switch (size_type(type_marker()))
        {
            case Size_type::k_zero:
                sz = 1;
                break;
            case Size_type::k_one:
                sz = data()[1];
                break;
            case Size_type::k_two:
                sz = *(reinterpret_cast<const uint16_t*>(data() + 1));
                break;
            default:
                sz = *(reinterpret_cast<const uint64_t*>(data() + 1));
                break;
        };
        return sz;
    }

    // ----------------------------------------------------------------
    // A null ingredient, but the pointer is special. 
    // ----------------------------------------------------------------

    const Ngrdnt::Ptr k_not_found = Ngrdnt::make();

    // ----------------------------------------------------------------
    // Simple Ngrdnt
    // ----------------------------------------------------------------

    namespace
    {
        template <Ngrdnt_type IT>
        inline std::unique_ptr<uint8_t[]> build_ngrdnt(const uint64_t data_size,
                uint8_t** current)
        {
            const Size_type st = size_type_necessary(data_size);
            const int header_size = ngrdnt_header_size(st);
            const uint64_t full_size = data_size + header_size;
            std::unique_ptr<uint8_t[]> ptr(new uint8_t[full_size]);
            *current = ptr.get();

            // type marker.
            **current = type_marker(st, IT);
            (*current)++;

            // Size
            if (0 < data_size)
            {
                memcpy(*current, &full_size, size_size(st));
                (*current) += size_size(st);
            }

            return ptr;
        }

        template <Ngrdnt_type IT>
        inline Ngrdnt::Ptr copy_to_ngrdnt(const uint64_t data_size,
                const void* data)
        {
            uint8_t* current;
            std::unique_ptr<uint8_t[]> ptr(build_ngrdnt<IT>(data_size, &current));

            if (0 < data_size)
            {
                memcpy(current, data, data_size);
            }

            return Ngrdnt::adopt(std::move(ptr));
        }

        template <typename NT, Ngrdnt_type IT>
        inline const NT* ngrdnt_data(const Ngrdnt::Ptr& val)
        {
            const NT* result = nullptr;
            if (IT == ngrdnt_type(val->type_marker()))
            {
                const size_t header_size = ngrdnt_header_size(val->type_marker());
                result = reinterpret_cast<const NT*>(val->data() + header_size);
            }
            return result;
        }
    }; // namespace watson::(anonymous)

    Ngrdnt::Ptr new_ngrdnt()
    {
        return Ngrdnt::make();
    }

    Ngrdnt::Ptr new_ngrdnt(const std::string& val)
    {
        return copy_to_ngrdnt<Ngrdnt_type::k_string>(val.size(), val.c_str());
    }

    Ngrdnt::Ptr new_ngrdnt(const bool val)
    {
        if (val)
        {
            return copy_to_ngrdnt<Ngrdnt_type::k_true>(0, nullptr);
        }
        return copy_to_ngrdnt<Ngrdnt_type::k_false>(0, nullptr);
    }


    Ngrdnt::Ptr new_ngrdnt(const double val)
    {
        return copy_to_ngrdnt<Ngrdnt_type::k_float>(sizeof(double), &val);
    }

    Ngrdnt::Ptr new_ngrdnt(const int32_t val)
    {
        return copy_to_ngrdnt<Ngrdnt_type::k_int32>(sizeof(int32_t), &val);
    }

    Ngrdnt::Ptr new_ngrdnt(const int64_t val)
    {
        return copy_to_ngrdnt<Ngrdnt_type::k_int64>(sizeof(int64_t), &val);
    }

    Ngrdnt::Ptr new_ngrdnt(const uint64_t val)
    {
        return copy_to_ngrdnt<Ngrdnt_type::k_uint64>(sizeof(uint64_t), &val);
    }

    Ngrdnt::Ptr new_ngrdnt(const std::vector<bool>& val)
    {
        uint64_t data_size = (val.size() / 8) + (val.size() % 8 == 0 ? 0 : 1);

        std::unique_ptr<uint8_t[]> ptr(new uint8_t[data_size]);
        memset(ptr.get(), 0, data_size);

        for (int h = 0; h < val.size(); ++h)
        {
            if (val[h])
            {
                const uint8_t offset = h % 8;
                const uint8_t indx = h >> 3;
                const uint8_t flag = 1;

                ptr[indx] |= flag << offset;
            }
        }

        return copy_to_ngrdnt<Ngrdnt_type::k_flags>(data_size, ptr.get());
    }

    bool is_null(const Ngrdnt::Ptr& val)
    {
        return ngrdnt_type(val->type_marker()) == Ngrdnt_type::k_null;
    }

    bool to_bool(const Ngrdnt::Ptr& val)
    {
        bool result = false;
        switch (ngrdnt_type(val->type_marker()))
        {
            case Ngrdnt_type::k_null:
            case Ngrdnt_type::k_false:
                result = false;
                break;
            case Ngrdnt_type::k_int32:
                result = to_int32(val) ? true : false;
                break;
            case Ngrdnt_type::k_int64:
                result = to_int64(val) ? true : false;
                break;
            case Ngrdnt_type::k_uint64:
                result = to_uint64(val) ? true : false;
                break;
            default:
                result = true;
                break;
        }
        return result;
    }

    double to_double(const Ngrdnt::Ptr& val)
    {
        auto result = ngrdnt_data<double, Ngrdnt_type::k_float>(val);
        return result ? *result : 0.0f;
    }

    int32_t to_int32(const Ngrdnt::Ptr& val)
    {
        auto result = ngrdnt_data<int32_t, Ngrdnt_type::k_int32>(val);
        return result ? *result : 0;
    }

    int64_t to_int64(const Ngrdnt::Ptr& val)
    {
        auto result = ngrdnt_data<int64_t, Ngrdnt_type::k_int64>(val);
        return result ? *result : 0;
    }

    uint64_t to_uint64(const Ngrdnt::Ptr& val)
    {
        auto result = ngrdnt_data<uint64_t, Ngrdnt_type::k_uint64>(val);
        return result ? *result : 0;
    }

    std::vector<bool> to_flags(const Ngrdnt::Ptr& val)
    {
        const size_t header_size = ngrdnt_header_size(val->type_marker());
        const uint64_t flag_count = (val->size() - header_size) * 8;
        std::vector<bool> result(flag_count);

        for (int h = 0; h < flag_count; ++h)
        {
            const uint8_t offset = h % 8;
            const uint8_t indx = h >> 3;
            const uint8_t flag = 1 << offset;

            result[h] = val->data()[indx + header_size] & flag;
        }

        return result;
    }

    std::string to_string(const Ngrdnt::Ptr& val)
    {
        size_t header_size;
        std::string result;

        switch(ngrdnt_type(val->type_marker()))
        {
            case Ngrdnt_type::k_null:
                result = "null";
                break;
            case Ngrdnt_type::k_true:
                result = "true";
                break;
            case Ngrdnt_type::k_false:
                result = "false";
                break;
            case Ngrdnt_type::k_float:
                result = std::to_string(to_double(val));
                break;
            case Ngrdnt_type::k_int32:
                result = std::to_string(to_int32(val));
                break;
            case Ngrdnt_type::k_int64:
                result = std::to_string(to_int64(val));
                break;
            case Ngrdnt_type::k_uint64:
                result = std::to_string(to_uint64(val));
                break;
            case Ngrdnt_type::k_string:
                header_size = ngrdnt_header_size(val->type_marker());
                result = std::string(reinterpret_cast<const char*>(val->data() + header_size),
                        val->size() - header_size);
                break;
            default:
                break;
        };
        return result;
    }

    std::string to_dump(const Ngrdnt::Ptr& val)
    {
        std::ostringstream oss;
        std::hex(oss);
        oss.fill('0');
        oss << "0x[" << static_cast<const unsigned>(val->type_marker()) <<
                "={ " << std::setw(2) <<
                static_cast<const unsigned>(size_type(val->type_marker()));
        oss << ' ' << std::setw(2) <<
                static_cast<const unsigned>(ngrdnt_type(val->type_marker()));
        oss << " } {";

        uint64_t sz = val->size();
        for (int h = 0; h < size_size(size_type(val->type_marker())); ++h)
        {
            oss << ' ' << std::setw(2) <<
                    static_cast<const unsigned>(*(reinterpret_cast<uint8_t*>(&sz) + h));
        }
        oss << " }";
        for (int h = ngrdnt_header_size(val->type_marker()); h < val->size(); ++h)
        {
            oss << ' ' << std::setw(2) << static_cast<const unsigned>(val->data()[h]);
        }
        oss << ']';
        return oss.str();
    }


    // ----------------------------------------------------------------
    // Compressed class
    // ----------------------------------------------------------------

    Compressed::Compressed() :
            child_(Ngrdnt::make())
    {
    }

    Compressed::Compressed(Ngrdnt::Ptr&& raw) :
            child_(std::move(raw))
    {
    }

    Compressed::Compressed(const Ngrdnt::Ptr& raw)
    {
        const size_t header_size = ngrdnt_header_size(raw->type_marker());
        const uint8_t* data = raw->data() + header_size;
        const size_t data_size = raw->size() - header_size;

        size_t output_size;
        bool result = snappy::GetUncompressedLength(reinterpret_cast<const char*>(data),
                    data_size,
                    &output_size);
        assert(result);

        std::unique_ptr<uint8_t[]> output(new uint8_t[output_size]);
        result = snappy::RawUncompress(reinterpret_cast<const char*>(data),
                data_size,
                reinterpret_cast<char*>(output.get()));
        assert(result);

        child_ = Ngrdnt::adopt(std::move(output));
    }


    // ----------------------------------------------------------------
    // Map class
    // ----------------------------------------------------------------

    Map::Map(Map::Children&& c) :
            children_(std::move(c))
    {
    }

    Map::Map(const Ngrdnt::Ptr& raw) :
            Map()
    {
        const size_t header_size = ngrdnt_header_size(raw->type_marker());
        const uint8_t* ptr = raw->data() + header_size;
        const uint8_t* const end = raw->data() + raw->size();
        assert(end >= ptr);

        while (end > ptr)
        {
            // Read the key.
            Children::key_type key = *reinterpret_cast<const Children::key_type*>(ptr);
            ptr += sizeof(Children::key_type);

            // Store the value.
            children_.insert(Children::value_type(key,
                    Ngrdnt::clone(ptr)));

            // Advance the ptr.
            ptr += Ngrdnt::temp(ptr)->size();
        }
    }

    const Ngrdnt::Ptr& Map::operator[](uint32_t key) const
    {
        auto iter = children().find(key);
        if (iter == children().end())
        {
            return k_not_found;
        }
        return iter->second;
    }

    // ----------------------------------------------------------------
    // Bytes class
    // ----------------------------------------------------------------

    Bytes::Bytes() :
            size_(0),
            ptr_(new uint8_t[size_ + sizeof(uint32_t)]),
            marshal_hint_(reinterpret_cast<uint32_t*>(ptr_.get())),
            data_(ptr_.get() + sizeof(uint32_t))
    {
        memset(ptr_.get(), 0, sizeof(uint32_t));
    }

    Bytes::Bytes(const Bytes& o) :
            size_(o.size()),
            ptr_(new uint8_t[size_ + sizeof(uint32_t)]),
            marshal_hint_(reinterpret_cast<uint32_t*>(ptr_.get())),
            data_(ptr_.get() + sizeof(uint32_t))
    {
        *reinterpret_cast<uint32_t*>(ptr_.get()) = o.marshal_hint();
        memcpy(ptr_.get() + sizeof(uint32_t), o.data(), size_);
    }

    Bytes::Bytes(std::unique_ptr<uint8_t[]>&& v,
            uint64_t v_sz) :
            size_(v_sz - 4),
            ptr_(std::move(v)),
            marshal_hint_(reinterpret_cast<uint32_t*>(ptr_.get())),
            data_(ptr_.get() + sizeof(uint32_t))
    {
    }

    Bytes::Bytes(const Ngrdnt::Ptr& raw) :
            size_(raw->size() - ngrdnt_header_size(raw->type_marker()) - sizeof(uint32_t)),
            ptr_(new uint8_t[size_ + sizeof(uint32_t)]),
            marshal_hint_(reinterpret_cast<uint32_t*>(ptr_.get())),
            data_(ptr_.get() + sizeof(uint32_t))
    {
        const size_t header_size = ngrdnt_header_size(raw->type_marker());
        const uint8_t* ptr = raw->data() + header_size;

        memcpy(ptr_.get(), ptr, size_ + sizeof(uint32_t));
    }

    Bytes& Bytes::operator=(const Bytes& rhs)
    {
        (*this) = Bytes(rhs);
        return (*this);
    }

    Bytes::Bytes(const uint64_t sz, const uint32_t* mh, const uint8_t* const d) :
            size_(sz),
            ptr_(nullptr),
            marshal_hint_(mh),
            data_(d)
    {
    }

    // ----------------------------------------------------------------
    // Native to WatSON methods.
    // ----------------------------------------------------------------

    Ngrdnt::Ptr new_ngrdnt(const Container& val)
    {
        // Calculate the necessary size first.
        uint64_t sz = 0;
        for (auto ing : val.children())
        {
            sz += ing->size();
        }

        uint8_t* current;
        std::unique_ptr<uint8_t[]> ptr(build_ngrdnt<Ngrdnt_type::k_container>(sz, &current));
        const uint8_t* const end = current + sz;

        for (auto ing : val.children())
        {
            assert(end > current);

            memcpy(current, ing->data(), ing->size());
            current += ing->size();
        }
        return Ngrdnt::adopt(std::move(ptr));
    }

    Ngrdnt::Ptr new_ngrdnt(const Library& val)
    {
        std::vector<Ngrdnt::Ptr> cache(val.size());
        uint64_t sz = 0;
        for (uint32_t h = 0; h < val.size(); ++h)
        {
            cache[h] = new_ngrdnt(val[h]);
            sz += cache[h]->size();
        }

        uint8_t* current;
        std::unique_ptr<uint8_t[]> ptr(build_ngrdnt<Ngrdnt_type::k_library>(sz, &current));
        const uint8_t* const end = current + sz;

        for (auto r : cache)
        {
            assert(end > current);

            memcpy(current, r->data(), r->size());
            current += r->size();
        }

        return Ngrdnt::adopt(std::move(ptr));
    }

    Ngrdnt::Ptr new_ngrdnt(const Compressed& val)
    {
        uint64_t sz = snappy::MaxCompressedLength(val->size());

        std::unique_ptr<char[]> buffer(new char[sz]);

        snappy::RawCompress(reinterpret_cast<const char*>(val->data()),
                val->size(),
                reinterpret_cast<char*>(buffer.get()),
                reinterpret_cast<size_t*>(&sz));

        // I was originally not copying this. But I was noticing that Max
        // Compressed Length can be pretty big. This doesn't use Ngrdnt::clone
        // because the initial data isn't loaded into a Ngrdnt.
        uint8_t* current;
        std::unique_ptr<uint8_t[]> ptr(build_ngrdnt<Ngrdnt_type::k_zip>(sz, &current));
        memcpy(current, buffer.get(), sz);

        return Ngrdnt::adopt(std::move(ptr));
    }

    Ngrdnt::Ptr new_ngrdnt(const Map& val)
    {
        uint64_t sz = 0;
        for (auto h : val.children())
        {
            sz += h.second->size() + 4;
        }
        
        uint8_t* current;
        std::unique_ptr<uint8_t[]> ptr(build_ngrdnt<Ngrdnt_type::k_map>(sz, &current));
        const uint8_t* const end = current + sz;

        for (auto h : val.children())
        {
            assert(end > current);

            *reinterpret_cast<uint32_t*>(current) = h.first;
            current += sizeof(uint32_t);

            memcpy(current, h.second->data(), h.second->size());
            current += h.second->size();
        }
        return Ngrdnt::adopt(std::move(ptr));
    }

    Ngrdnt::Ptr new_ngrdnt(const Bytes& val)
    {
        uint64_t sz = val.size() + sizeof(uint32_t);

        uint8_t* current;
        std::unique_ptr<uint8_t[]> ptr(build_ngrdnt<Ngrdnt_type::k_binary>(sz, &current));

        *reinterpret_cast<uint32_t*>(current) = val.marshal_hint();
        current += sizeof(uint32_t);

        memcpy(current, val.data(), val.size());

        return Ngrdnt::adopt(std::move(ptr));
    }

}; // namespace watson


// ----------------------------------------------------------------
// WatSON stream methods.
// ----------------------------------------------------------------

namespace
{
    bool successful_read_loop(std::istream& is, char* target, size_t sz)
    {
        int read_bytes = 0;
        while (read_bytes < sz && is.good())
        {
            is.read(target + read_bytes, sz - read_bytes);
            read_bytes += is.gcount();
        }
        return is.good();
    }
}; // namespace (anonymous)

std::istream& operator>>(std::istream& is, watson::Ngrdnt::Ptr& val)
{
    char buffer[9];
    memset(buffer, 0, 9);
    if (!successful_read_loop(is, buffer, 1))
    {
        return is;
    }

    watson::Size_type st = watson::size_type(*buffer);

    if (!successful_read_loop(is, buffer + 1, watson::size_size(st)))
    {
        throw std::ios_base::failure("Unable to read the WatSON Size from the input stream.");
    }
    uint64_t sz = (watson::Size_type::k_zero == st) ? 1 : *reinterpret_cast<uint64_t*>(buffer + 1);
    assert(sz > watson::size_size(st));
    const uint64_t offset = watson::ngrdnt_header_size(st);

    std::unique_ptr<uint8_t[]> data(new uint8_t[sz]);
    char* ptr = reinterpret_cast<char*>(data.get());
    memcpy(ptr, buffer, offset);
    ptr += offset;
    sz -= offset;

    if (!successful_read_loop(is, ptr, sz))
    {
        throw std::ios_base::failure("Unable to read the WatSON Element data from the input stream.");
    }
    
    val = watson::Ngrdnt::adopt(std::move(data));

    return is;
}

std::ostream& operator<<(std::ostream& os, const watson::Ngrdnt::Ptr& val)
{
    os.write(reinterpret_cast<const char*>(val->data()), val->size());
    return os;
}

namespace watson
{


    // ----------------------------------------------------------------
    // WatSON Glossary methods.
    // ----------------------------------------------------------------

    Glossary::Glossary(const Library& l) :
        names(l.size())
    {
        for (int h = 0; h < l.size(); ++h)
        {
            names[h] = l[h];
            index[l[h]] = h;
        }
    }

    std::list<uint32_t> xlate(const Glossary& g, const std::list<std::string>& names)
    {
        std::list<uint32_t> retval;

        for (auto name : names) {
            auto iter = g.index.find(name);
            retval.push_back(iter != g.index.end() ? iter->second : 0);
        }
        return retval;
    }

    std::list<std::string> xlate(const Glossary& g, const std::list<uint32_t>& keys)
    {
        std::list<std::string> retval;

        for (auto key : keys) {
            retval.push_back(key < g.names.size() ? g.names[key] : "");
        }
        return retval;
    }


    // ----------------------------------------------------------------
    // WatSON Recipe methods.
    // ----------------------------------------------------------------

    Recipe::Recipe(Ngrdnt::Ptr&& c)
    {
        if (Ngrdnt_type::k_container == ngrdnt_type(c->type_marker()))
        {
            container_ = Container(Ngrdnt::clone(c));
        }
        else
        {
            container_.mutable_children().push_back(std::move(c));
        }

        for (const auto child : container_.children())
        {
            if (Ngrdnt_type::k_library == ngrdnt_type(child->type_marker()))
            {
                glossary_ = Glossary(Library(child));
                break;
            }
        }
    }

    Recipe::Recipe(const Ngrdnt::Ptr& raw) :
        Recipe(Ngrdnt::clone(raw))
    {
    }

    const Ngrdnt::Ptr Recipe::ngrdnt(const std::list<uint32_t>& steps) const
    {
        if (steps.empty()) {
            return k_not_found;
        }

        auto iter = steps.begin();
        Ngrdnt::Ptr retval = container_[*iter];

        for (++iter; iter != steps.end();)
        {
            switch (ngrdnt_type(retval->type_marker()))
            {
                case Ngrdnt_type::k_container:
                    {
                        Container tmp(retval);
                        if (*iter >= tmp.size())
                        {
                            return k_not_found;
                        }
                        retval = tmp[*iter];
                        ++iter;
                    }
                    break;
                case Ngrdnt_type::k_map:
                    retval = Map(retval)[*iter];
                    ++iter;
                    break;
                case Ngrdnt_type::k_zip:
                    retval = *Compressed(retval);
                    break;
                default:
                    return k_not_found;
                    break;
            }
        }
        return retval;
    }

    Recipe Recipe::recipe(const std::list<uint32_t>& steps) const
    {
        Recipe retval(ngrdnt(steps));

        if (retval.glossary().empty() && !glossary().empty())
        {
            retval.glossary_ = glossary_;
        }

        return retval;
    }

}; // namespace watson
