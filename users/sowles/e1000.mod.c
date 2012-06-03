#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);

struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
 .name = KBUILD_MODNAME,
 .init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
 .exit = cleanup_module,
#endif
 .arch = MODULE_ARCH_INIT,
};

static const struct modversion_info ____versions[]
__used
__attribute__((section("__versions"))) = {
	{ 0xdd106cb3, "module_layout" },
	{ 0xf0dc837, "__kmap_atomic" },
	{ 0x3ce4ca6f, "disable_irq" },
	{ 0xf24217d9, "netdev_info" },
	{ 0xe5a422d3, "kmalloc_caches" },
	{ 0x12da5bb2, "__kmalloc" },
	{ 0x566f042f, "pcix_set_mmrbc" },
	{ 0xf9a482f9, "msleep" },
	{ 0x68e2f221, "_raw_spin_unlock" },
	{ 0x67c5a03d, "debugfs_create_dir" },
	{ 0x70f19da0, "mem_map" },
	{ 0x3ec8886f, "param_ops_int" },
	{ 0x9cd40a58, "page_address" },
	{ 0x89a7b303, "dev_set_drvdata" },
	{ 0x7ce0601b, "dma_set_mask" },
	{ 0x73b320ae, "napi_complete" },
	{ 0x84e6ed8b, "pci_disable_device" },
	{ 0x8d4bd76f, "netif_carrier_on" },
	{ 0x8949858b, "schedule_work" },
	{ 0xc0a3d105, "find_next_bit" },
	{ 0x81416de0, "netif_carrier_off" },
	{ 0x4205ad24, "cancel_work_sync" },
	{ 0x13125148, "x86_dma_fallback_dev" },
	{ 0xeae3dfd6, "__const_udelay" },
	{ 0xfb0e29f, "init_timer_key" },
	{ 0x6baae653, "cancel_delayed_work_sync" },
	{ 0x95fbe0a4, "mutex_unlock" },
	{ 0x5857b225, "ioread16_rep" },
	{ 0x999e8297, "vfree" },
	{ 0x47c7b0d2, "cpu_number" },
	{ 0x8cc79cab, "iowrite16_rep" },
	{ 0x7e65a7f0, "__alloc_pages_nodemask" },
	{ 0xc499ae1e, "kstrdup" },
	{ 0x7d11c268, "jiffies" },
	{ 0x5028d5e6, "pcix_get_mmrbc" },
	{ 0xe22a5eaa, "skb_trim" },
	{ 0xe2d5255a, "strcmp" },
	{ 0xf92e72e3, "__netdev_alloc_skb" },
	{ 0x99f6ffcd, "__pskb_pull_tail" },
	{ 0x73ba88d4, "pci_set_master" },
	{ 0x2bc95bd4, "memset" },
	{ 0xae1cb471, "pci_restore_state" },
	{ 0xd941fb89, "dev_err" },
	{ 0xf97456ea, "_raw_spin_unlock_irqrestore" },
	{ 0xe70213c7, "__mutex_init" },
	{ 0x50eedeb8, "printk" },
	{ 0xf64c621e, "free_netdev" },
	{ 0xb6ed1e53, "strncpy" },
	{ 0x3e096d, "register_netdev" },
	{ 0xb4390f9a, "mcount" },
	{ 0x8597bc2f, "debugfs_remove" },
	{ 0x73e20c1c, "strlcpy" },
	{ 0x16305289, "warn_slowpath_null" },
	{ 0x231dde1e, "__pci_enable_wake" },
	{ 0x82b267f8, "mutex_lock" },
	{ 0xed93f29e, "__kunmap_atomic" },
	{ 0x98779947, "dev_close" },
	{ 0x39c6d058, "netif_napi_add" },
	{ 0xf33c230c, "dma_release_from_coherent" },
	{ 0x2072ee9b, "request_threaded_irq" },
	{ 0x812454e4, "dev_kfree_skb_any" },
	{ 0x847bd456, "contig_page_data" },
	{ 0x87d76bf2, "dma_alloc_from_coherent" },
	{ 0xccfcb5c1, "dev_open" },
	{ 0xe523ad75, "synchronize_irq" },
	{ 0xb7db8262, "pci_set_mwi" },
	{ 0x4059792f, "print_hex_dump" },
	{ 0xe27e79, "pci_select_bars" },
	{ 0x3ff62317, "local_bh_disable" },
	{ 0x5bedb6cd, "netif_device_attach" },
	{ 0x62cd3bd7, "napi_gro_receive" },
	{ 0xd7ef3d2f, "_dev_info" },
	{ 0x40a9b349, "vzalloc" },
	{ 0x8ff4079b, "pv_irq_ops" },
	{ 0x3bde6076, "netif_device_detach" },
	{ 0xc1dc7165, "__alloc_skb" },
	{ 0x3af98f9e, "ioremap_nocache" },
	{ 0x1f832567, "pci_bus_read_config_word" },
	{ 0xfd5e2957, "__napi_schedule" },
	{ 0x77edf722, "schedule_delayed_work" },
	{ 0x6b2dc060, "dump_stack" },
	{ 0x799aca4, "local_bh_enable" },
	{ 0x85a3be50, "eth_type_trans" },
	{ 0xb7fdaa3f, "pskb_expand_head" },
	{ 0x6ad9c0a0, "netdev_err" },
	{ 0x29f7fc0b, "pci_unregister_driver" },
	{ 0xcc5005fe, "msleep_interruptible" },
	{ 0x9e9d2695, "kmem_cache_alloc_trace" },
	{ 0x67f7403e, "_raw_spin_lock" },
	{ 0xed072dc4, "pci_ioremap_bar" },
	{ 0x21fb443e, "_raw_spin_lock_irqsave" },
	{ 0xf6ebc03b, "net_ratelimit" },
	{ 0xed9c81ad, "pci_set_power_state" },
	{ 0x420c671, "netdev_warn" },
	{ 0x56d4b2d5, "eth_validate_addr" },
	{ 0x2087984b, "pci_clear_mwi" },
	{ 0xfcec0987, "enable_irq" },
	{ 0x37a0cba, "kfree" },
	{ 0x2e60bace, "memcpy" },
	{ 0x47ff36bf, "___pskb_trim" },
	{ 0x9db437e7, "debugfs_create_blob" },
	{ 0xf59f197, "param_array_ops" },
	{ 0x1daa9167, "dma_supported" },
	{ 0xedc03953, "iounmap" },
	{ 0x3ba7a417, "pci_prepare_to_sleep" },
	{ 0xb0de54b9, "__pci_register_driver" },
	{ 0x2288378f, "system_state" },
	{ 0xa445ab2d, "put_page" },
	{ 0xb352177e, "find_first_bit" },
	{ 0x2cc51ec8, "dev_warn" },
	{ 0x3a15925f, "unregister_netdev" },
	{ 0xdbedbf7e, "__netif_schedule" },
	{ 0x4c8e57be, "consume_skb" },
	{ 0xa37c592a, "pci_enable_device_mem" },
	{ 0x85670f1d, "rtnl_is_locked" },
	{ 0x6dabcfe0, "skb_put" },
	{ 0xce7026ef, "pci_enable_device" },
	{ 0x1fb345d6, "pci_wake_from_d3" },
	{ 0xb2d08e27, "pci_release_selected_regions" },
	{ 0x9328561f, "pci_request_selected_regions" },
	{ 0xc3fe87c8, "param_ops_uint" },
	{ 0x5c3ec0cf, "dev_get_drvdata" },
	{ 0x9e7d6bd0, "__udelay" },
	{ 0x3a044fd8, "dma_ops" },
	{ 0xd0034574, "device_set_wakeup_enable" },
	{ 0xc2d711e1, "krealloc" },
	{ 0xf20dabd8, "free_irq" },
	{ 0xabef3552, "pci_save_state" },
	{ 0xe914e41e, "strcpy" },
	{ 0x8bda25ba, "alloc_etherdev_mqs" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";

MODULE_ALIAS("pci:v00008086d00001000sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d00001001sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d00001004sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d00001008sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d00001009sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d0000100Csv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d0000100Dsv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d0000100Esv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d0000100Fsv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d00001010sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d00001011sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d00001012sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d00001013sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d00001014sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d00001015sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d00001016sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d00001017sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d00001018sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d00001019sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d0000101Asv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d0000101Dsv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d0000101Esv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d00001026sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d00001027sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d00001028sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d00001075sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d00001076sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d00001077sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d00001078sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d00001079sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d0000107Asv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d0000107Bsv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d0000107Csv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d0000108Asv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d00001099sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d000010B5sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d00002E6Esv*sd*bc*sc*i*");

MODULE_INFO(srcversion, "C2B9EBB1B2EEC2845C0F9FD");
