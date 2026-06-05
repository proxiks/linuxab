/* SPDX-License-Identifier: GPL-2.0 */
/*
 * linuxab/include/linuxab/block/nvme.h
 * NVMe 1.4/2.0 driver for linuxab server
 */

#ifndef _LINUXAB_NVME_H
#define _LINUXAB_NVME_H

#include "types.h"

#define NVME_PCI_CLASS      0x010802

/* Register offsets */
#define NVME_REG_CAP        0x0000  /* Controller Capabilities */
#define NVME_REG_VS         0x0008  /* Version */
#define NVME_REG_INTMS      0x000C  /* Interrupt Mask Set */
#define NVME_REG_INTMC      0x0010  /* Interrupt Mask Clear */
#define NVME_REG_CC         0x0014  /* Controller Configuration */
#define NVME_REG_CSTS       0x001C  /* Controller Status */
#define NVME_REG_NSSR       0x0020  /* NVM Subsystem Reset */
#define NVME_REG_AQA        0x0024  /* Admin Queue Attributes */
#define NVME_REG_ASQ        0x0028  /* Admin Submission Queue Base */
#define NVME_REG_ACQ        0x0030  /* Admin Completion Queue Base */
#define NVME_REG_CMBLOC     0x0038  /* Controller Memory Buffer Location */
#define NVME_REG_CMBSZ      0x003C  /* Controller Memory Buffer Size */
#define NVME_REG_BPINFO     0x0040  /* Boot Partition Information */
#define NVME_REG_BPRSEL     0x0044  /* Boot Partition Read Select */
#define NVME_REG_BPMBL      0x0048  /* Boot Partition Memory Buffer Location */
#define NVME_REG_CQC        0x0050  /* Completion Queue Configuration */
#define NVME_REG_SQC        0x0058  /* Submission Queue Configuration */

/* CAP register */
#define NVME_CAP_MQES_SHIFT     0
#define NVME_CAP_MQES_MASK      0xFFFF
#define NVME_CAP_CQR            (1ULL << 16)
#define NVME_CAP_AMS_SHIFT      17
#define NVME_CAP_TO_SHIFT       24
#define NVME_CAP_TO_MASK        0xFF
#define NVME_CAP_DSTRD_SHIFT    32
#define NVME_CAP_DSTRD_MASK     0xF
#define NVME_CAP_NSSRS_SHIFT    36
#define NVME_CAP_CSS_SHIFT      37
#define NVME_CAP_CSS_MASK       0x1FF
#define NVME_CAP_MPSMIN_SHIFT   48
#define NVME_CAP_MPSMIN_MASK    0xF
#define NVME_CAP_MPSMAX_SHIFT   52
#define NVME_CAP_MPSMAX_MASK    0xF
#define NVME_CAP_PMRS           (1ULL << 56)

/* CC register */
#define NVME_CC_EN              (1 << 0)
#define NVME_CC_CSS_SHIFT       4
#define NVME_CC_CSS_MASK        0x7
#define NVME_CC_MPS_SHIFT       7
#define NVME_CC_MPS_MASK        0xF
#define NVME_CC_AMS_SHIFT       11
#define NVME_CC_AMS_MASK        0x7
#define NVME_CC_SHN_SHIFT       14
#define NVME_CC_SHN_MASK        0x3
#define NVME_CC_IOSQES_SHIFT    16
#define NVME_CC_IOSQES_MASK     0xF
#define NVME_CC_IOCQES_SHIFT    20
#define NVME_CC_IOCQES_MASK     0xF

#define NVME_CC_CSS_NVM         0
#define NVME_CC_SHN_NONE        0
#define NVME_CC_SHN_NORMAL      1
#define NVME_CC_SHN_ABRUPT      2

/* CSTS register */
#define NVME_CSTS_RDY           (1 << 0)
#define NVME_CSTS_CFS           (1 << 1)
#define NVME_CSTS_SHST_SHIFT    2
#define NVME_CSTS_SHST_MASK     0x3
#define NVME_CSTS_NSSRO         (1 << 4)
#define NVME_CSTS_PP            (1 << 5)

/* Admin opcodes */
#define NVME_ADMIN_CREATE_SQ    0x01
#define NVME_ADMIN_CREATE_CQ    0x05
#define NVME_ADMIN_IDENTIFY     0x06
#define NVME_ADMIN_DELETE_SQ    0x00
#define NVME_ADMIN_DELETE_CQ    0x04
#define NVME_ADMIN_GET_FEATURES 0x0A
#define NVME_ADMIN_SET_FEATURES 0x09
#define NVME_ADMIN_GET_LOG_PAGE 0x02
#define NVME_ADMIN_FORMAT_NVM   0x80
#define NVME_ADMIN_SECURITY_SEND 0x81
#define NVME_ADMIN_SECURITY_RECV 0x82
#define NVME_ADMIN_FABRICS      0x7F

/* NVM opcodes */
#define NVME_CMD_WRITE          0x01
#define NVME_CMD_READ           0x02
#define NVME_CMD_FLUSH          0x00
#define NVME_CMD_WRITE_UNCORR   0x04
#define NVME_CMD_COMPARE        0x05
#define NVME_CMD_WRITE_ZEROES   0x08
#define NVME_CMD_DSM            0x09
#define NVME_CMD_RESV_REGISTER  0x0D
#define NVME_CMD_RESV_REPORT  0x0E
#define NVME_CMD_RESV_ACQUIRE 0x11
#define NVME_CMD_RESV_RELEASE 0x15
#define NVME_CMD_COPY           0x19

/* Identify CNS */
#define NVME_ID_CNS_NS        0x00
#define NVME_ID_CNS_CTRL      0x01
#define NVME_ID_CNS_NS_LIST   0x02
#define NVME_ID_CNS_NS_DESC_LIST 0x03
#define NVME_ID_CNS_CS_NS     0x05
#define NVME_ID_CNS_CS_CTRL   0x06
#define NVME_ID_CNS_NS_IOCS   0x0C
#define NVME_ID_CNS_CTRL_IOCS 0x0D
#define NVME_ID_CNS_IOCS      0x1E

/* Submission queue entry */
struct nvme_command {
    uint8_t     opcode;
    uint8_t     flags;
    uint16_t    command_id;
    uint32_t    nsid;
    uint32_t    cdw2;
    uint32_t    cdw3;
    uint64_t    metadata;
    uint64_t    prp1;
    uint64_t    prp2;
    uint32_t    cdw10;
    uint32_t    cdw11;
    uint32_t    cdw12;
    uint32_t    cdw13;
    uint32_t    cdw14;
    uint32_t    cdw15;
};

/* Completion queue entry */
struct nvme_completion {
    uint32_t    result;
    uint32_t    rsvd;
    uint16_t    sq_head;
    uint16_t    sq_id;
    uint16_t    command_id;
    uint16_t    status;
};

#define NVME_CQ_STATUS_PHASE    (1 << 0)
#define NVME_CQ_STATUS_SC_SHIFT 1
#define NVME_CQ_STATUS_SC_MASK  0xFF
#define NVME_CQ_STATUS_SCT_SHIFT 9
#define NVME_CQ_STATUS_SCT_MASK 0x7
#define NVME_CQ_STATUS_CRD_SHIFT 13
#define NVME_CQ_STATUS_M        (1 << 14)
#define NVME_CQ_STATUS_DNR      (1 << 15)

/* Identify controller */
struct nvme_id_ctrl {
    uint16_t    vid;
    uint16_t    ssvid;
    char        sn[20];
    char        mn[40];
    char        fr[8];
    uint8_t     rab;
    uint8_t     ieee[3];
    uint8_t     cmic;
    uint8_t     mdts;
    uint16_t    cntlid;
    uint32_t    ver;
    uint32_t    rtd3r;
    uint32_t    rtd3e;
    uint32_t    oaes;
    uint32_t    ctratt;
    uint16_t    rrls;
    uint8_t     rsvd102[9];
    uint8_t     cntrltype;
    uint8_t     fguid[16];
    uint16_t    crdt1;
    uint16_t    crdt2;
    uint16_t    crdt3;
    uint8_t     rsvd134[122];
    uint8_t     nvmsr;
    uint8_t     vwci;
    uint8_t     mec;
    uint16_t    oacs;
    uint8_t     acl;
    uint8_t     aerl;
    uint8_t     frmw;
    uint8_t     lpa;
    uint8_t     elpe;
    uint8_t     npss;
    uint8_t     avscc;
    uint8_t     apsta;
    uint16_t    wctemp;
    uint16_t    cctemp;
    uint16_t    mtfa;
    uint32_t    hmpre;
    uint32_t    hmmin;
    uint64_t    tnvmcap[2];
    uint64_t    unvmcap[2];
    uint32_t    rpmbs;
    uint16_t    edstt;
    uint8_t     dsto;
    uint8_t     fwug;
    uint16_t    kas;
    uint16_t    hctma;
    uint16_t    mntmt;
    uint16_t    mxtmt;
    uint32_t    sanicap;
    uint32_t    hmmminds;
    uint16_t    hmmmindl;
    uint16_t    rsvd566;
    uint64_t    anacap;
    uint32_t    anagrpmax;
    uint32_t    nanagrpid;
    uint32_t    pels;
    uint16_t    domainid;
    uint8_t     rsvd588[10];
    uint8_t     megcap[16];
    uint8_t     rsvd608[128];
    uint8_t     sqes;
    uint8_t     cqes;
    uint16_t    maxcmd;
    uint32_t    nn;
    uint16_t    oncs;
    uint16_t    fuses;
    uint8_t     fna;
    uint8_t     vwc;
    uint16_t    awun;
    uint16_t    awupf;
    uint8_t     icsvscc;
    uint8_t     nwpc;
    uint16_t    acwu;
    uint16_t    ocfs;
    uint32_t    sgls;
    uint32_t    mnan;
    uint8_t     rsvd740[224];
    uint8_t     subnqn[256];
    uint8_t     rsvd1024[768];
    uint32_t    ioccs[512];
    uint8_t     rsvd3072[1024];
    struct {
        uint16_t    ms;
        uint8_t     lbads;
        uint8_t     rp;
        uint8_t     rsvd;
    } lbaf[16];
    uint8_t     rsvd4096[3968];
};

/* Identify namespace */
struct nvme_id_ns {
    uint64_t    nsze;
    uint64_t    ncap;
    uint64_t    nuse;
    uint8_t     nsfeat;
    uint8_t     nlbaf;
    uint8_t     flbas;
    uint8_t     mc;
    uint8_t     dpc;
    uint8_t     dps;
    uint8_t     nmic;
    uint8_t     rescap;
    uint8_t     fpi;
    uint8_t     dlfeat;
    uint16_t    nawun;
    uint16_t    nawupf;
    uint16_t    nacwu;
    uint16_t    nabsn;
    uint16_t    nabo;
    uint16_t    nabspf;
    uint16_t    noiob;
    uint64_t    nvmcap[2];
    uint16_t    npwg;
    uint16_t    npwa;
    uint16_t    npdg;
    uint16_t    npda;
    uint16_t    nows;
    uint16_t    nssrl;
    uint16_t    nmic;
    uint16_t    rsvd74;
    uint64_t    anagrp;
    uint32_t    nvmsetid;
    uint16_t    endgid;
    uint8_t     nguid[16];
    uint8_t     eui64[8];
    struct {
        uint32_t    lba_format;
    } lbaf[64];
    uint8_t     rsvd192[192];
    uint8_t     vs[3712];
};

/* LBA format */
#define NVME_LBAF_RP_BEST       0
#define NVME_LBAF_RP_BETTER     1
#define NVME_LBAF_RP_GOOD       2
#define NVME_LBAF_RP_DEGRADED   3

/* Features */
#define NVME_FEAT_ARBITRATION   0x01
#define NVME_FEAT_POWER_MGMT    0x02
#define NVME_FEAT_LBA_RANGE     0x03
#define NVME_FEAT_TEMP_THRESH   0x04
#define NVME_FEAT_ERR_RECOVERY  0x05
#define NVME_FEAT_VOLATILE_WC   0x06
#define NVME_FEAT_NUM_QUEUES    0x07
#define NVME_FEAT_IRQ_COALESCE  0x08
#define NVME_FEAT_IRQ_CONFIG    0x09
#define NVME_FEAT_WRITE_ATOMIC  0x0A
#define NVME_FEAT_ASYNC_EVENT   0x0B
#define NVME_FEAT_AUTO_PST      0x0C
#define NVME_FEAT_HOST_MEM_BUF  0x0D
#define NVME_FEAT_TIMESTAMP     0x0E
#define NVME_FEAT_PLM_CONFIG   0x13
#define NVME_FEAT_PLM_WINDOW    0x14
#define NVME_FEAT_HOST_BEHAVIOR 0x16
#define NVME_FEAT_SANITIZE      0x17
#define NVME_FEAT_ENDURANCE_EVT 0x18
#define NVME_FEAT_SW_PROGRESS   0x80
#define NVME_FEAT_HOST_ID       0x81
#define NVME_FEAT_RESV_MASK     0x82
#define NVME_FEAT_RESV_PERSIST  0x83

/* Log pages */
#define NVME_LOG_ERROR          0x01
#define NVME_LOG_SMART          0x02
#define NVME_LOG_FW_SLOT        0x03
#define NVME_LOG_CHANGED_NS     0x04
#define NVME_LOG_CMD_EFFECTS    0x05
#define NVME_LOG_DEVICE_SELF_TEST 0x06
#define NVME_LOG_TELEMETRY_HOST 0x07
#define NVME_LOG_TELEMETRY_CTRL 0x08
#define NVME_LOG_ENDURANCE_GRP  0x09
#define NVME_LOG_PREDICTABLE_LAT 0x0A
#define NVME_LOG_FW_COMMIT_HISTORY 0x0B
#define NVME_LOG_LATENCY        0x0C
#define NVME_LOG_ENDURANCE      0x0D
#define NVME_LOG_MEDIA_UNIT_STATUS 0x0E
#define NVME_LOG_SUPPORTED_CAP_CONFIG 0x0F
#define NVME_LOG_DISCOVER       0x70
#define NVME_LOG_RESERVATION    0x80
#define NVME_LOG_SANITIZE       0x81

/* SMART log */
struct nvme_smart_log {
    uint8_t     critical_warning;
    uint16_t    composite_temperature;
    uint8_t     available_spare;
    uint8_t     available_spare_threshold;
    uint8_t     percentage_used;
    uint8_t     rsvd6[26];
    uint64_t    data_units_read[2];
    uint64_t    data_units_written[2];
    uint64_t    host_read_commands[2];
    uint64_t    host_write_commands[2];
    uint64_t    controller_busy_time[2];
    uint64_t    power_cycles[2];
    uint64_t    power_on_hours[2];
    uint64_t    unsafe_shutdowns[2];
    uint64_t    media_errors[2];
    uint64_t    num_err_log_entries[2];
    uint32_t    warning_temp_time;
    uint32_t    critical_temp_time;
    uint16_t    temp_sensor[8];
    uint32_t    thm_temp1_trans_count;
    uint32_t    thm_temp2_trans_count;
    uint32_t    thm_temp1_total_time;
    uint32_t    thm_temp2_total_time;
    uint8_t     rsvd232[280];
};

/* Namespace descriptor */
struct nvme_ns_id_desc {
    uint8_t     nidt;
    uint8_t     nidl;
    uint8_t     rsvd[2];
};

#define NVME_NIDT_EUI64         0x01
#define NVME_NIDT_NGUID         0x02
#define NVME_NIDT_UUID          0x03
#define NVME_NIDT_CSI           0x04

/* Queue */
struct nvme_queue {
    struct nvme_command *sq_cmds;
    struct nvme_completion *cqes;
    uint64_t sq_dma_addr;
    uint64_t cq_dma_addr;
    uint32_t __iomem *q_db;
    uint16_t q_depth;
    uint16_t cq_head;
    uint16_t sq_tail;
    uint16_t sq_head;
    uint16_t qid;
    uint8_t  cq_phase;
    uint8_t  cqe_seen;
    uint32_t last_cq_head;
    uint32_t flags;
    void *dev;
};

struct nvme_dev {
    struct pci_dev *pdev;
    void __iomem *bar;
    uint64_t cap;
    uint32_t ctrl_config;
    uint32_t io_queue_depth;
    uint16_t nr_queues;
    uint16_t max_qid;
    uint16_t db_stride;
    uint32_t page_size;
    uint32_t max_transfer_shift;
    uint32_t io_sqes;
    uint32_t io_cqes;
    struct nvme_queue *queues;
    struct nvme_queue admin_q;
    struct nvme_id_ctrl ctrl;
    uint32_t nr_ns;
    struct nvme_ns *ns;
    struct dma_pool *prp_page_pool;
    struct dma_pool *prp_small_pool;
    uint64_t dma_addr;
    void *dma_virt;
    atomic_t refcount;
    char serial[20];
    char model[40];
    char firmware_rev[8];
    uint32_t quirks;
};

struct nvme_ns {
    struct nvme_dev *dev;
    uint32_t ns_id;
    uint64_t lba_count;
    uint32_t lba_shift;
    uint32_t ms;
    uint8_t  pi_type;
    uint8_t  ext;
    uint16_t flags;
    struct gendisk *disk;
    struct request_queue *queue;
    struct nvme_id_ns id;
    uint64_t mode_select_num_blocks;
    uint32_t mode_select_block_len;
};

/* Quirks */
#define NVME_QUIRK_DELAY_BEFORE_CHK_RDY     (1 << 0)
#define NVME_QUIRK_DISABLE_WRITE_ZEROES   (1 << 1)
#define NVME_QUIRK_DISABLE_AER            (1 << 2)
#define NVME_QUIRK_IGNORE_DEV_SUBNQN      (1 << 3)
#define NVME_QUIRK_BOGUS_NID              (1 << 4)
#define NVME_QUIRK_NO_NS_DESC_LIST        (1 << 5)
#define NVME_QUIRK_ALWAYS_USE_LEGACY_IO   (1 << 6)

int nvme_probe(struct pci_dev *pdev);
void nvme_remove(struct pci_dev *pdev);
int nvme_reset_ctrl(struct nvme_dev *dev);
int nvme_enable_ctrl(struct nvme_dev *dev);
void nvme_shutdown_ctrl(struct nvme_dev *dev);
int nvme_init_queue(struct nvme_queue *nvmeq, uint16_t qid, uint16_t depth);
void nvme_suspend_queue(struct nvme_queue *nvmeq);
int nvme_submit_cmd(struct nvme_queue *nvmeq, struct nvme_command *cmd, bool sync);
int nvme_submit_admin_cmd(struct nvme_dev *dev, struct nvme_command *cmd, uint32_t timeout);
int nvme_identify_ctrl(struct nvme_dev *dev, struct nvme_id_ctrl *ctrl);
int nvme_identify_ns(struct nvme_dev *dev, uint32_t nsid, struct nvme_id_ns *ns);
int nvme_set_features(struct nvme_dev *dev, uint32_t fid, uint32_t dword11, uint32_t *result);
int nvme_get_features(struct nvme_dev *dev, uint32_t fid, uint32_t dword11, uint32_t *result);
int nvme_get_log_page(struct nvme_dev *dev, uint32_t lid, uint32_t nsid, void *log, size_t size);
int nvme_format_nvm(struct nvme_dev *dev, uint32_t nsid, uint8_t lbaf, uint8_t ses);

#endif /* _LINUXAB_NVME_H */