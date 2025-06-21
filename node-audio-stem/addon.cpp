#include "dsp.hpp"
#include "model.hpp"
#include "tensor.hpp"
#include <Eigen/Core>
#include <Eigen/Dense>
#include <cassert>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <libnyquist/Common.h>
#include <libnyquist/Decoders.h>
#include <libnyquist/Encoders.h>
#include <napi.h>
#include <sstream>
#include <string>
#include <unsupported/Eigen/FFT>
#include <vector>

using namespace demucscpp;
using namespace nqr;

static Eigen::MatrixXf load_audio_file(std::string filename)
{
    // load a wav file with libnyquist
    std::shared_ptr<AudioData> fileData = std::make_shared<AudioData>();

    NyquistIO loader;

    loader.Load(fileData.get(), filename);

    if (fileData->sampleRate != demucscpp::SUPPORTED_SAMPLE_RATE)
    {
        std::cerr << "[ERROR] demucs.cpp only supports the following sample "
                     "rate (Hz): "
                  << SUPPORTED_SAMPLE_RATE << std::endl;
        exit(1);
    }

    if (fileData->channelCount != 2 && fileData->channelCount != 1)
    {
        std::cerr << "[ERROR] demucs.cpp only supports mono and stereo audio"
                  << std::endl;
        exit(1);
    }

    // number of samples per channel
    size_t N = fileData->samples.size() / fileData->channelCount;

    // create a struct to hold two float vectors for left and right channels
    Eigen::MatrixXf ret(2, N);

    if (fileData->channelCount == 1)
    {
        // Mono case
        for (size_t i = 0; i < N; ++i)
        {
            ret(0, i) = fileData->samples[i]; // left channel
            ret(1, i) = fileData->samples[i]; // right channel
        }
    }
    else
    {
        // Stereo case
        for (size_t i = 0; i < N; ++i)
        {
            ret(0, i) = fileData->samples[2 * i];     // left channel
            ret(1, i) = fileData->samples[2 * i + 1]; // right channel
        }
    }

    return ret;
}

// write a function to write a StereoWaveform to a wav file
static void write_audio_file(const Eigen::MatrixXf &waveform,
                             std::string filename)
{
    // create a struct to hold the audio data
    std::shared_ptr<AudioData> fileData = std::make_shared<AudioData>();

    // set the sample rate
    fileData->sampleRate = SUPPORTED_SAMPLE_RATE;

    // set the number of channels
    fileData->channelCount = 2;

    // set the number of samples
    fileData->samples.resize(waveform.cols() * 2);

    // write the left channel
    for (long int i = 0; i < waveform.cols(); ++i)
    {
        fileData->samples[2 * i] = waveform(0, i);
        fileData->samples[2 * i + 1] = waveform(1, i);
    }
}

// Replace the Separate function with this updated version
Napi::Boolean Separate(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();
    
    // Check if we have at least 2 arguments
    if (info.Length() < 2) {
        Napi::TypeError::New(env, "Expected at least 2 arguments").ThrowAsJavaScriptException();
        return Napi::Boolean::New(env, false);
    }
    
    std::string input_path = info[0].As<Napi::String>();
    std::string output_dir = info[1].As<Napi::String>();
    
    // Optional progress callback (3rd argument)
    Napi::Function progressCallback;
    bool hasCallback = false;
    
    if (info.Length() >= 3 && info[2].IsFunction()) {
        progressCallback = info[2].As<Napi::Function>();
        hasCallback = true;
    }

    // load audio passed as argument
    std::string wav_file = input_path;

    // output dir passed as argument
    std::string out_dir = output_dir;

    Eigen::MatrixXf audio = load_audio_file(wav_file);
    Eigen::Tensor3dXf out_targets;

    // initialize a struct demucs_model
    struct demucs_model model{};

    std::string modelPath = "node_modules/@prodemmi/audio-stem/models/ggml-model-htdemucs-4s-f16.bin";

    // debug some members of model
    auto ret = load_demucs_model(modelPath.c_str(), &model);
    if (!ret)
    {
        std::cerr << "Error loading model" << std::endl;
        Napi::Error::New(env, "Error loading model").ThrowAsJavaScriptException();
        return Napi::Boolean::New(env, false);
    }

    // Create progress callback that calls JavaScript function
    demucscpp::ProgressCallback cppProgressCallback = nullptr;
    
    if (hasCallback) {
        cppProgressCallback = [&env, &progressCallback](float progress, const std::string &log_message) {
            // Convert progress to percentage (0.0-100.0) as float
            float percentage = progress * 100.0f;
            
            // Call the JavaScript callback with progress info
            // Check if env is still valid before calling
            if (!env.IsExceptionPending()) {
                // Create an object with progress details
                Napi::Object progressObj = Napi::Object::New(env);
                progressObj.Set("percentage", Napi::Number::New(env, percentage));
                progressObj.Set("message", Napi::String::New(env, log_message));
                
                progressCallback.Call({progressObj});
            }
        };
    } else {
        // Default callback that prints to console
        cppProgressCallback = [](float progress, const std::string &log_message) {
            float percentage = progress * 100.0f;
            std::cout << std::setw(3) << std::setfill(' ')
                      << percentage << "% " << log_message << std::endl;
        };
    }

    // create 4 audio matrix same size, to hold output
    Eigen::Tensor3dXf audio_targets =
        demucscpp::demucs_inference(model, audio, cppProgressCallback);

    out_targets = audio_targets;

    int nb_out_sources = model.is_4sources ? 4 : 6;

    for (int target = 0; target < nb_out_sources; ++target)
    {
        // now write the 4 audio waveforms to files in the output dir
        // using libnyquist
        // join out_dir with "/target_0.wav"
        // using std::filesystem::path;

        std::filesystem::path p = out_dir;
        // make sure the directory exists
        std::filesystem::create_directories(p);

        auto p_target = p / "target_0.wav";

        // target 0,1,2,3 map to drums,bass,other,vocals

        std::string target_name;

        switch (target)
        {
        case 0:
            target_name = "drums";
            break;
        case 1:
            target_name = "bass";
            break;
        case 2:
            target_name = "other";
            break;
        case 3:
            target_name = "vocals";
            break;
        case 4:
            target_name = "guitar";
            break;
        case 5:
            target_name = "piano";
            break;
        default:
            std::cerr << "Error: target " << target << " not supported"
                      << std::endl;
            Napi::Error::New(env, "Unsupported target").ThrowAsJavaScriptException();
            return Napi::Boolean::New(env, false);
        }

        // insert target_name into the path after the digit
        // e.g. target_name_0_drums.wav
        p_target.replace_filename("target_" + std::to_string(target) + "_" +
                                  target_name + ".wav");

        Eigen::MatrixXf target_waveform(2, audio.cols());

        // copy the input stereo wav file into all 4 targets
        for (int channel = 0; channel < 2; ++channel)
        {
            for (int sample = 0; sample < audio.cols(); ++sample)
            {
                target_waveform(channel, sample) =
                    out_targets(target, channel, sample);
            }
        }

        write_audio_file(target_waveform, p_target);
    }

    return Napi::Boolean::New(env, true);
}

Napi::Object Init(Napi::Env env, Napi::Object exports)
{
    exports.Set("separate", Napi::Function::New(env, Separate));
    return exports;
}

NODE_API_MODULE(demucs_addon, Init)