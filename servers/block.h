/* SPDX-License-Identifier: GPL-2.0 */
/*
 * linuxab/include/linuxab/block/block.h
 * Block device layer for server I/O
 */

#ifndef _LINUXAB_BLOCK_H
#define _LINUXAB_BLOCK_H

#include "types.h"
#include "atomic.h"

#define SECTOR_SHIFT        9
#define SECTOR_SIZE         (1 << SECTOR_SHIFT)     /* 512 bytes */
#define SECTOR_MASK         (SECTOR_SIZE - 1)

#define BLOCK_SIZE          4096
#define BLOCK_SIZE_BITS     12

/* Bio operation flags */
#define REQ_OP_READ         0
#define REQ_OP_WRITE        1
#define REQ_OP_FLUSH        2
#define REQ_OP_DISCARD      3
#define REQ_OP_SECURE_ERASE 4
#define REQ_OP_WRITE_ZEROES 5
#define REQ_OP_ZONE_RESET   6
#define REQ_OP_ZONE_OPEN    7
#define REQ_OP_ZONE_CLOSE   8
#define REQ_OP_ZONE_FINISH  9
#define REQ_OP_SCSI_IN      32
#define REQ_OP_SCSI_OUT     33
#define REQ_OP_DRV_IN       34
#define REQ_OP_DRV_OUT      35

#define REQ_FAILFAST_DEV    (1 << 0)
#define REQ_FAILFAST_TRANSPORT  (1 << 1)
#define REQ_FAILFAST_DRIVER (1 << 2)
#define REQ_SYNC            (1 << 3)
#define REQ_META            (1 << 4)
#define REQ_PRIO            (1 << 5)
#define REQ_NOMERGE         (1 << 6)
#define REQ_IDLE            (1 << 7)
#define REQ_INTEGRITY       (1 << 8)
#define REQ_FUA             (1 << 9)
#define REQ_PREFLUSH        (1 << 10)
#define REQ_RAHEAD          (1 << 11)
#define REQ_BACKGROUND      (1 << 12)
#define REQ_NOWAIT          (1 << 13)

/* Bio vector */
struct bio_vec {
    uint64_t    bv_page;        /* Physical page */
    uint32_t    bv_len;         /* Length in bytes */
    uint32_t    bv_offset;      /* Offset within page */
};

struct bio {
    struct bio      *bi_next;       /* Next bio in chain */
    struct block_device *bi_bdev;
    uint64_t        bi_sector;      /* Device sector */
    uint32_t        bi_opf;         /* Operation + flags */
    uint32_t        bi_flags;
    uint32_t        bi_vcnt;        /* Number of vectors */
    uint32_t        bi_max_vecs;    /* Max vectors */
    uint32_t        bi_idx;         /* Current index */
    atomic_t        bi_cnt;         /* Reference count */
    uint32_t        bi_size;        /* Remaining I/O count */
    void            *bi_private;
    void            (*bi_end_io)(struct bio *);
    struct bio_vec  *bi_io_vec;     /* Vector array */
    struct bio_vec    bi_inline_vecs[0];
};

struct request {
    struct list_head queuelist;
    struct request   *next_rq;
    struct bio       *bio;
    struct bio       *biotail;
    uint64_t         __sector;
    uint32_t         nr_sectors;
    uint32_t         __data_len;
    uint32_t         cmd_flags;
    uint16_t         rq_flags;
    uint8_t          cmd[16];       /* SCSI/NVMe CDB */
    uint32_t         tag;
    uint32_t         internal_tag;
    void             *special;      /* NVMe/SCSI private */
    void             *end_io_data;
    void             (*end_io)(struct request *, int);
    struct gendisk   *rq_disk;
    struct block_device *part;
};

struct request_queue {
    struct list_head    queue_head;
    struct elevator_queue *elevator;
    uint64_t            nr_requests;
    uint64_t            nr_congestion_on;
    uint64_t            nr_congestion_off;
    uint32_t            max_sectors;
    uint32_t            max_hw_sectors;
    uint32_t            max_segments;
    uint32_t            max_segment_size;
    uint32_t            logical_block_size;
    uint32_t            physical_block_size;
    uint32_t            alignment_offset;
    uint32_t            io_min;
    uint32_t            io_opt;
    uint32_t            dma_pad_mask;
    uint32_t            dma_alignment;
    atomic_t            refcnt;
    bool                mq;
    /* MQ specific */
    struct blk_mq_tag_set *tag_set;
    uint32_t            nr_hw_queues;
    uint32_t            queue_depth;
};

struct gendisk {
    int                 major;
    int                 first_minor;
    int                 minors;
    char                disk_name[32];
    struct block_device *part0;
    struct request_queue *queue;
    uint64_t            capacity;   /* Sectors */
    uint32_t            flags;
    void                *private_data;
    struct disk_part_tbl *part_tbl;
    struct block_device *slave_dir;
};

struct block_device {
    dev_t               bd_dev;
    struct inode        *bd_inode;
    struct super_block  *bd_super;
    struct block_device *bd_contains;
    struct block_device *bd_this;
    unsigned            bd_block_size;
    loff_t              bd_size;
    struct gendisk      *bd_disk;
    struct list_head    bd_list;
    atomic_t            bd_openers;
    uint32_t            bd_partno;
    uint64_t            bd_start_sect;
    uint64_t            bd_nr_sectors;
    void                *bd_private;
};

struct elevator_queue {
    struct elevator_type *type;
    void                *elevator_data;
    void                (*elevator_merge_fn)(struct request_queue *, struct request **, struct bio *);
    bool                (*elevator_allow_merge_fn)(struct request_queue *, struct request *, struct bio *);
    void                (*elevator_bio_merged_fn)(struct request_queue *, struct request *, struct bio *);
    struct request *    (*elevator_request_fn)(struct request_queue *);
    void                (*elevator_add_req_fn)(struct request_queue *, struct request *);
    void                (*elevator_completed_req_fn)(struct request_queue *, struct request *);
    struct request *    (*elevator_former_req_fn)(struct request_queue *, struct request *);
    struct request *    (*elevator_latter_req_fn)(struct request_queue *, struct request *);
};

struct elevator_type {
    struct list_head list;
    const char      *elevator_name;
    struct module   *owner;
    void            (*elevator_init_fn)(struct request_queue *);
    void            (*elevator_exit_fn)(struct elevator_queue *);
};

struct blk_mq_tag_set {
    struct request_queue **queues;
    uint32_t            nr_hw_queues;
    uint32_t            queue_depth;
    uint32_t            reserved_tags;
    uint32_t            nr_maps;
    void                *driver_data;
    int                 (*init)(struct request_queue *, void *);
    void                (*exit)(struct request_queue *);
    int                 (*poll)(struct request_queue *);
};

/* Bio operations */
struct bio *bio_alloc(uint32_t gfp_mask, uint32_t nr_iovecs);
void bio_put(struct bio *bio);
void bio_endio(struct bio *bio);
int submit_bio(struct bio *bio);
struct bio *bio_clone_fast(struct bio *bio, uint32_t gfp_mask, struct bio_set *bs);
void bio_advance(struct bio *bio, uint32_t bytes);

/* Request queue */
struct request_queue *blk_alloc_queue(int node_id);
void blk_cleanup_queue(struct request_queue *q);
void blk_queue_make_request(struct request_queue *q, void *fn);
void blk_queue_max_hw_sectors(struct request_queue *q, uint32_t max_hw_sectors);
void blk_queue_logical_block_size(struct request_queue *q, uint32_t size);
void blk_queue_physical_block_size(struct request_queue *q, uint32_t size);

/* Request */
struct request *blk_mq_alloc_request(struct request_queue *q, uint32_t op, uint32_t flags);
void blk_mq_free_request(struct request *rq);
void blk_mq_end_request(struct request *rq, int error);
void blk_mq_start_request(struct request *rq);

/* Elevator */
int elevator_init(struct request_queue *q, char *name);
void elevator_exit(struct elevator_queue *e);

/* Gendisk */
struct gendisk *alloc_disk(int minors);
void del_gendisk(struct gendisk *disk);
void set_capacity(struct gendisk *disk, uint64_t sectors);
void add_disk(struct gendisk *disk);

/* Block device */
struct block_device *bdget(dev_t dev);
void bdput(struct block_device *bdev);
int blkdev_get(struct block_device *bdev, uint32_t mode);
void blkdev_put(struct block_device *bdev);

/* I/O schedulers */
extern struct elevator_type elevator_noop;
extern struct elevator_type elevator_deadline;
extern struct elevator_type elevator_cfq;
extern struct elevator_type elevator_mq_deadline;
extern struct elevator_type elevator_bfq;
extern struct elevator_type elevator_kyber;

/* Server features */
#define BLKDEV_DISCARD  1
#define BLKDEV_WRITE_ZEROES 2
#define BLKDEV_FUA      4
#define BLKDEV_SECURE_ERASE 8

/* RAID */
#define RAID_LEVEL_0    0
#define RAID_LEVEL_1    1
#define RAID_LEVEL_5    5
#define RAID_LEVEL_6    6
#define RAID_LEVEL_10   10

struct mddev; /* RAID device */
struct md_personality {
    const char *name;
    int level;
    int (*run)(struct mddev *mddev);
    void (*stop)(struct mddev *mddev);
    int (*make_request)(struct mddev *mddev, struct bio *bio);
};

#endif /* _LINUXAB_BLOCK_H */