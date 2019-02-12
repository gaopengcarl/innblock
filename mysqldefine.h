/*************************************************************************
  > File Name: mysqldefine.h
  > Author: gaopeng QQ:22389860 all right reserved
  > Mail: gaopp_200217@163.com
  > Created Time: Thu 28 Sep 2017 03:16:02 PM CST
 ************************************************************************/

/** 1.file page header 1-38 **/
/** The byte offsets on a file page for various variables @{
 *  */
#define FIL_PAGE_SPACE_OR_CHKSUM 0	/*!< in < MySQL-4.0.14 space id the
									  page belongs to (== 0) but in later
									  versions the 'new' checksum of the
									  page */
#define FIL_PAGE_OFFSET		4	/*!< page offset inside space */
#define FIL_PAGE_PREV		8	/*!< if there is a 'natural'
								  predecessor of the page, its
								  offset.  Otherwise FIL_NULL.
								  This field is not set on BLOB
								  pages, which are stored as a
								  singly-linked list.  See also
								  FIL_PAGE_NEXT. */
#define FIL_PAGE_NEXT		12	/*!< if there is a 'natural' successor
								  of the page, its offset.
								  Otherwise FIL_NULL.
								  B-tree index pages
								  (FIL_PAGE_TYPE contains FIL_PAGE_INDEX)
								  on the same PAGE_LEVEL are maintained
								  as a doubly linked list via
								  FIL_PAGE_PREV and FIL_PAGE_NEXT
								  in the collation order of the
								  smallest user record on each page. */
#define FIL_PAGE_LSN		16	/*!< lsn of the end of the newest
								  modification log record to the page */
#define	FIL_PAGE_TYPE		24	/*!< file page type: FIL_PAGE_INDEX,...,
								  2 bytes.

								  The contents of this field can only
								  be trusted in the following case:
								  if the page is an uncompressed
								  B-tree index page, then it is
								  guaranteed that the value is
								  FIL_PAGE_INDEX.
								  The opposite does not hold.

								  In tablespaces created by
								  MySQL/InnoDB 5.1.7 or later, the
								  contents of this field is valid
								  for all uncompressed pages. */
#define FIL_PAGE_FILE_FLUSH_LSN	26	/*!< this is only defined for the
									  first page of the system tablespace:
									  the file has been flushed to disk
									  at least up to this LSN. For
									  FIL_PAGE_COMPRESSED pages, we store
									  the compressed page control information
									  in these 8 bytes. */
#define	FIL_NULL 0xFFFFFFFF       /*no PAGE_NEXT or PAGE_PREV */
/** starting from 4.1.x this contains the space id of the page */
#define FIL_PAGE_ARCH_LOG_NO_OR_SPACE_ID  34

#define FIL_PAGE_SPACE_ID  FIL_PAGE_ARCH_LOG_NO_OR_SPACE_ID

#define FIL_PAGE_DATA		38U	/*!< start of the data on the page */


/** 2.page type **/

/** File page types (values of FIL_PAGE_TYPE) @{
 *  */
#define FIL_PAGE_INDEX		17855	/*!< B-tree node noraml data page*/
#define FIL_PAGE_RTREE		17854	/*!< B-tree node */
#define FIL_PAGE_UNDO_LOG	2	/*!< Undo log page */
#define FIL_PAGE_INODE		3	/*!< Index node */
#define FIL_PAGE_IBUF_FREE_LIST	4	/*!< Insert buffer free list */
/* File page types introduced in MySQL/InnoDB 5.1.7 */
#define FIL_PAGE_TYPE_ALLOCATED	0	/*!< Freshly allocated page */
#define FIL_PAGE_IBUF_BITMAP	5	/*!< Insert buffer bitmap */
#define FIL_PAGE_TYPE_SYS	6	/*!< System page */
#define FIL_PAGE_TYPE_TRX_SYS	7	/*!< Transaction system data */
#define FIL_PAGE_TYPE_FSP_HDR	8	/*!< File space header */
#define FIL_PAGE_TYPE_XDES	9	/*!< Extent descriptor page */
#define FIL_PAGE_TYPE_BLOB	10	/*!< Uncompressed BLOB page */
#define FIL_PAGE_TYPE_ZBLOB	11	/*!< First compressed BLOB page */
#define FIL_PAGE_TYPE_ZBLOB2	12	/*!< Subsequent compressed BLOB page */
#define FIL_PAGE_TYPE_UNKNOWN	13	/*!< In old tablespaces, garbage
									  in FIL_PAGE_TYPE is replaced with this
									  value when flushing pages. */
#define FIL_PAGE_COMPRESSED	14	/*!< Compressed page */
#define FIL_PAGE_ENCRYPTED	15	/*!< Encrypted page */
#define FIL_PAGE_COMPRESSED_AND_ENCRYPTED 16
/*!< Compressed and Encrypted page */
#define FIL_PAGE_ENCRYPTED_RTREE 17	/*!< Encrypted R-tree page */

/** 3.index page header 38-94 **/

#define FSEG_HDR_SPACE		0	/*!< space id of the inode */
#define FSEG_HDR_PAGE_NO	4	/*!< page number of the inode */
#define FSEG_HDR_OFFSET		8	/*!< byte offset of the inode */

#define FSEG_HEADER_SIZE	10	/*!< Length of the file system
								  header, in bytes */

#define	PAGE_HEADER	FIL_PAGE_DATA	/* index page header starts at this
									   offset */
/*-----------------------------*/
#define PAGE_N_DIR_SLOTS 0	/* number of slots in page directory */
#define	PAGE_HEAP_TOP	 2	/* pointer to record heap top */
#define	PAGE_N_HEAP	 4	/* number of records in the heap,
                                bit 15=flag: new-style compact page format */
#define	PAGE_FREE	 6	/* pointer to start of page free record list */
#define	PAGE_GARBAGE	 8	/* number of bytes in deleted records */
#define	PAGE_LAST_INSERT 10	/* pointer to the last inserted record, or
							   NULL if this info has been reset by a delete,
							   for example */
#define	PAGE_DIRECTION	 12	/* last insert direction: PAGE_LEFT, ... */
#define	PAGE_N_DIRECTION 14	/* number of consecutive inserts to the same
							   direction */
#define	PAGE_N_RECS	 16	/* number of user records on the page */
#define PAGE_MAX_TRX_ID	 18	/* highest id of a trx which may have modified
							   a record on the page; trx_id_t; defined only
							   in secondary indexes and in the insert buffer
							   tree */
#define PAGE_HEADER_PRIV_END 26	/* end of private data structure of the page
								   header which are set in a page create */
/*----*/
#define	PAGE_LEVEL	 26	/* level of the node in an index tree; the
						   leaf level is the level 0.  This field should
						   not be written to after page creation. */
#define	PAGE_INDEX_ID	 28	/* index id where the page belongs.
							   This field should not be written to after
							   page creation. */


#define PAGE_BTR_SEG_LEAF 36	/* file segment header for the leaf pages in
								   a B-tree: defined only on the root page of a
								   B-tree, but not in the root of an ibuf tree */
#define PAGE_BTR_IBUF_FREE_LIST	PAGE_BTR_SEG_LEAF
#define PAGE_BTR_IBUF_FREE_LIST_NODE PAGE_BTR_SEG_LEAF
/* in the place of PAGE_BTR_SEG_LEAF and _TOP
 *                                 there is a free list base node if the page is
 *                                                                 the root page of an ibuf tree, and at the same
 *                                                                                                 place is the free list node if the page is in
 *                                                                                                                                 a free list */
#define PAGE_BTR_SEG_TOP (36 + FSEG_HEADER_SIZE)
/* file segment header for the non-leaf pages
 *                                 in a B-tree: defined only on the root page of
 *                                                                 a B-tree, but not in the root of an ibuf
 *                                                                                                 tree */
//The file segment header points to the inode describing the file segment.
#define FSEG_HDR_SPACE		0	/*!< space id of the inode */
#define FSEG_HDR_PAGE_NO	4	/*!< page number of the inode */
#define FSEG_HDR_OFFSET		8	/*!< byte offset of the inode */

#define FSEG_HEADER_SIZE	10	/*!< Length of the file system
                                        header, in bytes */

/** 4.INFIMUM && SUPREMUM **/

/* Number of extra bytes in a new-style record,
 * in addition to the data and the offsets */
#define REC_N_NEW_EXTRA_BYTES	5 //new-style记录扩展字节

/* Record status values */
#define REC_STATUS_ORDINARY	0 //普通记录
#define REC_STATUS_NODE_PTR	1 //非叶子结点带指针
#define REC_STATUS_INFIMUM	2
#define REC_STATUS_SUPREMUM	3

/* The following four constants are needed in page0zip.cc in order to
 * efficiently compress and decompress pages. */

/* The offset of heap_no in a compact record */
#define REC_NEW_HEAP_NO		4
/* The shift of heap_no in a compact record.
 * The status is stored in the low-order bits. */
#define	REC_HEAP_NO_SHIFT	3

/* Length of a B-tree node pointer, in bytes */
#define REC_NODE_PTR_SIZE	4

/*----*/
#define PAGE_DATA	(PAGE_HEADER + 36 + 2 * FSEG_HEADER_SIZE)
/* start of data on the page */
#define PAGE_NEW_INFIMUM	(PAGE_DATA + REC_N_NEW_EXTRA_BYTES)
/* offset of the page infimum record on a
 *                                 new-style compact page */
#define PAGE_NEW_SUPREMUM	(PAGE_DATA + 2 * REC_N_NEW_EXTRA_BYTES + 8)
/* offset of the page supremum record on a
 *                                 new-style compact page */
#define PAGE_NEW_SUPREMUM_END (PAGE_NEW_SUPREMUM + 8)
/* offset of the page supremum record end on
 *                                 a new-style compact page */

/** 5.page end **/
#define FIL_PAGE_END_LSN_OLD_CHKSUM 8	/*!< the low 4 bytes of this are used
										  to store the page checksum, the
										  last 4 bytes should be identical
										  to the last 4 bytes of FIL_PAGE_LSN */
#define FIL_PAGE_DATA_END	8	/*!< size of the page trailer */


/** 6.PAGE DIRECTORY **/

/* Offset of the directory start down from the page end. We call the
 * slot with the highest file address directory start, as it points to
 * the first record in the list of records. */
#define	PAGE_DIR		FIL_PAGE_DATA_END

/* We define a slot in the page directory as two bytes */
#define	PAGE_DIR_SLOT_SIZE	2

/* The offset of the physically lower end of the directory, counted from
 * page end, when the page is empty */
#define PAGE_EMPTY_DIR_START	(PAGE_DIR + 2 * PAGE_DIR_SLOT_SIZE)

/* The maximum and minimum number of records owned by a directory slot. The
 * number may drop below the minimum in the first and the last slot in the
 * directory. */
#define PAGE_DIR_SLOT_MAX_N_OWNED	8
#define	PAGE_DIR_SLOT_MIN_N_OWNED	4

#define FAIL_CHK 8
#define FAIL_LSN 4



/*
 *  part 7:
 *   Add by author:gaopeng
 *    Here is table space header/file space header every tablespace have only one at page 0
 *     in Fsp0fsp.h 38-150
 *     */
#define FIL_ADDR_SIZE   6       /* address size is 6 bytes */
#define FLST_BASE_NODE_SIZE     (4 + 2 * FIL_ADDR_SIZE)
/*
 *                   list--length:4
 *                   FIL_ADDR_SIZE prv page node   :4
 *                  offset      :2
 *                  FIL_ADDR_SIZE nxt page node   :4
 *                  offset      :2   
 *                                                       */

#define FSP_SPACE_ID            0       /* space id */
#define FSP_NOT_USED            4       /* this field contained a value up to
										   which we know that the modifications
										   in the database have been flushed to
										   the file space; not used now */
#define FSP_SIZE                8       /* Current size of the space in
										   pages */
#define FSP_FREE_LIMIT          12      /* Minimum page number for which the
										   free list has not been initialized:
										   the pages >= this limit are, by
										   definition, free; note that in a
										   single-table tablespace where size
										   < 64 pages, this number is 64, i.e.,
										   we have initialized the space
										   about the first extent, but have not
										   physically allocated those pages to the
										   file */
#define FSP_SPACE_FLAGS         16      /* fsp_space_t.flags, similar to
										   dict_table_t::flags */
#define FSP_FRAG_N_USED         20      /* number of used pages in the
										   FSP_FREE_FRAG list */
#define FSP_FREE                24      /* list of free extents */
#define FSP_FREE_FRAG           (24 + FLST_BASE_NODE_SIZE)
/* list of partially free extents not
 *                                         belonging to any segment */
#define FSP_FULL_FRAG           (24 + 2 * FLST_BASE_NODE_SIZE)
/* list of full extents not belonging
 *                                         to any segment */
#define FSP_SEG_ID              (24 + 3 * FLST_BASE_NODE_SIZE)
/* 8 bytes which give the first unused
 *                                         segment id */
#define FSP_SEG_INODES_FULL     (32 + 3 * FLST_BASE_NODE_SIZE)
/* list of pages containing segment
 *                                         headers, where all the segment inode
 *                                                                                 slots are reserved */
#define FSP_SEG_INODES_FREE     (32 + 4 * FLST_BASE_NODE_SIZE)
/* list of pages containing segment
 *                                         headers, where not all the segment
 *                                                                                 header slots are reserved */
/* File space header size */
#define FSP_HEADER_SIZE         (32 + 5 * FLST_BASE_NODE_SIZE)

#define FSP_FREE_ADD            4       /* this many free extents are added
										   to the free list from above
										   FSP_FREE_LIMIT at a time */
