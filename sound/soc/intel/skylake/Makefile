snd-soc-skl-objs := skl.o skl-sdw-pcm.o skl-nhlt.o skl-messages.o \
skl-topology.o skl-sdw-pcm.o skl-compress.o skl-probe.o

ifdef CONFIG_DEBUG_FS
  snd-soc-skl-objs += skl-debug.o
endif
ifdef CONFIG_SND_SOC_INTEL_GP_GEN2HONDA_MACH
 snd-soc-skl-objs += skl-gen2Honda-pcm.o
else
 snd-soc-skl-objs += skl-pcm.o
endif
obj-$(CONFIG_SND_SOC_INTEL_SKYLAKE) += snd-soc-skl.o

# Skylake IPC Support
snd-soc-skl-ipc-objs := skl-sst-ipc.o skl-sst-dsp.o cnl-sst-dsp.o \
		skl-sst-cldma.o skl-sst.o bxt-sst.o cnl-sst.o \
		skl-sst-utils.o skl-fwlog.o

 snd-soc-skl-ipc-objs += cnl-acpi.o

obj-$(CONFIG_SND_SOC_INTEL_SKYLAKE) += snd-soc-skl-ipc.o

