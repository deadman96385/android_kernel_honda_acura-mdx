# For include/trace/define_trace.h to include trace.h
CFLAGS_regmap.o := -I$(src)

obj-$(CONFIG_REGMAP) += regmap.o regcache.o
obj-$(CONFIG_REGMAP) += regcache-rbtree.o regcache-lzo.o regcache-flat.o
obj-$(CONFIG_DEBUG_FS) += regmap-debugfs.o
obj-$(CONFIG_REGMAP_AC97) += regmap-ac97.o
obj-$(CONFIG_REGMAP_I2C) += regmap-i2c.o
obj-$(CONFIG_REGMAP_SPI) += regmap-spi.o
obj-$(CONFIG_REGMAP_SPMI) += regmap-spmi.o
obj-$(CONFIG_REGMAP_MMIO) += regmap-mmio.o
obj-$(CONFIG_REGMAP_IRQ) += regmap-irq.o
obj-$(CONFIG_REGMAP_SDW) += regmap-sdw.o
