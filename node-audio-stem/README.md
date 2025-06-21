# @prodemmi/audio-stem

> Native Node.js addon for audio stem separation (e.g. vocals, guitars, drums) using [demucs.cpp](http://github.com/sevagh/demucs.cpp) project.

> ⚠️ **Currently supports only Linux devices.**
> 
## Features

- Separate audio files into individual stems (vocals, drums, guitars, etc.)
- High performance via native C++ addon
- Simple API with optional progress callback

---

## Installation

```bash
npm install @prodemmi/audio-stem
# or
yarn add @prodemmi/audio-stem
```

## Examples

### Simple Usage:
```js
const AudioStem = require("@prodemmi/audio-stem");

const audio_input = "Ebi-Goriz.mp3";
const output_dir = "./output/";

AudioStem.separate(audio_input, output_dir);
```

### Using with Progress Bar:
```js
const AudioStem = require("@prodemmi/audio-stem");
const cliProgress = require('cli-progress');

const audio_input = "Ebi-Goriz.mp3";
const output_dir = "./output/";

const loadingBar = new cliProgress.SingleBar({
    format: 'Audio Separation |{bar}| {percentage}% | ETA: {eta_formatted} | Step: {step}',
    barCompleteChar: '█',
    barIncompleteChar: '░',
    hideCursor: true,
    etaAsynchronousUpdate: true,
    etaBuffer: 10,
    fps: 10
}, cliProgress.Presets.shades_classic);

loadingBar.start(100.0, 0.0, { step: 'Initializing...' });

AudioStem.separate(audio_input, output_dir, function (progressInfo) {
    let progress, message;

    if (typeof progressInfo === 'object') {
        progress = progressInfo.percentage;
        message = progressInfo.message || 'Processing...';
    } else {
        progress = progressInfo;
        message = 'Processing...';
    }

    if (progress >= 99.9) {
        loadingBar.update(100.0, { step: 'Finalizing...' });

        setTimeout(() => {
            loadingBar.stop();

            console.log(`\n✅ Audio separation completed!`);
            console.log('Output files saved to %s', output_dir);

            const fs = require('fs');

            try {
                const files = fs.readdirSync(output_dir)
                    .filter(file => file.endsWith('.wav'))
                    .map(file => `  📁 ${file}`)
                    .join('\n');

                if (files) {
                    console.log('\nGenerated files:');
                    console.log(files);
                }
            } catch (err) {
            }
        }, 100);
    }
});

process.on('SIGINT', () => {
    loadingBar.stop();
    console.log('\n❌ Process interrupted');
    process.exit(1);
});

process.on('uncaughtException', (err) => {
    loadingBar.stop();
    console.error('\n❌ Error during audio separation:', err.message);
    process.exit(1);
});
```