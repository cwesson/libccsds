/**
 * @file test/spp_test.cpp
 */

#include "ccsds/spp.h"
#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

/**
 * @ingroup unittest
 * @{
 */

/**
 * Space Packet Protocol test group.
 */
TEST_GROUP(SpacePacketTestGroup)
{
    void teardown()
    {
        mock().clear();
    }
};

/**
 * Deadend CCSDS service for unit tests.
 */
class test_service : public ccsds::base_service {
    public:
        test_service() = default;
        virtual ~test_service() = default;

        /**
         * Access the buffer.
         * @return Pointer to the buffer.
         */
        const ccsds::base_du& get() const
        {
            return *last;
        }
    
    private:
        virtual ccsds::error transfer(std::unique_ptr<const ccsds::base_du> sdu) override
        {
            last.swap(sdu);
            return ccsds::error();
        }

        std::unique_ptr<const ccsds::base_du> last; ///< Link list of SDU.
};

/**
 * Loopback CCSDS service for unit tests.
 */
class loopback_service : public ccsds::base_service {
    public:
        loopback_service() = default;
        virtual ~loopback_service() = default;

        /**
         * Set service to loopback to.
         * @param serv CCSDS service to receive packets.
         */
        void set_service(ccsds::spp::octet_service& serv)
        {
            service = &serv;
        }
    
    private:
        virtual ccsds::error transfer(std::unique_ptr<const ccsds::base_du> sdu) override
        {
            if(service != nullptr){
                // This casting is only safe because we know how the test was written
                ccsds::base_du* du = const_cast<ccsds::base_du*>(sdu.release());
                std::unique_ptr<ccsds::spp::pdu> pdu(reinterpret_cast<ccsds::spp::pdu*>(du));
                service->reception(std::move(pdu));
            }else{
                FAIL("Loopback service has not been set.");
            }
            return ccsds::error();
        }

        ccsds::spp::octet_service* service; ///< Service to loopback to.
};

/**
 * Test indication function for receiving packets.
 */
void test_indication(std::unique_ptr<const ccsds::base_du> sdu, ccsds::spp::apid id, bool packet_loss)
{
    mock().actualCall("test_indication");
    (void)sdu;
    CHECK_EQUAL(0x1AB, id);
    CHECK_FALSE(packet_loss);
}

/**
 * Test space packet reception.
 */
TEST(SpacePacketTestGroup, ReceptionTest)
{
    loopback_service loopback;
    ccsds::spp::octet_service service(static_cast<ccsds::spp::apid>(0x1AB), &loopback);
    loopback.set_service(service);
    service.set_indication(&test_indication);

    // Test first packet
    uint8_t data[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    std::unique_ptr<ccsds::buffered_du> sdu = std::make_unique<ccsds::buffered_du>(&data, sizeof(data));
    mock().expectOneCall("test_indication");
    ccsds::error e = service.request(std::move(sdu), false, ccsds::spp::TELECOMMAND);
    mock().checkExpectations();
    CHECK_EQUAL(ccsds::error::code::NONE, static_cast<int>(e));
}

/**
 * Test space packet assembly.
 */
TEST(SpacePacketTestGroup, AssemblyTest)
{
    test_service test;
    ccsds::spp::octet_service service(static_cast<ccsds::spp::apid>(0x1AB), &test);

    // Test first packet
    uint8_t data[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    std::unique_ptr<ccsds::buffered_du> sdu = std::make_unique<ccsds::buffered_du>(&data, sizeof(data));
    ccsds::error e = service.request(std::move(sdu), false, ccsds::spp::TELECOMMAND);
    CHECK_EQUAL(ccsds::error::code::NONE, static_cast<int>(e));

    const ccsds::base_du& packet = test.get();
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
    sdu = std::make_unique<ccsds::buffered_du>(&data, sizeof(data));
    e = service.request(std::move(sdu), false, 0x1A5A);
    CHECK_EQUAL(ccsds::error::code::NONE, static_cast<ccsds::error::code>(e));

    const ccsds::base_du& packet2 = test.get();
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
    sdu = std::make_unique<ccsds::buffered_du>(&data, sizeof(data));
    e = service.request(std::move(sdu), false, ccsds::spp::TELEMETRY);
    CHECK_EQUAL(false, static_cast<bool>(e));

    const ccsds::base_du& packet3 = test.get();
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
