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
#include <list>
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

        using Ptr = std::shared_ptr<Ngrdnt>;

        /*!
         \brief Create a view of a memory region containing WatSON Ngrdnt
         data.

         The resulting object is not responsibile for the memory. This is
         useful for temporary Ngrdnt objects, or when you know the
         top level Recipe object will be long lived.

         \param bytes The bytes to observe.
         \return A new Ngrdnt object.
         */
        static inline const Ngrdnt::Ptr temp(const uint8_t* bytes)
        {
            return Ngrdnt::Ptr(new Ngrdnt(bytes));
        }

        /*!
         \brief Create a new Ngrdnt from a series of bytes containing
         Ngrdnt data.

         The resulting object is a copy of the data provided. If you do not
         need copying, use the Ngrdnt::temp() method.

         \param bytes The bytes to copy.
         \return A new Ngrdnt object.
         */
        static inline Ngrdnt::Ptr clone(const uint8_t* bytes)
        {
            return Ngrdnt::clone(temp(bytes));
        }

        /*!
         \brief Create a copy of an Ngrdnt Object.

         \param o The original Ngrdnt Object.
         \return A new Ngrdnt object.
        */
        static inline Ngrdnt::Ptr clone(const Ngrdnt::Ptr& o)
        {
            return Ngrdnt::Ptr(new Ngrdnt(*o));
        }

        /*!
         \brief Crate a new null value Ngrdnt.

         \return A new Ngrdnt object.
        */
        static inline Ngrdnt::Ptr make()
        {
            return Ngrdnt::Ptr(new Ngrdnt());
        }

        /*!
         \brief Take ownership of memory containing WatSON Ngrdnt data.

         \param bytes The bytes to adopt.
         \param p The parent of this object.
         \return A new Ngrdnt object.
         */
        static inline Ngrdnt::Ptr adopt(std::unique_ptr<uint8_t[]>&& bytes,
                const Ngrdnt::Ptr& p = Ngrdnt::Ptr(nullptr))
        {
            return Ngrdnt::Ptr(new Ngrdnt(std::move(bytes), p));
        }

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

        //! Get the parent Ngrdnt.
        inline const Ngrdnt::Ptr& parent() const { return parent_; }

        //! Set the parent Ngrdnt.
        inline void parent(const Ngrdnt::Ptr& p) { parent_ = p; }

    private:
        /*!
         \brief Constructor for creating blank Ngrdnt objects.
         \sa Ngrdnt::make()
         */
        Ngrdnt();
        
        /*!
         \brief Constructor for creating copies of Ngrdnt objects.
         \sa Ngrdnt::clone(const Ngrdnt::Ptr&)
         */
        Ngrdnt(const Ngrdnt& o);

        /*!
         \brief Constructor for creating temp Ngrdnt objects.
         \sa Ngrdnt::temp(const uint8_t*)
         */
        explicit Ngrdnt(const uint8_t* d);

        /*!
         \brief Constructor for creating an Ngrdnt around bytes.
         \sa Ngrdnt::adopt(std::unique_ptr<uint8_t[]>&&, const Ngrdnt::Ptr&, const Ngrdnt::Ptr&))
         */
        explicit Ngrdnt(std::unique_ptr<uint8_t[]>&& bytes,
                const Ngrdnt::Ptr& p);

        // Data for the object.
        std::unique_ptr<uint8_t[]> ptr_;
        const uint8_t* data_;

        //! Context for where the Ngrdnt was in the recipe. 
        Ngrdnt::Ptr parent_;
    }; // class watson::Ngrdnt


    extern const Ngrdnt::Ptr k_not_found;

    Ngrdnt::Ptr new_ngrdnt();
    Ngrdnt::Ptr new_ngrdnt(const std::string& val);
    inline Ngrdnt::Ptr new_ngrdnt(const char* val) { return new_ngrdnt(std::string(val)); }
    Ngrdnt::Ptr new_ngrdnt(const bool val);
    Ngrdnt::Ptr new_ngrdnt(const double val);
    Ngrdnt::Ptr new_ngrdnt(const int32_t val);
    Ngrdnt::Ptr new_ngrdnt(const int64_t val);
    Ngrdnt::Ptr new_ngrdnt(const uint64_t val);
    Ngrdnt::Ptr new_ngrdnt(const std::vector<bool>& val);

    bool is_null(const Ngrdnt::Ptr& val);
    bool to_bool(const Ngrdnt::Ptr& val);
    double to_double(const Ngrdnt::Ptr& val);
    int32_t to_int32(const Ngrdnt::Ptr& val);
    int64_t to_int64(const Ngrdnt::Ptr& val);
    uint64_t to_uint64(const Ngrdnt::Ptr& val);
    std::vector<bool> to_flags(const Ngrdnt::Ptr& val);
    std::string to_string(const Ngrdnt::Ptr& val);
    std::string to_dump(const Ngrdnt::Ptr& val);

    /*!
     \brief WatSON Basic Container Ngrdnt

     This is specialized into Container and Library types further below.
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
        explicit Basic_container(const Ngrdnt::Ptr& raw)
        {
            ETL etl;
            const uint8_t* ptr = raw->data() + size_size(size_type(raw->type_marker())) + 1;
            const uint8_t* const end = raw->data() + raw->size();

            while (end > ptr)
            {
                // Store the value.
                children_.emplace_back(etl(Ngrdnt::clone(ptr)));

                // Advance the ptr.
                ptr += Ngrdnt::temp(ptr)->size();
            }
        }
        // TODO need a cheap way to make temp collections.
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
        const Ngrdnt::Ptr& operator()(const Ngrdnt::Ptr& i)
        {
            return i;
        }
    }; // struct watson::Ngrdnt_identity

    //! Ngrdnt to string functor.
    struct Ngrdnt_string
    {
        const std::string operator()(const Ngrdnt::Ptr& i)
        {
            return to_string(i);
        }
    }; // struct watson::Ngrdnt_string

    //! WatSON Container type.
    using Container = Basic_container<Ngrdnt::Ptr, Ngrdnt_identity>;

    //! WatSON Library type.
    using Library = Basic_container<std::string, Ngrdnt_string>;

    /*!
     \brief WatSON Map Ngrdnt

     A map is a key/value lookup. Keys are integers. If you need to pass
     the key names, you must provide them as an array.
     \since 0.1
     \sa http://watsonspec.org/
     */
    class Map
    {
    public:
        using Children = std::map<uint32_t, Ngrdnt::Ptr>;

        Map() = default;
        Map(const Map& o) = default;
        Map(Map&& o) = default;
        explicit Map(Children&& c);
        explicit Map(const Ngrdnt::Ptr& raw);
        // TODO Need a cheap way to do temp maps.
        ~Map() = default;
        Map& operator=(const Map& rhs) = default;
        Map& operator=(Map&& rhs) = default;

        inline Children& mutable_children() { return children_; }
        inline const Children& children() const { return children_; }
        inline const size_t size() const { return children().size(); }
        const Ngrdnt::Ptr& operator[](uint32_t key) const;
    private:
        Children children_;
    }; // class watson::Map

    /*!
     \brief WatSON Compressed Ngrdnt

     Represents a Snappy compressed Ngrdnt.
     /since 0.1
     \sa http://watsonspec.org
     */
    class Compressed
    {
    public:
        Compressed();
        Compressed(const Compressed& o) = default;
        Compressed(Compressed&& o) = default;
        explicit Compressed(Ngrdnt::Ptr&& c);
        explicit Compressed(const Ngrdnt::Ptr& raw);
        ~Compressed() = default;
        Compressed& operator=(const Compressed& rhs) = default;
        Compressed& operator=(Compressed&& rhs) = default;

        inline Ngrdnt::Ptr& mutable_child() { return child_; }
        inline const Ngrdnt::Ptr& operator*() const { return child_; }
        inline const Ngrdnt* operator->() const { return child_.get(); }
    private:
        Ngrdnt::Ptr child_;
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
        explicit Bytes(const Ngrdnt::Ptr& raw);
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

    Ngrdnt::Ptr new_ngrdnt(const Container& val);
    Ngrdnt::Ptr new_ngrdnt(const Library& val);
    Ngrdnt::Ptr new_ngrdnt(const Compressed& val);
    Ngrdnt::Ptr new_ngrdnt(const Map& val);
    Ngrdnt::Ptr new_ngrdnt(const Bytes& val);

}; // namespace watson

/*!
 \brief Extract data with format.

 Extract a watson::Ngrdnt object from the datastream.
 \param is The input stream to read from.
 \param val The watson::Ngrdnt to store the data in.
 \return The input stream passed.
 */
std::istream& operator>>(std::istream& is, watson::Ngrdnt::Ptr& val);

/*!
 \brief Insert data with format.

 Insert an watson::Ngrdnt object to the datastream.
 \param os The output stream to write to.
 \param val The watson::Ngrdnt to copy to binary.
 \return The output stream passed.
 */
std::ostream& operator<<(std::ostream& os, const watson::Ngrdnt::Ptr& val);

namespace watson
{
    /*!
     \brief WatSON Glossary

     A glossary is a key/value lookup for map keys. Map keys are
     transmitted as integers. A library object is used to
     communicate the string version of the keys. The glossary object
     provides mechanisms for looking up names from map keys or map keys
     from names.
     \since 0.1
     \sa http://watsonspec.org/
     */
    struct Glossary
    {
        Glossary() = default;
        Glossary(Glossary&& o) = default;
        Glossary(const Glossary& o) = default;
        explicit Glossary(const Library& l);
        Glossary& operator=(const Glossary& rhs) = default;
        Glossary& operator=(Glossary&& rhs) = default;

        bool empty() const { return names.empty(); }

        std::vector<std::string> names;
        std::map<std::string, uint32_t> index;
    }; // class watson::Glossary

    /*!
     \brief Translate a list of strings into a list of map keys.

     Unknown names are translated to map key 0
     \param g The glossary to use.
     \param names The names to translate.
     \param List of keys, in the same order as provided.
     \since 0.1
     */
    std::list<uint32_t> xlate(const Glossary& g, const std::list<std::string>& names);

    /*!
     \brief Translate a list of strings into a list of map keys.

     Unknown map keys are translated to an empty string.
     \param g The glossary to use.
     \param keys The names to translate.
     \param List of keys, in the same order as provided.
     \since 0.1
     */
    std::list<std::string> xlate(const Glossary& g, const std::list<uint32_t>& keys);

    /*!
     \brief WatSON Recipe.

     A WatSON Recipe is a structured collection of ingredients. In
     this particular implementation, it consists of a top level
     container, with a single "Library" object as the first item,
     and any number of following items. 

     \since 0.1
     \sa http://watsonspec.org/
     */
    class Recipe
    {
    public:
        Recipe() = default;
        Recipe(Recipe&& o) = default;
        Recipe(const Recipe& o) = default;
        explicit Recipe(Ngrdnt::Ptr&& c);
        explicit Recipe(const Ngrdnt::Ptr& raw);
        Recipe& operator=(Recipe&& rhs) = default;
        Recipe& operator=(const Recipe& rhs) = default;

        inline const Container& container() const { return container_; }
        inline const Glossary& glossary() const { return glossary_; }

        const Ngrdnt::Ptr ngrdnt(const std::list<uint32_t>& steps) const;
        Recipe recipe(const std::list<uint32_t>& steps) const;
    private:
        Container container_;
        Glossary glossary_;
    };

    inline std::list<uint32_t> xlate(const Recipe& r, const std::list<std::string>& steps) { return xlate(r.glossary(), steps); }
    inline std::list<std::string> xlate(const Recipe& r, const std::list<uint32_t>& steps) { return xlate(r.glossary(), steps); }
}; // namespace watson
