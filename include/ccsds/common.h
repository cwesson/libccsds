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

    private:
        code value;  ///< Error code.
};

/**
 * Generic zero-copy buffer for SDU.
 */
class base_sdu {
    public:
        /**
         * Constructor.
         */
        base_sdu() :
            ptr(nullptr)
        {}

        /**
         * Destructor.
         */
        virtual ~base_sdu() = default;

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
                return size() + ptr->size();
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
         * Append an SDU.
         * @param sdu SDU to append.
         */
        void append(std::unique_ptr<const base_sdu> sdu)
        {
            ptr.swap(sdu);
        }

        /**
         * Get the next buffer in the chain.
         * @return The next buffer.
         */
        const base_sdu& next() const
        {
            return *ptr;
        }
    
    private:
        std::unique_ptr<const base_sdu> ptr; ///< Link list of SDU.
};

/**
 * Zero-copy buffer for SDU with opaque data.
 */
class buffered_sdu : public base_sdu {
    public:
        /**
         * Constructor.
         * @param buf Buffer containing opaque data.
         * @param len Length of buf in bytes.
         */
        buffered_sdu(void* buf, size_t len) :
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
        void*  buffer; ///< Buffer for the SDU.
        size_t length; ///< Length of buffer in bytes.
};

/**
 * Zero-copy buffer for SDU.
 * @tparam T Type of the buffer
 */
template<typename T>
class sdu : public base_sdu {
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
        T buffer; ///< Buffer for the SDU.
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
         * Transfer an SDU from another service.
         * @param sdu SDU to transfer.
         * @retval error::code::NONE if successful.
         * @retval other if failure.
         */
        virtual ccsds::error transfer(std::unique_ptr<const base_sdu> sdu) = 0;
};

#if (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
    /**
     * Convert uint16 to big endian.
     * @param x Little endian uint16.
     * @return Big endian uint16.
     */
    inline uint16_t htons(uint16_t x)
    {
        return ((x & 0xFF) << 8) | ((x >> 8) & 0xFF);
    }
#elif (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
    inline uint16_t htons(uint16_t x)
    {
        return x;
    }
#else
    #error "Unknown endian"
#endif

/** @} */ // group ccsds
} // namespace ccsds
