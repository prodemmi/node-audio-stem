import AudioStem from '@prodemmi/audio-stem';

const audio_input = "Ebi-Goriz.mp3";
const output_dir = "./output/";

AudioStem.separate(audio_input, output_dir);