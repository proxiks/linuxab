/* SPDX-License-Identifier: GPL-2.0 */
/*
 * linuxab/include/linuxab/net/bridge.h
 * Ethernet bridge (802.1D) for linuxab server
 */

#ifndef _LINUXAB_BRIDGE_H
#define _LINUXAB_BRIDGE_H

#include "types.h"

#define BR_HAIRPIN_MODE     (1 << 0)
#define BR_BPDU_GUARD       (1 << 1)
#define BR_ROOT_BLOCK       (1 << 2)
#define BR_MULTICAST_FAST_LEAVE (1 << 3)
#define BR_ADMIN_COST       (1 << 4)
#define BR_LEARNING         (1 << 5)
#define BR_FLOOD            (1 << 6)
#define BR_PROXYARP         (1 << 7)
#define BR_PROXYARP_WIFI    (1 << 8)
#define BR_MCAST_TO_UCAST   (1 << 9)
#define BR_VLAN_TUNNEL      (1 << 10)
#define BR_ISOLATED         (1 << 11)
#define BR_MRP_AWARE        (1 << 12)
#define BR_MRP_IN_OPEN      (1 << 13)

#define BR_STATE_DISABLED   0
#define BR_STATE_LISTENING  1
#define BR_STATE_LEARNING   2
#define BR_STATE_FORWARDING 3
#define BR_STATE_BLOCKING   4

#define BR_VERSION          "2.3"

/* Bridge forwarding database */
struct net_bridge_fdb_entry {
    struct hlist_node hlist;
    struct net_bridge_port *dst;
    struct net_bridge *br;
    unsigned long updated;
    unsigned long used;
    uint8_t mac_addr[6];
    uint16_t flags;
    atomic_t use_count;
};

#define FDB_LOCAL       (1 << 0)
#define FDB_STATIC      (1 << 1)
#define FDB_ADDED_BY_USER  (1 << 2)
#define FDB_ADDED_BY_EXT_LEARN (1 << 3)
#define FDB_OFFLOADED   (1 << 4)
#define FDB_STICKY      (1 << 5)

/* Bridge port */
struct net_bridge_port {
    struct net_bridge *br;
    struct net_device *dev;
    struct list_head list;
    uint8_t state;
    uint8_t priority;
    uint16_t port_no;
    unsigned int flags;
    uint32_t path_cost;
    uint32_t designated_root[2];
    uint32_t designated_cost;
    uint32_t designated_bridge[2];
    uint16_t designated_port;
    uint16_t designated_port_no;
    uint32_t forward_delay_timer;
    uint32_t hold_timer;
    uint32_t message_age_timer;
    uint32_t topology_change_ack;
    uint32_t config_pending;
    uint8_t topology_change_state;
    uint8_t topology_change_detected;
    struct timer_list forward_delay_timer;
    struct timer_list hold_timer;
    struct timer_list message_age_timer;
    struct kobject kobj;
    struct rcu_head rcu;
    struct net_bridge_port *backup_port;
    struct net_bridge_port *designated_port;
    uint32_t offloaded;
    uint32_t hw_mode;
    uint32_t isolated;
};

/* Bridge VLAN */
struct net_bridge_vlan {
    uint16_t vid;
    uint16_t flags;
    uint16_t pvid;
    struct net_bridge_port *port;
    struct net_bridge_vlan *vlan;
    struct list_head vlist;
    struct rcu_head rcu;
};

#define BRIDGE_VLAN_INFO_MASTER (1 << 0)
#define BRIDGE_VLAN_INFO_PVID   (1 << 1)
#define BRIDGE_VLAN_INFO_UNTAGGED (1 << 2)
#define BRIDGE_VLAN_INFO_RANGE_BEGIN (1 << 3)
#define BRIDGE_VLAN_INFO_RANGE_END (1 << 4)
#define BRIDGE_VLAN_INFO_BRENTRY (1 << 5)
#define BRIDGE_VLAN_INFO_ONLY_OPTS (1 << 6)

/* Bridge MDB (multicast database) */
struct net_bridge_mdb_entry {
    struct net_bridge_port_group __rcu *ports;
    struct br_ip addr;
    struct timer_list timer;
    struct hlist_node hlist[2];
    struct net_bridge *br;
    struct net_bridge_mdb_entry *mglist;
    struct net_bridge_port *from;
    struct rcu_head rcu;
    unsigned long queries;
    uint8_t eth_addr[6];
    uint16_t flags;
};

#define MDB_TEMPORARY       (1 << 0)
#define MDB_PERMANENT       (1 << 1)

struct br_ip {
    union {
        uint32_t uip4;
        uint8_t uip6[16];
        uint8_t uip[16];
    } u;
    uint16_t proto;
};

struct net_bridge_port_group {
    struct net_bridge_port __rcu *port;
    struct net_bridge_port_group __rcu *next;
    struct timer_list timer;
    struct br_ip addr;
    struct net_bridge_mdb_entry *mdb;
    uint16_t flags;
    struct rcu_head rcu;
};

/* Bridge */
struct net_bridge {
    struct net_device *dev;
    struct list_head port_list;
    struct hlist_head hash[256];
    spinlock_t hash_lock;
    struct list_head fdb_list;
    struct hlist_head mdb_list;
    struct timer_list gc_timer;
    struct timer_list hello_timer;
    struct timer_list tcn_timer;
    struct timer_list topology_change_timer;
    uint32_t designated_root[2];
    uint32_t bridge_id[2];
    uint32_t root_path_cost;
    uint32_t max_age;
    uint32_t hello_time;
    uint32_t forward_delay;
    uint32_t bridge_max_age;
    uint32_t bridge_hello_time;
    uint32_t bridge_forward_delay;
    uint32_t ageing_time;
    uint32_t topology_change_time;
    uint32_t gc_interval;
    uint32_t hello_timer_value;
    uint32_t tcn_timer_value;
    uint32_t topology_change_timer_value;
    uint8_t topology_change;
    uint8_t topology_change_detected;
    uint8_t stp_enabled;
    uint8_t group_addr[6];
    uint16_t root_port;
    uint16_t priority;
    uint32_t vlan_filtering;
    uint32_t vlan_protocol;
    uint32_t default_pvid;
    struct list_head vlan_list;
    struct mutex vlan_lock;
    uint32_t mcast_snooping;
    uint32_t mcast_querier;
    uint32_t mcast_query_use_ifaddr;
    uint32_t mcast_router;
    uint32_t mcast_stats_enabled;
    uint32_t mcast_igmp_version;
    uint32_t mcast_mld_version;
    uint32_t nf_call_iptables;
    uint32_t nf_call_ip6tables;
    uint32_t nf_call_arptables;
    uint32_t nf_call_ebtables;
};

/* STP BPDU */
struct br_config_bpdu {
    uint32_t topology_change:1;
    uint32_t topology_change_ack:1;
    uint32_t root_port;
    uint32_t designated_root[2];
    uint32_t designated_cost;
    uint32_t designated_bridge[2];
    uint32_t designated_port;
};

/* Functions */
struct net_bridge *br_add_bridge(struct net *net, const char *name);
void br_del_bridge(struct net *net, const char *name);
struct net_bridge_port *br_add_if(struct net_bridge *br, struct net_device *dev);
int br_del_if(struct net_bridge *br, struct net_device *dev);
void br_fdb_insert(struct net_bridge *br, struct net_bridge_port *source,
                    const uint8_t *addr, uint16_t flags);
struct net_bridge_fdb_entry *br_fdb_get(struct net_bridge *br, const uint8_t *addr);
void br_fdb_put(struct net_bridge_fdb_entry *fdb);
void br_fdb_update(struct net_bridge *br, struct net_bridge_port *source,
                    const uint8_t *addr);
void br_fdb_delete_by_port(struct net_bridge *br, struct net_bridge_port *p, int do_all);
int br_fdb_fillbuf(struct net_bridge *br, void *buf, uint32_t buflen);
void br_fdb_cleanup(unsigned long data);
void br_fdb_init(void);

void br_stp_recalculate_bridge_id(struct net_bridge *br);
void br_stp_change_bridge_id(struct net_bridge *br, const uint8_t *addr);
void br_stp_set_bridge_priority(struct net_bridge *br, uint16_t newprio);
void br_stp_set_port_priority(struct net_bridge_port *p, uint8_t newprio);
void br_stp_set_path_cost(struct net_bridge_port *p, uint32_t path_cost);
void br_stp_enable_port(struct net_bridge_port *p);
void br_stp_disable_port(struct net_bridge_port *p);
void br_stp_change_mtu(struct net_bridge *br);
void br_stp_timer_init(struct net_bridge *br);
void br_stp_port_timer_init(struct net_bridge_port *p);
void br_stp_rcv(const struct stp_proto *proto, struct sk_buff *skb,
                 struct net_device *dev);
void br_stp_config_bpdu_rcv(struct net_bridge *br, struct net_bridge_port *p,
                             const struct br_config_bpdu *bpdu);
void br_stp_tcn_bpdu_rcv(struct net_bridge *br, struct net_bridge_port *p);

void br_multicast_rcv(struct net_bridge *br, struct net_bridge_port *port,
                       struct sk_buff *skb);
void br_multicast_add_port(struct net_bridge_port *port);
void br_multicast_del_port(struct net_bridge_port *port);
void br_multicast_enable(struct net_bridge *br);
void br_multicast_disable(struct net_bridge *br);
int br_multicast_igmp3_report(struct net_bridge *br, struct net_bridge_port *port,
                               struct sk_buff *skb);
int br_multicast_mld2_report(struct net_bridge *br, struct net_bridge_port *port,
                              struct sk_buff *skb);

int br_vlan_add(struct net_bridge *br, uint16_t vid, uint16_t flags);
int br_vlan_delete(struct net_bridge *br, uint16_t vid);
int br_vlan_filter_toggle(struct net_bridge *br, unsigned long val);
int br_vlan_get_pvid(struct net_bridge_port *port, uint16_t *pvid);
int br_vlan_get_info(struct net_bridge_port *port, uint16_t vid,
                      struct net_bridge_vlan *vlan);

#endif /* _LINUXAB_BRIDGE_H */