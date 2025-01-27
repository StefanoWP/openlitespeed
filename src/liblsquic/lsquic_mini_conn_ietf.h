/* Copyright (c) 2017 - 2019 LiteSpeed Technologies Inc.  See LICENSE. */
/*
 * lsquic_mini_conn_ietf.h -- Mini connection used by the IETF QUIC
 */

#ifndef LSQUIC_MINI_CONN_IETF_H
#define LSQUIC_MINI_CONN_IETF_H 1

struct lsquic_conn;
struct lsquic_engine_public;
struct lsquic_packet_in;

enum { MCSBIT_WANTREAD, MCSBIT_WANTWRITE, };

struct mini_crypto_stream
{
    unsigned        mcs_read_off;
    unsigned        mcs_write_off;
    enum {
        MCS_WANTREAD    = 1 << MCSBIT_WANTREAD,
        MCS_WANTWRITE   = 1 << MCSBIT_WANTWRITE,
        MCS_CREATED     = 1 << 2,
    }               mcs_flags:8;
    enum enc_level  mcs_enc_level:8;
};

typedef uint64_t packno_set_t;
#define MAX_PACKETS ((sizeof(packno_set_t) * 8) - 1)

struct ietf_mini_conn
{
    struct lsquic_conn              imc_conn;
    struct conn_cid_elem            imc_cces[3];
    struct lsquic_engine_public    *imc_enpub;
    lsquic_time_t                   imc_created;
    enum {
        IMC_ENC_SESS_INITED     = 1 << 0,
        IMC_QUEUED_ACK_INIT     = 1 << 1,
        IMC_QUEUED_ACK_HSK      = IMC_QUEUED_ACK_INIT << PNS_HSK,
        IMC_QUEUED_ACK_APP      = IMC_QUEUED_ACK_INIT << PNS_APP,
        IMC_ERROR               = 1 << 4,
        IMC_HSK_OK              = 1 << 5,
        IMC_HSK_FAILED          = 1 << 6,
        IMC_HAVE_TP             = 1 << 7,
        IMC_RETRY_MODE          = 1 << 8,
        IMC_RETRY_DONE          = 1 << 9,
        IMC_IGNORE_INIT         = 1 << 10,
#define IMCBIT_PNS_BIT_SHIFT 11
        IMC_MAX_PNS_BIT_0       = 1 << 11,
        IMC_MAX_PNS_BIT_1       = 1 << 12,
        IMC_TLS_ALERT           = 1 << 13,
        IMC_ABORT_ERROR         = 1 << 14,
        IMC_ABORT_ISAPP         = 1 << 15,
        IMC_BAD_TRANS_PARAMS    = 1 << 16,
        IMC_ADDR_VALIDATED      = 1 << 17,
        IMC_HSK_PACKET_SENT     = 1 << 18,
    }                               imc_flags;
    struct mini_crypto_stream       imc_streams[N_ENC_LEVS];
    void                           *imc_stream_ps[N_ENC_LEVS];
    struct {
        struct stream_frame    *frame;   /* Latest frame - on stack - be careful. */
        enum enc_level          enc_level;
    }                               imc_last_in;
    TAILQ_HEAD(, lsquic_packet_in)  imc_app_packets;
    TAILQ_HEAD(, lsquic_packet_out) imc_packets_out;
    packno_set_t                    imc_sent_packnos;
    packno_set_t                    imc_recvd_packnos[N_PNS];
    packno_set_t                    imc_acked_packnos[N_PNS];
    lsquic_time_t                   imc_largest_recvd[N_PNS];
    struct lsquic_rtt_stats         imc_rtt_stats;
    unsigned                        imc_error_code;
    unsigned                        imc_bytes_in;
    unsigned                        imc_bytes_out;
    unsigned char                   imc_next_packno;
    unsigned char                   imc_hsk_count;
    /* We don't send more than eight in the first flight, and so it's OK to
     * use uint8_t.  This value is also used as a boolean: when ECN black
     * hole is detected, it is set to zero to indicate that black hole
     * detection is no longer active.
     */
    uint8_t                         imc_ecn_packnos;
    uint8_t                         imc_ack_exp;
    uint8_t                         imc_ecn_counts_in[N_PNS][4];
    uint8_t                         imc_ecn_counts_out[N_PNS][4];
    uint8_t                         imc_incoming_ecn;
    uint8_t                         imc_tls_alert;
    struct network_path             imc_path;
};

struct lsquic_conn *
lsquic_mini_conn_ietf_new (struct lsquic_engine_public *,
               const struct lsquic_packet_in *,
               enum lsquic_version, int is_ipv4, const struct lsquic_cid *);

int
lsquic_mini_conn_ietf_ecn_ok (const struct ietf_mini_conn *);
#endif
