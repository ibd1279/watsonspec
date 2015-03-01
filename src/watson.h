#pragma once
/*!
 \file watson/watson.h
 \brief WatSON format header.

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

#include <cassert>
#include <cstdint>
#include <istream>
#include <map>
#include <memory>
#include <ostream>
#include <unordered_map>
#include <string>
#include <vector>

namespace watson
{
    /*!
     \brief WatSON Size Types.
     \since 0.1
     \sa http://watsonspec.org/
     */
    enum class Size_type : uint8_t
    {
        k_zero = 0x00, //!< zero byte size.
        k_one = 0x01, //!< One byte size.
        k_two = 0x02, //!< two byte size.
        k_eight = 0x03, //!< eight byte size.
    }; // enum class watson::Size_type

    /*!
     \brief WatSON Ngrdnt Types.
     \since 0.1
     \sa http://watsonspec.org/
     */
    enum class Ngrdnt_type : uint8_t
    {
        k_null = 0x3F, //!< Null value type.
        k_true = 0x31, //!< True value type.
        k_false = 0x30, //!< False value type.
        k_flags = 0x22, //!< Bit flag value type.
        k_float = 0x24, //!< Double value type. IEEE 754.
        k_int32 = 0x29, //!< Signed 32bit integer type.
        k_int64 = 0x2C, //!< Signed 64bit integer type.
        k_uint64 = 0x35, //!< Unsigned 64bit integer type.
        k_string = 0x33, //!< String type
        k_header = 0x08, //!< Header type.
        k_library = 0x0C, //!< Library type.
        k_container = 0x03, //!< Container (array) type.
        k_zip = 0x1A, //!< Compression type.
        k_map = 0x0D, //!< Map type.
        k_binary = 0x02, //!< Bytes type.
    }; // enum class watson::Ngrdnt_type

    /*!
     \brief Extract Size_type from a type-marker.
     \since 0.1

     This expects to get the raw type-marker.

     \param t The type-marker.
     \return Size of the size.
     \sa http://watsonspec.org/
     */
    inline constexpr Size_type size_type(const uint8_t t)
    {
        return static_cast<Size_type>((t & 0xC0) >> 6);
    }

    /*!
     \brief Get the number of bytes for a given Size_type.
     \since 0.1

     \param t The Size_type.
     \return number of bytes for the size.
     \sa http://watsonspec.org/
     */
    inline constexpr size_t size_size(const Size_type t)
    {
        return Size_type::k_eight == t ? 8 : static_cast<uint8_t>(t);
    }

    /*!
     \brief Necessary size type for a given amount of data.
     \since 0.1
     \param data_size The size of the data.
     \return the Smallest Size_type possible.
     */
    inline constexpr Size_type size_type_necessary(const uint64_t data_size)
    {
        return data_size == 0 ? Size_type::k_zero :
                data_size < 0xFE ? Size_type::k_one :
                data_size < 0xFFFE ? Size_type::k_two :
                Size_type::k_eight;
    }

    /*!
     \brief Get the number of bytes for the Ngrdnt header
     \since 0.1

     \param t The Size_type.
     \return Size of the Ngrdnt header.
     \sa http://watsonspec.org/
     */
    inline constexpr uint64_t ngrdnt_header_size(const Size_type st)
    {
        return size_size(st) + 1;
    }

    /*!
     \brief Get the number of bytes for the Ngrdnt header
     \since 0.1

     \param t The type-marker.
     \return Size of the Ngrdnt header.
     \sa http://watsonspec.org/
     */
    inline constexpr uint64_t ngrdnt_header_size(const uint8_t t)
    {
        return ngrdnt_header_size(size_type(t));
    }

    /*!
     \brief Extract Ngrdnt_type from a type-marker.
     \since 0.1

     This expects to get the raw type byte.

     \param t The type-marker to test.
     \return The Ngrdnt_type
     \sa http://watsonspec.org/
     */
    inline constexpr Ngrdnt_type ngrdnt_type(const uint8_t t)
    {
        return static_cast<Ngrdnt_type>(t & 0x3F);
    }

    /*!
     \brief Create a type marker
     \since 0.1
     \param st The Size_type
     \param it The Type.
     \return The type-marker.
     */
    inline constexpr uint8_t type_marker(const Size_type st,
            const Ngrdnt_type it)
    {
        return (static_cast<uint8_t>(st) << 6) | static_cast<uint8_t>(it);
    }

    /*!
     \brief WatSON raw Ngrdnt.
     \since 0.1

     This is a raw, unparsed object from a WatSON Recipe. Every WatSON Type
     can be represented by an Ngrdnt.

     The binary format is:
     \code
     (Ngrdnt) ::= (type-marker) [(size) (byte) {(byte)}]
     \endcode

     The highest two bits of \c type-marker represent the Size_type. The lower six
     bits represent the Type.

     \sa http://watsonspec.org/
     */
    class Ngrdnt
    {
    public:

        /*!
         \brief Create a view of a memory region containing WatSON Ngrdnt
         data.

         The resulting object is not responsibile for the memory. This is
         useful for temporary Ngrdnt objects, or when you know the
         top level Recipe object will be long lived.

         \param bytes The bytes to observe.
         \return A new Ngrdnt object.
         */
        static inline const Ngrdnt temp(const uint8_t* bytes)
        {
            return Ngrdnt(bytes);
        }

        /*!
         \brief Create a new Ngrdnt from a series of bytes containing
         Ngrdnt data.

         The resulting object is a copy of the data provided. If you do not
         need copying, use the Ngrdnt::temp() method.

         \param bytes The bytes to copy.
         \return A new Ngrdnt object.
         */
        static inline Ngrdnt clone_from(const uint8_t* bytes)
        {
            return Ngrdnt(temp(bytes));
        }

        /*!
         \brief Crate a new null value Ngrdnt.
         */
        Ngrdnt();

        /*!
         \brief Copy constructor.
         \param o The other Ngrdnt.
         */
        Ngrdnt(const Ngrdnt& o);

        /*!
         \brief Move constructor.
         \param o The other Ngrdnt.
         */
        Ngrdnt(Ngrdnt&& o);

        /*!
         \brief Take ownership of memory containing WatSON Ngrdnt data.

         The resulting object is responsible for freeing the
         associated memory.

         \param bytes The bytes to adopt.
         */
        explicit Ngrdnt(std::unique_ptr<uint8_t[]>&& bytes);

        //! Destructor.
        ~Ngrdnt() = default;

        /*!
         \brief Copy assignment operator.
         \param rhs The right hand side.
         \return This object.
         */
        Ngrdnt& operator=(const Ngrdnt& rhs);

        /*!
         \brief Move assignment operator.
         \param rhs The right hand side.
         \return This object.
         */
        Ngrdnt& operator=(Ngrdnt&& rhs);

        //! The Type of the Ngrdnt.
        inline uint8_t type_marker() const { return data()[0]; }

        //! The size of the Ngrdnt.
        uint64_t size() const;

        //! Raw data pointer.
        inline const uint8_t* data() const { return data_; }
    private:
        //! Constructor for creating temp Ngrdnt objects.
        explicit Ngrdnt(const uint8_t* d);

        std::unique_ptr<uint8_t[]> ptr_;
        const uint8_t* data_;
    }; // class watson::Ngrdnt

    Ngrdnt new_ngrdnt();
    Ngrdnt new_ngrdnt(const std::string& val);
    inline Ngrdnt new_ngrdnt(const char* val) { return new_ngrdnt(std::string(val)); }
    Ngrdnt new_ngrdnt(const bool val);
    Ngrdnt new_ngrdnt(const double val);
    Ngrdnt new_ngrdnt(const int32_t val);
    Ngrdnt new_ngrdnt(const int64_t val);
    Ngrdnt new_ngrdnt(const uint64_t val);
    Ngrdnt new_ngrdnt(const std::vector<bool>& val);

    bool is_null(const Ngrdnt& val);
    bool to_bool(const Ngrdnt& val);
    double to_double(const Ngrdnt& val);
    int32_t to_int32(const Ngrdnt& val);
    int64_t to_int64(const Ngrdnt& val);
    uint64_t to_uint64(const Ngrdnt& val);
    std::vector<bool> to_flags(const Ngrdnt& val);
    std::string to_string(const Ngrdnt& val);
    std::string to_dump(const Ngrdnt& val);

    /*!
     \brief WatSON Basic Container Ngrdnt
     \since 0.1
     \sa http://watsonspec.org/
     */
    template <class T, typename ETL>
    class Basic_container
    {
    public:
        //! Underlying container.
        using Children = std::vector<T>;

        Basic_container() = default;
        Basic_container(const Basic_container& o) = default;
        Basic_container(Basic_container&& o) = default;
        explicit Basic_container(Children&& c) :
                children_(std::move(c))
        {
        }
        explicit Basic_container(const Ngrdnt& raw)
        {
            ETL etl;
            const uint8_t* ptr = raw.data() + size_size(size_type(raw.type_marker())) + 1;
            const uint8_t* const end = raw.data() + raw.size();

            while (end > ptr)
            {
                // Store the value.
                children_.emplace_back(etl(Ngrdnt::temp(ptr)));

                // Advance the ptr.
                ptr += Ngrdnt::temp(ptr).size();
            }
        }
        ~Basic_container() = default;
        Basic_container& operator=(const Basic_container& rhs) = default;
        Basic_container& operator=(Basic_container&& rhs) = default;

        inline Children& mutable_children() { return children_; }
        inline const Children& children() const { return children_; }
        inline const size_t size() const { return children().size(); }
        inline const T& operator[](uint32_t indx) const { return children()[indx]; }
    private:
        Children children_;
    }; // class watson::Basic_container

    //! Ngrdnt identity functor
    struct Ngrdnt_identity
    {
        const Ngrdnt& operator()(const Ngrdnt& i)
        {
            return i;
        }
    }; // struct watson::Ngrdnt_identity

    //! Ngrdnt to string functor.
    struct Ngrdnt_string
    {
        const std::string operator()(const Ngrdnt& i)
        {
            return to_string(i);
        }
    }; // struct watson::Ngrdnt_string

    //! WatSON Container type.
    using Container = Basic_container<Ngrdnt, Ngrdnt_identity>;

    //! WatSON Library type.
    using Library = Basic_container<std::string, Ngrdnt_string>;

    /*!
     \brief WatSON Header Ngrdnt
     \since 0.1
     \sa http://watsonspec.org/
     */
    class Header
    {
    public:
        using Children = std::map<std::string, Ngrdnt>;

        static const Ngrdnt k_not_found;

        Header() = default;
        Header(const Header& o) = default;
        Header(Header&& o) = default;
        Header(Children&& c);
        explicit Header(const Ngrdnt& raw);
        ~Header() = default;
        Header& operator=(const Header& rhs) = default;
        Header& operator=(Header&& rhs) = default;

        inline Children& mutable_children() { return children_; }
        inline const Children& children() const { return children_; }
        inline const size_t size() const { return children().size(); }
        const Ngrdnt& operator[](const std::string& key) const;
    private:
        Children children_;
    }; // class watson::Header

    /*!
     \brief WatSON Map Ngrdnt
     \since 0.1
     \sa http://watsonspec.org/
     */
    class Map
    {
    public:
        using Children = std::map<uint32_t, Ngrdnt>;

        static const Ngrdnt k_not_found;

        Map() = default;
        Map(const Map& o) = default;
        Map(Map&& o) = default;
        explicit Map(Children&& c);
        explicit Map(const Ngrdnt& raw);
        ~Map() = default;
        Map& operator=(const Map& rhs) = default;
        Map& operator=(Map&& rhs) = default;

        inline Children& mutable_children() { return children_; }
        inline const Children& children() const { return children_; }
        inline const size_t size() const { return children().size(); }
        const Ngrdnt& operator[](uint32_t key) const;
    private:
        Children children_;
    }; // class watson::Map

    class Compressed
    {
    public:
        Compressed() = default;
        Compressed(const Compressed& o) = default;
        Compressed(Compressed&& o) = default;
        explicit Compressed(Ngrdnt&& c);
        explicit Compressed(const Ngrdnt& raw);
        ~Compressed() = default;
        Compressed& operator=(const Compressed& rhs) = default;
        Compressed& operator=(Compressed&& rhs) = default;

        inline Ngrdnt& mutable_child() { return child_; }
        inline const Ngrdnt& child() const { return child_; }
        inline const Ngrdnt& operator*() const { return child_; }
        inline const Ngrdnt* operator->() const { return &child_; }
    private:
        Ngrdnt child_;
    }; // class watson::Compressed

    /*!
     \brief WatSON Bytes Ngrdnt
     \since 0.1

     Bytes ngrdnts are a specific format identifierfollowed by a series
     of bytes. WatSON Bytes are not null terminated.

     \code
     (String-Ngrdnt) ::= (Type) (64-bit-size) (Subtype) [(Binary-Data) {(Binary-Data)}]
     (Subtype) ::= (Null-Terminated-C-String)
     (Binary-Data) ::= (Unsigned-8bit-Integer)
     \endcode

     \sa http://watsonspec.org/
     */
    class Bytes
    {
    public:
        static const uint8_t k_empty_bytes_data[];
        static inline const Bytes temp(const uint64_t sz,
                const uint32_t* mh,
                const uint8_t* const d)
        {
            return Bytes(sz, mh, d);
        }

        Bytes();
        Bytes(const Bytes& o);
        Bytes(Bytes&& o) = default;
        Bytes(std::unique_ptr<uint8_t[]>&& v, uint64_t v_sz);
        explicit Bytes(const Ngrdnt& raw);
        ~Bytes() = default;
        Bytes& operator=(const Bytes& rhs);
        Bytes& operator=(Bytes&& rhs) = default;

        inline const uint32_t marshal_hint() const { return *marshal_hint_; }
        inline uint64_t size() const { return size_; }
        inline const uint8_t* data() const { return data_; }
    private:
        Bytes(const uint64_t sz,
                const uint32_t* mh,
                const uint8_t* const d);

        uint64_t size_;
        std::unique_ptr<uint8_t[]> ptr_;
        const uint32_t* marshal_hint_;
        const uint8_t* data_;
    }; // class watson::Container

    Ngrdnt new_ngrdnt(const Container& val);
    Ngrdnt new_ngrdnt(const Library& val);
    Ngrdnt new_ngrdnt(const Header& val);
    Ngrdnt new_ngrdnt(const Compressed& val);
    Ngrdnt new_ngrdnt(const Map& val);
    Ngrdnt new_ngrdnt(const Bytes& val);

}; // namespace watson

/*!
 \brief Extract data with format.

 Extract a watson::Ngrdnt object from the datastream.
 \param is The input stream to read from.
 \param val The watson::Ngrdnt to store the data in.
 \return The input stream passed.
 */
std::istream& operator>>(std::istream& is, watson::Ngrdnt& val);

/*!
 \brief Insert data with format.

 Insert an watson::Ngrdnt object to the datastream.
 \param os The output stream to write to.
 \param val The watson::Ngrdnt to copy to binary.
 \return The output stream passed.
 */
std::ostream& operator<<(std::ostream& os, const watson::Ngrdnt& val);
