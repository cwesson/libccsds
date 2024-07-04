/**
 * @file spp_service.cpp
 * @ingroup spp
 */

#include "ccsds/spp.h"

namespace ccsds {
namespace spp {
/**
 * @ingroup spp
 * @{
 */

packet_service::packet_service(ccsds::base_service* subnetwork) :
    subnetwork(subnetwork),
    callback(nullptr),
    last_count(-1)
{

}

octet_service::octet_service(apid id, ccsds::base_service* subnetwork) :
    service(subnetwork),
    callback(nullptr),
    id(id),
    packet_count(0),
    last_count(-1)
{

}

ccsds::error packet_service::request(std::unique_ptr<const ccsds::spp::pdu> pdu, int qos)
{
    (void)qos;

    return transfer(std::move(pdu));
}

ccsds::error octet_service::request(std::unique_ptr<const ccsds::base_du> sdu, bool secondary, packet_type type)
{
    auto pdu = assembly(std::move(sdu), secondary, type);
    return service.transfer(std::move(pdu));
}

ccsds::error octet_service::request(std::unique_ptr<const ccsds::base_du> sdu, bool secondary, uint16_t name)
{
    auto pdu = assembly(std::move(sdu), secondary, name);
    return service.transfer(std::move(pdu));
}

/**
 * Primary header identification assembly helpers.
 */
enum {
    PACKET_VERSION_1     = 0b000, ///< Packet version number.
    PACKET_VERSION_SHIFT = 13,    ///< Packet version shift in identification field.
    PACKET_TYPE_MASK     = 0x1,   ///< Packet type mask in identification field.
    PACKET_TYPE_SHIFT    = 12,    ///< Packet type shift in identification field.
    PACKET_SEC_HDR_SHIFT = 11,    ///< Packet secondary header flag shift in identification field.
    PACKET_APID_MASK     = 0x7FF, ///< APID mask in identification field.
};

/**
 * Primary header sequence assembly helpers.
 */
enum {
    SEQUENCE_UNSEGMENTED = 0b11,   ///< Unsegmented user data flag.
    SEQUENCE_FLAGS_SHIFT = 14,     ///< Sequence flags shift in sequence_control field.
    SEQUENCE_COUNT_MASK  = 0x3FFF, ///< Sequence count mask in sequence_control field.
};

std::unique_ptr<const ccsds::spp::pdu> octet_service::assembly(std::unique_ptr<const ccsds::base_du> sdu, bool secondary, packet_type type)
{
    std::unique_ptr<ccsds::spp::pdu> pdu = std::make_unique<ccsds::spp::pdu>();
    primary_header* header = &(*pdu)->header;

    header->identification = ccsds::htons(
            (PACKET_VERSION_1 << PACKET_VERSION_SHIFT)         // packet version number
            | ((type & PACKET_TYPE_MASK) << PACKET_TYPE_SHIFT) // packet type
            | ((!!secondary) << PACKET_SEC_HDR_SHIFT)          // secondary header flag
            | (id & PACKET_APID_MASK));                        // APID

    header->sequence_control = ccsds::htons(
            (SEQUENCE_UNSEGMENTED << SEQUENCE_FLAGS_SHIFT) // sequence flags
            | (packet_count++ & SEQUENCE_COUNT_MASK));     // packet sequence count

    header->data_length = ccsds::htons(sdu->totalSize() - 1);

    // Attach the header to the packet
    pdu->append(std::move(sdu));

    return pdu;
}

std::unique_ptr<const ccsds::spp::pdu> octet_service::assembly(std::unique_ptr<const ccsds::base_du> sdu, bool secondary, uint16_t name)
{
    std::unique_ptr<ccsds::spp::pdu> pdu = std::make_unique<ccsds::spp::pdu>();
    primary_header* header = &(*pdu)->header;

    header->identification = ccsds::htons(
            (PACKET_VERSION_1 << PACKET_VERSION_SHIFT)         // packet version number
            | ((TELECOMMAND & PACKET_TYPE_MASK) << PACKET_TYPE_SHIFT) // packet type
            | ((!!secondary) << PACKET_SEC_HDR_SHIFT)          // secondary header flag
            | (id & PACKET_APID_MASK));                        // APID

    header->sequence_control = ccsds::htons(
            (SEQUENCE_UNSEGMENTED << SEQUENCE_FLAGS_SHIFT) // sequence flags
            | (name & SEQUENCE_COUNT_MASK));     // packet sequence count

    header->data_length = ccsds::htons(sdu->totalSize() - 1);

    // Attach the header to the packet
    pdu->append(std::move(sdu));

    return pdu;
}

ccsds::error packet_service::transfer(std::unique_ptr<const ccsds::base_du> sdu)
{
    if(subnetwork != nullptr){
        return subnetwork->transfer(std::move(sdu));
    }else{
        return error(error::code::NO_NETWORK);
    }
}

ccsds::error octet_service::transfer(std::unique_ptr<const ccsds::base_du> sdu)
{
    (void)sdu;
    return error(error::code::NO_SUPPORT);
}

void packet_service::set_indication(indication func)
{
    callback = func;
}

void packet_service::reception(std::unique_ptr<const ccsds::spp::pdu> pdu)
{
    const primary_header* header = &(*pdu)->header;

    // Check for possible packet loss
    uint16_t count = header->sequence_control & SEQUENCE_COUNT_MASK;
    bool loss = true;
    if(count == ((last_count + 1) & SEQUENCE_COUNT_MASK)){
        loss = false;
    }
    last_count = count;

    if(callback != nullptr){
        // Extract APID
        apid id = static_cast<apid>(header->identification & PACKET_APID_MASK);

        callback(std::move(pdu), id, loss);
    }
}

void octet_service::set_indication(indication func)
{
    callback = func;
}

void octet_service::reception(std::unique_ptr<ccsds::spp::pdu> pdu)
{
    const primary_header* header = &(*pdu)->header;

    // Extract APID
    apid pdu_id = static_cast<apid>(ccsds::ntohs(header->identification) & PACKET_APID_MASK);
    if(pdu_id == id){
        // Check for possible packet loss
        uint16_t count = ccsds::ntohs(header->sequence_control) & SEQUENCE_COUNT_MASK;
        bool loss = true;
        if(count == ((last_count + 1) & SEQUENCE_COUNT_MASK)){
            loss = false;
        }
        last_count = count;

        if(callback != nullptr){
            callback(std::move(pdu->pop()), id, loss);
        }
    }
}

/** @} */ // group spp
} // namespace spp
} // namespace ccsds
