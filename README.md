# qcom-va-driver（Venus 适配版）

[English](README.en.md)

通过 Linux V4L2 stateful M2M 接口，为 Qualcomm Venus（SM8250）提供
VA-API 硬件编解码。

## 关于

衍生自 [radxa-pkg/qcom-va-driver](https://github.com/radxa-pkg/qcom-va-driver)
1.1.1。原项目面向 Qualcomm Iris（SC8280XP），本版本针对 SM8250 的 Venus
做了 6 处适配（驱动名匹配、capture pool 大小、控制项兼容性等），
并新增了 VP8 解码与编码支持。

## 支持

| 编解码器 | 解码 | 编码 |
| --- | --- | --- |
| H.264 / AVC | Constrained Baseline, Main, High (8-bit) | 同左 (8-bit) |
| H.265 / HEVC | Main (8-bit), Main10 (10-bit) | Main (8-bit) |
| VP9 | Profile 0, 2 (8/10-bit) | — |
| VP8 | Version 0.3 (8-bit) | Version 0.3 (8-bit) |

- 仅支持逐行扫描 4:2:0
- 无 VPP：不支持缩放、去隔行、色彩转换
- 不支持 AV1、MPEG-2（VA-API 未暴露对应 profile）

## 安装

使用 CI 构建的 Debian 包：

```bash
sudo dpkg -i qcom-va-driver_1.1.1-venus4_arm64.deb
```

或从源码构建后直接安装（见下节）：

```bash
sudo ninja -C build install
```

驱动安装到 `/usr/lib/aarch64-linux-gnu/dri/`，由 libva 自动加载，
无需设置 `LIBVA_DRIVER_NAME`。

卸载：

```bash
sudo dpkg -r qcom-va-driver
```

## 构建

```bash
sudo apt install meson ninja-build libva-dev libdrm-dev \
                  libegl-dev libgles-dev libgbm-dev
meson setup build -Dprefix=/usr -Dcdsp_vpp=disabled -Dvulkan_vpp=disabled
ninja -C build
sudo ninja -C build install
```

缺少 `libegl-dev` / `libgles-dev` / `libgbm-dev` 时，`gpu_copy` 会被自动禁用，
零拷贝不可用。

## 验证

```bash
vainfo
```

应列出 H.264 / HEVC / VP9 的 `VAEntrypointVLD`，以及 H.264 / HEVC 的
`VAEntrypointEncSlice`。

## 实测环境

| 项目 | 值 |
| --- | --- |
| 设备 | Xiaomi Mi Pad 5 Pro（代号 elish） |
| SoC | Qualcomm Snapdragon 870（SM8250），Venus 编解码器 |
| 系统 | Armbian 26.8.14（Ubuntu 24.04 noble） |
| 内核 | 6.18.44-current-sm8250 |
| VA-API | libva 1.20.0 |
| 测试程序 | Firefox 159.0.1、ffmpeg 6.1.1、mpv 0.37.0 |

## 实测数据

| 项目 | 结果 |
| --- | --- |
| 解码 1080p H.264 | 用户态 0.31 s / 3 s 素材 |
| 解码 4K HEVC | 用户态 1.29 s / 3 s 素材（软解 7.75 s） |
| 编码 1080p H.264 | 90 帧 / 0.50 s |
| 编码 4K HEVC | 60 帧 / 0.80 s |
| 解码 4K VP8 | 用户态 0.15 s / 5 s 素材（软解 2.46 s） |
| 编码 4K VP8 | 60 帧 / 1.63 s |
| Firefox 播放 4K HEVC | RDD 进程约 4.6% CPU |
| mpv 零拷贝 | `--hwdec=vaapi`，`VO: [gpu] vaapi[nv12]` |

## Firefox

将 `firefox-user.js` 放入 Firefox 配置目录，或在 `about:config` 中设置：

```js
user_pref("media.ffmpeg.vaapi.enabled", true);
```

重启后 `about:support` → 媒体 → 硬件视频解码，应显示为启用。

## 注意

ffmpeg 自带的 `v4l2m2m` 编解码器与本硬件不兼容（`VIDIOC_G_FMT` 失败）。
请改用 VA-API（`-hwaccel vaapi`、`h264_vaapi` / `hevc_vaapi`）或
GStreamer 的 `v4l2h264dec` / `v4l2h265dec`。

## 上游与致谢

- 原始项目：[radxa-pkg/qcom-va-driver](https://github.com/radxa-pkg/qcom-va-driver) 1.1.1
- 源码获取：[radxa-pkg/libva-v4l2](https://github.com/radxa-pkg/libva-v4l2)
- 许可证：MIT
- 版权：Copyright (c) 2026 libva-v4l2 contributors
- 适配、调试与文档：DeepSeek AI 助手（2026-11）

本衍生版本同样以 MIT 协议发布，原始版权声明与许可条款完整保留。
