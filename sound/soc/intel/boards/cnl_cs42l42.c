/*
 *  cnl_cs42l42.c - ASOC Machine driver for Intel cnl_cs42l42 platform
 *		with CS42L42 soundwire codec.
 *
 *  Copyright (C) 2016 Intel Corp
 *  Author: Hardik Shah <hardik.t.shah@intel.com>
 *
 * Based on
 *	moor_dpcm_florida.c - ASOC Machine driver for Intel Moorefield platform
 *  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful, but
 *  WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  General Public License for more details.
 *
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 */
#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/module.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/io.h>
#include <linux/async.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/acpi.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/soc.h>
#include <sound/jack.h>
#include <linux/input.h>

struct cnl_cs42l42_mc_private {
	u8		pmic_id;
	void __iomem    *osc_clk0_reg;
	int bt_mode;
};

static const struct snd_soc_dapm_widget cnl_cs42l42_widgets[] = {
	SND_SOC_DAPM_HP("Headphones", NULL),
	SND_SOC_DAPM_MIC("AMIC", NULL),
	SND_SOC_DAPM_MIC("SoC DMIC", NULL),
};

static const struct snd_soc_dapm_route cnl_cs42l42_map[] = {
	/*Headphones*/
	{ "Headphones", NULL, "HP" },
	{ "I2NP", NULL, "AMIC" },

	/* SWM map link the SWM outs to codec AIF */
	{ "Playback", NULL, "SDW Tx"},
	{ "SDW Tx", NULL, "sdw_codec0_out"},


	{ "sdw_codec0_in", NULL, "SDW Rx" },
	{ "SDW Rx", NULL, "Capture" },

	{"DMic", NULL, "SoC DMIC"},
	{"DMIC01 Rx", NULL, "Capture"},
	{"dmic01_hifi", NULL, "DMIC01 Rx"},

};

static const struct snd_kcontrol_new cnl_cs42l42_controls[] = {
	SOC_DAPM_PIN_SWITCH("Headphones"),
};


static inline
struct snd_soc_codec *cnl_cs42l42_get_codec(struct snd_soc_card *card)
{
	bool found = false;
	struct snd_soc_codec *codec;

	list_for_each_entry(codec, &card->codec_dev_list, card_list) {
		if (!strstr(codec->component.name, "sdw-slave0-00:01:fa:42:42:00")) {
			pr_debug("codec was %s", codec->component.name);
			continue;
		} else {
			found = true;
			break;
		}
	}
	if (found == false) {
		pr_err("%s: cant find codec", __func__);
		BUG();
		return NULL;
	}
	return codec;
}

static struct snd_soc_dai *cnl_cs42l42_get_codec_dai(struct snd_soc_card *card,
							const char *dai_name)
{
	struct snd_soc_pcm_runtime *rtd;

	list_for_each_entry(rtd, &card->rtd_list, list) {
		if (!strncmp(rtd->codec_dai->name, dai_name,
			     strlen(dai_name)))
			return rtd->codec_dai;
	}
	pr_err("%s: unable to find codec dai\n", __func__);

	return NULL;
}

static int cnl_cs42l42_init(struct snd_soc_pcm_runtime *runtime)
{
	int ret;
	struct snd_soc_card *card = runtime->card;
	struct snd_soc_codec *cs_codec = cnl_cs42l42_get_codec(card);
	struct snd_soc_dai *cs_dai = cnl_cs42l42_get_codec_dai(card, "cs42l42");

	pr_info("Entry %s\n", __func__);
	card->dapm.idle_bias_off = true;

	/*Switch to PLL */
	ret = snd_soc_dai_set_sysclk(cs_dai, 0, 9600000, 0);
	if (ret != 0) {
		dev_err(cs_codec->dev, "Failed to set SYSCLK to FLL1: %d\n", ret);
		return ret;
	}

	ret = snd_soc_add_card_controls(card, cnl_cs42l42_controls,
					ARRAY_SIZE(cnl_cs42l42_controls));
	if (ret) {
		pr_err("unable to add card controls\n");
		return ret;
	}
	return 0;
}

static unsigned int rates_48000[] = {
	48000,
	16000,
	8000,
};

static struct snd_pcm_hw_constraint_list constraints_48000 = {
	.count = ARRAY_SIZE(rates_48000),
	.list  = rates_48000,
};

static int cnl_cs42l42_startup(struct snd_pcm_substream *substream)
{
	return snd_pcm_hw_constraint_list(substream->runtime, 0,
			SNDRV_PCM_HW_PARAM_RATE,
			&constraints_48000);
}

static struct snd_soc_ops cnl_cs42l42_ops = {
	.startup = cnl_cs42l42_startup,
};

static int cnl_cs42l42_codec_fixup(struct snd_soc_pcm_runtime *rtd,
			    struct snd_pcm_hw_params *params)
{
	struct snd_soc_dai *be_cpu_dai;
	int slot_width = 24;
	int ret = 0;
	struct snd_interval *rate = hw_param_interval(params,
			SNDRV_PCM_HW_PARAM_RATE);
	struct snd_interval *channels = hw_param_interval(params,
						SNDRV_PCM_HW_PARAM_CHANNELS);

	pr_debug("Invoked %s for dailink %s\n", __func__, rtd->dai_link->name);
	slot_width = 24;
	rate->min = rate->max = 48000;
	channels->min = channels->max = 2;
	snd_mask_none(hw_param_mask(params, SNDRV_PCM_HW_PARAM_FORMAT));
	snd_mask_set(hw_param_mask(params, SNDRV_PCM_HW_PARAM_FORMAT),
						SNDRV_PCM_FORMAT_S24_LE);

	pr_info("param width set to:0x%x\n",
			snd_pcm_format_width(params_format(params)));
	pr_info("Slot width = %d\n", slot_width);

	be_cpu_dai = rtd->cpu_dai;
	return ret;
}

static const struct snd_soc_pcm_stream cnl_cs42l42_dai_params = {
	.formats = SNDRV_PCM_FMTBIT_S24_LE,
	.rate_min = 48000,
	.rate_max = 48000,
	.channels_min = 4,
	.channels_max = 4,
};

struct snd_soc_dai_link cnl_cs42l42_msic_dailink[] = {
	{
		.name = "Bxtn Audio Port",
		.stream_name = "Audio",
		.cpu_dai_name = "System Pin",
		.codec_name = "snd-soc-dummy",
		.codec_dai_name = "snd-soc-dummy-dai",
		.platform_name = "0000:02:18.0",
		.init = cnl_cs42l42_init,
		.ignore_suspend = 1,
		.nonatomic = 1,
		.dynamic = 1,
		.dpcm_playback = 1,
		.dpcm_capture = 1,
		.ops = &cnl_cs42l42_ops,
	},
	{
		.name = "SDW0-Codec",
		.cpu_dai_name = "SDW Pin",
		.platform_name = "0000:02:18.0",
		.codec_name = "sdw-slave0-00:01:fa:42:42:00",
		.codec_dai_name = "cs42l42",
		.be_hw_params_fixup = cnl_cs42l42_codec_fixup,
		.ignore_suspend = 1,
		.no_pcm = 1,
		.dpcm_playback = 1,
		.dpcm_capture = 1,
	},
	{
		.name = "dmic01",
		.cpu_dai_name = "DMIC01 Pin",
		.codec_name = "dmic-codec",
		.codec_dai_name = "dmic-hifi",
		.platform_name = "0000:02:18.0",
		.ignore_suspend = 1,
		.no_pcm = 1,
		.dpcm_capture = 1,
	},

};

/* SoC card */
static struct snd_soc_card snd_soc_card_cnl_cs42l42 = {
	.name = "cnl_cs42l42-audio",
	.dai_link = cnl_cs42l42_msic_dailink,
	.num_links = ARRAY_SIZE(cnl_cs42l42_msic_dailink),
	.dapm_widgets = cnl_cs42l42_widgets,
	.num_dapm_widgets = ARRAY_SIZE(cnl_cs42l42_widgets),
	.dapm_routes = cnl_cs42l42_map,
	.num_dapm_routes = ARRAY_SIZE(cnl_cs42l42_map),
};


static int snd_cnl_cs42l42_mc_probe(struct platform_device *pdev)
{
	int ret_val = 0;
	struct cnl_cs42l42_mc_private *drv;

	pr_debug("Entry %s\n", __func__);

	drv = devm_kzalloc(&pdev->dev, sizeof(*drv), GFP_KERNEL);
	if (!drv)
		return -ENOMEM;

	snd_soc_card_cnl_cs42l42.dev = &pdev->dev;
	snd_soc_card_set_drvdata(&snd_soc_card_cnl_cs42l42, drv);
	/* Register the card */
	ret_val = snd_soc_register_card(&snd_soc_card_cnl_cs42l42);
	if (ret_val && (ret_val != -EPROBE_DEFER)) {
		pr_err("snd_soc_register_card failed %d\n", ret_val);
		goto unalloc;
	}
	platform_set_drvdata(pdev, &snd_soc_card_cnl_cs42l42);
	pr_info("%s successful\n", __func__);
	return ret_val;

unalloc:
	return ret_val;
}

static int snd_cnl_cs42l42_mc_remove(struct platform_device *pdev)
{
	struct snd_soc_card *soc_card = platform_get_drvdata(pdev);
	struct cnl_cs42l42_mc_private *drv = snd_soc_card_get_drvdata(soc_card);

	pr_debug("In %s\n", __func__);

	devm_kfree(&pdev->dev, drv);
	snd_soc_card_set_drvdata(soc_card, NULL);
	snd_soc_unregister_card(soc_card);
	platform_set_drvdata(pdev, NULL);
	return 0;
}

static struct platform_driver snd_cnl_cs42l42_mc_driver = {
	.driver = {
		.owner = THIS_MODULE,
		.name = "cnl_cs42l42",
	},
	.probe = snd_cnl_cs42l42_mc_probe,
	.remove = snd_cnl_cs42l42_mc_remove,
};

static int snd_cnl_cs42l42_driver_init(void)
{
	pr_info("Canonlake Machine Driver cnl_cs42l42: cs42l42 registered\n");
	return platform_driver_register(&snd_cnl_cs42l42_mc_driver);
}
module_init(snd_cnl_cs42l42_driver_init);

static void snd_cnl_cs42l42_driver_exit(void)
{
	pr_debug("In %s\n", __func__);
	platform_driver_unregister(&snd_cnl_cs42l42_mc_driver);
}
module_exit(snd_cnl_cs42l42_driver_exit)

MODULE_DESCRIPTION("ASoC CNL Machine driver");
MODULE_AUTHOR("Hardik Shah <hardik.t.shah>");
MODULE_LICENSE("GPL v2");
MODULE_ALIAS("platform:cnl_cs42l42");
