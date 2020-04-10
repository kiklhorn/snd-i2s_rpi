/*
 * =====================================================================================
 *
 *       Filename:  snd-i2s_rpi
 *
 *    Description:  Driver for the SPH0645LM4H I2S MEMS microphone.
 *
 *        Version:  0.0.3
 *        Created:  2020-04-04
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Huan Truong (htruong@tnhh.net), originally written by Paul Creaser
 *   Organization:  Crankshaft (http://getcrankshaft.com)
 *
 * =====================================================================================
 */
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/kernel.h>
#include <linux/kmod.h>
#include <linux/platform_device.h>
#include <sound/simple_card.h>
#include <linux/delay.h>
#include "snd-i2s_rpi.h"

/*
 * modified for linux 4.1.5
 * inspired by https://github.com/msperl/spi-config
 * with thanks for https://github.com/notro/rpi-source/wiki
 * as well as Florian Meier for the rpi i2s and dma drivers
 *
 * to use a differant (simple-card compatible) codec
 * change the codec name string in two places and the
 * codec_dai name string. (see codec's source file)
 *
 *
 * N.B. playback vs capture is determined by the codec choice
 * */

void device_release_callback(struct device *dev) { /*  do nothing */ };

static short rpi_platform_generation = 0;
module_param(rpi_platform_generation, short, 0);
MODULE_PARM_DESC(rpi_platform_generation, "Raspberry Pi generation: 0=Zero, 1=Others");

static struct asoc_simple_card_info default_snd_rpi_simple_card_info = {
  .card = "snd_rpi_i2s_card", // -> snd_soc_card.name
  .name = "simple-card_codec_link", // -> snd_soc_dai_link.name
  // Available codecs can be found in /sys/kernel/debug/asoc/codecs
  .codec = "snd-soc-dummy", // -> snd_soc_dai_link.codec_name
  // ASoC platform strings can be found in /sys/kernel/debug/asoc/platforms
  .platform = "not-set.i2s",
  // Note: use SND_SOC_DAIFMT_CBS_CFM instead of SND_SOC_DAIFMT_CBS_CFS
  // if I2S device must be in slave state.
  .daifmt = SND_SOC_DAIFMT_I2S | SND_SOC_DAIFMT_NB_NF | SND_SOC_DAIFMT_CBS_CFS,
  .cpu_dai = {
    .name = "not-set.i2s", // -> snd_soc_dai_link.cpu_dai_name
    .sysclk = 0 
  },
  .codec_dai = {
    // Available dais can be found in /sys/kernel/debug/asoc/dais
    .name = "snd-soc-dummy-dai", //"dmic-codec", // -> snd_soc_dai_link.codec_dai_name
    .sysclk = 0 
  },
};

static struct platform_device default_snd_rpi_simple_card_device = {
  .name = "asoc-simple-card", // Module alias.
  .id = 0,
  .num_resources = 0,
  .dev = {
    .release = &device_release_callback,
    .platform_data = &default_snd_rpi_simple_card_info,
  },
};

// ASoC platform strings can be found in /sys/kernel/debug/asoc/platforms
static const char *pri_platform = "20203000.i2s";  // Zero
static const char *alt_platform = "3f203000.i2s";  // Others

int i2s_rpi_init(void)
{
  const char *dmaengine = "bcm2708-dmaengine";
  const char *card_platform;
  int ret;

  printk(KERN_INFO "snd-i2s_rpi: Version %s\n", SND_I2S_RPI_VERSION);

  card_platform = pri_platform;
  if (rpi_platform_generation != 0) {
    card_platform = alt_platform;
  }

  printk(KERN_INFO "snd-i2s_rpi: Setting platform to %s\n", card_platform);

  ret = request_module(dmaengine);
  if (ret != 0) {
    pr_alert("Unable to request module load '%s': %d\n",
             dmaengine, ret);
  }

  default_snd_rpi_simple_card_info.platform = card_platform;
  default_snd_rpi_simple_card_info.cpu_dai.name = card_platform;

  ret = platform_device_register(&default_snd_rpi_simple_card_device);
  if (ret != 0) {
    pr_alert("Unable to register platform device '%s': %d\n",
             default_snd_rpi_simple_card_device.name, ret);
  }

  return ret;
}

void i2s_rpi_exit(void)
{
  platform_device_unregister(&default_snd_rpi_simple_card_device);
}

module_init(i2s_rpi_init);
module_exit(i2s_rpi_exit);

MODULE_DESCRIPTION("ASoC simple-card I2S Microphone");
MODULE_AUTHOR("Huan Truong <htruong@tnhh.net>");
MODULE_LICENSE("GPL v2");

