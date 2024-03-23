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
    
    private:
        virtual ccsds::error transfer(std::unique_ptr<const ccsds::base_sdu> sdu) override
        {
            last.swap(sdu);
            return ccsds::error();
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

    // Test firs packet
    uint8_t data[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    std::unique_ptr<ccsds::buffered_sdu> sdu = std::make_unique<ccsds::buffered_sdu>(&data, sizeof(data));
    service.request(std::move(sdu), false, ccsds::spp::TELECOMMAND);

    const ccsds::base_sdu& packet = test.get();
    CHECK_EQUAL(2, packet.length());
    CHECK_EQUAL(6, packet.size());
    CHECK_EQUAL(16, packet.totalSize());

    const uint8_t* bytes = static_cast<const uint8_t*>(packet.get());
    CHECK_EQUAL(0x11, bytes[0]); // telecommand, high byte of 1AB
    CHECK_EQUAL(0xAB, bytes[1]); // low byte of 1AB
    CHECK_EQUAL(0xC0, bytes[2]); // sequence flags, high byte of packet count
    CHECK_EQUAL(0x00, bytes[3]); // low byte of packet count
    CHECK_EQUAL(0x00, bytes[4]); // high byte of data length
    CHECK_EQUAL(sizeof(data)-1, bytes[5]); // low byte of data length

    bytes = static_cast<const uint8_t*>(packet.next().get());
    for(size_t i = 0; i < sizeof(data); ++i){
        CHECK_EQUAL(data[i], bytes[i]);
    }

    // Test a named packet
    sdu = std::make_unique<ccsds::buffered_sdu>(&data, sizeof(data));
    service.request(std::move(sdu), false, 0x1A5A);

    const ccsds::base_sdu& packet2 = test.get();
    CHECK_EQUAL(2, packet2.length());
    CHECK_EQUAL(6, packet2.size());
    CHECK_EQUAL(16, packet2.totalSize());

    bytes = static_cast<const uint8_t*>(packet2.get());
    CHECK_EQUAL(0x11, bytes[0]); // telecommand, high byte of 1AB
    CHECK_EQUAL(0xAB, bytes[1]); // low byte of 1AB
    CHECK_EQUAL(0xDA, bytes[2]); // sequence flags, high byte of packet count
    CHECK_EQUAL(0x5A, bytes[3]); // low byte of packet count
    CHECK_EQUAL(0x00, bytes[4]); // high byte of data length
    CHECK_EQUAL(sizeof(data)-1, bytes[5]); // low byte of data length

    bytes = static_cast<const uint8_t*>(packet2.next().get());
    for(size_t i = 0; i < sizeof(data); ++i){
        CHECK_EQUAL(data[i], bytes[i]);
    }

    // Test another packet
    sdu = std::make_unique<ccsds::buffered_sdu>(&data, sizeof(data));
    service.request(std::move(sdu), false, ccsds::spp::TELEMETRY);

    const ccsds::base_sdu& packet3 = test.get();
    CHECK_EQUAL(2, packet3.length());
    CHECK_EQUAL(6, packet3.size());
    CHECK_EQUAL(16, packet3.totalSize());

    bytes = static_cast<const uint8_t*>(packet3.get());
    CHECK_EQUAL(0x01, bytes[0]); // telecommand, high byte of 1AB
    CHECK_EQUAL(0xAB, bytes[1]); // low byte of 1AB
    CHECK_EQUAL(0xC0, bytes[2]); // sequence flags, high byte of packet count
    CHECK_EQUAL(0x01, bytes[3]); // low byte of packet count
    CHECK_EQUAL(0x00, bytes[4]); // high byte of data length
    CHECK_EQUAL(sizeof(data)-1, bytes[5]); // low byte of data length

    bytes = static_cast<const uint8_t*>(packet3.next().get());
    for(size_t i = 0; i < sizeof(data); ++i){
        CHECK_EQUAL(data[i], bytes[i]);
    }
}

/** @} */ // group unittest
