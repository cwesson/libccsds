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
 * Generic zero-copy buffer for SDU.
 */
class base_sdu {
    public:
        /**
         * Constructor.
         */
        base_sdu() :
            next(nullptr)
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
            next.swap(sdu);
        }
    
    private:
        std::unique_ptr<const base_sdu> next; ///< Link list of SDU.
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
         */
        virtual void transfer(std::unique_ptr<const base_sdu> sdu) = 0;
};

/** @} */ // group ccsds
} // namespace ccsds
