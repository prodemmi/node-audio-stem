const AudioStem = require('@prodemmi/audio-stem');
const cliProgress = require('cli-progress');

const audio_input = "Ebi-Goriz.mp3";
const output_dir = "./output/";

const loadingBar = new cliProgress.SingleBar({
    format: 'Audio Separation {bar} {percentage}% | Remain Time: {eta_formatted} | Step: {step}',
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
    } else {
        loadingBar.update(progress);
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