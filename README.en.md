# qcom-va-driver (Venus build)

[中文说明](README.md)

VA-API backend for Qualcomm Venus (SM8250) through the Linux V4L2 stateful
memory-to-memory interface, providing hardware video decoding and encoding.

## About

Derived from [radxa-pkg/qcom-va-driver](https://github.com/radxa-pkg/qcom-va-driver)
1.1.1. Upstream targets Qualcomm Iris (SC8280XP); this build adapts it to the
Venus block found on SM8250 with six changes (driver-name matching, capture
pool size, control compatibility, and others). Full source is available in
`qcom-va-driver-1.1.1-venus-src.tar.gz`.

## Support

| Codec | Decoding | Encoding |
| --- | --- | --- |
| H.264 / AVC | Constrained Baseline, Main, High (8-bit) | Same profiles (8-bit) |
| H.265 / HEVC | Main (8-bit), Main10 (10-bit) | Main (8-bit) |
| VP9 | Profile 0, 2 (8/10-bit) | — |

- Progressive 4:2:0 only
- No VPP: no scaling, deinterlacing or colour conversion
- No AV1, VP8 or MPEG-2

## Installation

```bash
sudo dpkg -i qcom-va-driver_1.1.1-venus3_arm64.deb
```

Three builds are provided:

| Package | Contents |
| --- | --- |
| venus3 | Decode + encode + GPU zero-copy (recommended) |
| venus2 | Decode + encode |
| venus1 | Decode only |

The driver installs into `/usr/lib/aarch64-linux-gnu/dri/` and is picked up by
libva automatically; `LIBVA_DRIVER_NAME` does not need to be set.

Uninstall:

```bash
sudo dpkg -r qcom-va-driver
```

## Building

```bash
sudo apt install meson ninja-build libva-dev libdrm-dev \
                  libegl-dev libgles-dev libgbm-dev
tar xf qcom-va-driver-1.1.1-venus-src.tar.gz
cd qcom-va-driver-master
meson setup build -Dprefix=/usr -Dcdsp_vpp=disabled -Dvulkan_vpp=disabled
ninja -C build
sudo ninja -C build install
```

If `libegl-dev` / `libgles-dev` / `libgbm-dev` are missing, `gpu_copy` is
disabled automatically and zero-copy is unavailable (this matches the venus2
build).

## Verification

```bash
vainfo
```

It should report `VAEntrypointVLD` for H.264 / HEVC / VP9 and
`VAEntrypointEncSlice` for H.264 / HEVC.

## Test environment

| Item | Value |
| --- | --- |
| Device | Xiaomi Mi Pad 5 Pro (codename elish) |
| SoC | Qualcomm Snapdragon 870 (SM8250), Venus codec |
| OS | Armbian 26.8.14 (Ubuntu 24.04 noble) |
| Kernel | 6.18.44-current-sm8250 |
| VA-API | libva 1.20.0 |
| Applications tested | Firefox 159.0.1, ffmpeg 6.1.1, mpv 0.37.0 |

## Benchmarks

| Item | Result |
| --- | --- |
| Decode 1080p H.264 | 0.31 s user time for 3 s of video |
| Decode 4K HEVC | 1.29 s user time for 3 s of video (software: 7.75 s) |
| Encode 1080p H.264 | 90 frames in 0.50 s |
| Encode 4K HEVC | 60 frames in 0.80 s |
| Firefox 4K HEVC playback | ~4.6% CPU in the RDD process |
| mpv zero-copy | `--hwdec=vaapi`, `VO: [gpu] vaapi[nv12]` |

## Firefox

Copy `firefox-user.js` into the Firefox profile, or set via `about:config`:

```js
user_pref("media.ffmpeg.vaapi.enabled", true);
```

After a restart, `about:support` → Media → Hardware Video Decoding should list
H.264 / VP8 / VP9 / HEVC.

## Notes

The `v4l2m2m` codecs shipped with ffmpeg do not work with this hardware
(`VIDIOC_G_FMT` fails). Use VA-API instead (`-hwaccel vaapi`, `h264_vaapi` /
`hevc_vaapi`), or the GStreamer elements `v4l2h264dec` / `v4l2h265dec`.

## Upstream and credits

- Upstream: [radxa-pkg/qcom-va-driver](https://github.com/radxa-pkg/qcom-va-driver) 1.1.1
- Source package: [radxa-pkg/libva-v4l2](https://github.com/radxa-pkg/libva-v4l2)
- License: MIT
- Copyright: Copyright (c) 2026 libva-v4l2 contributors

This derivative is released under the same MIT license; the original copyright
notice and permission notice are retained in full.
