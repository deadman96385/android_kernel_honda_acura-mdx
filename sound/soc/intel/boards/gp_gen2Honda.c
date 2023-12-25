/*
 * Intel Broxton-P I2S Machine Driver for IVI reference platform
 * Copyright (c) 2017, Intel Corporation.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 */

#include <linux/module.h>
#include <linux/platform_device.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/soc.h>
#include <sound/pcm_params.h>

//static const struct snd_kcontrol_new gp_gen2Honda_controls[] = {
//	SOC_DAPM_PIN_SWITCH("Speaker"),
//};

static const struct snd_soc_dapm_widget gp_gen2Honda_widgets[] = {
  SND_SOC_DAPM_HP("Honda_I2S2_SYNC2_PB", NULL),
  SND_SOC_DAPM_HP("Honda_I2S2_SYNC3_PB", NULL),
  SND_SOC_DAPM_HP("Honda_I2S2_SYNC4_PB", NULL),
  SND_SOC_DAPM_HP("Honda_I2S4_SYNC1_PB", NULL),
  SND_SOC_DAPM_HP("Honda_I2S4_SYNC5_PB", NULL),
  SND_SOC_DAPM_MIC("Honda_I2S5_SYNC0_CP", NULL),
  SND_SOC_DAPM_HP("Honda_I2S1_BT_PB", NULL),
  SND_SOC_DAPM_MIC("Honda_I2S1_BT_CP", NULL),
  SND_SOC_DAPM_HP("Honda_I2S3_SYNC3_PB", NULL),
};

static const struct snd_soc_dapm_route gp_gen2Honda_map[] = {

	/* Speaker BE connections */

  { "Honda_I2S2_SYNC2_PB", NULL, "ssp1 Tx"},
	{ "ssp1 Tx", NULL, "Honda_I2S2_out"},

  { "Honda_I2S2_SYNC3_PB", NULL, "ssp1-b Tx"},
	{ "ssp1-b Tx", NULL, "Honda_I2S2_out1"},

  { "Honda_I2S2_SYNC4_PB", NULL, "ssp1-c Tx"},
	{ "ssp1-c Tx", NULL, "Honda_I2S2_out2"},

  { "Honda_I2S4_SYNC1_PB", NULL, "ssp3 Tx"},
	{ "ssp3 Tx", NULL, "Honda_I2S4_out"},

  { "Honda_I2S4_SYNC5_PB", NULL, "ssp3-b Tx"},
	{ "ssp3-b Tx", NULL, "Honda_I2S4_out1"},

  { "Honda_I2S5_in", NULL, "ssp4 Rx"},
	{ "ssp4 Rx", NULL, "Honda_I2S5_SYNC0_CP"},

  { "Honda_I2S1_BT_PB", NULL, "ssp0 Tx"},
	{ "ssp0 Tx", NULL, "Honda_I2S1_out"},

  { "Honda_I2S1_in", NULL, "ssp0 Rx"},
	{ "ssp0 Rx", NULL, "Honda_I2S1_BT_CP"},

  { "Honda_I2S3_SYNC3_PB", NULL, "ssp2 Tx"},
        { "ssp2 Tx", NULL, "Honda_I2S3_out"},

};

static int gp_gen2Honda_ssp0_fixup(struct snd_soc_pcm_runtime *rtd,
				struct snd_pcm_hw_params *params)
{
    struct snd_mask *fmt = hw_param_mask(params, SNDRV_PCM_HW_PARAM_FORMAT);
    struct snd_interval *rate = hw_param_interval(params, SNDRV_PCM_HW_PARAM_RATE);
    rate->min = rate->max = 16000;
    /* set SSP to 32 bit */
    snd_mask_none(fmt);
    snd_mask_set(fmt, SNDRV_PCM_FORMAT_S16_LE);
    return 0;
}

/* broxton digital audio interface glue - connects codec <--> CPU */
static struct snd_soc_dai_link gp_gen2Honda_dais[] = {
	/* Front End DAI links */
	{
		.name = "Honda Playback Sync1 Port",
		.stream_name = "Honda_I2S4_SYNC1", /*mapped to topology name*/
		.cpu_dai_name = "I2S4 Sync1 Pb Pin",/*needs to ne same as .name in skl-pcm.c*/
		.platform_name = "0000:00:0e.0",
		.nonatomic = 1,
		.dynamic = 1,
		.codec_name = "snd-soc-dummy",
		.codec_dai_name = "snd-soc-dummy-dai",
		.trigger = {
			SND_SOC_DPCM_TRIGGER_POST,
			SND_SOC_DPCM_TRIGGER_POST
		},
		.dpcm_playback = 1,
	},
    {
		.name = "Honda Playback Sync2 Port",
		.stream_name = "Honda_I2S2_SYNC2", /*mapped to topology name*/
		.cpu_dai_name = "I2S2 Sync2 Pb Pin",/*needs to ne same as .name in skl-pcm.c*/
		.platform_name = "0000:00:0e.0",
		.nonatomic = 1,
		.dynamic = 1,
		.codec_name = "snd-soc-dummy",
		.codec_dai_name = "snd-soc-dummy-dai",
		.trigger = {
			SND_SOC_DPCM_TRIGGER_POST,
			SND_SOC_DPCM_TRIGGER_POST
		},
		.dpcm_playback = 1,
	},
	{
		.name = "Honda Playback Sync3 Port",
		.stream_name = "Honda_I2S2_SYNC3", /*mapped to topology name*/
		.cpu_dai_name = "I2S2 Sync3 Pb Pin",/*needs to ne same as .name in skl-pcm.c*/
		.platform_name = "0000:00:0e.0",
		.nonatomic = 1,
		.dynamic = 1,
		.codec_name = "snd-soc-dummy",
		.codec_dai_name = "snd-soc-dummy-dai",
		.trigger = {
			SND_SOC_DPCM_TRIGGER_POST,
			SND_SOC_DPCM_TRIGGER_POST
		},
		.dpcm_playback = 1,
	},
	{
		.name = "Honda Playback Sync4 Port",
		.stream_name = "Honda_I2S2_SYNC4", /*mapped to topology name*/
		.cpu_dai_name = "I2S2 Sync4 Pb Pin",/*needs to ne same as .name in skl-pcm.c*/
		.platform_name = "0000:00:0e.0",
		.nonatomic = 1,
		.dynamic = 1,
		.codec_name = "snd-soc-dummy",
		.codec_dai_name = "snd-soc-dummy-dai",
		.trigger = {
			SND_SOC_DPCM_TRIGGER_POST,
			SND_SOC_DPCM_TRIGGER_POST
		},
		.dpcm_playback = 1,
	},
	{
		.name = "Honda Playback Sync5 Port",
		.stream_name = "Honda_I2S4_SYNC5", /*mapped to topology name*/
		.cpu_dai_name = "I2S4 Sync5 Pb Pin",/*needs to ne same as .name in skl-pcm.c*/
		.platform_name = "0000:00:0e.0",
		.nonatomic = 1,
		.dynamic = 1,
		.codec_name = "snd-soc-dummy",
		.codec_dai_name = "snd-soc-dummy-dai",
		.trigger = {
			SND_SOC_DPCM_TRIGGER_POST,
			SND_SOC_DPCM_TRIGGER_POST
		},
		.dpcm_playback = 1,
	},
        {
                .name = "Honda Capture Sync0 Port",
                .stream_name = "Honda_I2S5_SYNC0", /*mapped to topology name*/
                .cpu_dai_name = "I2S5 Sync0 Cp Pin", /*needs to ne same as .name in skl-pcm.c*/
                .codec_name = "snd-soc-dummy",
                .codec_dai_name = "snd-soc-dummy-dai",
                .platform_name = "0000:00:0e.0",
                .init = NULL,
                .dpcm_capture = 1,
                .ignore_suspend = 1,
                .nonatomic = 1,
                .dynamic = 1,
        },
	{
		.name = "Honda Playback BT Port",
		.stream_name = "Honda_I2S1_BT", /*mapped to topology name*/
		.cpu_dai_name = "I2S1 BT Pb Pin",/*needs to ne same as .name in skl-pcm.c*/
		.platform_name = "0000:00:0e.0",
		.nonatomic = 1,
		.dynamic = 1,
		.codec_name = "snd-soc-dummy",
		.codec_dai_name = "snd-soc-dummy-dai",
		.trigger = {
			SND_SOC_DPCM_TRIGGER_POST,
			SND_SOC_DPCM_TRIGGER_POST
		},
		.dpcm_playback = 1,
                .be_hw_params_fixup = gp_gen2Honda_ssp0_fixup,
	},
    {
		.name = "Honda Capture BT Port",
		.stream_name = "Honda_I2S1_BT_CP", /*mapped to topology name*/
		.cpu_dai_name = "I2S1 BT Cp Pin", /*needs to ne same as .name in skl-pcm.c*/
		.codec_name = "snd-soc-dummy",
		.codec_dai_name = "snd-soc-dummy-dai",
		.platform_name = "0000:00:0e.0",
		.init = NULL,
		.dpcm_capture = 1,
		.ignore_suspend = 1,
		.nonatomic = 1,
		.dynamic = 1,
	},
        {
                .name = "Honda Playback Sync6 Port",
                .stream_name = "Honda_I2S3_SYNC3", /*mapped to topology name*/
                .cpu_dai_name = "I2S3 Sync6 Pb Pin",/*needs to ne same as .name in skl-pcm.c*/
                .platform_name = "0000:00:0e.0",
                .nonatomic = 1,
                .dynamic = 1,
                .codec_name = "snd-soc-dummy",
                .codec_dai_name = "snd-soc-dummy-dai",
                .trigger = {
                        SND_SOC_DPCM_TRIGGER_POST,
                        SND_SOC_DPCM_TRIGGER_POST
                },
                .dpcm_playback = 1,
        },

	/* Probe DAI links*/
	{
		.name = "Bxt Compress Probe playback",
		.stream_name = "Probe Playback",
		.cpu_dai_name = "Compress Probe0 Pin",
		.codec_name = "snd-soc-dummy",
		.codec_dai_name = "snd-soc-dummy-dai",
		.platform_name = "0000:00:0e.0",
		.init = NULL,
		.nonatomic = 1,
	},
	{
		.name = "Bxt Compress Probe capture",
		.stream_name = "Probe Capture",
		.cpu_dai_name = "Compress Probe1 Pin",
		.codec_name = "snd-soc-dummy",
		.codec_dai_name = "snd-soc-dummy-dai",
		.platform_name = "0000:00:0e.0",
		.init = NULL,
		.nonatomic = 1,
	},
	/* Trace Buffer DAI links */
	{
		.name = "Bxt Trace Buffer0",
		.stream_name = "Core 0 Trace Buffer",
		.cpu_dai_name = "TraceBuffer0 Pin",
		.codec_name = "snd-soc-dummy",
		.codec_dai_name = "snd-soc-dummy-dai",
		.platform_name = "0000:00:0e.0",
		.capture_only = true,
		.ignore_suspend = 1,
	},
	{
		.name = "Bxt Trace Buffer1",
		.stream_name = "Core 1 Trace Buffer",
		.cpu_dai_name = "TraceBuffer1 Pin",
		.codec_name = "snd-soc-dummy",
		.codec_dai_name = "snd-soc-dummy-dai",
		.platform_name = "0000:00:0e.0",
		.capture_only = true,
		.ignore_suspend = 1,
	},
	/* Back End DAI links */
	{
		/* SSP1 Mono Playback*/
		.name = "SSP1-Codec",
		.id = 0,
		.cpu_dai_name = "SSP1 Pin",
		.codec_name = "snd-soc-dummy",
		.codec_dai_name = "snd-soc-dummy-dai",
		.platform_name = "0000:00:0e.0",
		.ignore_suspend = 1,
		.dpcm_playback = 1,
		.no_pcm = 1,
	},
	{
		/* SSP1 B Mono Playback*/
		.name = "SSP1-B-Codec",
		.id = 0,
		.cpu_dai_name = "SSP1-B Pin",
		.codec_name = "snd-soc-dummy",
		.codec_dai_name = "snd-soc-dummy-dai",
		.platform_name = "0000:00:0e.0",
		.ignore_suspend = 1,
		.dpcm_playback = 1,
		.no_pcm = 1,
	},
	{
		/* SSP1 C Mono Playback*/
		.name = "SSP1-C-Codec",
		.id = 0,
		.cpu_dai_name = "SSP1-C Pin",
		.codec_name = "snd-soc-dummy",
		.codec_dai_name = "snd-soc-dummy-dai",
		.platform_name = "0000:00:0e.0",
		.ignore_suspend = 1,
		.dpcm_playback = 1,
		.no_pcm = 1,
	},
	{
		/* SSP3 Stereo Playback*/
		.name = "SSP3-Codec",
		.id = 1,
		.cpu_dai_name = "SSP3 Pin",
		.codec_name = "snd-soc-dummy",
		.codec_dai_name = "snd-soc-dummy-dai",
		.platform_name = "0000:00:0e.0",
		.ignore_suspend = 1,
		.dpcm_playback = 1,
		.no_pcm = 1,
	},
        {
		/* SSP3 B Mono Playback*/
		.name = "SSP3-B-Codec",
		.id = 0,
		.cpu_dai_name = "SSP3-B Pin",
		.codec_name = "snd-soc-dummy",
		.codec_dai_name = "snd-soc-dummy-dai",
		.platform_name = "0000:00:0e.0",
		.ignore_suspend = 1,
		.dpcm_playback = 1,
		.no_pcm = 1,
	},
	{
                /* SSP4 - Mic In */
                .name = "SSP4-Codec",
                .id = 2,
                .cpu_dai_name = "SSP4 Pin",
                .codec_name = "snd-soc-dummy",
                .codec_dai_name = "snd-soc-dummy-dai",
                .platform_name = "0000:00:0e.0",
                .ignore_suspend = 1,
                .dpcm_capture = 1,
                .no_pcm = 1,
        },
	{
		/* SSP0 Mono Playback*/
		.name = "SSP0-Codec",
		.id = 3,
		.cpu_dai_name = "SSP0 Pin",
		.codec_name = "snd-soc-dummy",
		.codec_dai_name = "snd-soc-dummy-dai",
		.platform_name = "0000:00:0e.0",
		.ignore_suspend = 1,
                .dpcm_capture = 1,
		.dpcm_playback = 1,
		.no_pcm = 1,
	},
        {
                /* SSP2 surround Playback*/
                .name = "SSP2-Codec",
                .id = 4,
                .cpu_dai_name = "SSP2 Pin",
                .codec_name = "snd-soc-dummy",
                .codec_dai_name = "snd-soc-dummy-dai",
                .platform_name = "0000:00:0e.0",
                .ignore_suspend = 1,
                .dpcm_playback = 1,
                .dpcm_capture = 1,
                .no_pcm = 1,
        },

};

static int bxt_add_dai_link(struct snd_soc_card *card,
			struct snd_soc_dai_link *link)
{
	link->platform_name = "0000:00:0e.0";
	link->nonatomic = 1;
	return 0;
}

/* audio machine driver for gen2Honda */
static struct snd_soc_card gp_gen2Honda = {
	.name = "gp_gen2Honda",
	.dai_link = gp_gen2Honda_dais,
	.num_links = ARRAY_SIZE(gp_gen2Honda_dais),
	//.controls = gp_gen2Honda_controls,
	//.num_controls = ARRAY_SIZE(gp_gen2Honda_controls),
	.dapm_widgets = gp_gen2Honda_widgets,
	.num_dapm_widgets = ARRAY_SIZE(gp_gen2Honda_widgets),
	.dapm_routes = gp_gen2Honda_map,
	.num_dapm_routes = ARRAY_SIZE(gp_gen2Honda_map),
	.fully_routed = true,
	.add_dai_link = bxt_add_dai_link,
};

static int gp_gen2Honda_audio_probe(struct platform_device *pdev)
{
	dev_info(&pdev->dev, "%s registering %s\n", __func__, pdev->name);
	gp_gen2Honda.dev = &pdev->dev;
	return snd_soc_register_card(&gp_gen2Honda);
}

static int gp_gen2Honda_audio_remove(struct platform_device *pdev)
{
	snd_soc_unregister_card(&gp_gen2Honda);
	return 0;
}

static struct platform_driver gp_gen2Honda_audio = {
	.probe = gp_gen2Honda_audio_probe,
	.remove = gp_gen2Honda_audio_remove,
	.driver = {
		.name = "gen2Honda",
		.pm = &snd_soc_pm_ops,
	},
};

module_platform_driver(gp_gen2Honda_audio)

/* Module information */
MODULE_DESCRIPTION("Honda I2S Audio for Gen2Honda");
MODULE_LICENSE("GPL v2");
MODULE_ALIAS("platform:gpmrb_machine");
