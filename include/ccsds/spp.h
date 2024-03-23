/**
 * @file ccsds/spp.h
 * Space Packet Protocol
 * @ingroup spp
 */

#ifndef CCSDS_SPP_H_
#define CCSDS_SPP_H_

#include "ccsds/common.h"
#include <cstdint>
#include <memory>

namespace ccsds {
namespace spp {
/**
 * @ingroup ccsds
 * @defgroup spp Space Packet Protocol
 * @{
 * Space Packet Protocol services
 * @see https://public.ccsds.org/Pubs/133x0b2e1.pdf
 * @cite ccsds-spp
 */

#pragma pack(push,1)

/**
 * Variable-length, octet-aligned data.
 * @requirement SPP-2
 */
typedef uint8_t octet_string[];

/**
 * Space Packet Primary Header.
 * @requirement SPP-15
 */
struct primary_header {
    uint16_t      identification;   ///< Packet version number, packet type, sec. hdr. flag, APID
    uint16_t      sequence_control; ///< Sequence flags, sequence count
    uint16_t      data_length;      ///< Data length - 1
};

static_assert(sizeof(primary_header) == 6);

/**
 * Space Packet.
 * @requirement SPP-14
 */
struct space_packet {
    primary_header header;      ///< Packet primary header.
    octet_string   packet_data; ///< @requirement SPP-16 Packet data field.
};

static_assert(sizeof(space_packet) == 6);

/**
 * Application Protocol Identifier (APID)
 */
enum apid : uint16_t {
    APID_MIN  = 0,     ///< Minimum APID value.
    APID_MAX  = 0x7FE, ///< Maximum APID value.
    APID_IDLE = 0x7FF  ///< APID reserved for Idle Packets.
};

/**
 * Space packet packet type.
 */
enum packet_type : bool {
    TELEMETRY   = 0, ///< Telemetry packet type.
    TELECOMMAND = 1, ///< Telecommand packet type.
};

/**
 * Space Packet Transmit Service.
 */
class packet_service : public ccsds::base_service {
    public:
        /**
         * Constructor.
         * @param subnetwork Subnetwork to transmit packets on.
         */
        packet_service(ccsds::base_service* subnetwork);

        /**
         * Destructor.
         */
        virtual ~packet_service() = default;

        /**
         * Send a pre-formated space packet.
         * @requirement SPP-10
         * @param packet Packet to send.
         * @param id APID of the packet.
         * @requirement SPP-3
         * @param qos Quality of Service requirement.
         * @requirement SPP-5
         * @retval error::code::NONE if successful.
         * @retval other from the subnetwork.
         */
        ccsds::error request(std::unique_ptr<const ccsds::sdu<ccsds::spp::primary_header>> packet, apid id, int qos = 0);

        /**
         * Callback function for receiving an octet string.
         * @requirement SPP-11
         * @param packet Packet to send.
         * @param id APID of the packet.
         * @requirement SPP-3
         * @param packet_loss Packet Loss Indicator.
         * @requirement SPP-4
         */
        typedef void (*indication)(ccsds::sdu<ccsds::spp::primary_header> *packet, apid id, bool packet_loss);

        /**
         * Set the indication callback function.
         * @param func Callback function.
         */
        void set_indication(indication* func);

        /**
         * Transfer an SDU from another service.
         * @requirement SPP-20
         * @param sdu SDU to transfer.
         * @retval error::code::NO_NETWORK if a subnetwork has not been configured.
         * @retval other from the subnetwork.
         */
        virtual ccsds::error transfer(std::unique_ptr<const ccsds::base_sdu> sdu) override;

    private:
        ccsds::base_service* subnetwork; ///< Subnetwork to transmit packets on.
        indication*          callback;   ///< Indication callback function.
};

/**
 * Space Packet Receive Service.
 */
class octet_service : public ccsds::base_service {
    public:
        /**
         * Constructor.
         * @param id APID of the service.
         * @requirement SPP-7
         * @param subnetwork Subnetwork to transmit packets on.
         */
        octet_service(apid id, ccsds::base_service* subnetwork);

        /**
         * Destructor.
         */
        virtual ~octet_service() = default;

        /**
         * Send a space packet using a packet count with the given octet string.
         * @requirement SPP-12
         * @param packet Packet to send.
         * @requirement SPP-6
         * @param secondary Secondary header indicator.
         * @requirement SPP-8
         * @param type Packet type.
         * @retval error::code::NONE if successful.
         * @retval other from the subnetwork.
         */
        ccsds::error request(std::unique_ptr<const ccsds::base_sdu> packet, bool secondary, packet_type type);

        /**
         * Send a space packet using a packet name with the given octet string.
         * @requirement SPP-12
         * @param packet Packet to send.
         * @requirement SPP-6
         * @param secondary Secondary header indicator.
         * @requirement SPP-8
         * @param name Packet name.
         * @retval error::code::NONE if successful.
         * @retval other from the subnetwork.
         */
        ccsds::error request(std::unique_ptr<const ccsds::base_sdu> packet, bool secondary, uint16_t name);

        /**
         * Callback function for receiving an octet string.
         * @requirement SPP-13
         * @param packet Packet to send.
         * @param id APID of the packet.
         * @requirement SPP-7
         * @param data_loss data Loss Indicator.
         * @requirement SPP-9
         */
        typedef void (*indication)(ccsds::base_sdu *packet, apid id, bool data_loss);

        /**
         * Set the indication callback function.
         * @param func Callback function.
         */
        void set_indication(indication* func);

    private:
        /**
         * Transfer an SDU from another service.
         * @warning The octet service does not allow direct transfers,
         * @requirement SPP-20
         * @param sdu SDU to transfer.
         * @retval error::code::NO_SUPPORT The octet service does not allow direct transfers,
         */
        virtual ccsds::error transfer(std::unique_ptr<const ccsds::base_sdu> sdu) override;

    protected:
        /**
         * Assemble a space packet with a packet count.
         * @requirement SPP-19
         * @param packet Packet to send.
         * @requirement SPP-6
         * @param id APID of the packet.
         * @requirement SPP-7
         * @param secondary Secondary header indicator.
         * @requirement SPP-8
         * @param type Packet type.
         * @return Newly assembled pace packet.
         */
        std::unique_ptr<const ccsds::sdu<ccsds::spp::primary_header>> assembly(std::unique_ptr<const ccsds::base_sdu> packet, apid id, bool secondary, packet_type type);

        /**
         * Assemble a space packet with a packet name.
         * @requirement SPP-19
         * @param packet Packet to send.
         * @requirement SPP-6
         * @param id APID of the packet.
         * @requirement SPP-7
         * @param secondary Secondary header indicator.
         * @requirement SPP-8
         * @param name Packet name.
         * @return Newly assembled pace packet.
         */
        std::unique_ptr<const ccsds::sdu<ccsds::spp::primary_header>> assembly(std::unique_ptr<const ccsds::base_sdu> packet, apid id, bool secondary, uint16_t name);

    private:
        packet_service service;      ///< Underlying packet service
        indication*    callback;     ///< Indication callback function.
        apid           id;           ///< APID for the service.
        uint16_t       packet_count; ///< Current packet count.
};

#pragma pack(pop)

/** @} */ // group spp
} // namespace spp
} // namespace ccsds

#endif // CCSDS_SPP_H_
