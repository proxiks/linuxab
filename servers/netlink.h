/* SPDX-License-Identifier: GPL-2.0 */
/*
 * linuxab/include/linuxab/netlink.h
 * Netlink socket interface for linuxab
 */

#ifndef _LINUXAB_NETLINK_H
#define _LINUXAB_NETLINK_H

#include "types.h"

/* Netlink socket constants */
#define AF_NETLINK          16
#define PF_NETLINK          AF_NETLINK

/* Netlink protocol types */
#define NETLINK_ROUTE       0   /* Routing/device hook */
#define NETLINK_UNUSED      1   /* Unused number */
#define NETLINK_USERSOCK    2   /* Reserved for user mode socket protocols */
#define NETLINK_FIREWALL    3   /* Unused number, formerly ip_queue */
#define NETLINK_SOCK_DIAG   4   /* socket monitoring */
#define NETLINK_NFLOG       5   /* netfilter/iptables ULOG */
#define NETLINK_XFRM        6   /* ipsec */
#define NETLINK_SELINUX     7   /* SELinux event notifications */
#define NETLINK_ISCSI       8   /* Open-iSCSI */
#define NETLINK_AUDIT       9   /* auditing */
#define NETLINK_FIB_LOOKUP  10
#define NETLINK_CONNECTOR   11
#define NETLINK_NETFILTER   12  /* netfilter subsystem */
#define NETLINK_IP6_FW      13
#define NETLINK_DNRTMSG     14  /* DECnet routing messages */
#define NETLINK_KOBJECT_UEVENT 15 /* Kernel messages to userspace */
#define NETLINK_GENERIC     16
#define NETLINK_SMC         17  /* SMC monitoring */

#define NETLINK_MAX         32  /* Should be enough for now */

/* Netlink message flags */
#define NLM_F_REQUEST       0x01  /* It is request message */
#define NLM_F_MULTI         0x02  /* Multipart message, terminated by NLMSG_DONE */
#define NLM_F_ACK           0x04  /* Reply with ack, with zero or error code */
#define NLM_F_ECHO          0x08  /* Echo this request */
#define NLM_F_DUMP_INTR     0x10  /* Dump was inconsistent due to sequence change */
#define NLM_F_DUMP_FILTERED 0x20  /* Dump was filtered as requested */

/* Modifiers to GET request */
#define NLM_F_ROOT      0x100   /* specify tree root */
#define NLM_F_MATCH     0x200   /* return all matching */
#define NLM_F_ATOMIC    0x400   /* atomic GET */
#define NLM_F_DUMP      (NLM_F_ROOT | NLM_F_MATCH)

/* Modifiers to NEW request */
#define NLM_F_REPLACE   0x100   /* Override existing */
#define NLM_F_EXCL      0x200   /* Do not touch, if it exists */
#define NLM_F_CREATE    0x400   /* Create, if it does not exist */
#define NLM_F_APPEND    0x800   /* Add to end of list */

/* Netlink message types */
#define NLMSG_NOOP      0x1     /* Nothing */
#define NLMSG_ERROR     0x2     /* Error */
#define NLMSG_DONE      0x3     /* End of a dump */
#define NLMSG_OVERRUN   0x4     /* Data lost */

#define NLMSG_MIN_TYPE  0x10    /* < 0x10: reserved control messages */

/* Netlink message header */
struct nlmsghdr {
    uint32_t nlmsg_len;      /* Length of message including header */
    uint16_t nlmsg_type;     /* Message content */
    uint16_t nlmsg_flags;    /* Additional flags */
    uint32_t nlmsg_seq;      /* Sequence number */
    uint32_t nlmsg_pid;      /* Sending process port ID */
} __attribute__((packed));

/* Netlink socket address */
struct sockaddr_nl {
    uint16_t nl_family;      /* AF_NETLINK */
    uint16_t nl_pad;         /* Zero */
    uint32_t nl_pid;         /* Port ID */
    uint32_t nl_groups;      /* Multicast groups mask */
} __attribute__((packed));

/* Netlink message error */
struct nlmsgerr {
    int32_t  error;          /* Negative errno or 0 for acknowledgements */
    struct nlmsghdr msg;     /* Message header that caused the error */
} __attribute__((packed));

/* Netlink attribute header */
struct nlattr {
    uint16_t nla_len;        /* Length of attribute including header */
    uint16_t nla_type;       /* Attribute type */
    /* Attribute data follows */
} __attribute__((packed));

/* Netlink attribute flags (nla_type high bits) */
#define NLA_F_NESTED        (1 << 15)
#define NLA_F_NET_BYTEORDER (1 << 14)
#define NLA_TYPE_MASK       ~(NLA_F_NESTED | NLA_F_NET_BYTEORDER)

/* Netlink attribute policy types */
#define NLA_UNSPEC          0
#define NLA_U8              1
#define NLA_U16             2
#define NLA_U32             3
#define NLA_U64             4
#define NLA_STRING          5
#define NLA_FLAG            6
#define NLA_MSECS           7
#define NLA_NESTED          8
#define NLA_NESTED_ARRAY    9
#define NLA_BITFIELD32      10
#define NLA_REJECT          11
#define NLA_BINARY          12
#define NLA_S8              13
#define NLA_S16             14
#define NLA_S32             15
#define NLA_S64             16
#define NLA_MIN_LEN         17

/* Netlink attribute policy max length */
#define NLA_POLICY_MAX_ATTRS    128

/* Netlink attribute policy entry */
struct nla_policy {
    uint16_t type;           /* NLA_* type */
    uint16_t len;            /* Max/min length depending on type */
};

/* Netlink callback structure for message processing */
struct netlink_callback {
    struct nlmsghdr *nlh;
    uint32_t dump;
    uint32_t done;
    uint32_t seq;
    uint32_t args[4];
    void *data;
};

/* Netlink kernel socket structure */
struct netlink_sock {
    uint32_t portid;         /* Port ID / PID */
    uint32_t dst_portid;     /* Destination port ID */
    uint32_t dst_group;      /* Destination group */
    uint32_t groups;         /* Subscribed groups */
    uint32_t flags;
    void *sk;                /* Back-pointer to generic socket */
};

/* Netlink multicast group definition */
struct netlink_multicast_group {
    const char *name;
    uint32_t group;
};

/* Netlink family operations */
struct netlink_kernel_cfg {
    uint32_t groups;         /* Number of multicast groups */
    void (*input)(struct netlink_sock *sk, struct nlmsghdr *nlh);
    int (*bind)(struct netlink_sock *sk, uint32_t group);
    void (*unbind)(struct netlink_sock *sk, uint32_t group);
    int (*compare)(struct netlink_sock *sk1, struct netlink_sock *sk2);
};

/* Netlink family registration */
struct netlink_family {
    uint32_t id;             /* NETLINK_* protocol */
    const char *name;        /* Family name */
    uint32_t version;        /* Family version */
    uint32_t maxattr;        /* Maximum attribute type */
    struct nla_policy *policy; /* Attribute policy */
    struct netlink_kernel_cfg *cfg;
};

/*
 * Netlink helper macros
 */
#define NLMSG_ALIGNTO       4U
#define NLMSG_ALIGN(len)    (((len) + NLMSG_ALIGNTO - 1) & ~(NLMSG_ALIGNTO - 1))
#define NLMSG_HDRLEN        ((int)NLMSG_ALIGN(sizeof(struct nlmsghdr)))
#define NLMSG_LENGTH(len)   ((len) + NLMSG_HDRLEN)
#define NLMSG_SPACE(len)    NLMSG_ALIGN(NLMSG_LENGTH(len))
#define NLMSG_DATA(nlh)     ((void *)(((char *)nlh) + NLMSG_LENGTH(0)))
#define NLMSG_NEXT(nlh, len) \
    ((len) -= NLMSG_ALIGN((nlh)->nlmsg_len), \
     (struct nlmsghdr *)(((char *)(nlh)) + NLMSG_ALIGN((nlh)->nlmsg_len)))
#define NLMSG_OK(nlh, len)  \
    ((len) >= (int)sizeof(struct nlmsghdr) && \
     (nlh)->nlmsg_len >= sizeof(struct nlmsghdr) && \
     (nlh)->nlmsg_len <= (len))
#define NLMSG_PAYLOAD(nlh, len) ((nlh)->nlmsg_len - NLMSG_SPACE((len)))

#define NLMSG_TAIL(nmsg) \
    ((struct nlattr *)(((void *)(nmsg)) + NLMSG_ALIGN((nmsg)->nlmsg_len)))

/* Attribute helpers */
#define NLA_ALIGNTO         4
#define NLA_ALIGN(len)      (((len) + NLA_ALIGNTO - 1) & ~(NLA_ALIGNTO - 1))
#define NLA_HDRLEN          ((int)NLA_ALIGN(sizeof(struct nlattr)))
#define NLA_LENGTH(len)     ((len) + NLA_HDRLEN)
#define NLA_SPACE(len)      NLA_ALIGN(NLA_LENGTH(len))
#define NLA_DATA(nla)       ((void *)(((char *)(nla)) + NLA_LENGTH(0)))
#define NLA_NEXT(nla, attrlen) \
    ((attrlen) -= NLA_ALIGN((nla)->nla_len), \
     (struct nlattr *)(((char *)(nla)) + NLA_ALIGN((nla)->nla_len)))
#define NLA_OK(nla, attrlen) \
    ((attrlen) >= (int)sizeof(struct nlattr) && \
     (nla)->nla_len >= sizeof(struct nlattr) && \
     (nla)->nla_len <= (attrlen))
#define NLA_PAYLOAD(nla)  ((nla)->nla_len - NLA_LENGTH(0))

/* Netlink socket creation flags */
#define NETLINK_SOCK_KERNEL     0x01
#define NETLINK_SOCK_USER       0x02
#define NETLINK_SOCK_BROADCAST  0x04

/* Netlink socket states */
#define NETLINK_S_UNUSED        0x00
#define NETLINK_S_ESTABLISHED   0x01
#define NETLINK_S_BOUND         0x02
#define NETLINK_S_LISTENING     0x04
#define NETLINK_S_CLOSED        0x08

/* Netlink buffer sizes */
#define NETLINK_SMALL_BUF_SIZE  4096
#define NETLINK_DEF_BUF_SIZE    16384
#define NETLINK_MAX_BUF_SIZE    65536

/* Netlink port ID allocation */
#define NETLINK_PORTID_KERNEL   0
#define NETLINK_PORTID_AUTO     0xFFFFFFFF

/*
 * Netlink API function prototypes
 */

/* Socket operations */
int netlink_sock_create(int protocol, uint32_t flags, struct netlink_sock **sk);
void netlink_sock_release(struct netlink_sock *sk);
int netlink_sock_bind(struct netlink_sock *sk, struct sockaddr_nl *addr);
int netlink_sock_send(struct netlink_sock *sk, struct nlmsghdr *nlh, uint32_t dst_pid);
int netlink_sock_recv(struct netlink_sock *sk, void *buf, uint32_t len, uint32_t *from_pid);

/* Multicast */
int netlink_sock_join_group(struct netlink_sock *sk, uint32_t group);
int netlink_sock_leave_group(struct netlink_sock *sk, uint32_t group);
int netlink_multicast(struct netlink_sock *sk, struct nlmsghdr *nlh, uint32_t group, uint32_t flags);

/* Message helpers */
struct nlmsghdr *netlink_msg_new(uint32_t type, uint32_t flags, uint32_t size);
void netlink_msg_free(struct nlmsghdr *nlh);
int netlink_msg_put_attr(struct nlmsghdr *nlh, uint32_t max_len, uint16_t type, const void *data, uint16_t len);
struct nlattr *netlink_msg_attr_start(struct nlmsghdr *nlh, uint16_t type);
void netlink_msg_attr_end(struct nlmsghdr *nlh, struct nlattr *attr);

/* Kernel interface */
int netlink_register_family(struct netlink_family *family);
void netlink_unregister_family(struct netlink_family *family);
int netlink_unicast(struct netlink_sock *sk, struct nlmsghdr *nlh, uint32_t portid);
int netlink_broadcast(struct netlink_sock *sk, struct nlmsghdr *nlh, uint32_t group);

/* Attribute parsing */
int netlink_attr_parse(struct nlattr *tb[], int maxtype, struct nlattr *head, int len,
                       struct nla_policy *policy);
void *netlink_attr_get(struct nlattr *nla);
uint32_t netlink_attr_get_u32(struct nlattr *nla);
uint16_t netlink_attr_get_u16(struct nlattr *nla);
uint8_t netlink_attr_get_u8(struct nlattr *nla);
uint64_t netlink_attr_get_u64(struct nlattr *nla);
const char *netlink_attr_get_string(struct nlattr *nla);
int netlink_attr_len(struct nlattr *nla);
int netlink_attr_type(struct nlattr *nla);

/* Dump helpers */
int netlink_dump_start(struct netlink_sock *sk, struct nlmsghdr *nlh,
                       struct netlink_callback *cb);
void netlink_dump_done(struct netlink_callback *cb);

/* Netlink notification */
void netlink_notify(struct netlink_family *family, uint32_t group, struct nlmsghdr *nlh);

/* Netlink table / diagnostics */
int netlink_table_init(void);
void netlink_table_fini(void);

#endif /* _LINUXAB_NETLINK_H */