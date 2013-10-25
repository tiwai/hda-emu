/* FIXME: this is no portable at all */

#define get_unaligned_le16(ptr)	*((u16*)(ptr))
#define get_unaligned_le32(ptr)	*((u32*)(ptr))
#define get_unaligned_le64(ptr)	*((u64*)(ptr))

#define put_unaligned_le16(val, dst)	(*(u16 *)(dst) = (u16)(val))
#define put_unaligned_le32(val, dst)	(*(u32 *)(dst) = (u32)(val))
#define put_unaligned_le64(val, dst)	(*(u64 *)(dst) = (u64)(val))
