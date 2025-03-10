/*
 * Copyright (c) 2009, Wei Mingzhi <whistler@openoffice.org>.
 * All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses>.
 */

#include "pad.h"

#if SDL_VERSION_ATLEAST(1,3,0)
static SDL_HapticEffect haptic_rumbleEffect;
#endif

void JoyInitHaptic()
{
#if SDL_VERSION_ATLEAST(1,3,0)
  uint8_t i;
  unsigned int haptic_query = 0;
  for (i = 0; i < 2; i++)
  {
    if (g.PadState[i].JoyDev && SDL_JoystickIsHaptic(g.PadState[i].JoyDev))
    {
      if (g.PadState[i].haptic != NULL)
      {
        SDL_HapticClose(g.PadState[i].haptic);
        g.PadState[i].haptic = NULL;
      }

      g.PadState[i].haptic = SDL_HapticOpenFromJoystick(g.PadState[i].JoyDev);
      if (g.PadState[i].haptic == NULL)
        continue;

      if (SDL_HapticRumbleSupported(g.PadState[i].haptic) == SDL_FALSE)
      {
        printf("\nRumble not supported\n");
        g.PadState[i].haptic = NULL;
        continue;
      }

      if (SDL_HapticRumbleInit(g.PadState[i].haptic) != 0)
      {
        printf("\nFailed to initialize rumble: %s\n", SDL_GetError());
        g.PadState[i].haptic = NULL;
        continue;
      }
    }
  }
#endif
}

int JoyHapticRumble(int pad, uint32_t low, uint32_t high)
{
#if SDL_VERSION_ATLEAST(1,3,0)
  float mag;

  if (g.PadState[pad].haptic) {

    /* Stop the effect if it was playing. */
    SDL_HapticRumbleStop(g.PadState[pad].haptic);

    mag = ((high * 2 + low) / 6) / 127.5;
    //printf("low: %d high: %d mag: %f\n", low, high, mag);

    if(SDL_HapticRumblePlay(g.PadState[pad].haptic, mag, 500) != 0)
    {
      printf("\nFailed to play rumble: %s\n", SDL_GetError());
      return 1;
    }
  }
#endif
  return 0;
}

void InitSDLJoy() {
	uint8_t				i;

	g.PadState[0].JoyKeyStatus = 0xFFFF;
	g.PadState[1].JoyKeyStatus = 0xFFFF;

	for (i = 0; i < 2; i++) {
		if (g.cfg.PadDef[i].DevNum >= 0) {
			g.PadState[i].JoyDev = SDL_JoystickOpen(g.cfg.PadDef[i].DevNum);
		} else {
			g.PadState[i].JoyDev = NULL;
		}
#if !SDL_VERSION_ATLEAST(1,3,0) && defined(__linux__)
		g.PadState[i].VibrateDev = -1;
		g.PadState[i].VibrateEffect = -1;
#endif
	}

#if SDL_VERSION_ATLEAST(1,3,0)
  if (has_haptic)
  {
    JoyInitHaptic();
  }
#endif

	SDL_JoystickEventState(SDL_IGNORE);

	InitAnalog();
}

void DestroySDLJoy() {
	uint8_t				i;

	if (SDL_WasInit(SDL_INIT_JOYSTICK)) {
		for (i = 0; i < 2; i++) {
			if (g.PadState[i].JoyDev != NULL) {
#if SDL_VERSION_ATLEAST(1,3,0)
        if (g.PadState[i].haptic != NULL)
        {
          SDL_HapticClose(g.PadState[i].haptic);
          g.PadState[i].haptic = NULL;
        }
#endif
				SDL_JoystickClose(g.PadState[i].JoyDev);
			}
		}
	}

	for (i = 0; i < 2; i++) {
		g.PadState[i].JoyDev = NULL;
	}
}

static void bdown(int pad, int bit)
{
	if(bit < 16)
		g.PadState[pad].JoyKeyStatus &= ~(1 << bit);
	else if(bit == DKEY_ANALOG) {
		if(++g.PadState[pad].PadModeKey == 10)
			g.PadState[pad].PadModeSwitch = 1;
		else if(g.PadState[pad].PadModeKey > 10)
			g.PadState[pad].PadModeKey = 11;
	}
}

static void bup(int pad, int bit)
{
	if(bit < 16)
		g.PadState[pad].JoyKeyStatus |= (1 << bit);
	else if(bit == DKEY_ANALOG)
		g.PadState[pad].PadModeKey = 0;
}

void CheckJoy() {
	uint8_t				i, j, n;

	SDL_JoystickUpdate();

	for (i = 0; i < 2; i++) {
		if (g.PadState[i].JoyDev == NULL) {
			continue;
		}

		for (j = 0; j < DKEY_TOTAL; j++) {
			switch (g.cfg.PadDef[i].KeyDef[j].JoyEvType) {
				case AXIS:
					n = abs(g.cfg.PadDef[i].KeyDef[j].J.Axis) - 1;

					if (g.cfg.PadDef[i].KeyDef[j].J.Axis > 0) {
						if (SDL_JoystickGetAxis(g.PadState[i].JoyDev, n) > 16383) {
							bdown(i, j);
						} else {
							bup(i, j);
						}
					} else if (g.cfg.PadDef[i].KeyDef[j].J.Axis < 0) {
						if (SDL_JoystickGetAxis(g.PadState[i].JoyDev, n) < -16383) {
							bdown(i, j);
						} else {
							bup(i, j);
						}
					}
					break;

				case HAT:
					n = (g.cfg.PadDef[i].KeyDef[j].J.Hat >> 8);

					if (SDL_JoystickGetHat(g.PadState[i].JoyDev, n) & (g.cfg.PadDef[i].KeyDef[j].J.Hat & 0xFF)) {
						bdown(i, j);
					} else {
						bup(i, j);
					}
					break;

				case BUTTON:
					if (SDL_JoystickGetButton(g.PadState[i].JoyDev, g.cfg.PadDef[i].KeyDef[j].J.Button)) {
						bdown(i, j);
					} else {
						bup(i, j);
					}
					break;

				default:
					break;
			}
		}
	}

	CheckAnalog();
}
