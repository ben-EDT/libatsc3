/*
 * atsc3_mmt_mpu_parser.c
 *
 *  Created on: Jan 26, 2019
 *      Author: jjustman
 *
 *
 *      do the heavy lifting...
 *
 *
 *  jjustman-2020-11-11 - todo: refactor extract() unsafe calls with block_t() to prevent over-reads
 */

#include "atsc3_mmt_mpu_parser.h"

int _MMT_MPU_PARSER_DEBUG_ENABLED = 0;
int _MMT_MPU_PARSER_TRACE_ENABLED = 0;

//release our packet_header once we have a concrete object type
mmtp_mpu_packet_t* mmtp_mpu_packet_parse_and_free_packet_header_from_block_t(mmtp_packet_header_t** mmtp_packet_header_p, block_t* udp_packet) {
    mmtp_packet_header_t* mmtp_packet_header = *mmtp_packet_header_p;
    mmtp_mpu_packet_t* mmtp_mpu_packet = NULL;
    if(mmtp_packet_header) {
        mmtp_mpu_packet = mmtp_mpu_packet_parse_from_block_t(mmtp_packet_header, udp_packet);
        mmtp_packet_header_free(mmtp_packet_header_p);
    }
    
    return mmtp_mpu_packet;
}

mmtp_mpu_packet_t* mmtp_mpu_packet_parse_from_block_t(mmtp_packet_header_t* mmtp_packet_header, block_t* udp_packet) {
    
	if(!mmtp_packet_header) {
		__MMT_MPU_PARSER_ERROR("mmtp_mpu_packet_parse_from_block_t: mmtp_packet_header is NULL!");
		return NULL;
	}

	//MMT data_type: 0x0 -> MPU
	if(mmtp_packet_header->mmtp_payload_type != 0x0) {
		__MMT_MPU_PARSER_WARN("mmtp_mpu_packet_parse_from_block_t: mmtp_payload_type is not 0x0, type is: 0x%x",  mmtp_packet_header->mmtp_payload_type);
		return NULL;
	}

    int mmtp_mpu_payload_length = block_Remaining_size(udp_packet);
    uint8_t* udp_raw_buf = block_Get(udp_packet);
    uint8_t *buf = udp_raw_buf;

    //confirm we have at least enough to try and parse the mpu packet header
    if(mmtp_mpu_payload_length < ATSC3_MMTP_MPU_PACKET_PARSE_FROM_BLOCK_T_MINIMUM_SANE_LENGTH_BYTES) {
        __MMT_MPU_PARSER_WARN("mmtp_mpu_packet_parse_from_block_t: mmtp_mpu_payload_length: %d is less than sane mmt packet header length of: %d, bailing!",  mmtp_mpu_payload_length, ATSC3_MMTP_MPU_PACKET_PARSE_FROM_BLOCK_T_MINIMUM_SANE_LENGTH_BYTES);
        return NULL;
    }

	mmtp_mpu_packet_t* mmtp_mpu_packet = mmtp_mpu_packet_new();
	memcpy(mmtp_mpu_packet, mmtp_packet_header, sizeof(mmtp_packet_header_t));

	//create a sub_flow with this packet_id
	__MMT_MPU_PARSER_DEBUG("mmtp_mpu_packet_parse_from_block_t: udp_packet->i_pos: %d, udp_packet->p_size: %d, udp_packet->p_buffer: %p, mmtp_packet_id is: %d, mmtp_payload_type: 0x%x, packet_counter: %d",
		udp_packet->i_pos,
		udp_packet->p_size,
		udp_packet->p_buffer,
		mmtp_packet_header->mmtp_packet_id,
		mmtp_packet_header->mmtp_payload_type,
		mmtp_packet_header->packet_counter);

    //pull the mpu and frag information

    uint8_t mpu_payload_length_block[2];

    //+2 bytes, total read: 2 bytes
    buf = (uint8_t*)extract(buf, (uint8_t*)&mpu_payload_length_block, 2);
    mmtp_mpu_packet->data_unit_length = (mpu_payload_length_block[0] << 8) | mpu_payload_length_block[1];

    __MMT_MPU_PARSER_TRACE("mmtp_mpu_packet_parse_from_block_t: doing mpu_payload_length: %hu (0x%X 0x%X)",
            mmtp_mpu_packet->data_unit_length, mpu_payload_length_block[0], mpu_payload_length_block[1]);

    uint8_t mpu_fragmentation_info;

    //+1 bytes, total read: 3 bytes
    buf = (uint8_t*)extract(buf, &mpu_fragmentation_info, 1);

    mmtp_mpu_packet->mpu_fragment_type = (mpu_fragmentation_info & 0xF0) >> 4;
    mmtp_mpu_packet->mpu_timed_flag = (mpu_fragmentation_info & 0x8) >> 3;
    mmtp_mpu_packet->mpu_fragmentation_indicator = (mpu_fragmentation_info & 0x6) >> 1;
    mmtp_mpu_packet->mpu_aggregation_flag = (mpu_fragmentation_info & 0x1);

    __MMT_MPU_PARSER_TRACE("mmtp_mpu_packet_parse_from_block_t: mpu_fragment_type: %d, mpu_fragmentation_indicator: 0x%x, mpu_fragment_type: 0x%x, mpu_timed_flag: 0x%x, mpu_fragmentation_indicator: 0x%x, mpu_aggregation_flag: 0x%x",
        mmtp_mpu_packet->mpu_fragment_type,
        mpu_fragmentation_info,
        mmtp_mpu_packet->mpu_fragment_type,
        mmtp_mpu_packet->mpu_timed_flag,
        mmtp_mpu_packet->mpu_fragmentation_indicator,
        mmtp_mpu_packet->mpu_aggregation_flag);

    uint8_t mpu_fragment_counter;

    //+1 bytes, total read: 4 bytes
    buf = (uint8_t*)extract(buf, &mpu_fragment_counter, 1);
    mmtp_mpu_packet->mpu_fragment_counter = mpu_fragment_counter;

    //re-fanagle
    uint8_t mpu_sequence_number_block[4];

    //+4 bytes, total read: 8 bytes
    buf = (uint8_t*)extract(buf, (uint8_t*)&mpu_sequence_number_block, 4);
    mmtp_mpu_packet->mpu_sequence_number = ntohl(*((uint32_t*)(&mpu_sequence_number_block[0])));

    __MMT_MPU_PARSER_TRACE("mmtp_mpu_packet_parse_from_block_t: mpu_payload_length: %hu (0x%X 0x%X), mpu_fragmentation_counter: %d, mpu_sequence_number: %d",
            mmtp_mpu_packet->data_unit_length,
            mpu_payload_length_block[0],
            mpu_payload_length_block[1],
            mmtp_mpu_packet->mpu_fragment_counter,
            mmtp_mpu_packet->mpu_sequence_number);

    int remaining_du_packet_len = -1;

    //TODO: if FEC_type != 0, parse out source_FEC_payload_ID trailing bits...
    do {
        //pull out aggregate packets data unit length
        int to_read_packet_length = -1;
        //mpu_fragment_type

        //only read DU length if mpu_aggregation_flag=1
        if(mmtp_mpu_packet->mpu_aggregation_flag) {
            uint8_t data_unit_length_block[2];
            buf = (uint8_t*)extract(buf, (uint8_t*)&data_unit_length_block, 2);
            mmtp_mpu_packet->data_unit_length = (data_unit_length_block[0] << 8) | (data_unit_length_block[1]);
            to_read_packet_length = mmtp_mpu_packet->data_unit_length;
            __MMT_MPU_PARSER_TRACE("mmtp_mpu_packet_parse_from_block_t: %d, mpu_aggregation_flag:1, to_read_packet_length: %d",
                    mmtp_mpu_packet->data_unit_length, to_read_packet_length);

        } else {
            to_read_packet_length = mmtp_mpu_payload_length - (buf - udp_raw_buf);
            __MMT_MPU_PARSER_TRACE("mmtp_mpu_packet_parse_from_block_t: using data_unit_size from packet length: mpu_aggregation_flag:0, raw packet size: %d, buf: %p, raw_buf: %p, to_read_packet_length: %d",
                    mmtp_mpu_payload_length,
                    buf,
                    udp_raw_buf,
                    to_read_packet_length);
        }

        if(to_read_packet_length > (mmtp_mpu_payload_length - (buf - udp_raw_buf))) {
            __MMT_MPU_PARSER_ERROR("mmtp_mpu_packet_parse_from_block_t: to_read_packet_length: %d is larger than remaining packet length: %lu, mmtp_mpu_packet_parse_from_block_t: udp_packet->i_pos: %d, udp_packet->p_size: %d, udp_packet->p_buffer: %p, mmtp_packet_id is: %d, mmtp_payload_type: 0x%x, packet_counter: %d",
                    to_read_packet_length,
					(mmtp_mpu_payload_length - (buf - udp_raw_buf)),
                    udp_packet->i_pos,
                    udp_packet->p_size,
                    udp_packet->p_buffer,
                    mmtp_packet_header->mmtp_packet_id,
                    mmtp_packet_header->mmtp_payload_type,
                    mmtp_packet_header->packet_counter);


            goto error;
        }

        //copy remaining packet bytes into du_type block_t*
        //mpu fragment type: 0x0 -> MPU Metadata
        if(mmtp_mpu_packet->mpu_fragment_type == 0x0) {
            __MMT_MPU_PARSER_TRACE("mmtp_mpu_packet_parse_from_block_t: writing to du_mpu_metadata_block, mpu_fragement_type is: 0x%x, len: %d, buf: %p",
                    mmtp_mpu_packet->mpu_fragment_type,
                    to_read_packet_length,
                    buf);

            mmtp_mpu_packet->du_mpu_metadata_block = block_Alloc(to_read_packet_length);
            block_Write(mmtp_mpu_packet->du_mpu_metadata_block, buf, to_read_packet_length);
        } else if(mmtp_mpu_packet->mpu_fragment_type == 0x2) {
            /**
             process timed and mmthsample box here
             **/

            block_t* temp_timed_buffer = block_Alloc(to_read_packet_length);
            block_Write(temp_timed_buffer, buf, to_read_packet_length);
            block_Rewind(temp_timed_buffer);

            if(!atsc3_mmt_mpu_sample_format_parse(mmtp_mpu_packet, temp_timed_buffer)) {
                goto error;
            }

            mmtp_mpu_packet->du_mfu_block = block_Duplicate_from_position(temp_timed_buffer);

            __MMT_MPU_PARSER_TRACE("mmtp_mpu_packet_parse_from_block_t: writing to du_mfu_block, mpu_fragement_type is: 0x%x, len: %d, buf: %p",
                                    mmtp_mpu_packet->mpu_fragment_type,
                                    mmtp_mpu_packet->du_mfu_block->p_size,
                                    mmtp_mpu_packet->du_mfu_block);

            block_Destroy(&temp_timed_buffer);

        } else if(mmtp_mpu_packet->mpu_fragment_type == 0x1) {
            __MMT_MPU_PARSER_TRACE("mmtp_mpu_packet_parse_from_block_t: writing to du_movie_fragment_block, mpu_fragement_type is: 0x%x, len: %d, buf: %p",
                                    mmtp_mpu_packet->mpu_fragment_type,
                                    to_read_packet_length,
                                    buf);

            mmtp_mpu_packet->du_movie_fragment_block = block_Alloc(to_read_packet_length);
            block_Write(mmtp_mpu_packet->du_movie_fragment_block, buf, to_read_packet_length);
        }
        buf += to_read_packet_length;

        remaining_du_packet_len = mmtp_mpu_payload_length - (buf - udp_raw_buf);

        if(remaining_du_packet_len && !mmtp_mpu_packet->mpu_aggregation_flag) {
            __MMT_MPU_PARSER_ERROR("mmtp_mpu_packet_parse_from_block_t: after reading du, remaining bytes are: %d", remaining_du_packet_len);
        }

        compute_ntp32_to_seconds_microseconds(mmtp_mpu_packet->mmtp_timestamp, &mmtp_mpu_packet->mmtp_timestamp_s, &mmtp_mpu_packet->mmtp_timestamp_us);
        __MMT_MPU_PARSER_TRACE("mmtp_mpu_packet_parse_from_block_t: converting mmtp_timestamp: %u to seconds: %hu, microseconds: %hu",
                mmtp_mpu_packet->mmtp_timestamp,
                mmtp_mpu_packet->mmtp_timestamp_s,
                mmtp_mpu_packet->mmtp_timestamp_us);

    } while(mmtp_mpu_packet->mpu_aggregation_flag && remaining_du_packet_len > 0);

	__MMT_MPU_PARSER_TRACE("mmtp_mpu_packet_parse_from_block_t: returning mmtp_mpu_packet: %p", mmtp_mpu_packet);

	return mmtp_mpu_packet;

error:

    if(mmtp_mpu_packet) {
        mmtp_mpu_packet_free(&mmtp_mpu_packet);
    }
	return NULL;
}


//	mmt_mpu_parse_payload();
//
//	} else
//#if _ISO230081_1_MMTP_GFD_SUPPORT_
//	if(mmtp_payload_type == 0x1) {
//		_MMSM_ERROR_23008_1("MMTP_GFD: not supported for packet_id: %-10hu (0x%04x)", mmtp_packet_id, mmtp_packet_id);
//		goto failed;
//	} else
//#endif
//	if(mmtp_payload_type == 0x2) {
//		uint32_t new_size = udp_raw_buf_size - (raw_packet_ptr - udp_raw_buf);
//		raw_packet_ptr = mmt_signaling_message_parse_packet_header(mmtp_payload_fragments, raw_packet_ptr, new_size);
//
//		new_size = udp_raw_buf_size - (raw_packet_ptr - udp_raw_buf);
//		raw_packet_ptr = mmt_signaling_message_parse_packet(mmtp_payload_fragments, raw_packet_ptr, new_size);
//
//	} else {
//		_MMTP_WARN("mmtp_mpu_packet_parse_from_block_t: unknown payload type of 0x%x for %-10hu (0x%04x)", mmtp_payload_type,
//				mmtp_packet_id, mmtp_packet_id);
//		goto failed;
//	}
//
//	return mmtp_payload_fragments;
//
//failed:
//	if(mmtp_payload_fragments) {
//		free(mmtp_payload_fragments);
//		mmtp_payload_fragments = NULL;
//	}
//	return NULL;
//
//}




