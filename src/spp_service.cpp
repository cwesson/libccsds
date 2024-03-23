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
    callback(nullptr)
{

}

octet_service::octet_service(apid id, ccsds::base_service* subnetwork) :
    service(subnetwork),
    callback(nullptr),
    id(id),
    packet_count(0)
{

}

ccsds::error octet_service::request(std::unique_ptr<const ccsds::base_sdu> packet, bool secondary, packet_type type)
{
    auto sdu = assembly(std::move(packet), id, secondary, type);
    return service.transfer(std::move(sdu));
}

ccsds::error octet_service::request(std::unique_ptr<const ccsds::base_sdu> packet, bool secondary, uint16_t name)
{
    auto sdu = assembly(std::move(packet), id, secondary, name);
    return service.transfer(std::move(sdu));
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

std::unique_ptr<const ccsds::sdu<ccsds::spp::primary_header>> octet_service::assembly(std::unique_ptr<const ccsds::base_sdu> packet, apid id, bool secondary, packet_type type)
{
    std::unique_ptr<ccsds::sdu<ccsds::spp::primary_header>> header = std::make_unique<ccsds::sdu<ccsds::spp::primary_header>>();

    (*header)->identification = ccsds::htons(
            (PACKET_VERSION_1 << PACKET_VERSION_SHIFT)         // packet version number
            | ((type & PACKET_TYPE_MASK) << PACKET_TYPE_SHIFT) // packet type
            | ((!!secondary) << PACKET_SEC_HDR_SHIFT)          // secondary header flag
            | (id & PACKET_APID_MASK));                        // APID

    (*header)->sequence_control = ccsds::htons(
            (SEQUENCE_UNSEGMENTED << SEQUENCE_FLAGS_SHIFT) // sequence flags
            | (packet_count++ & SEQUENCE_COUNT_MASK));     // packet sequence count

    (*header)->data_length = ccsds::htons(packet->totalSize() - 1);

    // Attach the header to the packet
    header->append(std::move(packet));

    return header;
}

std::unique_ptr<const ccsds::sdu<ccsds::spp::primary_header>> octet_service::assembly(std::unique_ptr<const ccsds::base_sdu> packet, apid id, bool secondary, uint16_t name)
{
    std::unique_ptr<ccsds::sdu<ccsds::spp::primary_header>> header = std::make_unique<ccsds::sdu<ccsds::spp::primary_header>>();

    (*header)->identification = ccsds::htons(
            (PACKET_VERSION_1 << PACKET_VERSION_SHIFT)         // packet version number
            | ((TELECOMMAND & PACKET_TYPE_MASK) << PACKET_TYPE_SHIFT) // packet type
            | ((!!secondary) << PACKET_SEC_HDR_SHIFT)          // secondary header flag
            | (id & PACKET_APID_MASK));                        // APID

    (*header)->sequence_control = ccsds::htons(
            (SEQUENCE_UNSEGMENTED << SEQUENCE_FLAGS_SHIFT) // sequence flags
            | (name & SEQUENCE_COUNT_MASK));     // packet sequence count

    (*header)->data_length = ccsds::htons(packet->totalSize() - 1);

    // Attach the header to the packet
    header->append(std::move(packet));

    return header;
}

ccsds::error packet_service::transfer(std::unique_ptr<const ccsds::base_sdu> sdu)
{
    if(subnetwork != nullptr){
        return subnetwork->transfer(std::move(sdu));
    }else{
        return error(error::code::NO_NETWORK);
    }
}

ccsds::error octet_service::transfer(std::unique_ptr<const ccsds::base_sdu> sdu)
{
    (void)sdu;
    return error(error::code::NO_SUPPORT);
}

/** @} */ // group spp
} // namespace spp
} // namespace ccsds
