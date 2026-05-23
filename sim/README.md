# LVGL Music UI Simulator

This host-side simulator runs the music player UI in an SDL2 window at the same
logical resolution as the device: `640x172`.

## Build

```sh
cmake -S sim -B build-sim
cmake --build build-sim --target music_ui_sim
```

SDL2 is required. On macOS:

```sh
brew install sdl2 pkg-config
```

## Run With Real MQTT

```sh
build-sim/music_ui_sim \
  --mqtt-host 192.168.31.100 \
  --mqtt-user mqtt \
  --mqtt-pass 'YOUR_MQTT_PASSWORD' \
  --topic shairport/livingroom
```

To avoid putting the password in shell history:

```sh
export SHAIRPORT_MQTT_PASSWORD='YOUR_MQTT_PASSWORD'
build-sim/music_ui_sim --mqtt-host 192.168.31.100 --mqtt-user mqtt --topic shairport/livingroom
```

## Run Offline

```sh
build-sim/music_ui_sim --offline
```

Offline mode cycles generated Chinese and English metadata without connecting to
MQTT. It is useful for layout and font checks.

## Save A Screenshot

```sh
build-sim/music_ui_sim --offline --run-ms 1200 --screenshot /tmp/music-ui.bmp
```
