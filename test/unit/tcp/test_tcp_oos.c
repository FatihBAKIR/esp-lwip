#include "test_tcp_oos.h"

#include "lwip/tcp.h"
#include "lwip/stats.h"
#include "tcp_helper.h"

#if !LWIP_STATS || !TCP_STATS || !MEMP_STATS
#error "This tests needs TCP- and MEMP-statistics enabled"
#endif
#if !TCP_QUEUE_OOSEQ
#error "This tests needs TCP_QUEUE_OOSEQ enabled"
#endif


/* Setups/teardown functions */

static void
tcp_oos_setup(void)
{
  tcp_remove_all();
}

static void
tcp_oos_teardown(void)
{
  tcp_remove_all();
}



/* Test functions */

/** create multiple segments and pass them to tcp_input in a wrong
 * order to see if ooseq-caching works correctly
 * FIN is received in out-of-sequence segments only */
START_TEST(test_tcp_recv_ooseq_FIN_OOSEQ)
{
  struct test_tcp_counters counters;
  struct tcp_pcb* pcb;
  struct pbuf *p1, *p2, *p3, *p4, *pinseq;
  char data[] = {
     1,  2,  3,  4,
     5,  6,  7,  8,
     9, 10, 11, 12,
    13, 14, 15, 16};
  struct ip_addr remote_ip, local_ip;
  u16_t data_len;
  u16_t remote_port = 0x100, local_port = 0x101;
  struct netif netif;
  LWIP_UNUSED_ARG(_i);

  /* initialize local vars */
  memset(&netif, 0, sizeof(netif));
  IP4_ADDR(&local_ip, 192, 168, 1, 1);
  IP4_ADDR(&remote_ip, 192, 168, 1, 2);
  data_len = sizeof(data);
  /* initialize counter struct */
  memset(&counters, 0, sizeof(counters));
  counters.expected_data_len = data_len;
  counters.expected_data = data;

  /* create and initialize the pcb */
  pcb = test_tcp_new_counters_pcb(&counters);
  EXPECT_RET(pcb != NULL);
  tcp_set_state(pcb, ESTABLISHED, &local_ip, &remote_ip, local_port, remote_port);

  /* create segments */
  /* pinseq is sent as last segment! */
  pinseq = tcp_create_rx_segment(pcb, &data[0],  4, 0, 0, TCP_ACK);
  /* p1: 8 bytes before FIN */
  /*     seqno: 8..15 */
  p1     = tcp_create_rx_segment(pcb, &data[8],  8, 8, 0, TCP_ACK|TCP_FIN);
  /* p2: 4 bytes before p1, including the first 4 bytes of p1 (partly duplicate) */
  /*     seqno: 4..11 */
  p2     = tcp_create_rx_segment(pcb, &data[4],  8, 4, 0, TCP_ACK);
  /* p3: same as p2 but 2 bytes longer */
  /*     seqno: 4..13 */
  p3     = tcp_create_rx_segment(pcb, &data[4], 10, 4, 0, TCP_ACK);
  /* p4: 14 bytes before FIN, includes data from p1 and p2, plus partly from pinseq */
  /*     seqno: 2..15 */
  p4     = tcp_create_rx_segment(pcb, &data[2], 14, 2, 0, TCP_ACK|TCP_FIN);
  EXPECT(pinseq != NULL);
  EXPECT(p1 != NULL);
  EXPECT(p2 != NULL);
  EXPECT(p3 != NULL);
  EXPECT(p4 != NULL);
  if ((pinseq != NULL) && (p1 != NULL) && (p2 != NULL) && (p3 != NULL) && (p4 != NULL)) {
    /* pass the segment to tcp_input */
    tcp_input(p1, &netif);
    /* check if counters are as expected */
    EXPECT(counters.close_calls == 0);
    EXPECT(counters.recv_calls == 0);
    EXPECT(counters.recved_bytes == 0);
    EXPECT(counters.err_calls == 0);
    /* @todo check ooseq queue? */

    /* pass the segment to tcp_input */
    tcp_input(p2, &netif);
    /* check if counters are as expected */
    EXPECT(counters.close_calls == 0);
    EXPECT(counters.recv_calls == 0);
    EXPECT(counters.recved_bytes == 0);
    EXPECT(counters.err_calls == 0);
    /* @todo check ooseq queue? */

    /* pass the segment to tcp_input */
    tcp_input(p3, &netif);
    /* check if counters are as expected */
    EXPECT(counters.close_calls == 0);
    EXPECT(counters.recv_calls == 0);
    EXPECT(counters.recved_bytes == 0);
    EXPECT(counters.err_calls == 0);
    /* @todo check ooseq queue? */

    /* pass the segment to tcp_input */
    tcp_input(p4, &netif);
    /* check if counters are as expected */
    EXPECT(counters.close_calls == 0);
    EXPECT(counters.recv_calls == 0);
    EXPECT(counters.recved_bytes == 0);
    EXPECT(counters.err_calls == 0);
    /* @todo check ooseq queue? */

    /* pass the segment to tcp_input */
    tcp_input(pinseq, &netif);
    /* check if counters are as expected */
    EXPECT(counters.close_calls == 1);
    EXPECT(counters.recv_calls == 1);
    EXPECT(counters.recved_bytes == data_len);
    EXPECT(counters.err_calls == 0);
    EXPECT(pcb->ooseq == NULL);
  }

  /* make sure the pcb is freed */
  EXPECT(lwip_stats.memp[MEMP_TCP_PCB].used == 1);
  tcp_abort(pcb);
  EXPECT(lwip_stats.memp[MEMP_TCP_PCB].used == 0);
}
END_TEST


/** create multiple segments and pass them to tcp_input in a wrong
 * order to see if ooseq-caching works correctly
 * FIN is received IN-SEQUENCE at the end */
START_TEST(test_tcp_recv_ooseq_FIN_INSEQ)
{
  struct test_tcp_counters counters;
  struct tcp_pcb* pcb;
  struct pbuf *p1, *p2, *p3, *p4, *p5, *p6, *pinseq, *pinseqFIN;
  char data[] = {
     1,  2,  3,  4,
     5,  6,  7,  8,
     9, 10, 11, 12,
    13, 14, 15, 16};
  struct ip_addr remote_ip, local_ip;
  u16_t data_len;
  u16_t remote_port = 0x100, local_port = 0x101;
  struct netif netif;
  LWIP_UNUSED_ARG(_i);

  /* initialize local vars */
  memset(&netif, 0, sizeof(netif));
  IP4_ADDR(&local_ip, 192, 168, 1, 1);
  IP4_ADDR(&remote_ip, 192, 168, 1, 2);
  data_len = sizeof(data);
  /* initialize counter struct */
  memset(&counters, 0, sizeof(counters));
  counters.expected_data_len = data_len;
  counters.expected_data = data;

  /* create and initialize the pcb */
  pcb = test_tcp_new_counters_pcb(&counters);
  EXPECT_RET(pcb != NULL);
  tcp_set_state(pcb, ESTABLISHED, &local_ip, &remote_ip, local_port, remote_port);

  /* create segments */
  /* p1: 7 bytes - 2 before FIN */
  /*     seqno: 1..2 */
  p1     = tcp_create_rx_segment(pcb, &data[1],  2, 1, 0, TCP_ACK);
  /* p2: 4 bytes before p1, including the first 4 bytes of p1 (partly duplicate) */
  /*     seqno: 4..11 */
  p2     = tcp_create_rx_segment(pcb, &data[4],  8, 4, 0, TCP_ACK);
  /* p3: same as p2 but 2 bytes longer and one byte more at the front */
  /*     seqno: 3..13 */
  p3     = tcp_create_rx_segment(pcb, &data[3], 11, 3, 0, TCP_ACK);
  /* p4: 13 bytes - 2 before FIN - should be ignored as contained in p1 and p3 */
  /*     seqno: 2..13 */
  p4     = tcp_create_rx_segment(pcb, &data[2], 12, 2, 0, TCP_ACK);
  /* pinseq is the first segment that is held back to create ooseq! */
  /*     seqno: 0..3 */
  pinseq = tcp_create_rx_segment(pcb, &data[0],  4, 0, 0, TCP_ACK);
  /* p5: last byte before FIN */
  /*     seqno: 15 */
  p5     = tcp_create_rx_segment(pcb, &data[15], 1, 15, 0, TCP_ACK);
  /* p6: same as p5, should be ignored */
  p6     = tcp_create_rx_segment(pcb, &data[15], 1, 15, 0, TCP_ACK);
  /* pinseqFIN: last 2 bytes plus FIN */
  /*     only segment containing seqno 14 and FIN */
  pinseqFIN = tcp_create_rx_segment(pcb,  &data[14], 2, 14, 0, TCP_ACK|TCP_FIN);
  EXPECT(pinseq != NULL);
  EXPECT(p1 != NULL);
  EXPECT(p2 != NULL);
  EXPECT(p3 != NULL);
  EXPECT(p4 != NULL);
  EXPECT(p5 != NULL);
  EXPECT(p6 != NULL);
  EXPECT(pinseqFIN != NULL);
  if ((pinseq != NULL) && (p1 != NULL) && (p2 != NULL) && (p3 != NULL) && (p4 != NULL)
    && (p5 != NULL) && (p6 != NULL) && (pinseqFIN != NULL)) {
    /* pass the segment to tcp_input */
    tcp_input(p1, &netif);
    /* check if counters are as expected */
    EXPECT(counters.close_calls == 0);
    EXPECT(counters.recv_calls == 0);
    EXPECT(counters.recved_bytes == 0);
    EXPECT(counters.err_calls == 0);
    /* @todo check ooseq queue? */

    /* pass the segment to tcp_input */
    tcp_input(p2, &netif);
    /* check if counters are as expected */
    EXPECT(counters.close_calls == 0);
    EXPECT(counters.recv_calls == 0);
    EXPECT(counters.recved_bytes == 0);
    EXPECT(counters.err_calls == 0);
    /* @todo check ooseq queue? */

    /* pass the segment to tcp_input */
    tcp_input(p3, &netif);
    /* check if counters are as expected */
    EXPECT(counters.close_calls == 0);
    EXPECT(counters.recv_calls == 0);
    EXPECT(counters.recved_bytes == 0);
    EXPECT(counters.err_calls == 0);
    /* @todo check ooseq queue? */

    /* pass the segment to tcp_input */
    tcp_input(p4, &netif);
    /* check if counters are as expected */
    EXPECT(counters.close_calls == 0);
    EXPECT(counters.recv_calls == 0);
    EXPECT(counters.recved_bytes == 0);
    EXPECT(counters.err_calls == 0);
    /* @todo check ooseq queue? */

    /* pass the segment to tcp_input */
    tcp_input(pinseq, &netif);
    /* check if counters are as expected */
    EXPECT(counters.close_calls == 0);
    EXPECT(counters.recv_calls == 1);
    EXPECT(counters.recved_bytes == 14);
    EXPECT(counters.err_calls == 0);
    EXPECT(pcb->ooseq == NULL);

    /* pass the segment to tcp_input */
    tcp_input(p5, &netif);
    /* check if counters are as expected */
    EXPECT(counters.close_calls == 0);
    EXPECT(counters.recv_calls == 1);
    EXPECT(counters.recved_bytes == 14);
    EXPECT(counters.err_calls == 0);
    /* @todo check ooseq queue? */
    EXPECT(pcb->ooseq != NULL);
    if(pcb->ooseq != NULL) {
      EXPECT(pcb->ooseq->next == NULL);
    }

    /* pass the segment to tcp_input */
    tcp_input(p6, &netif);
    /* check if counters are as expected */
    EXPECT(counters.close_calls == 0);
    EXPECT(counters.recv_calls == 1);
    EXPECT(counters.recved_bytes == 14);
    EXPECT(counters.err_calls == 0);
    /* @todo check ooseq queue? */
    EXPECT(pcb->ooseq != NULL);
    if(pcb->ooseq != NULL) {
      EXPECT(pcb->ooseq->next == NULL);
    }

    /* pass the segment to tcp_input */
    tcp_input(pinseqFIN, &netif);
    /* check if counters are as expected */
    EXPECT(counters.close_calls == 1);
    EXPECT(counters.recv_calls == 2);
    EXPECT(counters.recved_bytes == data_len);
    EXPECT(counters.err_calls == 0);
    EXPECT(pcb->ooseq == NULL);
  }

  /* make sure the pcb is freed */
  EXPECT(lwip_stats.memp[MEMP_TCP_PCB].used == 1);
  tcp_abort(pcb);
  EXPECT(lwip_stats.memp[MEMP_TCP_PCB].used == 0);
}
END_TEST

/** Create the suite including all tests for this module */
Suite *
tcp_oos_suite(void)
{
  TFun tests[] = {
    test_tcp_recv_ooseq_FIN_OOSEQ,
    test_tcp_recv_ooseq_FIN_INSEQ,
  };
  return create_suite("TCP_OOS", tests, sizeof(tests)/sizeof(TFun), tcp_oos_setup, tcp_oos_teardown);
}