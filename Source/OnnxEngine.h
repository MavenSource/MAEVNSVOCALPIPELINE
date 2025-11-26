/**
 * @file OnnxEngine.h
 * @brief ONNX Runtime wrapper for AI model inference
 * 
 * This module provides a high-performance interface to ONNX Runtime for
 * loading and executing AI models in real-time audio processing context.
 * Supports hot-reloading, thread-safe inference, and multiple concurrent models.
 */

#pragma once

#include <JuceHeader.h>
#include <onnxruntime_cxx_api.h>
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include "Utilities.h"

namespace MAEVN
{

//==============================================================================
/**
 * @brief ONNX model wrapper with inference capabilities
 */
class OnnxModel
{
public:
    OnnxModel();
    ~OnnxModel();
    
    /**
     * @brief Load ONNX model from file
     * @param modelPath Full path to .onnx file
     * @return true if loaded successfully
     */
    bool loadModel(const juce::String& modelPath);
    
    /**
     * @brief Run inference on input data
     * @param inputData Input tensor data
     * @param inputShape Shape of input tensor
     * @param outputData Output tensor data (allocated by caller)
     * @return true if inference succeeded
     */
    bool runInference(const std::vector<float>& inputData,
                     const std::vector<int64_t>& inputShape,
                     std::vector<float>& outputData);
    
    /**
     * @brief Check if model is loaded and ready
     */
    bool isReady() const { return modelLoaded; }
    
    /**
     * @brief Get model input shape
     */
    std::vector<int64_t> getInputShape() const { return inputShape; }
    
    /**
     * @brief Get model output shape
     */
    std::vector<int64_t> getOutputShape() const { return outputShape; }
    
    /**
     * @brief Unload model and free resources
     */
    void unload();
    
private:
    std::unique_ptr<Ort::Env> environment;
    std::unique_ptr<Ort::Session> session;
    std::unique_ptr<Ort::SessionOptions> sessionOptions;
    
    std::vector<const char*> inputNames;
    std::vector<const char*> outputNames;
    std::vector<int64_t> inputShape;
    std::vector<int64_t> outputShape;
    
    bool modelLoaded;
    juce::CriticalSection modelLock;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OnnxModel)
};

//==============================================================================
/**
 * @brief Main ONNX Engine - manages multiple models and provides inference
 */
class OnnxEngine
{
public:
    OnnxEngine();
    ~OnnxEngine();
    
    /**
     * @brief Initialize ONNX Runtime environment
     * @return true if initialization succeeded
     */
    bool initialize();
    
    /**
     * @brief Load a model with a specific role identifier
     * @param role Model identifier (e.g., "808", "vocal_tts", "piano")
     * @param modelPath Path to .onnx file
     * @return true if loaded successfully
     */
    bool loadModel(const juce::String& role, const juce::String& modelPath);
    
    /**
     * @brief Run inference using a specific model
     * @param role Model identifier
     * @param inputData Input tensor data
     * @param inputShape Shape of input tensor
     * @param outputData Output tensor data
     * @return true if inference succeeded
     */
    bool runInference(const juce::String& role,
                     const std::vector<float>& inputData,
                     const std::vector<int64_t>& inputShape,
                     std::vector<float>& outputData);
    
    /**
     * @brief Load all models from config file
     * @param configPath Path to config.json
     * @return Number of models loaded
     */
    int loadModelsFromConfig(const juce::String& configPath);
    
    /**
     * @brief Hot reload a specific model
     * @param role Model identifier to reload
     * @return true if reloaded successfully
     */
    bool reloadModel(const juce::String& role);
    
    /**
     * @brief Check if a model is loaded and ready
     * @param role Model identifier
     */
    bool isModelReady(const juce::String& role) const;
    
    /**
     * @brief Unload a specific model
     * @param role Model identifier
     */
    void unloadModel(const juce::String& role);
    
    /**
     * @brief Unload all models
     */
    void unloadAllModels();
    
    /**
     * @brief Get list of loaded model roles
     */
    juce::StringArray getLoadedModels() const;
    
    /**
     * @brief Enable/disable GPU acceleration if available
     */
    void setUseGPU(bool useGPU);
    
private:
    std::unordered_map<juce::String, std::unique_ptr<OnnxModel>> models;
    std::unordered_map<juce::String, juce::String> modelPaths; // for hot reloading
    
    bool initialized;
    bool useGPU;
    mutable juce::CriticalSection engineLock;
    
    /**
     * @brief Parse config.json to get model paths
     */
    juce::var parseConfigFile(const juce::String& configPath);
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OnnxEngine)
};

} // namespace MAEVN
