/*
 * Copyright (C) 2000-2004 James Courtier-Dutton
 * Copyright (C) 2005 Nathan Hurst
 *
 * This file is part of the speaker-test tool.
 *
 * This small program sends a simple sinusoidal wave to your speakers.
 *
 * speaker-test is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * speaker-test is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA
 *
 *
 * Main program by James Courtier-Dutton (including some source code fragments from the alsa project.)
 * Some cleanup from Daniel Caujolle-Bert <segfault@club-internet.fr>
 * Pink noise option added Nathan Hurst, 
 *   based on generator by Phil Burk (pink.c)
 *
 * Changelog:
 *   0.0.8 Added support for pink noise output.
 * Changelog:
 *   0.0.7 Added support for more than 6 channels.
 * Changelog:
 *   0.0.6 Added support for different sample formats.
 *
 * speaker-test.c,v 1.13 2005/11/08 16:43:36 tiwai Exp
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sched.h>
#include <errno.h>
#include <getopt.h>
#include <inttypes.h>
#include <locale.h>

#define ALSA_PCM_NEW_HW_PARAMS_API
#define ALSA_PCM_NEW_SW_PARAMS_API
#include <alsa/asoundlib.h>
#include <sys/time.h>
#include <math.h>
#include "pink.h"
#include "aconfig.h"
#include "gettext.h"

static char              *device      = "plughw:0,0";       /* playback device */
static snd_pcm_format_t   format      = SND_PCM_FORMAT_S16; /* sample format */
static unsigned int       rate        = 48000;	            /* stream rate */
static unsigned int       channels    = 1;	            /* count of channels */
static unsigned int       speaker     = 0;	            /* count of channels */
static unsigned int       buffer_time = 500000;	            /* ring buffer length in us */
static unsigned int       period_time = 100000;	            /* period time in us */
#define PERIODS 4
static double             freq        = 440;                /* sinusoidal wave frequency in Hz */
static int                test_type   = 1;                  /* Test type. 1 = noise, 2 = sine wave */
static pink_noise_t pink;
static snd_output_t      *output      = NULL;
static snd_pcm_uframes_t  buffer_size;
static snd_pcm_uframes_t  period_size;
static const char        *channel_name[] = {
  N_("Front Left"),
  N_("Front Right"),
  N_("Rear Left"),
  N_("Rear Right"),
  N_("Center"),
  N_("LFE"),
  N_("Side Left"),
  N_("Side Right"),
  N_("8"),
  N_("9"),
  N_("10"),
  N_("11"),
  N_("12"),
  N_("13"),
  N_("14"),
  N_("15"),
  N_("16")
};

static const int	channels4[] = {
  0,
  1,
  3,
  2
};
static const int	channels6[] = {
  0,
  4,
  1,
  3,
  2,
  5
}; 
static const int	channels8[] = {
  0,
  4,
  1,
  7,
  3,
  2,
  6,
  5
}; 

static void generate_sine(uint8_t *frames, int channel, int count, double *_phase) {
  double phase = *_phase;
  double max_phase = 1.0 / freq;
  double step = 1.0 / (double)rate;
  double res;
  float fres;
  int    chn;
  int32_t  ires;
  int8_t *samp8 = (int8_t*) frames;
  int16_t *samp16 = (int16_t*) frames;
  int32_t *samp32 = (int32_t*) frames;
  float   *samp_f = (float*) frames;
  int sample_size_bits = snd_pcm_format_width(format); 

  while (count-- > 0) {
    //res = sin((phase * 2 * M_PI) / max_phase - M_PI) * 32767;
    //res = sin((phase * 2 * M_PI) / max_phase - M_PI) * 32767;
    //res = (sin((phase * 2 * M_PI) / max_phase - M_PI)) * 0x03fffffff; /* Don't use MAX volume */
    //if (res > 0) res = 10000;
    //if (res < 0) res = -10000;

    /* printf("%e\n",res); */
    //ires = res;
    //ires = ((16 - (count & 0xf)) <<24);
    //ires = 0;

    for(chn=0;chn<channels;chn++) {
      if (sample_size_bits == 8) {
        if (chn==channel) {
          res = (sin((phase * 2 * M_PI) / max_phase - M_PI)) * 0x03fffffff; /* Don't use MAX volume */
          ires = res;
	  *samp8++ = ires >> 24;
	  //*samp8++ = 0x12;
        } else {
	  *samp8++ = 0;
        }
      } else if (sample_size_bits == 16) {
        if (chn==channel) {
          res = (sin((phase * 2 * M_PI) / max_phase - M_PI)) * 0x03fffffff; /* Don't use MAX volume */
          ires = res;
	  *samp16++ = ires >>16;
	  //*samp16++ = 0x1234;
        } else {
	  //*samp16++ = (ires >>16)+1;
	  *samp16++ = 0;
        }
      } else if ((sample_size_bits == 32) && (format == SND_PCM_FORMAT_FLOAT_LE)) {
        if (chn==channel) {
          res = (sin((phase * 2 * M_PI) / max_phase - M_PI)) * 0.75 ; /* Don't use MAX volume */
          fres = res;
	  *samp_f++ = fres;
	  //*samp32++ = 0xF2345678;
	//printf("res=%lf, ires=%d 0x%x, samp32=0x%x\n",res,ires, ires, samp32[-1]);
        } else {
	  //*samp32++ = ires+0x10000;
	  //*samp32++ = ires;
	  *samp_f++ = 0.0;
        }
      } else if ((sample_size_bits == 32) && (format != SND_PCM_FORMAT_FLOAT_LE)) {
        if (chn==channel) {
          res = (sin((phase * 2 * M_PI) / max_phase - M_PI)) * 0x03fffffff; /* Don't use MAX volume */
          ires = res;
	  *samp32++ = ires;
	  //*samp32++ = 0xF2345678;
	//printf("res=%lf, ires=%d 0x%x, samp32=0x%x\n",res,ires, ires, samp32[-1]);
        } else {
	  //*samp32++ = ires+0x10000;
	  //*samp32++ = ires;
	  *samp32++ = 0;
        }
      }
    }

    phase += step;
    if (phase >= max_phase)
      phase -= max_phase;
  }

  *_phase = phase;
}

/* Pink noise is a better test than sine wave because we can tell
 * where pink noise is coming from more easily that a sine wave.
 */


static void generate_pink_noise( uint8_t *frames, int channel, int count) {
  double   res;
  int      chn;
  int32_t  ires;
  float    fres;
  int8_t  *samp8 = (int8_t*) frames;
  int16_t *samp16 = (int16_t*) frames;
  int32_t *samp32 = (int32_t*) frames;
  float   *samp_f = (float*) frames;
  int sample_size_bits = snd_pcm_format_width(format); 

  while (count-- > 0) {
    for(chn=0;chn<channels;chn++) {
      if (sample_size_bits == 8) {
        if (chn==channel) {
	  res = generate_pink_noise_sample(&pink) * 0x03fffffff; /* Don't use MAX volume */
	  ires = res;
	  *samp8++ = ires >> 24;
        } else {
	  *samp8++ = 0;
        }
      } else if (sample_size_bits == 16) {
        if (chn==channel) {
	  res = generate_pink_noise_sample(&pink) * 0x03fffffff; /* Don't use MAX volume */
	  ires = res;
	  *samp16++ = ires >>16;
        } else {
	  *samp16++ = 0;
        }
      } else if ((sample_size_bits == 32) && (format == SND_PCM_FORMAT_FLOAT_LE)) {
        if (chn==channel) {
	  res = generate_pink_noise_sample(&pink) * 0.75; /* Don't use MAX volume */
          fres = res;
	  *samp_f++ = fres;
        } else {
	  *samp_f++ = 0.0;
        }
      } else if ((sample_size_bits == 32) && (format != SND_PCM_FORMAT_FLOAT_LE)) {
        if (chn==channel) {
	  res = generate_pink_noise_sample(&pink) * 0x03fffffff; /* Don't use MAX volume */
	  ires = res;
	  *samp32++ = ires;
        } else {
	  *samp32++ = 0;
        }
      }
    }

  }
     
}

static int set_hwparams(snd_pcm_t *handle, snd_pcm_hw_params_t *params, snd_pcm_access_t access) {
  unsigned int rrate;
  int          err, dir;
  snd_pcm_uframes_t     period_size_min;
  snd_pcm_uframes_t     period_size_max;
  snd_pcm_uframes_t     buffer_size_min;
  snd_pcm_uframes_t     buffer_size_max;
  snd_pcm_uframes_t     buffer_time_to_size;



  /* choose all parameters */
  err = snd_pcm_hw_params_any(handle, params);
  if (err < 0) {
    printf(_("Broken configuration for playback: no configurations available: %s\n"), snd_strerror(err));
    return err;
  }

  /* set the interleaved read/write format */
  err = snd_pcm_hw_params_set_access(handle, params, access);
  if (err < 0) {
    printf(_("Access type not available for playback: %s\n"), snd_strerror(err));
    return err;
  }

  /* set the sample format */
  err = snd_pcm_hw_params_set_format(handle, params, format);
  if (err < 0) {
    printf(_("Sample format not available for playback: %s\n"), snd_strerror(err));
    return err;
  }

  /* set the count of channels */
  err = snd_pcm_hw_params_set_channels(handle, params, channels);
  if (err < 0) {
    printf(_("Channels count (%i) not available for playbacks: %s\n"), channels, snd_strerror(err));
    return err;
  }

  /* set the stream rate */
  rrate = rate;
  err = snd_pcm_hw_params_set_rate(handle, params, rate, 0);
  if (err < 0) {
    printf(_("Rate %iHz not available for playback: %s\n"), rate, snd_strerror(err));
    return err;
  }

  if (rrate != rate) {
    printf(_("Rate doesn't match (requested %iHz, get %iHz, err %d)\n"), rate, rrate, err);
    return -EINVAL;
  }

  printf(_("Rate set to %iHz (requested %iHz)\n"), rrate, rate);
  /* set the buffer time */
  buffer_time_to_size = ( (snd_pcm_uframes_t)buffer_time * rate) / 1000000;
  err = snd_pcm_hw_params_get_buffer_size_min(params, &buffer_size_min);
  err = snd_pcm_hw_params_get_buffer_size_max(params, &buffer_size_max);
  dir=0;
  err = snd_pcm_hw_params_get_period_size_min(params, &period_size_min,&dir);
  dir=0;
  err = snd_pcm_hw_params_get_period_size_max(params, &period_size_max,&dir);
  printf(_("Buffer size range from %lu to %lu\n"),buffer_size_min, buffer_size_max);
  printf(_("Period size range from %lu to %lu\n"),period_size_min, period_size_max);
  printf(_("Periods = %d\n"), PERIODS);
  printf(_("Buffer time size %lu\n"),buffer_time_to_size);

  buffer_size = buffer_time_to_size;
  //buffer_size=8096;
  //buffer_size=15052;
  buffer_size=buffer_size_max;
  if (buffer_size_max < buffer_size) buffer_size = buffer_size_max;
  if (buffer_size_min > buffer_size) buffer_size = buffer_size_min;
  //buffer_size=0x800;
  period_size = buffer_size/PERIODS;
  buffer_size = period_size*PERIODS;
  //period_size = 510;
  printf(_("To choose buffer_size = %lu\n"),buffer_size);
  printf(_("To choose period_size = %lu\n"),period_size);
  dir=0;
  err = snd_pcm_hw_params_set_period_size_near(handle, params, &period_size, &dir);
  if (err < 0) {
    printf(_("Unable to set period size %lu for playback: %s\n"), period_size, snd_strerror(err));
    return err;
  }
  dir=0;
  err = snd_pcm_hw_params_get_period_size(params, &period_size, &dir);
  if (err < 0)  printf(_("Unable to get period size for playback: %s\n"), snd_strerror(err));
                                                                                                                             
  dir=0;
  err = snd_pcm_hw_params_set_buffer_size_near(handle, params, &buffer_size);
  if (err < 0) {
    printf(_("Unable to set buffer size %lu for playback: %s\n"), buffer_size, snd_strerror(err));
    return err;
  }
  err = snd_pcm_hw_params_get_buffer_size(params, &buffer_size);
  printf(_("was set period_size = %lu\n"),period_size);
  printf(_("was set buffer_size = %lu\n"),buffer_size);
  if (2*period_size > buffer_size) {
    printf(_("buffer to small, could not use\n"));
    return err;
  }


  /* write the parameters to device */
  err = snd_pcm_hw_params(handle, params);
  if (err < 0) {
    printf(_("Unable to set hw params for playback: %s\n"), snd_strerror(err));
    return err;
  }

  return 0;
}

static int set_swparams(snd_pcm_t *handle, snd_pcm_sw_params_t *swparams) {
  int err;

  /* get the current swparams */
  err = snd_pcm_sw_params_current(handle, swparams);
  if (err < 0) {
    printf(_("Unable to determine current swparams for playback: %s\n"), snd_strerror(err));
    return err;
  }

  /* start the transfer when a buffer is full */
  err = snd_pcm_sw_params_set_start_threshold(handle, swparams, buffer_size);
  if (err < 0) {
    printf(_("Unable to set start threshold mode for playback: %s\n"), snd_strerror(err));
    return err;
  }

  /* allow the transfer when at least period_size frames can be processed */
  err = snd_pcm_sw_params_set_avail_min(handle, swparams, period_size);
  if (err < 0) {
    printf(_("Unable to set avail min for playback: %s\n"), snd_strerror(err));
    return err;
  }

  /* align all transfers to 1 sample */
  err = snd_pcm_sw_params_set_xfer_align(handle, swparams, 1);
  if (err < 0) {
    printf(_("Unable to set transfer align for playback: %s\n"), snd_strerror(err));
    return err;
  }

  /* write the parameters to the playback device */
  err = snd_pcm_sw_params(handle, swparams);
  if (err < 0) {
    printf(_("Unable to set sw params for playback: %s\n"), snd_strerror(err));
    return err;
  }

  return 0;
}

/*
 *   Underrun and suspend recovery
 */

static int xrun_recovery(snd_pcm_t *handle, int err) {
  if (err == -EPIPE) {	/* under-run */
    err = snd_pcm_prepare(handle);
    if (err < 0)
      printf(_("Can't recovery from underrun, prepare failed: %s\n"), snd_strerror(err));
    return 0;
  } 
  else if (err == -ESTRPIPE) {

    while ((err = snd_pcm_resume(handle)) == -EAGAIN)
      sleep(1);	/* wait until the suspend flag is released */

    if (err < 0) {
      err = snd_pcm_prepare(handle);
      if (err < 0)
        printf(_("Can't recovery from suspend, prepare failed: %s\n"), snd_strerror(err));
    }

    return 0;
  }

  return err;
}

/*
 *   Transfer method - write only
 */

static int write_loop(snd_pcm_t *handle, int channel, int periods, uint8_t *frames) {
  double phase = 0;
  uint8_t *ptr;
  int    err, cptr, n;
  int    bytes_per_frame=snd_pcm_frames_to_bytes(handle, 1);

  for(n = 0; n < periods; n++) {
    if(test_type==1)
      generate_pink_noise(frames, channel, period_size);
    else
      generate_sine(frames, channel, period_size, &phase);
      
    ptr = frames;
    cptr = period_size;

    while (cptr > 0) {

      err = snd_pcm_writei(handle, ptr, cptr);

      if (err == -EAGAIN)
        continue;

      if (err < 0) {
        printf(_("Write error: %d,%s\n"), err, snd_strerror(err));
        if (xrun_recovery(handle, err) < 0) {
          printf(_("xrun_recovery failed: %d,%s\n"), err, snd_strerror(err));
	  return -1;
        }
        break;	/* skip one period */
      }

      ptr += (err * bytes_per_frame);
      cptr -= err;
    }
  }

  return 0;
}

static void help(void)
{
  int k;

  printf(
	 _("Usage: speaker-test [OPTION]... \n"
	   "-h,--help	help\n"
	   "-D,--device	playback device\n"
	   "-r,--rate	stream rate in Hz\n"
	   "-c,--channels	count of channels in stream\n"
	   "-f,--frequency	sine wave frequency in Hz\n"
	   "-F,--format	sample format\n"
	   "-b,--buffer	ring buffer size in us\n"
	   "-p,--period	period size in us\n"
	   "-t,--test	1=use pink noise, 2=use sine wave\n"
	   "-s,--speaker	single speaker test. Values 1=Left or 2=right\n"
	   "\n"));
#if 1
  printf(_("Recognized sample formats are:"));
  for (k = 0; k < SND_PCM_FORMAT_LAST; ++k) {
    const char *s = snd_pcm_format_name(k);
    if (s)
      printf(" %s", s);
  }

  printf("\n\n");
#endif

}

int main(int argc, char *argv[]) {
  snd_pcm_t            *handle;
  int                   err, morehelp;
  snd_pcm_hw_params_t  *hwparams;
  snd_pcm_sw_params_t  *swparams;
  uint8_t              *frames;
  int                   chn;
  double		time1,time2,time3;
  struct   timeval	tv1,tv2;

  struct option         long_option[] = {
    {"help",      0, NULL, 'h'},
    {"device",    1, NULL, 'D'},
    {"rate",      1, NULL, 'r'},
    {"channels",  1, NULL, 'c'},
    {"frequency", 1, NULL, 'f'},
    {"format",    1, NULL, 'F'},
    {"buffer",    1, NULL, 'b'},
    {"period",    1, NULL, 'p'},
    {"test",      1, NULL, 't'},
    {"speaker",   1, NULL, 's'},
    {NULL,        0, NULL, 0  },
  };

#ifdef ENABLE_NLS
  setlocale(LC_ALL, "");
  textdomain(PACKAGE);
#endif

  snd_pcm_hw_params_alloca(&hwparams);
  snd_pcm_sw_params_alloca(&swparams);
 
  morehelp = 0;

  printf("\nspeaker-test %s\n\n", SPEAKER_TEST_VERSION);
  while (1) {
    int c;
    
    if ((c = getopt_long(argc, argv, "hD:r:c:f:F:b:p:t:s:", long_option, NULL)) < 0)
      break;
    
    switch (c) {
    case 'h':
      morehelp++;
      break;
    case 'D':
      device = strdup(optarg);
      break;
    case 'F':
      format = snd_pcm_format_value(optarg);
      break;
    case 'r':
      rate = atoi(optarg);
      rate = rate < 4000 ? 4000 : rate;
      rate = rate > 196000 ? 196000 : rate;
      break;
    case 'c':
      channels = atoi(optarg);
      channels = channels < 1 ? 1 : channels;
      channels = channels > 1024 ? 1024 : channels;
      break;
    case 'f':
      freq = atoi(optarg);
      freq = freq < 50 ? 50 : freq;
      freq = freq > 5000 ? 5000 : freq;
      break;
    case 'b':
      buffer_time = atoi(optarg);
      buffer_time = buffer_time < 1000 ? 1000 : buffer_time;
      buffer_time = buffer_time > 1000000 ? 1000000 : buffer_time;
      break;
    case 'p':
      period_time = atoi(optarg);
      period_time = period_time < 1000 ? 1000 : period_time;
      period_time = period_time > 1000000 ? 1000000 : period_time;
      break;
    case 't':
      test_type = atoi(optarg);
      test_type = test_type < 1 ? 1 : test_type;
      test_type = test_type > 2 ? 2 : test_type;
      break;
    case 's':
      speaker = atoi(optarg);
      speaker = speaker < 1 ? 0 : speaker;
      speaker = speaker > channels ? 0 : speaker;
      if (speaker==0) {
        printf(_("Invalid parameter for -s option.\n"));
        exit(EXIT_FAILURE);
      }  
      break;
    default:
      printf(_("Unknown option '%c'\n"), c);
      exit(EXIT_FAILURE);
      break;
    }
  }

  if (morehelp) {
    help();
    exit(EXIT_SUCCESS);
  }

  err = snd_output_stdio_attach(&output, stdout, 0);
  if (err < 0) {
    printf(_("Output failed: %s\n"), snd_strerror(err));
    exit(EXIT_FAILURE);
  }

  printf(_("Playback device is %s\n"), device);
  printf(_("Stream parameters are %iHz, %s, %i channels\n"), rate, snd_pcm_format_name(format), channels);
  if(test_type==1)
    printf(_("Using 16 octaves of pink noise\n"));
  else
    printf(_("Sine wave rate is %.4fHz\n"), freq);

loop:
  while ((err = snd_pcm_open(&handle, device, SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
    printf(_("Playback open error: %d,%s\n"), err,snd_strerror(err));
    sleep(1);
  }

  if ((err = set_hwparams(handle, hwparams, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
    printf(_("Setting of hwparams failed: %s\n"), snd_strerror(err));
    snd_pcm_close(handle);
    goto loop;
    exit(EXIT_FAILURE);
  }
  //getchar();
  if ((err = set_swparams(handle, swparams)) < 0) {
    printf(_("Setting of swparams failed: %s\n"), snd_strerror(err));
    snd_pcm_close(handle);
    goto loop;
    exit(EXIT_FAILURE);
  }

  frames = malloc((period_size * channels * snd_pcm_format_width(format)) / 8);
  initialize_pink_noise( &pink, 16);
  
  if (frames == NULL) {
    printf(_("No enough memory\n"));
    exit(EXIT_FAILURE);
  }
  if (speaker==0) {
    while (1) {

      gettimeofday(&tv1, NULL);
      for(chn = 0; chn < channels; chn++) {
	int channel=chn;
	if (channels == 4) {
	    channel=channels4[chn];
	}
	if (channels == 6) {
	    channel=channels6[chn];
	}
	if (channels == 8) {
	    channel=channels8[chn];
	}
        printf(" %d - %s\n", channel, gettext(channel_name[channel]));

        err = write_loop(handle, channel, ((rate*3)/period_size), frames);
        //err = write_loop(handle, 255, ((rate*3)/period_size), frames);

        if (err < 0) {
          printf(_("Transfer failed: %s\n"), snd_strerror(err));
          free(frames);
          snd_pcm_close(handle);
	  printf(_("Pausing\n"));
	  goto loop ;
	  //pause();
	  //printf("Done Pausing\n");
          exit(EXIT_SUCCESS);
	  goto loop ;
        }
      }
      gettimeofday(&tv2, NULL);
      time1 = (double)tv1.tv_sec + ((double)tv1.tv_usec / 1000000.0);
      time2 = (double)tv2.tv_sec + ((double)tv2.tv_usec / 1000000.0);
      time3 = time2 - time1;
      printf(_("Time per period = %lf\n"), time3 );
    }
  } else {
    printf("  - %s\n", gettext(channel_name[speaker-1]));
    err = write_loop(handle, speaker-1, ((rate*5)/period_size), frames);

    if (err < 0) {
      printf(_("Transfer failed: %s\n"), snd_strerror(err));
    }
  }


  free(frames);
  snd_pcm_close(handle);

  exit(EXIT_SUCCESS);
}
