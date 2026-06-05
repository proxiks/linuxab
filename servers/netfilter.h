/* SPDX-License-Identifier: GPL-2.0 */
/*
 * linuxab/include/linuxab/net/netfilter.h
 * Netfilter packet filtering framework for linuxab server
 */

#ifndef _LINUXAB_NETFILTER_H
#define _LINUXAB_NETFILTER_H

#include "types.h"
#include "skbuff.h"

/* Hooks */
#define NF_INET_PRE_ROUTING     0
#define NF_INET_LOCAL_IN        1
#define NF_INET_FORWARD         2
#define NF_INET_LOCAL_OUT       3
#define NF_INET_POST_ROUTING    4
#define NF_INET_NUMHOOKS        5

#define NF_ARP_IN               0
#define NF_ARP_OUT              1
#define NF_ARP_FORWARD          2
#define NF_ARP_NUMHOOKS         3

#define NF_BRIDGE_PRE_ROUTING   0
#define NF_BRIDGE_LOCAL_IN      1
#define NF_BRIDGE_FORWARD       2
#define NF_BRIDGE_LOCAL_OUT     3
#define NF_BRIDGE_POST_ROUTING  4
#define NF_BRIDGE_NUMHOOKS      6

/* Verdicts */
#define NF_DROP         0
#define NF_ACCEPT       1
#define NF_STOLEN       2
#define NF_QUEUE        3
#define NF_REPEAT       4
#define NF_STOP         5
#define NF_MAX_VERDICT  NF_STOP

/* Protocol families */
#define NFPROTO_UNSPEC      0
#define NFPROTO_INET        1
#define NFPROTO_IPV4        2
#define NFPROTO_ARP         3
#define NFPROTO_NETDEV      5
#define NFPROTO_BRIDGE      7
#define NFPROTO_IPV6        10
#define NFPROTO_DECNET      12
#define NFPROTO_NUMPROTO    13

/* Priority levels (lower = earlier) */
#define NF_IP_PRI_FIRST         INT_MIN
#define NF_IP_PRI_CONNTRACK_DEFRAG  -400
#define NF_IP_PRI_RAW           -300
#define NF_IP_PRI_SELINUX_FIRST -200
#define NF_IP_PRI_CONNTRACK     -200
#define NF_IP_PRI_MANGLE        -150
#define NF_IP_PRI_NAT_DST       -100
#define NF_IP_PRI_FILTER         0
#define NF_IP_PRI_SECURITY        50
#define NF_IP_PRI_NAT_SRC         100
#define NF_IP_PRI_SELINUX_LAST    180
#define NF_IP_PRI_CONNTRACK_HELPER  200
#define NF_IP_PRI_CONNTRACK_CONFIRM 2147483647
#define NF_IP_PRI_LAST            INT_MAX

/* Hook operations */
struct nf_hook_ops {
    struct list_head list;
    nf_hookfn *hook;
    struct module *owner;
    uint8_t pf;
    unsigned int hooknum;
    int priority;
};

typedef unsigned int nf_hookfn(void *priv,
                                struct sk_buff *skb,
                                const struct nf_hook_state *state);

struct nf_hook_state {
    uint8_t hook;
    uint8_t pf;
    struct net_device *in;
    struct net_device *out;
    struct sock *sk;
    int (*okfn)(struct net *, struct sock *, struct sk_buff *);
};

/* Netfilter table / chain */
struct nft_table {
    char name[32];
    uint32_t use;
    uint16_t flags;
    uint16_t family;
    struct list_head chains;
    struct list_head sets;
    struct list_head objects;
    struct list_head flowtables;
};

struct nft_chain {
    char name[32];
    struct nft_table *table;
    uint32_t use;
    uint64_t handle;
    struct list_head rules;
    struct list_head list;
    uint8_t flags;
    uint8_t bound;
};

struct nft_rule {
    struct list_head list;
    uint64_t handle;
    uint32_t use;
    uint32_t dlen;
    uint8_t data[0];
};

/* Conntrack */
struct nf_conn {
    atomic_t use;
    uint32_t status;
    uint32_t timeout;
    uint16_t l3num;
    uint8_t l4proto;
    union nf_conntrack_man proto;
    struct nf_conntrack_tuple_hash tuplehash[2];
};

struct nf_conntrack_tuple {
    union {
        uint32_t u3;
        uint8_t u6[16];
    } src;
    union {
        uint32_t u3;
        uint8_t u6[16];
    } dst;
    union nf_conntrack_man_proto src;
    union nf_conntrack_man_proto dst;
};

struct nf_conntrack_tuple_hash {
    struct hlist_nulls_node hnnode;
    struct nf_conntrack_tuple tuple;
};

union nf_conntrack_man_proto {
    uint16_t all;
    struct { uint16_t port; } tcp;
    struct { uint16_t port; } udp;
    struct { uint8_t type, code; } icmp;
    struct { uint16_t id; } gre;
};

union nf_conntrack_man {
    union nf_conntrack_man_proto u3;
};

/* NAT */
struct nf_nat_range {
    uint32_t flags;
    uint32_t min_addr;
    uint32_t max_addr;
    union nf_conntrack_man_proto min_proto;
    union nf_conntrack_man_proto max_proto;
};

#define NF_NAT_RANGE_MAP_IPS        (1 << 0)
#define NF_NAT_RANGE_PROTO_SPECIFIED (1 << 1)
#define NF_NAT_RANGE_PROTO_RANDOM   (1 << 2)
#define NF_NAT_RANGE_PERSISTENT     (1 << 3)
#define NF_NAT_RANGE_PROTO_RANDOM_FULLY (1 << 4)
#define NF_NAT_RANGE_NETMAP         (1 << 5)
#define NF_NAT_RANGE_PROTO_RANDOM_ALL (1 << 6)

/* iptables match / target */
struct xt_match {
    char name[29];
    uint8_t revision;
    uint16_t family;
    uint16_t matchsize;
    uint16_t usersize;
    uint16_t hooks;
    uint8_t proto;
    uint8_t af;
    int (*match)(const struct sk_buff *skb, struct xt_action_param *par);
    int (*checkentry)(const struct xt_mtchk_param *par);
    void (*destroy)(const struct xt_mtdtor_param *par);
};

struct xt_target {
    char name[29];
    uint8_t revision;
    uint16_t family;
    uint16_t targetsize;
    uint16_t usersize;
    uint16_t hooks;
    uint8_t proto;
    uint8_t af;
    unsigned int (*target)(struct sk_buff *skb, const struct xt_action_param *par);
    int (*checkentry)(const struct xt_tgchk_param *par);
    void (*destroy)(const struct xt_tgdtor_param *par);
};

struct ipt_entry {
    struct ipt_ip ip;
    uint32_t nfcache;
    uint16_t target_offset;
    uint16_t next_offset;
    uint32_t comefrom;
    struct xt_counters counters;
    unsigned char elems[0];
};

struct ipt_ip {
    uint32_t src, dst;
    uint32_t smsk, dmsk;
    char iniface[16], outiface[16];
    uint8_t iniface_mask[16], outiface_mask[16];
    uint16_t proto;
    uint8_t flags;
    uint8_t invflags;
};

struct xt_counters {
    uint64_t pcnt;
    uint64_t bcnt;
};

struct xt_action_param {
    union {
        const struct xt_match *match;
        const struct xt_target *target;
    };
    union {
        const void *matchinfo, *targetinfo;
    };
    const struct net_device *in, *out;
    int fragoff;
    unsigned int thoff;
    uint16_t hook;
    uint8_t family;
    bool hotdrop;
};

/* Core functions */
int nf_register_hook(struct nf_hook_ops *reg);
void nf_unregister_hook(struct nf_hook_ops *reg);
int nf_register_hooks(struct nf_hook_ops *reg, unsigned int n);
void nf_unregister_hooks(struct nf_hook_ops *reg, unsigned int n);

unsigned int nf_hook(uint8_t pf, unsigned int hook, struct net *net,
                     struct sock *sk, struct sk_buff *skb,
                     struct net_device *indev, struct net_device *outdev,
                     int (*okfn)(struct net *, struct sock *, struct sk_buff *));

struct nf_conn *nf_conntrack_alloc(struct net *net, const struct nf_conntrack_tuple *orig,
                                    const struct nf_conntrack_tuple *repl);
void nf_conntrack_free(struct nf_conn *ct);
int nf_conntrack_confirm(struct sk_buff *skb);

int nf_nat_setup_info(struct nf_conn *ct, const struct nf_nat_range *range, uint32_t maniptype);

/* iptables */
int ipt_register_table(struct net *net, const struct xt_table *table,
                        const struct ipt_replace *repl);
void ipt_unregister_table(struct net *net, struct xt_table *table);

/* nftables */
int nft_register_table(struct net *net, struct nft_table *table);
void nft_unregister_table(struct net *net, struct nft_table *table);
int nft_add_rule(struct nft_chain *chain, struct nft_rule *rule);
int nft_delete_rule(struct nft_chain *chain, uint64_t handle);

#endif /* _LINUXAB_NETFILTER_H */