/**
 * Represents the progress of the audio separation process.
 */
export interface SeparationProgress {
  /** Progress percentage (0 to 100) */
  percentage: number;

  /** Current status message */
  message: string;
}

declare namespace AudioStem {
  /**
   * Separates audio stems (e.g., vocals, drums) from a given input file.
   *
   * @param audio_input - Path to the input audio file
   * @param output_dir - Directory where the separated stems will be saved
   * @param result_callback - Optional callback to receive progress updates
   */
  function separate(
    audio_input: string,
    output_dir: string,
    result_callback?: (progress: SeparationProgress) => void
  ): void;
}

export default AudioStem;
