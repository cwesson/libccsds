/**
 * @file ccsds/common.h
 */

#include <cstddef>
#include <memory>

namespace ccsds {
/**
 * @defgroup ccsds Consultative Committee for Space Data Systems
 * @{
 */

/**
 * Error codes used in libccsds.
 */
class error {
    public:
        /**
         * Error codes.
         */
        enum code {
            NONE = 0,    ///< No error.
            NO_SUPPORT,  ///< Function not supported.
            NO_NETWORK,  ///< Network unavailable.
        };

        /**
         * Constructor.
         * @param v Error code.
         */
        error(code v = NONE) :
            value(v)
        {}

        /**
         * int conversion.
         * @return Error code.
         */
        operator int()
        {
            return value;
        }

        /**
         * enum conversion.
         * @return Error code.
         */
        operator enum code()
        {
            return value;
        }

        /**
         * bool conversion.
         * @return true if the code is not NONE, false if the code is NONE.
         */
        operator bool()
        {
            return (value != NONE);
        }

    private:
        code value;  ///< Error code.
};

/**
 * Generic zero-copy buffer for data units (DU).
 */
class base_du {
    public:
        /**
         * Constructor.
         */
        base_du() :
            ptr(nullptr)
        {}

        /**
         * Destructor.
         */
        virtual ~base_du() = default;

        /**
         * Get the size of the buffer in bytes.
         * @return Buffer size in bytes.
         */
        virtual size_t size() const = 0;

        /**
         * Get the total size of all the buffers in bytes.
         * @return Buffer size in bytes.
         */
        size_t totalSize() const
        {
            if(ptr){
                return size() + ptr->totalSize();
            }else{
                return size();
            }
        }

        /**
         * Get the number of chained buffers,
         * @return number of buffers.
         */
        size_t length() const
        {
            if(ptr){
                return ptr->length() + 1;
            }else{
                return 1;
            }
        }

        /**
         * Access the buffer.
         * @return Pointer to the buffer.
         */
        virtual const void* get() const = 0;

        /**
         * Append an DU.
         * @param du DU to append.
         */
        void append(std::unique_ptr<const base_du> du)
        {
            ptr.swap(du);
        }

        /**
         * Get the next buffer in the chain.
         * @return The next buffer.
         */
        const base_du& next() const
        {
            return *ptr;
        }

        /**
         * Remove the first buffer in the chain and return the next buffer.
         * @return The next buffer.
         */
        std::unique_ptr<const base_du> pop()
        {
            return std::move(ptr);
        }

    private:
        std::unique_ptr<const base_du> ptr; ///< Link list of DU.
};

/**
 * Zero-copy buffer for DU with opaque data.
 * @note The caller is responsible for managing the buffer memory.
 */
class buffered_du : public base_du {
    public:
        /**
         * Constructor.
         * @param buf Buffer containing opaque data.
         * @param len Length of buf in bytes.
         */
        buffered_du(void* buf, size_t len) :
            buffer(buf),
            length(len)
        {}

        /**
         * Get the size of the buffer in bytes.
         * @return Buffer size in bytes.
         */
        virtual size_t size() const override
        {
            return length;
        }

        /**
         * Access the buffer.
         * @return Pointer to the buffer.
         */
        virtual const void* get() const override
        {
            return buffer;
        }

        /**
         * Access the buffer.
         * @return Pointer to the buffer.
         */
        void* operator->()
        {
            return buffer;
        }

        /**
         * Access the buffer.
         * @return Pointer to the buffer.
         */
        const void* operator->() const
        {
            return buffer;
        }
    
    private:
        void*  buffer; ///< Buffer for the DU.
        size_t length; ///< Length of buffer in bytes.
};

/**
 * Zero-copy buffer for DU.
 * @tparam T Type of the buffer
 */
template<typename T>
class du : public base_du {
    public:
        /**
         * Get the size of the buffer in bytes.
         * @return Buffer size in bytes.
         */
        virtual size_t size() const override
        {
            return sizeof(T);
        }

        /**
         * Access the buffer.
         * @return Pointer to the buffer.
         */
        virtual const void* get() const override
        {
            return &buffer;
        }

        /**
         * Access the buffer.
         * @return Pointer to the buffer.
         */
        T* operator->()
        {
            return &buffer;
        }

        /**
         * Access the buffer.
         * @return Pointer to the buffer.
         */
        const T* operator->() const
        {
            return &buffer;
        }
    
    private:
        T buffer; ///< Buffer for the DU.
};

/**
 * Generic CCSDS service.
 */
class base_service {
    public:
        /**
         * Constructor.
         */
        base_service() = default;

        /**
         * Destructor.
         */
        virtual ~base_service() = default;

        /**
         * Transfer a DU from another service.
         * @param du DU to transfer.
         * @retval error::code::NONE if successful.
         * @retval other if failure.
         */
        virtual ccsds::error transfer(std::unique_ptr<const base_du> du) = 0;
};

/**
 * Swap uint16 to endianness.
 * @param x Original endian uint16.
 * @return Swapped endian uint16.
 */
inline uint16_t swaps(uint16_t x)
{
    return ((x & 0xFF) << 8) | ((x >> 8) & 0xFF);
}

#if (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
    /**
     * Convert uint16 to big endian.
     * @param x Little endian uint16.
     * @return Big endian uint16.
     */
    inline uint16_t htons(uint16_t x)
    {
        return ccsds::swaps(x);
    }

    /**
     * Convert uint16 to little endian.
     * @param x Big endian uint16.
     * @return Little endian uint16.
     */
    inline uint16_t ntohs(uint16_t x)
    {
        return ccsds::swaps(x);
    }
#elif (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
    inline uint16_t htons(uint16_t x)
    {
        return x;
    }

    inline uint16_t ntohs(uint16_t x)
    {
        return x;
    }
#else
    #error "Unknown endian"
#endif

/** @} */ // group ccsds
} // namespace ccsds
