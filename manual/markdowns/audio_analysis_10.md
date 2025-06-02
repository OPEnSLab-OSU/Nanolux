
## Audio Processing Overview

The `AudioAnalysis` class samples the audio input at a fixed rate and buffer size, applies DC removal and a Hamming window, then performs a Fast Fourier Transform (FFT) to convert time-domain samples into frequency-domain magnitudes. After noise gating, these magnitudes are passed to `AudioPrism` modules and custom analysis functions that extract various audio features, such as peak frequency, volume, spectral deltas, and more. Internally, only the necessary analysis routines are run once per loop, and results are cached to make the pattern code both simple and efficient.

We recommend familiarizing yourself with FFTs, audio sampling fundamentals, and the [AudioPrism](https://github.com/udellc/AudioPrism) library before diving into this project.

## File Structure

```text
main/
├── audio_analysis.cpp   # Implements AudioAnalysis: sampling → FFT → AudioPrism modules → caching
├── audio_analysis.h     # Declares AudioAnalysis public API (processAudioFrame, getters, resetCache)
├── nanolux_util.h       # Main loop timing constants and helper functions
└── nanolux_types.h      # Hardware constants & helpers:
                         #   • ANALOG_PIN, SAMPLES, SAMPLING_FREQUENCY  
                         #   • MIN_FREQUENCY, MAX_FREQUENCY  
                         #   • map(), timing, etc.
```

## Accessing Audio Features

Call **`audioAnalysis.processAudioFrame()`** at the start of each loop to sample and FFT the latest audio frame, then use any combination of getters. Results are cached on first access each loop; call **`audioAnalysis.resetCache()`** at the end of your loop to clear all flags.

| Getter                         | Return Type | Description                                   |
| ------------------------------ | ----------- | --------------------------------------------- |
| `getVReal()`                   | `float*`    | Raw magnitude spectrum `[0…SAMPLES-1]`        |
| `getVReal(float alpha)`        | `float*`    | Smoothed magnitude spectrum with alpha factor |
| `getPeak()`                    | `float`     | Dominant frequency in Hz                      |
| `getVolume()`                  | `float`     | Mean amplitude                                |
| `getMaxDelta()`                | `int`       | Largest spectral change between loops         |
| `getDeltas()`                  | `float*`    | Spectral deltas per bin `[0…SAMPLES-1]`       |
| `getSalientFreqs()`            | `int*`      | Top 3 salient frequency bin indices           |
| `getCentroid()`                | `float`     | Spectral centroid                             |
| `getPercussionPresence()`      | `bool`      | `true` if percussion is detected              |
| `getNoisiness()`               | `float`     | Noise metric                                  |
| `getFiveBandSplit(int len)`    | `float*`    | Five-band energy split, mapped to `len` LEDs  |

## Adding a New Analysis Function

To add a new feature extraction or analysis routine, follow these steps:

1. **In `audio_analysis.h`**, add your result storage and update flag:
    ```cpp
    bool   myFeatureUpdated = false;
    MyType myFeature        = {};
    ```

2. **In `audio_analysis.cpp`**, implement the analysis routine and its getter:
    ```cpp
    void AudioAnalysis::updateMyFeature() {
      // compute myFeature from vReal or other data
    }

    MyType AudioAnalysis::getMyFeature() {
      if (!myFeatureUpdated) {
        updateMyFeature();
        myFeatureUpdated = true;
      }
      return myFeature;
    }
    ```

3. **In `resetCache()`**, clear your flag:
    ```cpp
    void AudioAnalysis::resetCache() {
      // existing flags...
      myFeatureUpdated = false;
    }
    ```

This pattern ensures that each analysis runs at most once per loop and provides a clean, consistent interface for patterns.

## Example Usage

```cpp
void loop() {
  // 1) Sample and FFT the current audio frame
  audioAnalysis.processAudioFrame(noiseThreshhold);

  // 2) Run your pattern code, calling any getters:
  float*   spectrum   = audioAnalysis.getVReal();
  float    peakFreq   = audioAnalysis.getPeak();
  float    vol        = audioAnalysis.getVolume();
  int      maxDelta   = audioAnalysis.getMaxDelta();
  float*   deltas     = audioAnalysis.getDeltas();
  int*     salients   = audioAnalysis.getSalientFreqs();
  float    centroid   = audioAnalysis.getCentroid();
  bool     percussion = audioAnalysis.getPercussionPresence();
  float    noise      = audioAnalysis.getNoisiness();
  float*   bands      = audioAnalysis.getFiveBandSplit(LED_COUNT);

  // ... drive LEDs based on those features ...

  // 3) Reset all cached flags for the next loop iteration
  audioAnalysis.resetCache();
}
```


