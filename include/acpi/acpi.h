#ifndef __ACPI_H__
#define __ACPI_H__

/* Copy-pasted from acpixf.h, acexcep.h and actypes.h. */

/*
 * Miscellaneous types
 */
typedef u32 acpi_status;	/* All ACPI Exceptions */
typedef u32 acpi_name;		/* 4-byte ACPI name */
typedef char *acpi_string;	/* Null terminated ASCII string */
typedef void *acpi_handle;	/* Actually a ptr to a NS Node */

typedef
acpi_status(*acpi_walk_callback) (acpi_handle object,
				  u32 nesting_level,
				  void *context, void **return_value);

/*
 * Success is always zero, failure is non-zero
 */
#define ACPI_SUCCESS(a)                 (!(a))
#define ACPI_FAILURE(a)                 (a)

#define ACPI_SKIP(a)                    (a == AE_CTRL_SKIP)
#define AE_OK                           (acpi_status) 0x0000

static inline acpi_status
acpi_get_devices(const char *HID,
		 acpi_walk_callback user_function,
		 void *context, void **return_value)
{
	return user_function(NULL, 0, context, NULL);
}

static inline bool acpi_dev_present(const char *hid) { return true; }
static inline bool acpi_dev_found(const char *hid) { return true; }

#endif				/* __ACPI_H__ */
