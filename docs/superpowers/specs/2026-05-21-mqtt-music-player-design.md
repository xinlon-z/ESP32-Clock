# MQTT Music Player UI Design

## Goal

Add a second device UI for Shairport Sync playback while keeping the clock as the default screen.

The device starts on the existing clock face. A left swipe switches to a music player screen. A right swipe switches back to the clock.

## Visual Direction

Use the approved Liquid Glass mockup as the target:

- Album cover is the primary visual element.
- A dark, cover-influenced background sits behind the content.
- The information area uses a translucent glass panel with subtle highlight edges.
- Controls use circular glass buttons for previous, play/pause, and next.
- The progress bar is minimal, with elapsed and duration labels.

On ESP32/LVGL, real-time blur is not required. Approximate the effect with a dark cover-derived background, translucent panels, fine border highlights, and soft shadows where cheap enough.

## Navigation

The screen manager owns two screens:

- `ClockFaceScreen`
- `MusicPlayerScreen`

Initial screen:

- `ClockFaceScreen`

Gestures:

- Left swipe on the clock screen switches to music.
- Right swipe on the music screen switches to clock.
- Short taps and small drags do not switch screens.

Gesture detection should use LVGL pointer events or the existing touch read path, with a horizontal threshold large enough to avoid accidental switches.

## Music Data Model

The music screen should render from a local state object, not directly from MQTT callbacks.

Fields:

- `active`
- `playing`
- `title`
- `artist`
- `album`
- `cover`
- `volume`
- `progress_start`
- `progress_current`
- `progress_end`
- `last_progress_update_ms`

MQTT topics from Shairport Sync:

- `shairport/livingroom/title`
- `shairport/livingroom/artist`
- `shairport/livingroom/album`
- `shairport/livingroom/cover`
- `shairport/livingroom/active`
- `shairport/livingroom/playing`
- `shairport/livingroom/volume`
- `shairport/livingroom/ssnc/prgr`

Progress calculation:

```text
elapsed_seconds = (current - start) / 44100
duration_seconds = (end - start) / 44100
progress_percent = (current - start) / (end - start)
```

Because `ssnc/prgr` is event-driven rather than a guaranteed one-second tick, the UI can locally estimate current progress while `playing = 1`.

## Controls

Buttons map to Shairport Sync remote commands:

- Previous: publish `previtem` to `shairport/livingroom/remote`
- Play/pause: publish `playpause` to `shairport/livingroom/remote`
- Next: publish `nextitem` to `shairport/livingroom/remote`

The first implementation can render the controls before wiring MQTT publish, but the button layout must reserve the final three-button structure.

## Implementation Scope

First firmware pass:

- Add screen switching infrastructure.
- Add `MusicPlayerScreen` with static placeholder data matching the approved Liquid Glass layout.
- Keep clock as default.
- Implement left/right swipe navigation.

Second pass:

- Add MQTT client state ingestion.
- Render real title, artist, album, progress, playing state, and volume.
- Add cover image pipeline.
- Wire remote control commands.

## Acceptance Criteria

- Device boots to the existing clock UI.
- Left swipe switches to the music player.
- Right swipe switches back to the clock.
- Existing power dimming and clock updates continue to work.
- Music screen visually matches the approved Liquid Glass direction within LVGL constraints.
- Music screen layout fits the 640x172 display without text overlap.
