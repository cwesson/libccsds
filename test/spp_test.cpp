/**
 * @file test/spp_test.cpp
 */

#include "ccsds/spp.h"
#include "CppUTest/CommandLineTestRunner.h"
#include "CppUTest/TestHarness.h"

/**
 * @ingroup unittest
 * @{
 */

/**
 * Space Packet Protocol test group.
 */
TEST_GROUP(SpacePacketTestGroup)
{

};

/**
 * CCSDS service for unit tests.
 */
class test_service : public ccsds::base_service {
    public:
        test_service() = default;
        virtual ~test_service() = default;

        /**
         * Access the buffer.
         * @return Pointer to the buffer.
         */
        const ccsds::base_sdu& get() const
        {
            return *last;
        }
    
    protected:
        virtual void transfer(std::unique_ptr<const ccsds::base_sdu> sdu) override
        {
            last.swap(sdu);
        }

        std::unique_ptr<const ccsds::base_sdu> last; ///< Link list of SDU.
};

/**
 * Test space packet assembly.
 */
TEST(SpacePacketTestGroup, AssemblyTest)
{
    test_service test;
    ccsds::spp::octet_service service(static_cast<ccsds::spp::apid>(0x1AB), &test);

    uint8_t data[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    std::unique_ptr<ccsds::buffered_sdu> sdu = std::make_unique<ccsds::buffered_sdu>(&data, sizeof(data));

    service.request(std::move(sdu), false, ccsds::spp::TELECOMMAND);

    const ccsds::base_sdu& packet = test.get();
    const uint8_t* bytes = static_cast<const uint8_t*>(packet.get());
    CHECK_EQUAL(0x11, bytes[0]); // telecommand, high byte of 1AB
    CHECK_EQUAL(0xAB, bytes[1]); // low byte of 1AB
}

/** @} */ // group unittest
