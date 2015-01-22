/******************************************************************************
**
**      Filename:       btp_cfg.c
**
**      Purpose:        Linux configuration and module loading/unloading for
**                      NanoBus/PCI adapters.
**
**      Functions:      init_module(), cleanup_module()
**
**      $Revision$
**
******************************************************************************/
/*****************************************************************************
**
**        Copyright (c) 1999-2000 by SBS Technologies, Inc.
**                     All Rights Reserved.
**              License governs use and distribution.
**
*****************************************************************************/

#ifndef LINT
static const char revcntrl[] = "@(#)"__FILE__"  $Revision$" __DATE__;
#endif  /* LINT */

/* Used by btpdd.h to know if it should define __NO_VERSION__ or not */

#define BTP_CFG_C

#include <linux/module.h>
#include <linux/vmalloc.h>

#include <asm/irq.h>
#include <asm/io.h>

#include "btdd.h"

#if     0 && !defined(NDEBUG)

#define DEBUG_INIT_SEQUENCE 1   /* For when you need the init_fail() routine */

#elif   !defined(DEBUG_INIT_SEQUENCE)

#define DEBUG_INIT_SEQUENCE 0   /* Don't put init_fail() in code */

#endif  /* 0 */

/******************************************************************************
**
**      Global symbols
**
******************************************************************************/

EXPORT_SYMBOL(bt_trace_lvl_g);
unsigned long bt_trace_lvl_g = BT_TRC_DEFAULT;

EXPORT_SYMBOL(bt_name_gp);
const char *bt_name_gp = BT_DRV_NAME;

unsigned int btp_major_g = 0;   /* Our major device number */
volatile bt_data32_t *bt_trace_mreg_gp;

#if     defined(DEBUG)
EXPORT_SYMBOL(bt_unit_array_gp);
#endif  /* DEBUG */

bt_unit_t *bt_unit_array_gp[BT_MAX_UNITS+1] = {
    NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL

};

/* default File operations structure */

struct file_operations btp_fops = {

#if     defined(__GCC__)
   .open=       btp_open,
   .release=    btp_close,
   .llseek=     btp_llseek,
   .read=       btp_read,
   .write=      btp_write,
   .ioctl=      btp_ioctl,
   .mmap=       btp_mmap
#else   /* __GCC__ */

#if     LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,0)
   llseek:	btp_llseek,
   read:	btp_read,
   write:	btp_write,
   ioctl:	btp_ioctl,
   mmap:	btp_mmap,
   open:	btp_open,
   release:	btp_close,
#else   /* LINUX_VERSION_CODE < KERNEL_VERSION(2,4,0) */
   btp_llseek,
   btp_read,
   btp_write,
   NULL,        /* readdir */
   NULL,        /* poll */
   btp_ioctl,
   btp_mmap,
   btp_open,
   NULL,        /* flush */
   btp_close,   /* release */
   NULL,        /* sync */
   NULL,        /* check_media_change */
   NULL,        /* revalidate */
   NULL         /* lock */
#endif  /*  LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,0) || \
            LINUX_VERSION_CODE <  KERNEL_VERSION(2,4,0) */

#endif  /* __GCC__ */
};

/******************************************************************************
**
**      Module definitions and Driver loading parameters
**
******************************************************************************/

unsigned int bt_major = 0;      /* Default major device number */
MODULE_PARM(bt_major, "i");
MODULE_PARM_DESC(bt_major, "Default major device number, 0 == auto configure it.");

unsigned long trace = 0;        /* Device driver trace level */
MODULE_PARM(trace, "l");
MODULE_PARM_DESC(trace, "Bit mask of module tracing flags.");

unsigned int icbr_q_size[BT_MAX_UNITS+1] = {
    DEFAULT_Q_SIZE, DEFAULT_Q_SIZE, DEFAULT_Q_SIZE, DEFAULT_Q_SIZE,
    DEFAULT_Q_SIZE, DEFAULT_Q_SIZE, DEFAULT_Q_SIZE, DEFAULT_Q_SIZE,
    DEFAULT_Q_SIZE, DEFAULT_Q_SIZE, DEFAULT_Q_SIZE, DEFAULT_Q_SIZE,
    DEFAULT_Q_SIZE, DEFAULT_Q_SIZE, DEFAULT_Q_SIZE, DEFAULT_Q_SIZE,
    
};
MODULE_PARM(icbr_q_size, "i");
MODULE_PARM_DESC(icbr_q_size, "Number of entries to create in the interrupt callback routine (ICBR) queue.");

unsigned long lm_size[BT_MAX_UNITS+1] = {
    DEFAULT_LMEM_SIZE, DEFAULT_LMEM_SIZE, DEFAULT_LMEM_SIZE, DEFAULT_LMEM_SIZE,
    DEFAULT_LMEM_SIZE, DEFAULT_LMEM_SIZE, DEFAULT_LMEM_SIZE, DEFAULT_LMEM_SIZE,
    DEFAULT_LMEM_SIZE, DEFAULT_LMEM_SIZE, DEFAULT_LMEM_SIZE, DEFAULT_LMEM_SIZE,
    DEFAULT_LMEM_SIZE, DEFAULT_LMEM_SIZE, DEFAULT_LMEM_SIZE, DEFAULT_LMEM_SIZE,
    
};

MODULE_PARM(lm_size,"0-" __MODULE_STRING(BT_MAX_UNITS) "l");
MODULE_PARM_DESC(lm_size, "Per unit array given the size of the local memory 
device. Default " __MODULE_STRING(DEFAULT_LMEM_SIZE) ".");

MODULE_AUTHOR("SBS Technologies, Inc.");
MODULE_DESCRIPTION("Device driver for SBS Technologies Connectivity Products PCI-VMEbus adapters.");
MODULE_SUPPORTED_DEVICE(BT_DRV_NAME);

/*
** Local function prototypes
*/
static int create_unit(bt_unit_t *unit_p);
static int destroy_unit(bt_unit_t *unit_p);

/*
** External function prototypes
*/
extern unsigned long bt_kvm2bus(void * vm_addr_p);
extern bt_error_t btk_irq_qs_init(bt_unit_t *unit_p, size_t q_size);
extern void btk_irq_qs_fini(bt_unit_t *unit_p, size_t q_size);
extern void btk_isr(int irq, void *vunit_p, struct pt_regs *regs);
extern void btk_setup(bt_unit_t *unit_p);

/*
**  Static variables
*/
BT_FILE_NUMBER(TRACE_BTP_CFG_C);


/*****************************************************************************
**
**      Name:           init_module
**
**      Purpose:        Loads and initializes the device driver
**
**      Args:
**          void
**
**      Modifies:
**          None
**          
**      Returns:
**          0           Success
**          Otherwise   Error number
**
**      Notes:
**
*****************************************************************************/

int init_module(
    void
    )
{
    int                 ret_val = 0;

    int                 result;         /* Return value from various PCI calls */

    struct pci_dev      *curr_dev_p;    /* Current PCI device we are inspecting */

    u16                 vendor, device; /* PCI vendor and device ID */

    unsigned int        local_pn;       /* Our assembly part number */

    unsigned int        dev_count;      /* How many of our device we have */

    unsigned short      unit;


    /*
    ** First take care of the case where PCI isn't supported but they
    ** are (for some strange reason known only to god) trying to load
    ** the driver for our PCI device.
    */

    FUNCTION("init_module");
    LOG_UNKNOWN_UNIT;

#if !defined(CONFIG_PCI)
    #error Must compile on a system with PCI support.
    return -ENODEV;
#endif

    if (!pci_present()) {
        printk("<0>" "No PCI support present.\n");
        WARN_STR("No PCI support present.\n");
        return -ENODEV;
    }

    if (0 != trace) {
        bt_trace_lvl_g = trace;
    }
    TRC_MSG(BT_TRC_CFG|BT_TRC_DETAIL, (LOG_FMT "Trace level = 0x%lx.\n",
        LOG_ARG, bt_trace_lvl_g));

    SET_MODULE_OWNER(&btp_fops);

    /* 
    ** Initialize our memory handling routines 
    */
    btk_mem_init();

    /* 
    ** Find our PCI devices 
    */
    curr_dev_p = NULL;
    dev_count = 0;
    do {
        bt_unit_t *unit_p;

        /* 
        ** Find our next device 
        */

#if     LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,0)
        curr_dev_p = pci_find_class(PCI_CLASS_BRIDGE_OTHER <<8, curr_dev_p);
        /*
        ** Nothing says this, but you have to shift the class code over
        ** by a byte in order to account for the interface specification
        ** field. The class that pci_find_class() matchs is really all
        ** three bytes (Base Class, Sub Class, IF spec) of the config
        ** space.
        */
        if (NULL == curr_dev_p) {
            /* End of chain, no more devices to find */
            break;
        }
       
        /* 
        ** Check if it is one of our cards 
        */
        result = pci_read_config_word(curr_dev_p, PCI_VENDOR_ID, &vendor);
        if (PCIBIOS_SUCCESSFUL != result) {
            /* Should always succeed, guess we ignore it and skip to the next device */
            continue;
        }
        if (BT_PCI_VENDOR_BIT3 != vendor) {
            /* Isn't our device, skip to the next device */
            continue;
        }

#elif   LINUX_VERSION_CODE >= KERNEL_VERSION(2,3,0)

        0&0&0&0; /* Untested */

        curr_dev_p = pci_find_device(BT_PCI_VENDOR_BIT3, PCI_ANY_ID, curr_dev_p);
        if (NULL == curr_dev_p) {
            /* End of chain, no more devices to find */
            break;
        }
#else   /* LINUX_VERSION_CODE < KERNEL_VERSION(2,3,0) */
        curr_dev_p = pci_find_class(PCI_CLASS_BRIDGE_OTHER <<8, curr_dev_p);
        /*
        ** Nothing says this, but you have to shift the class code over
        ** by a byte in order to account for the interface specification
        ** field. The class that pci_find_class() matchs is really all
        ** three bytes (Base Class, Sub Class, IF spec) of the config
        ** space.
        */
        if (NULL == curr_dev_p) {
            /* End of chain, no more devices to find */
            break;
        }
       
        /* 
        ** Check if it is one of our cards 
        */
        result = pci_read_config_word(curr_dev_p, PCI_VENDOR_ID, &vendor);
        if (PCIBIOS_SUCCESSFUL != result) {
            /* Should always succeed, guess we ignore it and skip to the next device */
            continue;
        }
        if (BT_PCI_VENDOR_BIT3 != vendor) {
            /* Isn't our device, skip to the next device */
            continue;
        }
#endif /*  LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,0) || \
           LINUX_VERSION_CODE >= KERNEL_VERSION(2,3,0) || \
           LINUX_VERSION_CODE <  KERNEL_VERSION(2,3,0) */

        /*
        ** Get the device id for this card */
        result = pci_read_config_word(curr_dev_p, PCI_DEVICE_ID, &device);
        if (PCIBIOS_SUCCESSFUL != result) {
            /* Should always succeed, guess we ignore it and skip to the next device */
            continue;
        }
        switch (device) {
          case BT_PCI_DEVICE_617:
            local_pn = BT_PN_PCI_DMA;
            break;

          case BT_PCI_DEVICE_614:
            local_pn = BT_PN_PCI;
            break;

          case BT_PCI_DEVICE_616:
            local_pn = BT_PN_PCI_NODMA;
            break;

          case BT_PCI_DEVICE_618:
            /* Could be either the 618 or the 628, can't tell the two apart */
            local_pn = BT_PN_PCI_FIBER;
            break;

          case BT_PCI_DEVICE_704:
            local_pn = BT_PN_PCI_FIBER_D64;
            break;

          default:
            local_pn = BT_PN_UNKNOWN;
            break;
        }
        if (BT_PN_UNKNOWN == local_pn) {
            /* Claimed to be our card, but we don't recognize the device ID */
            continue;
        }

        /*
        ** Make sure we are not over the allowed number of units
        */
        if (BT_MAX_UNITS < dev_count ) {
            WARN_STR("Exceeded the maximum number of units allowed.\n");
            break;
        }
        TRC_MSG(BT_TRC_CFG, (LOG_FMT "Found %d at bus %d.\n", 
            LOG_ARG, local_pn, curr_dev_p->bus->number));

        /* 
        ** Got another unit to handle 
        */
        unit_p = btk_mem_alloc(sizeof(bt_unit_t),0);
        if (NULL == unit_p) {
            WARN_MSG((LOG_FMT "Not enough memory to create unit structure for device at bus %d.\n",
                      LOG_ARG, curr_dev_p->bus->number));
            /*
            ** No sense in searching for more, they will just fail to
            ** allocate memory as well.
            */
            break;
        }
        BTK_BZERO(unit_p, sizeof(bt_unit_t));

        /*
        ** Initialize this unit
        */
        unit_p->loc_id = local_pn;
        unit_p->dev_p = curr_dev_p;
        if (0 != create_unit(unit_p)) {
            WARN_MSG((LOG_FMT "Could not create unit for device at bus %d.\n",
                LOG_ARG, curr_dev_p->bus->number));
            btk_mem_free(unit_p, sizeof(bt_unit_t));
        }
        dev_count++;
        
    } while (NULL != curr_dev_p);

    /*
    ** Check that we actually found a device
    */
    if (0 == dev_count) {
        TRC_STR(BT_TRC_CFG|BT_TRC_DETAIL, "Did not find any devices.\n");
        goto init_module_end;
    }

    /* 
    ** Get the major device number for our card 
    */
    ret_val = register_chrdev(bt_major, bt_name_gp, &btp_fops);
    if (ret_val < 0) {
        WARN_MSG((LOG_FMT "Could not register device with major number %d.\n",
            LOG_ARG, bt_major));
        btp_major_g = 0;
        goto init_module_cleanup;
    }
    btp_major_g = ret_val;
    ret_val = 0;

init_module_end:
    TRC_MSG(BT_TRC_CFG|BT_TRC_DETAIL,
        (LOG_FMT "Completed module initialization: returning %d.\n",
         LOG_ARG, ret_val));
    return ret_val;


init_module_cleanup:
    TRC_STR(BT_TRC_CFG|BT_TRC_DETAIL, "Failed initialization: Cleaning up.\n");

    for (unit = 0; unit <= BT_MAX_UNITS; unit++) {
        bt_unit_t *unit_p;

        unit_p = bt_unit_array_gp[unit];
        if (NULL != unit_p) {
            ret_val = destroy_unit(unit_p);
            if (ret_val < 0) {
                WARN_MSG((LOG_FMT "Error %d: Could not release unit %d.\n", 
                    LOG_ARG, -ret_val, (int) unit));
                continue;
            }
            dev_count--;
            if (0 == dev_count) {
                break;  /* Found all of the units that we created */
            }
        }
    }

    /* 
    ** Clean up for our memory handling routines 
    */
    btk_mem_fini();

    if (ret_val >= 0) {
        TRC_STR(BT_TRC_CFG, 
            "Overriding exit value in cleanup to make it an error value.\n");
        ret_val = -ENXIO;
    }

    TRC_MSG(BT_TRC_CFG|BT_TRC_DETAIL, ( LOG_FMT 
        "Failed initialization: return %d.\n", LOG_ARG, ret_val));
    return ret_val;
}


/*****************************************************************************
**
**      Name:           cleanup_module
**
**      Purpose:        Releases resources and unloads the device driver
**
**      Args:
**          void
**
**      Modifies:
**          None
**          
**      Returns:
**          void
**
**      Notes:
**
*****************************************************************************/

void cleanup_module(
    void
    )
{
    unsigned short      unit;
    int                 ret_val;
    bt_unit_t           *unit_p;

    FUNCTION("cleanup_module");
    LOG_UNKNOWN_UNIT;

    FENTRY;

    if (btp_major_g != 0) {
        unregister_chrdev(btp_major_g, bt_name_gp);
        btp_major_g = 0;
    }

    for (unit = 0; unit <= BT_MAX_UNITS; unit++) {
        unit_p = bt_unit_array_gp[unit];
        if (NULL != unit_p) {
            ret_val = destroy_unit(unit_p);
            if (ret_val < 0) {
                WARN_MSG((LOG_FMT "Error %d: Could not release unit %d.\n", 
                    LOG_ARG, -ret_val, (int) unit));
            }
        }
    }

    /* 
    ** Clean up for our memory handling routines 
    */
    btk_mem_fini();
    TRC_STR(BT_TRC_CFG, "Driver unloaded.\n");

    FEXIT(0);
    return;
}


/******************************************************************************
**
**      Local Functions
**
******************************************************************************/


/******************************************************************************
**
**              Description of initialization sequence
**
******************************************************************************/

/*
** The functions create_unit() and destroy_unit() use a table of functions
** to drive them. The table contains a pointer to an init_xxx() function
** and either a cleanup_xxx() function or a pointer to the cleanup_null()
** function.
**
** Each step of the initialization is given it's own function. The order
** it is placed in the table determines which order the functions are
** called in. 
**
** It is assumed that a function that fails has cleaned up after itself
** as best it could; the associated clean up function is not called, nor 
** are functions later in the list called.
**
** IMPORTANT: If init_xxx() returns an error, cleanup_xxx() is NOT called.
**
** Every function takes the same parameter, the unit pointer, and returns
** an integer. These functions are called from within the create_unit()
** and destroy_unit() functions.
**
** Since Linux returns -errno when indicating errors, we have done the
** same in these functions.
*/

/* init function signature */
typedef int init_fn_t(bt_unit_t *);

/* declare all the functions */
static init_fn_t 
    init_pci_config,    cleanup_pci_config,
    init_defaults,
    init_boot_parms,
    init_unit_array,    cleanup_unit_array,
    init_bit_maps,      cleanup_bit_maps,
    init_cookies,
    init_events,        cleanup_events,
    init_mutexs,        cleanup_mutexs,
    init_lists,         cleanup_lists,
    init_irq_q,         cleanup_irq_q,
    init_isr,           cleanup_isr,
    init_dma,           cleanup_dma,
    init_lm,            cleanup_lm,
    init_ldev,
    init_card,
    init_swapping,
#if     DEBUG_INIT_SEQUENCE > 0
    init_fail,  /* DEBUG only */
#endif  /* DEBUG_INIT_SEQUENCE > 0 */
                        cleanup_null;


#define INIT_FN(func_idx, unit_p) \
    (*(bt_init_func[func_idx][0]))(unit_p)

#define CLEANUP_FN(func_idx, unit_p) \
    (*(bt_init_func[func_idx][1]))(unit_p)

/* 
** The actual array of pointers 
*/
static init_fn_t * bt_init_func[][2] = {
    { init_defaults,          cleanup_null },
    { init_pci_config,        cleanup_pci_config },
    { init_cookies,           cleanup_null },
    { init_unit_array,        cleanup_unit_array },
    { init_boot_parms,        cleanup_null },
    { init_mutexs,            cleanup_mutexs },
    { init_events,            cleanup_events },
    { init_lists,             cleanup_lists },
    { init_irq_q,             cleanup_irq_q },
    { init_bit_maps,          cleanup_bit_maps },
    { init_dma,               cleanup_dma },
    { init_isr,               cleanup_isr },
    { init_ldev,              cleanup_null },
    { init_card,              cleanup_null },
    { init_swapping,          cleanup_null },
    { init_lm,                cleanup_lm },

#if     DEBUG_INIT_SEQUENCE > 0
    /*
    ** Insert this anywhere you believe you may have a problem.
    ** It allows you to test the initialization/cleanup routines by
    ** forcing initialization to fail at the defined spot.
    */
    { init_fail,              cleanup_null}, /* Force initialization to fail */
#endif /* DEBUG_INIT_SEQUENCE > 0 */

    /*
    ** Put in a dummy entry at the end.
    ** This makes it easier to move stuff around, since only the last entry
    ** doesn't have a comma. You can just move any of the other entries,
    ** no editting required.
    */
    { cleanup_null,           cleanup_null }
};

/* 
** autosize the array 
*/
#define NUM_INIT_FUNCS (sizeof(bt_init_func)/sizeof(bt_init_func[0]))

/******************************************************************************
**
**      Name:           create_unit()
**
**      Purpose:        Initializes the given unit structure
**
**      Args:
**          unit_p      Pointer to unit structure
**
**      Modifies:
**          Everything in the unit structure
**          
**      Returns:
**          0           success
**         <0           -ERRNO
**
**      Notes:
**
*****************************************************************************/

static int create_unit(
    bt_unit_t *unit_p
    )
{
    int ret_val = 0;

    int func_idx;       /* Index into initialization/cleanup function table */

    FUNCTION("create_unit");
    LOG_UNKNOWN_UNIT;

    FENTRY;

    /*
    ** We don't assign a unit number until the unit pointer is inserted
    ** in the global array.
    */
    unit_p->unit_number = BT_MOCK_UNIT;

    for (func_idx = 0; func_idx < NUM_INIT_FUNCS; func_idx++) {
        /* Call each initialization function in order */
        ret_val = INIT_FN(func_idx, unit_p);
        if (ret_val < 0) {
            /* Failed to initialize the unit, need to cleanup and return */
            TRC_MSG(BT_TRC_WARN|BT_TRC_CFG, (LOG_FMT
            "Failed on initialization function %d, error code = %d (0x%x).\n",
                LOG_ARG, func_idx, ret_val, ret_val));
            while (--func_idx >= 0) {
                if (0 >= CLEANUP_FN(func_idx, unit_p)) {
                    TRC_MSG(BT_TRC_WARN|BT_TRC_CFG, (LOG_FMT
                    "Failed on cleanup function %d, error code = %d (0x%x).\n",
                        LOG_ARG, func_idx, ret_val, ret_val));
                    /* Can't even finish cleaning up, this is bad. */
                    break;
                }
            }

            /* Don't call any more initialization functions */
            break;
        }
	/* This can be a good place to trace what happens in init_* functions. */
#if FALSE
	if (IS_CLR(unit_p->bt_status, BT_DMA_AVAIL)) {
	    TRC_MSG(BT_TRC_WARN, (LOG_FMT
		"init%d: unit_p->bt_status = 0x%x; BT_DMA_AVAIL bit is clear.\n",
                 LOG_ARG, func_idx, unit_p->bt_status));
	} else {
	    TRC_MSG(BT_TRC_WARN, (LOG_FMT
		"init%d: unit_p->bt_status = 0x%x; BT_DMA_AVAIL bit is set.\n",
                 LOG_ARG, func_idx, unit_p->bt_status));
	}
#endif  /* FALSE */
    }
    
    FEXIT(ret_val);
    return ret_val;
}

/******************************************************************************
**
**      Name:           destroy_unit()
**
**      Purpose:        Releases resources allocated in the unit structure
**
**      Args:
**          unit_p      Pointer to unit structure
**
**      Modifies:
**          Everything in the unit structure
**          
**      Returns:
**          0           success
**         <0           -ERRNO
**
**      Notes:
**
*****************************************************************************/

static int destroy_unit(
    bt_unit_t *unit_p
    )
{

    int ret_val = 0;

    int func_idx;       /* Index into initialization/cleanup function table */

    FUNCTION("destroy_unit");
    LOG_UNKNOWN_UNIT;

    FENTRY;

    for (func_idx = NUM_INIT_FUNCS-1; func_idx > -1; func_idx--) {
        /* Call each cleanup function in reverse order */
        ret_val = CLEANUP_FN(func_idx, unit_p);
        if (ret_val < 0) {
            TRC_MSG(BT_TRC_WARN|BT_TRC_CFG, (LOG_FMT
                "Failed on cleanup function %d, error code = %d (0x%x).\n",
                LOG_ARG, func_idx, ret_val, ret_val));
            /* Don't call any more cleanup functions */
            break;
        }
    }
    
    FEXIT(ret_val);
    return ret_val;
}

#if     DEBUG_INIT_SEQUENCE > 0

/******************************************************************************
**
**      Name:           init_fail()
**
**      Purpose:        Causes initialization to fail. Useful for debugging
**                      problems in the initialization or cleanup sequence.
**
**      Args:
**          unit_p      Pointer to unit structure
**
**      Modifies:
**          
**      Returns:
**          -ENXIO      Always returns an error
**
*****************************************************************************/

/* ARGSUSED */
static int init_fail (
    bt_unit_t * unit_p
    )
{
    FUNCTION("init_fail");
    LOG_UNIT(unit_p);
    int ret_val = -ENXIO;

    FENTRY;

    FEXIT(ret_val);
    return ret_val;
}

#endif  /* DEBUG_INIT_SEQUENCE > 0 */

/******************************************************************************
**
**      Name:           cleanup_null()
**
**      Purpose:        Does nothing, place holder for functions that
**                      don't need a separate cleanup.
**
**      Args:
**          unit_p      Pointer to unit structure
**
**      Modifies:
**          
**      Returns:
**          0           success
**
*****************************************************************************/

/* ARGSUSED */
static int cleanup_null(
    bt_unit_t *unit_p
    )
{
    return 0;
}


/******************************************************************************
**
**      Name:           init_pci_config()
**
**      Purpose:        Reads configuration information from PCI space, maps
**                      the device into memory space.
**
**      Args:
**          unit_p      Pointer to unit structure
**
**      Modifies:
**          
**      Returns:
**          0           success
**         <0           -ERRNO
**
**      Notes:
**
*****************************************************************************/

static int init_pci_config(
    bt_unit_t *unit_p
    )
{
    int ret_val = 0;

    struct pci_dev *curr_dev_p = unit_p->dev_p;
    unsigned long phys_addr;

    FUNCTION("init_pci_config");
    LOG_UNIT(unit_p);

    FENTRY;

#if     LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,0)
    /* 
    ** Get the kernel physical address of each of the windows 
    */
    unit_p->csr_phys_addr = pci_resource_start(curr_dev_p, 1)
        & PCI_BASE_ADDRESS_MEM_MASK;
    unit_p->mr_phys_addr = pci_resource_start(curr_dev_p, 2)
        & PCI_BASE_ADDRESS_MEM_MASK;
    unit_p->rr_phys_addr = pci_resource_start(curr_dev_p, 3)
        & PCI_BASE_ADDRESS_MEM_MASK;
#elif   LINUX_VERSION_CODE >= KERNEL_VERSION(2,3,0)
    ret_val = pci_enable_device(curr_dev_p);
    if (ret_val < 0) {
        WARN_STR("Could not enable the device.\n");
        goto init_pci_config_end;
    }

    /*
    ** Under Linux 2.3.x, we are supposed to check_mem_region() and
    ** request_mem_region() before using a portion of PCI space. I
    ** haven't put that code in here yet.
    */
#error "Driver doesn't check_mem_region() or request_mem_region() yet."

    0&0&0;      /* They also changed how you find your PCI windows */
    unit_p->csr_phys_addr = 0&0&0&0;
    unit_p->mr_phys_addr = 0&0&0&0;
    unit_p->rr_phys_addr = 0&0&0&0;

#else   /* LINUX_VERSION_CODE < KERNEL_VERSION(2,3,0) */
    /* 
    ** Get the kernel physical address of each of the windows 
    */
    unit_p->csr_phys_addr = curr_dev_p->base_address[1] & PCI_BASE_ADDRESS_MEM_MASK;
    unit_p->mr_phys_addr = curr_dev_p->base_address[2] & PCI_BASE_ADDRESS_MEM_MASK;
    unit_p->rr_phys_addr = curr_dev_p->base_address[3] & PCI_BASE_ADDRESS_MEM_MASK;
#endif /*  LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,0) || \
           LINUX_VERSION_CODE >= KERNEL_VERSION(2,3,0) || \
           LINUX_VERSION_CODE <  KERNEL_VERSION(2,3,0) */

    /*
    ** Rather than read it from configuration space, get the
    ** interrupt line from the curr_dev_p structure.
    */
    unit_p->irq = curr_dev_p->irq;

    /*
    ** PCI windows are always allocated on a boundary of their size.
    **
    ** Since our smallest PCI Memory window is 64KBytes, the addresses
    ** will always be aligned properly. The following will catch the
    ** problem should these assumptions ever fail.
    */
#if     PAGE_SIZE > SIZE_64KB
#error  "Need to adjust ioremap call, page size exceeds PCI window size"
#endif  /* PAGE_SIZE < SIZE_64KB */


    phys_addr = unit_p->csr_phys_addr;
    BTK_ASSERT(0 == (phys_addr % PAGE_SIZE));
    unit_p->csr_p = (bt_pci_reg_t *) ioremap(phys_addr, PAGE_SIZE);
    if (NULL == unit_p->csr_p) {
        ret_val = -ENOMEM;
        WARN_STR("Could not map in Node I/O registers.\n");
        goto init_pci_config_end;
    }

    phys_addr = unit_p->mr_phys_addr;
    BTK_ASSERT(0 == (phys_addr % PAGE_SIZE));
    unit_p->mreg_p = (caddr_t) ioremap(phys_addr, SIZE_64KB);
    bt_trace_mreg_gp = (volatile bt_data32_t *) (unit_p->mreg_p + 0xfffc);
    if (NULL == unit_p->mreg_p) {
        ret_val = -ENOMEM;
        WARN_STR("Could not map in mapping registers.\n");

        /* Need to unmap Node I/O first */
        iounmap((void *) unit_p->csr_p);
        goto init_pci_config_end;
    }

    /*
    ** Invalidate the PIO and DMA to PCI map regs to prevent accidentally
    ** messing up Linux memory, which normally leads to a panic
    */
    btk_put_mreg_range(unit_p, 0, BT_MAX_SDMA_BIT, BT_LMREG_CABLE_2_PCI, BT_MREG_INVALID);
    btk_put_mreg_range(unit_p, 0, BT_MAX_SDMA_BIT, BT_LMREG_DMA_2_PCI, BT_MREG_INVALID);

    phys_addr = unit_p->rr_phys_addr;
    BTK_ASSERT(0 == (phys_addr % PAGE_SIZE));
    unit_p->rmem_p = (caddr_t) ioremap(phys_addr, 32 * SIZE_1MB);
    if (NULL == unit_p->rmem_p) {
        ret_val = -ENOMEM;
        WARN_STR("Could not map in remote memory window.\n");

        /* Need to unmap Node I/O and mapping registers */
        iounmap((void *) unit_p->csr_p);
        iounmap((void *) unit_p->mreg_p);
        goto init_pci_config_end;
    }

    /* Most of our cards do DMA, doesn't hurt to set it on 616 which isn't a master */
    pci_set_master(curr_dev_p);

init_pci_config_end:
    FEXIT(ret_val);
    return ret_val;
}

/******************************************************************************
**
**      Name:           cleanup_pci_config()
**
**      Purpose:        Unmaps the device in PCI space
**
**      Args:
**          unit_p      Pointer to unit structure
**
**      Modifies:
**          
**      Returns:
**          0           success
**         <0           -ERRNO
**
**      Notes:
**
*****************************************************************************/

static int cleanup_pci_config(
    bt_unit_t *unit_p
    )
{
    int ret_val = 0;

    FUNCTION("cleanup_pci_config");
    LOG_UNIT(unit_p);

    FENTRY;

    /* Unmap the device in the kernel virtual address space */
    iounmap((void *) unit_p->csr_p);
    iounmap((void *) unit_p->mreg_p);
    iounmap((void *) unit_p->rmem_p);

    FEXIT(ret_val);
    return ret_val;
}


/******************************************************************************
**
**      Function:   init_defaults()
**
**      Purpose:    Set up the default values for many parameters.
**
**      Args:       unit_p      Pointer to the unit structure being initialized
**
**      Returns:    0           Success
**                  else        error number
**
**
**      Notes:
**      There is no corresponding cleanup routine for this function.
**
******************************************************************************/

static int init_defaults (
    bt_unit_t * unit_p
    )
{
    FUNCTION("init_defaults");
    LOG_UNIT(unit_p);
    int ret_val = 0;

    FENTRY;

    /* 
    ** Set the various defaults in the unit structure. 
    */
    unit_p->reset_timer = DEFAULT_RESET_TIMER;
    unit_p->dma_timeout = DEFAULT_DMA_TIMEOUT;
    unit_p->dma_threshold = DEFAULT_DMA_THRESHOLD;
    unit_p->dma_poll_size = DEFAULT_DMA_POLL;
    unit_p->dma_buf_size = SIZE_64KB;

    FEXIT(ret_val);
    return ret_val;
}


/******************************************************************************
**
**      Function:   init_boot_parms()
**
**      Purpose:    Set up the default values for many parameters.
**
**      Args:       unit_p      Pointer to the unit structure being initialized
**
**      Returns:    0           Success
**                  else        error number
**
**
**      Notes:
**      There is no corresponding cleanup routine for this function.
**
**      The unit number must already be defined before calling this routine.
**
**      The only parameters that you can specify by unit are the remote bus
**      address used for Local Memory, and the local memory size.
**
******************************************************************************/

static int init_boot_parms (
    bt_unit_t * unit_p
    )
{
    FUNCTION("init_boot_parms");
    LOG_UNIT(unit_p);
    int ret_val = 0;
    short unit;

    FENTRY;

    /* 
    ** Set the various boot parameters in the unit structure
    **
    ** For Linux, we use module parameters to do this 
    */
    unit = unit_p->unit_number;
    if ((unit < 0) || (unit > BT_MAX_UNITS)) {
        /* Invalid unit number in structure, oops! */
        WARN_STR("Unit number not defined yet!\n");
        unit_p->lm_size = DEFAULT_LMEM_SIZE;
        unit_p->q_size = DEFAULT_Q_SIZE;
    } else {
        unit_p->lm_size = lm_size[unit];
        unit_p->q_size = icbr_q_size[unit];
    }

    FEXIT(ret_val);
    return ret_val;
}


/******************************************************************************
**
**      Function:   init_unit_array()
**
**      Purpose:    Adds the unit to bt_unit_array_gp[]
**
**      Args:       unit_p      Pointer to the unit structure being initialized
**
**      Returns:    0           Success
**                  else        error number
**
**
**      Notes:
**
******************************************************************************/

static int init_unit_array (
    bt_unit_t * unit_p
    )
{
    FUNCTION("init_unit_array");
    LOG_UNIT(unit_p);
    int ret_val = 0;
    short unit;

    FENTRY;

    /* 
    ** Kernel prevents open from occurring while we load the driver. 
    */

    /* 
    ** Search for an available spot in the unit array 
    */
    ret_val = -ENODEV; /* Assume there is no room */
    for (unit = 0; unit < BT_MAX_UNITS + 1; unit++) {
        if(NULL == bt_unit_array_gp[unit]) {
            unit_p->unit_number = unit;
            bt_unit_array_gp[unit] = unit_p;
            ret_val = 0;
            break;
        }
    }
    if (0 > ret_val) {
        WARN_STR("Failed to add unit.");
    }

    FEXIT(ret_val);
    return ret_val;
}

/******************************************************************************
**
**      Function:   cleanup_unit_array()
**
**      Purpose:    Removes the unit from the bt_unit_array_gp[]
**
**      Args:       unit_p      Pointer to the unit structure being initialized
**
**      Returns:    0           Success
**                  else        error number
**
**
**      Notes:
**
******************************************************************************/

static int cleanup_unit_array (
    bt_unit_t * unit_p
    )
{
    FUNCTION("cleanup_unit_array");
    LOG_UNIT(unit_p);
    int ret_val = 0;
    short unit;

    FENTRY;

    unit = unit_p->unit_number;

    /* 
    ** Do a reasonably complete consistency check 
    */
    if ( (unit < 0) || (unit > BT_MAX_UNITS) || 
         (unit_p != bt_unit_array_gp[unit])) {
        WARN_MSG((LOG_FMT "Invalid: unit = %d  unit_p = %p  bt_unit_array_gp[unit] = %p.\n",
                  LOG_ARG, unit, unit_p, bt_unit_array_gp[unit]));
        ret_val = -ENXIO;
    } else {
        bt_unit_array_gp[unit] = NULL;
    }

    FEXIT(ret_val);
    return ret_val;
}


/******************************************************************************
**
**      Function:   init_bit_maps()
**
**      Purpose:    Initializes any bit maps used for resource tracking.
**
**      Args:       unit_p      Pointer to the unit structure being initialized
**
**      Returns:    0           Success
**                  else        error number
**
**
**      Notes:
**
******************************************************************************/

static int init_bit_maps (
    bt_unit_t * unit_p
    )
{
    FUNCTION("init_bit_maps");
    LOG_UNIT(unit_p);
    int ret_val = 0;
    FENTRY;

    if (BT_SUCCESS != 
            btk_bit_init(unit_p, BT_MAX_MMAP_BIT, &unit_p->mmap_aval_p)) {
        ret_val = -ENOMEM; /* Assume memory problem */
        goto init_bit_maps_end;
    } 

    if (BT_SUCCESS != 
            btk_bit_init(unit_p, BT_MAX_SDMA_BIT, &unit_p->sdma_aval_p)) {
        ret_val = -ENOMEM; /* Assume memory problem */
        (void) btk_bit_fini(unit_p, unit_p->mmap_aval_p);
        goto init_bit_maps_end;
    }

    /* 
    ** Number of mapping registers per system page, usually one. 
    */
    unit_p->mr_page = BT_SYS_PAGE_SIZE / BT_PAGE_SIZE;

init_bit_maps_end:
    FEXIT(ret_val);
    return ret_val;
}

/******************************************************************************
**
**      Function:   cleanup_bit_maps()
**
**      Purpose:    Removes the unit from the bt_bit_maps[]
**
**      Args:       unit_p      Pointer to the unit structure being initialized
**
**      Returns:    0           Success
**                  else        error number
**
**
**      Notes:
**
******************************************************************************/

static int cleanup_bit_maps (
    bt_unit_t * unit_p
    )
{
    FUNCTION("cleanup_bit_maps");
    LOG_UNIT(unit_p);
    int ret_val = 0;

    FENTRY;

    (void) btk_bit_fini(unit_p, unit_p->mmap_aval_p);
    (void) btk_bit_fini(unit_p, unit_p->sdma_aval_p);

    FEXIT(ret_val);
    return ret_val;
}



/******************************************************************************
**
**      Function:   init_lm()
**
**      Purpose:    Initializes the local memory buffer
**
**      Args:       unit_p      Pointer to the unit structure being initialized
**
**      Returns:    0           Success
**                  else        error number
**
**
**      Notes:
**
******************************************************************************/

static int init_lm(
    bt_unit_t * unit_p
    )
{
    FUNCTION("init_lm");
    LOG_UNIT(unit_p);
    int ret_val = 0;

    bt_error_t      retval = BT_SUCCESS;
    unsigned int    need;
    unsigned int    start;
    unsigned int    inx;
    unsigned long   pci_addr;   /* Bus address of memory */

    bt_data32_t     mreg_value = 0; /* Value to program mapping register to */

    FENTRY;

    TRC_MSG(BT_TRC_CFG | BT_TRC_DETAIL, 
           (LOG_FMT "Unit %d lm_size = 0x%x\n", 
           LOG_ARG, unit_p->unit_number, unit_p->lm_size));

    /* 
    ** Initialize all the mapping registers into PCI as invalid 
    */
    if (unit_p->lm_size != 0) {
        if (0 != unit_p->lm_size % PAGE_SIZE) {
            WARN_STR("Local Memory device size must be multiple of system page size.\n");
            unit_p->lm_size += PAGE_SIZE;
            unit_p->lm_size -= (unit_p->lm_size % PAGE_SIZE);
        }

        /*
        ** Allocate the memory for local memory.
        **
        ** Since it is always a multiple of the page size, it is better to
        ** call vmalloc() for the buffer.
        */

#if     LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,0)
        unit_p->lm_kaddr = vmalloc_dma(unit_p->lm_size);
#else   /* LINUX_VERSION_CODE <  KERNEL_VERSION(2,4,0) */
        unit_p->lm_kaddr = vmalloc(unit_p->lm_size);
#endif  /* LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,0) \
        || LINUX_VERSION_CODE <  KERNEL_VERSION(2,4,0) */

        if (NULL == unit_p->lm_kaddr) {
            WARN_STR("Not enough memory to allocate local memory device.\n");
            unit_p->lm_size = 0;
            goto init_lm_end;
        }
        BTK_BZERO(unit_p->lm_kaddr, unit_p->lm_size);

        /*
        ** Set up sequential mapping registers to point to our local memory
        ** buffer in kernel space.
        */
        need = unit_p->lm_size / BT_PAGE_SIZE;
        retval = btk_bit_alloc(unit_p, unit_p->sdma_aval_p, need, 1, &start);
        if (retval != BT_SUCCESS) {
            vfree(unit_p->lm_kaddr);
            unit_p->lm_size = 0;
            WARN_STR("No open mapping regs for local memory device");
            goto init_lm_end;
        }
        unit_p->lm_start = start;
        unit_p->lm_need = need;

        /*
        ** Setup parts of mapping ram that do not change per for loop
        ** Address modifer does not need to be programmed for PCI accesses
        */
        btk_setup_mreg(unit_p, BT_AXSLM, &mreg_value, BT_OP_BIND);

        for (inx = 0; inx < need; inx++) {
        
            /* 
            ** Need more than just virt_to_bus() when it is vmalloc()ed 
            */
            pci_addr = (bt_data32_t) bt_kvm2bus((void *) (unit_p->lm_kaddr + inx * BT_PAGE_SIZE));

            if (0 == pci_addr) {
                WARN_STR("Local memory disabled: Could not link to PCI address.\n");
                vfree(unit_p->lm_kaddr);
                unit_p->lm_size = 0;
                btk_bit_free(unit_p, unit_p->sdma_aval_p, start, need);
                break;
            }
            mreg_value = (mreg_value & ~ BT_MREG_ADDR_MASK) | ((bt_data32_t) pci_addr & BT_MREG_ADDR_MASK);

            /*
            ** Handle the case where PAGE_SIZE > BT_PAGE_SIZE (size of each
            ** mapping register).
            */
            btk_put_mreg(unit_p, start + inx, BT_LMREG_CABLE_2_PCI, mreg_value);
            btk_put_mreg(unit_p, start + inx, BT_LMREG_DMA_2_PCI, mreg_value);
        }
    }

    /*
    ** Setup logical device info for the local memory device
    */
    if (unit_p->lm_size != 0) {
        unit_p->kern_addr[BT_AXSLM] = unit_p->lm_kaddr;
        unit_p->kern_length[BT_AXSLM] = unit_p->lm_size;
        unit_p->data_size[BT_AXSLM] = DATA_ANY_SIZ;
        unit_p->logstat[BT_AXSLM] |= (STAT_ONLINE | STAT_READ | STAT_WRITE | STAT_MMAP);
    } else {
        unit_p->lm_kaddr = NULL;
        unit_p->kern_addr[BT_AXSLM] = NULL;
        unit_p->kern_length[BT_AXSLM] = 0;
        unit_p->data_size[BT_AXSLM] = 0;
        unit_p->logstat[BT_AXSLM] = 0;
    }
        

init_lm_end:
    FEXIT(ret_val);
    return ret_val;
}

/******************************************************************************
**
**      Function:   cleanup_lm()
**
**      Purpose:    Releases resources used by local memory device
**
**      Args:       unit_p      Pointer to the unit structure being initialized
**
**      Returns:    0           Success
**                  else        error number
**
**
**      Notes:
**
******************************************************************************/

static int cleanup_lm(
    bt_unit_t * unit_p
    )
{
    FUNCTION("cleanup_lm");
    LOG_UNIT(unit_p);
    int ret_val = 0;

    FENTRY;

    /* 
    ** Release mapping registers used for local memory device 
    */
    if ((unit_p->lm_size != 0) &&
        (unit_p->lm_kaddr != NULL)) {
        btk_put_mreg_range(unit_p, unit_p->lm_start, unit_p->lm_need, BT_LMREG_CABLE_2_PCI, BT_MREG_INVALID);
        btk_put_mreg_range(unit_p, unit_p->lm_start, unit_p->lm_need, BT_LMREG_DMA_2_PCI, BT_MREG_INVALID);
        btk_bit_free(unit_p, unit_p->sdma_aval_p, unit_p->lm_start, unit_p->lm_need);
        vfree(unit_p->lm_kaddr);
        unit_p->lm_kaddr = NULL;
        unit_p->lm_start = 0;
        unit_p->lm_need = 0;
    }

    FEXIT(ret_val);
    return ret_val;
}


/******************************************************************************
**
**      Function:   init_cookies()
**
**      Purpose:    Initializes the cookies used for mutexs and events.
**
**      Args:       unit_p      Pointer to the unit structure being initialized
**
**      Returns:    0           Success
**                  else        error number
**
**
**      Notes:
**
******************************************************************************/

static int init_cookies (
    bt_unit_t * unit_p
    )
{
    FUNCTION("init_cookies");
    LOG_UNIT(unit_p);
    int ret_val = 0;

    FENTRY;

    unit_p->hirq_cookie = unit_p->irq;
    unit_p->sirq_cookie = 0;
    unit_p->task_cookie = 0;

    FEXIT(ret_val);
    return ret_val;
}


/******************************************************************************
**
**      Function:   init_events()
**
**      Purpose:    Initializes all the events used by the device driver
**
**      Args:       unit_p      Pointer to the unit structure being initialized
**
**      Returns:    0           Success
**                  else        error number
**
**
**      Notes:
**
******************************************************************************/

static int init_events(
    bt_unit_t * unit_p
    )
{
    FUNCTION("init_events");
    LOG_UNIT(unit_p);
    int ret_val = 0;

    FENTRY;

    /* DMA transfers */
    ret_val = btk_event_init(unit_p, &unit_p->dma_event, FALSE, unit_p->sirq_cookie);
    if (BT_SUCCESS != ret_val) {
        WARN_STR("Could not create semaphore for DMA.\n");
        goto init_events_end;
    }

init_events_end:
    FEXIT(ret_val);
    return ret_val;
}

/******************************************************************************
**
**      Function:   cleanup_events()
**
**      Purpose:    Removes all the events used by the device driver.
**
**      Args:       unit_p      Pointer to the unit structure being initialized
**
**      Returns:    0           Success
**                  else        error number
**
**
**      Notes:
**
******************************************************************************/

static int cleanup_events (
    bt_unit_t * unit_p
    )
{
    FUNCTION("cleanup_events");
    LOG_UNIT(unit_p);
    int ret_val = 0;

    FENTRY;

    btk_event_fini(unit_p, &unit_p->dma_event);

    FEXIT(ret_val);
    return ret_val;
}


/******************************************************************************
**
**      Function:   init_mutexs()
**
**      Purpose:    Initializes a
**
**      Args:       unit_p      Pointer to the unit structure being initialized
**
**      Returns:    0           Success
**                  else        error number
**
**
**      Notes:
**
******************************************************************************/

static int init_mutexs (
    bt_unit_t * unit_p
    )
{
    FUNCTION("init_mutexs");
    LOG_UNIT(unit_p);
    int ret_val = 0;

    FENTRY;

    btk_mutex_init(unit_p, &(unit_p->mreg_mutex), 0);
    btk_mutex_init(unit_p, &(unit_p->open_mutex), 0);
    btk_mutex_init(unit_p, &(unit_p->dma_mutex), 0);
    btk_mutex_init(unit_p, &(unit_p->llist_mutex), 0);
    btk_rwlock_init(unit_p, &(unit_p->hw_rwlock));
    btk_mutex_init(unit_p, &(unit_p->isr_lock), unit_p->hirq_cookie);

    FEXIT(ret_val);
    return ret_val;
}

/******************************************************************************
**
**      Function:   cleanup_mutexs()
**
**      Purpose:    Removes 
**
**      Args:       unit_p      Pointer to the unit structure being initialized
**
**      Returns:    0           Success
**                  else        error number
**
**
**      Notes:
**
******************************************************************************/

static int cleanup_mutexs (
    bt_unit_t * unit_p
    )
{
    FUNCTION("cleanup_mutexs");
    LOG_UNIT(unit_p);
    int ret_val = 0;

    FENTRY;

    btk_mutex_fini(unit_p, &(unit_p->isr_lock));
    btk_rwlock_fini(unit_p, &(unit_p->hw_rwlock));
    btk_mutex_fini(unit_p, &(unit_p->llist_mutex));
    btk_mutex_fini(unit_p, &(unit_p->dma_mutex));
    btk_mutex_fini(unit_p, &(unit_p->open_mutex));
    btk_mutex_fini(unit_p, &(unit_p->mreg_mutex));

    FEXIT(ret_val);
    return ret_val;
}


/******************************************************************************
**
**      Function:   init_lists()
**
**      Purpose:    Initializes all the lists used by the device driver
**
**      Args:       unit_p      Pointer to the unit structure being initialized
**
**      Returns:    0           Success
**                  else        error number
**
**
**      Notes:
**
******************************************************************************/

static int init_lists(
    bt_unit_t * unit_p
    )
{
    FUNCTION("init_lists");
    LOG_UNIT(unit_p);
    int ret_val = 0;

    FENTRY;

    /*
    ** Lists for user interrupt handlers
    */
    btk_llist_init(&unit_p->qh_err_fn);
    btk_llist_init(&unit_p->qh_prg_fn);
    btk_llist_init(&unit_p->qh_iack_fn);
    btk_llist_init(&unit_p->qh_mmap_requests);
    btk_llist_init(&unit_p->qh_bind_requests);
    btk_llist_init(&unit_p->icbr_thread_list);

    FEXIT(ret_val);
    return ret_val;
}

/******************************************************************************
**
**      Function:   cleanup_lists()
**
**      Purpose:    Removes all the lists used by the device driver.
**
**      Args:       unit_p      Pointer to the unit structure being initialized
**
**      Returns:    0           Success
**                  else        error number
**
**
**      Notes:
**
******************************************************************************/

static int cleanup_lists (
    bt_unit_t * unit_p
    )
{
    FUNCTION("cleanup_lists");
    LOG_UNIT(unit_p);
    int ret_val = 0;

    FENTRY;

    /*
    ** No cleanup required on llists for user interrupt handlers
    */

    FEXIT(ret_val);
    return ret_val;
}

/******************************************************************************
**
**      Function:   init_irq_q()
**
**      Purpose:    Initializes interrupt queue
**
**      Args:       unit_p      Pointer to the unit structure being initialized
**
**      Returns:    0           Success
**                  else        error number
**
**
**      Notes:
**
******************************************************************************/

static int init_irq_q(
    bt_unit_t * unit_p
    )
{
    FUNCTION("init_irq_q");
    LOG_UNIT(unit_p);
    int ret_val = 0;

    FENTRY;

    ret_val = (int) btk_irq_qs_init(unit_p, unit_p->q_size);

    FEXIT(ret_val);
    return ret_val;
}

/******************************************************************************
**
**      Function:   cleanup_irq_q()
**
**      Purpose:    Removes the interrupt queue used by the driver.
**
**      Args:       unit_p      Pointer to the unit structure being initialized
**
**      Returns:    0           Success
**                  else        error number
**
**
**      Notes:
**
******************************************************************************/

static int cleanup_irq_q (
    bt_unit_t * unit_p
    )
{
    FUNCTION("cleanup_irq_q");
    LOG_UNIT(unit_p);
    int ret_val = 0;

    FENTRY;

    btk_irq_qs_fini(unit_p, unit_p->q_size);

    FEXIT(ret_val);
    return ret_val;
}


/******************************************************************************
**
**      Function:   init_isr()
**
**      Purpose:    Initializes the interrupt service routine
**
**      Args:       unit_p      Pointer to the unit structure being initialized
**
**      Returns:    0           Success
**                  else        error number
**
**
**      Notes:
**
******************************************************************************/

static int init_isr (
    bt_unit_t * unit_p
    )
{
    FUNCTION("init_isr");
    LOG_UNIT(unit_p);
    int ret_val = 0;

    FENTRY;

    /* 
    ** Register the ISR 
    */
    ret_val = request_irq(unit_p->irq, btk_isr, SA_SHIRQ, bt_name_gp, (void *) unit_p);
    if (0 > ret_val) {
        WARN_MSG((LOG_FMT "Could not register shared interrupt on level %d.\n",
            LOG_ARG, unit_p->irq));
        goto init_isr_end;
    }

init_isr_end:
    FEXIT(ret_val);
    return ret_val;
}

/******************************************************************************
**
**      Function:   cleanup_isr()
**
**      Purpose:    Removes the interrupt service routine
**
**      Args:       unit_p      Pointer to the unit structure being initialized
**
**      Returns:    0           Success
**                  else        error number
**
**
**      Notes:
**
******************************************************************************/

static int cleanup_isr (
    bt_unit_t * unit_p
    )
{
    FUNCTION("cleanup_isr");
    LOG_UNIT(unit_p);
    int ret_val = 0;

    FENTRY;

    /* 
    ** Turn off interrupts on the card  & Unregister our handler
    */
    btk_put_io(unit_p, LOC_INT_CTRL, LIC_DIS_INTR);
    free_irq(unit_p->irq, (void *) unit_p);

    FEXIT(ret_val);
    return ret_val;
}


/******************************************************************************
**
**      Function:   init_dma()
**
**      Purpose:    Initializes driver DMA structures.
**
**      Args:       unit_p      Pointer to the unit structure being initialized
**
**      Returns:    0           Success
**                  else        error number
**
**
**      Notes:
**
******************************************************************************/

static int init_dma (
    bt_unit_t * unit_p
    )
{
    FUNCTION("init_dma");
    LOG_UNIT(unit_p);
    int ret_val = 0;

    FENTRY;

    /* 
    ** Need to set BT_DMA_LOCAL if the local card can do DMA 
    */
    switch (unit_p->loc_id) {
      case BT_PN_PCI_DMA:
      case BT_PN_PCI_FIBER:
        unit_p->bt_status |= BT_DMA_LOCAL;
        break;
        
      case BT_PN_PCI_FIBER_D64:
        unit_p->bt_status |= BT_DMA_LOCAL;
        SET_BIT(unit_p->bt_status, BT_NEXT_GEN);
        break;

      case BT_PN_PCI:
      case BT_PN_PCI_NODMA:
        /* These don't have DMA, so don't set the bit */
        break;

      default:
        /* Unrecognized, so don't set the bit */
        break;
    }
    SET_BIT(unit_p->bt_status, BT_DMA_BLOCK);

    /*
    ** Not using kiobuf, so we need to allocated a kernel buffer to move
    ** the data in/out of. Since it is a large buffer that is a multiple 
    ** of the page size, it is better to call vmalloc() for the buffer.
    */

#if     LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,0)
    unit_p->dma_buf_p = vmalloc_dma(unit_p->dma_buf_size);
#else   /* LINUX_VERSION_CODE <  KERNEL_VERSION(2,4,0) */
    unit_p->dma_buf_p = vmalloc(unit_p->dma_buf_size);
#endif  /* LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,0) \
        || LINUX_VERSION_CODE <  KERNEL_VERSION(2,4,0) */

    if (NULL == unit_p->dma_buf_p) {
        WARN_STR("Could not allocated buffer for data transfer.\n");
        ret_val = -ENOMEM;
    }

    FEXIT(ret_val);
    return ret_val;
}

/******************************************************************************
**
**      Function:   cleanup_dma()
**
**      Purpose:    Removes 
**
**      Args:       unit_p      Pointer to the unit structure being initialized
**
**      Returns:    0           Success
**                  else        error number
**
**
**      Notes:
**
******************************************************************************/

static int cleanup_dma (
    bt_unit_t * unit_p
    )
{
    FUNCTION("cleanup_dma");
    LOG_UNIT(unit_p);
    int ret_val = 0;

    FENTRY;

    vfree(unit_p->dma_buf_p);

    FEXIT(ret_val);
    return ret_val;
}


/******************************************************************************
**
**      Function:   init_ldev()
**
**      Purpose:    Initializes the logical device fields
**
**      Args:       unit_p      Pointer to the unit structure being initialized
**
**      Returns:    0           Success
**                  else        error number
**
**
**      Notes:
**
******************************************************************************/

static int init_ldev (
    bt_unit_t * unit_p
    )
{
    FUNCTION("init_ldev");
    LOG_UNIT(unit_p);
    int ret_val = 0;

    FENTRY;

    /*
    ** Setup one time logical device info
    ** Note logical dev info that can change when cable is pulled
    ** is setup in btk_setup()
    */
    /*
    **  Setup logical unit_p->dev_vertex information for node registers
    */
    unit_p->kern_addr[BT_AXSIO] = (caddr_t) unit_p->csr_p;
    unit_p->kern_length[BT_AXSIO] = MAX_IOREGS;
    unit_p->data_size[BT_AXSIO] = BT_WIDTH_D32;
    unit_p->logstat[BT_AXSIO] |= STAT_ONLINE;

    /*
    **  Setup logical unit_p->dev_vertex information for remote A16 space
    */
    unit_p->kern_addr[BT_AXSRI] = unit_p->rmem_p;
    unit_p->kern_length[BT_AXSRI] = 0;
    unit_p->pio_addr_mod[BT_AXSRI] = BT_AMOD_A16;
    unit_p->dma_addr_mod[BT_AXSRI] = BT_AMOD_A16;
    unit_p->mmap_addr_mod[BT_AXSRI] = BT_AMOD_A16;
    unit_p->data_size[BT_AXSRI] = BT_WIDTH_D32;
    
    /*
    **  Setup logical unit_p->dev_vertex information for remote A24
    */
    unit_p->kern_addr[BT_AXS24] = unit_p->rmem_p;
    unit_p->kern_length[BT_AXS24] = 0;
    unit_p->pio_addr_mod[BT_AXS24] = BT_AMOD_A24;
    unit_p->dma_addr_mod[BT_AXS24] = BT_AMOD_A24;
    unit_p->mmap_addr_mod[BT_AXS24] = BT_AMOD_A24;
    unit_p->data_size[BT_AXS24] = BT_WIDTH_D32;

    /*
    **  Setup logical unit_p->dev_vertex information for remote A23 lower half
    */
    unit_p->kern_addr[BT_AXSRR] = unit_p->rmem_p;
    unit_p->kern_length[BT_AXSRR] = 0;
    unit_p->pio_addr_mod[BT_AXSRR] = BT_AMOD_A32;
    unit_p->dma_addr_mod[BT_AXSRR] = BT_AMOD_A32;
    unit_p->mmap_addr_mod[BT_AXSRR] = BT_AMOD_A32;
    unit_p->data_size[BT_AXSRR] = BT_WIDTH_D32;

    /*
    **  Setup logical unit_p->dev_vertex information for remote A23 upper half
    */
    unit_p->kern_addr[BT_AXSRE] = unit_p->rmem_p;
    unit_p->kern_length[BT_AXSRE] = 0;
    unit_p->pio_addr_mod[BT_AXSRE] = BT_AMOD_A32;
    unit_p->dma_addr_mod[BT_AXSRE] = BT_AMOD_A32;
    unit_p->mmap_addr_mod[BT_AXSRE] = BT_AMOD_A32;
    unit_p->data_size[BT_AXSRE] = BT_WIDTH_D32;
    /*
    ** setup logical unit_p->dev_vertex information for geographical
    ** addressing
    */

    unit_p->kern_addr[BT_AXSGEO]   = unit_p->rmem_p;
    unit_p->kern_length[BT_AXSGEO] =  0;
    unit_p->pio_addr_mod[BT_AXSGEO] = BT_AMOD_GEO;
    unit_p->dma_addr_mod[BT_AXSGEO] = BT_AMOD_GEO;
    unit_p->mmap_addr_mod[BT_AXSGEO]= BT_AMOD_GEO;
    unit_p->data_size[BT_AXSGEO]    = BT_WIDTH_D32;
    /*
    ** setup logical unit_p->dev_vertex information for multicast control
    ** addressing
    */

    unit_p->kern_addr[BT_AXSMCCTL]   = unit_p->rmem_p;
    unit_p->kern_length[BT_AXSMCCTL] =  0;
    unit_p->pio_addr_mod[BT_AXSMCCTL] = BT_AMOD_MCCTL;
    unit_p->dma_addr_mod[BT_AXSMCCTL] = BT_AMOD_MCCTL;
    unit_p->mmap_addr_mod[BT_AXSMCCTL]= BT_AMOD_MCCTL;
    unit_p->data_size[BT_AXSMCCTL]    = BT_WIDTH_D32;
    /*
    ** setup logical unit_p->dev_vertex information for chained block transfer
    ** addressing
    */

    unit_p->kern_addr[BT_AXSCBLT]   = unit_p->rmem_p;
    unit_p->kern_length[BT_AXSCBLT] =  0;
    unit_p->pio_addr_mod[BT_AXSCBLT] = BT_AMOD_CBLT;
    unit_p->dma_addr_mod[BT_AXSCBLT] = BT_AMOD_CBLT;
    unit_p->mmap_addr_mod[BT_AXSCBLT]= BT_AMOD_CBLT;
    unit_p->data_size[BT_AXSCBLT]    = BT_WIDTH_D32;

    /*
    **  Setup logical unit_p->dev_vertex information for local dual port
    */
    unit_p->data_size[BT_AXSLDP] = BT_WIDTH_D32;

    /*
    **  Setup logical unit_p->dev_vertex information for remote dual port
    */
    unit_p->data_size[BT_AXSRDP] = BT_WIDTH_D32;




    FEXIT(ret_val);
    return ret_val;
}


/******************************************************************************
**
**      Function:   init_card()
**
**      Purpose:    Initializes the adapter card
**
**      Args:       unit_p      Pointer to the unit structure being initialized
**
**      Returns:    0           Success
**                  else        error number
**
**
**      Notes:
**
******************************************************************************/

static int init_card (
    bt_unit_t * unit_p
    )
{
    FUNCTION("init_card");
    LOG_UNIT(unit_p);
    int ret_val = 0;

    FENTRY;

    btk_setup(unit_p);

    FEXIT(ret_val);
    return ret_val;
}


/******************************************************************************
**
**      Function:   init_swapping()
**
**      Purpose:    Initializes default swapping
**
**      Args:       unit_p      Pointer to the unit structure being initialized
**
**      Returns:    0           Success
**                  else        error number
**
**
**      Notes:
**
******************************************************************************/

static int init_swapping (
    bt_unit_t * unit_p
    )
{
    FUNCTION("init_swapping");
    LOG_UNIT(unit_p);
    
    int     inx, ret_val = 0;

    FENTRY;
    
    /*
    **  Setup swapping bits for logical unit_p->dev_vertexs depending on what 
    **  type of Adaptor we have
    */
    for (inx = 0; inx < BT_MAX_AXSTYPS; inx++) {
        switch (unit_p->rem_id) {
          case BT_PN_PCI_DMA:
          case BT_PN_PCI_NODMA:
          case BT_PN_PCI_FIBER:
          case BT_PN_PCI_FIBER_D64:
            unit_p->swap_bits[inx] = BT_SWAP_NONE;
            break;
	  case BT_PN_VME_NOINC:
          case BT_PN_VME_NODMA:
          case BT_PN_VME_SDMA:
      /*  case BT_PN_VME_SDMA_64:   BT_PN_VME_SDMA_64 == BT_PN_VME_FIBER_D64 */
	  case BT_PN_VME2_DMA:
	  case BT_PN_VME_DMA:
	  case BT_PN_VME:
      /*  case BT_PN_VME2:	    BT_PN_VME2 == BT_PN_VME_NODMA above.     */
          case BT_PN_VME_FIBER:
	  case BT_PN_VME_A24:
	  case BT_PN_VME_PCI:	/* EXPN may not be supported by this driver. */
	  case BT_PN_VME64:	/* 2866 may not be supported by this driver. */
	  case BT_PN_VME_NBDG:	/* NBDG may not be supported by this driver. */
          case BT_PN_VME_FIBER_D64:
            unit_p->swap_bits[inx] = BT_SWAP_NONE;
            break;
	  case 0:
	    ret_val = -1;
	    break;
          case BT_PN_QBUS:
            unit_p->swap_bits[inx] = BT_SWAP_QBUS;
            break;
          case BT_PN_MB:
            unit_p->swap_bits[inx] = BT_SWAP_MULTIBUS;
            break;
          default:
            /*
            ** For next gen assume PCI to PCI
            */
            if (unit_p->loc_id == BT_PN_PCI_FIBER_D64) {
              unit_p->swap_bits[inx] = BT_SWAP_NONE;
            } else {
              unit_p->swap_bits[inx] = BT_SWAP_NONE;
            }
            break;
        }
    }

    FEXIT(ret_val);
    return ret_val;
}
#if 0

/******************************************************************************
**
**      Function:   init_xxx()
**
**      Purpose:    Initializes a
**
**      Args:       unit_p      Pointer to the unit structure being initialized
**
**      Returns:    0           Success
**                  else        error number
**
**
**      Notes:
**
******************************************************************************/

static int init_xxx (
    bt_unit_t * unit_p
    )
{
    FUNCTION("init_xxx");
    LOG_UNIT(unit_p);
    int ret_val = 0;

    FENTRY;


init_xxx_end:
    FEXIT(ret_val);
    return ret_val;
}

/******************************************************************************
**
**      Function:   cleanup_xxx()
**
**      Purpose:    Removes 
**
**      Args:       unit_p      Pointer to the unit structure being initialized
**
**      Returns:    0           Success
**                  else        error number
**
**
**      Notes:
**
******************************************************************************/

static int cleanup_xxx (
    bt_unit_t * unit_p
    )
{
    FUNCTION("cleanup_xxx");
    LOG_UNIT(unit_p);
    int ret_val = 0;

    FENTRY;

    FEXIT(ret_val);
    return ret_val;
}

#endif /* 0 */
