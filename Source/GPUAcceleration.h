/**
 * @file GPUAcceleration.h
 * @brief GPU Acceleration support for ONNX Runtime (CUDA/DirectML)
 * 
 * This module provides infrastructure for GPU-accelerated inference using
 * CUDA (NVIDIA) or DirectML (Windows) execution providers.
 */

#pragma once

#include <JuceHeader.h>
#include <memory>
#include <vector>
#include "Utilities.h"

namespace MAEVN
{

//==============================================================================
/**
 * @brief GPU backend types
 */
enum class GPUBackend
{
    None,           ///< CPU only
    CUDA,           ///< NVIDIA CUDA
    DirectML,       ///< Windows DirectML
    CoreML,         ///< Apple CoreML
    OpenVINO,       ///< Intel OpenVINO
    TensorRT        ///< NVIDIA TensorRT
};

//==============================================================================
/**
 * @brief GPU device information
 */
struct GPUDeviceInfo
{
    juce::String name;              ///< Device name
    GPUBackend backend;             ///< Backend type
    int deviceIndex;                ///< Device index
    size_t totalMemory;             ///< Total GPU memory in bytes
    size_t freeMemory;              ///< Available GPU memory in bytes
    bool isAvailable;               ///< Whether device is usable
    juce::String driverVersion;     ///< Driver version string
    float computeCapability;        ///< CUDA compute capability (if applicable)
    
    GPUDeviceInfo()
        : backend(GPUBackend::None)
        , deviceIndex(0)
        , totalMemory(0)
        , freeMemory(0)
        , isAvailable(false)
        , computeCapability(0.0f)
    {}
};

//==============================================================================
/**
 * @brief GPU acceleration configuration
 */
struct GPUConfig
{
    GPUBackend preferredBackend;    ///< Preferred backend
    int deviceIndex;                ///< GPU device index to use
    size_t memoryLimit;             ///< Max GPU memory to use (0 = unlimited)
    bool fallbackToCPU;             ///< Fall back to CPU if GPU fails
    bool useGraphOptimization;      ///< Enable ONNX graph optimization
    bool useTensorCores;            ///< Use Tensor Cores (NVIDIA)
    int cudaStreams;                ///< Number of CUDA streams
    bool enableProfiling;           ///< Enable performance profiling
    
    GPUConfig()
        : preferredBackend(GPUBackend::None)
        , deviceIndex(0)
        , memoryLimit(0)
        , fallbackToCPU(true)
        , useGraphOptimization(true)
        , useTensorCores(true)
        , cudaStreams(1)
        , enableProfiling(false)
    {}
};

//==============================================================================
/**
 * @brief Performance metrics from GPU inference
 */
struct GPUPerformanceMetrics
{
    double inferenceTimeMs;         ///< Last inference time in milliseconds
    double averageInferenceTimeMs;  ///< Average inference time
    double minInferenceTimeMs;      ///< Minimum inference time
    double maxInferenceTimeMs;      ///< Maximum inference time
    size_t memoryUsed;              ///< GPU memory used in bytes
    int inferenceCount;             ///< Total inference count
    double gpuUtilization;          ///< GPU utilization percentage
    
    GPUPerformanceMetrics()
        : inferenceTimeMs(0.0)
        , averageInferenceTimeMs(0.0)
        , minInferenceTimeMs(std::numeric_limits<double>::max())
        , maxInferenceTimeMs(0.0)
        , memoryUsed(0)
        , inferenceCount(0)
        , gpuUtilization(0.0)
    {}
    
    void update(double newTime)
    {
        inferenceTimeMs = newTime;
        minInferenceTimeMs = std::min(minInferenceTimeMs, newTime);
        maxInferenceTimeMs = std::max(maxInferenceTimeMs, newTime);
        
        double totalTime = averageInferenceTimeMs * inferenceCount + newTime;
        inferenceCount++;
        averageInferenceTimeMs = totalTime / inferenceCount;
    }
    
    void reset()
    {
        *this = GPUPerformanceMetrics();
    }
};

//==============================================================================
/**
 * @brief Listener interface for GPU acceleration events
 */
class GPUAccelerationListener
{
public:
    virtual ~GPUAccelerationListener() = default;
    
    /**
     * @brief Called when GPU initialization completes
     */
    virtual void onGPUInitialized(bool success, const juce::String& message) = 0;
    
    /**
     * @brief Called when GPU becomes unavailable
     */
    virtual void onGPUUnavailable(const juce::String& reason) = 0;
    
    /**
     * @brief Called when falling back to CPU
     */
    virtual void onFallbackToCPU(const juce::String& reason) = 0;
    
    /**
     * @brief Called with performance metrics update
     */
    virtual void onPerformanceMetricsUpdated(const GPUPerformanceMetrics& metrics) = 0;
};

//==============================================================================
/**
 * @brief Main GPU Acceleration Manager
 * 
 * Handles GPU detection, initialization, and provides execution provider
 * configuration for ONNX Runtime.
 */
class GPUAccelerationManager
{
public:
    GPUAccelerationManager();
    ~GPUAccelerationManager();
    
    /**
     * @brief Initialize GPU acceleration
     * @param config Configuration settings
     * @return true if GPU initialization successful
     */
    bool initialize(const GPUConfig& config = GPUConfig());
    
    /**
     * @brief Check if GPU acceleration is available
     */
    bool isGPUAvailable() const { return gpuAvailable; }
    
    /**
     * @brief Check if GPU is currently active
     */
    bool isGPUActive() const { return gpuActive; }
    
    /**
     * @brief Get active backend
     */
    GPUBackend getActiveBackend() const { return activeBackend; }
    
    /**
     * @brief Get list of available GPU devices
     * @return Vector of device info structures
     */
    std::vector<GPUDeviceInfo> getAvailableDevices() const;
    
    /**
     * @brief Get info about currently active device
     */
    GPUDeviceInfo getActiveDeviceInfo() const { return activeDevice; }
    
    /**
     * @brief Get current configuration
     */
    const GPUConfig& getConfig() const { return currentConfig; }
    
    /**
     * @brief Set new configuration (may trigger re-initialization)
     * @param config New configuration
     * @return true if configuration applied successfully
     */
    bool setConfig(const GPUConfig& config);
    
    /**
     * @brief Get performance metrics
     */
    const GPUPerformanceMetrics& getPerformanceMetrics() const { return metrics; }
    
    /**
     * @brief Reset performance metrics
     */
    void resetPerformanceMetrics() { metrics.reset(); }
    
    /**
     * @brief Report an inference timing
     * @param timeMs Inference time in milliseconds
     */
    void reportInferenceTime(double timeMs);
    
    /**
     * @brief Check if a specific backend is available
     * @param backend Backend to check
     * @return true if available
     */
    bool isBackendAvailable(GPUBackend backend) const;
    
    /**
     * @brief Get recommended backend for current system
     */
    GPUBackend getRecommendedBackend() const;
    
    /**
     * @brief Force fallback to CPU
     */
    void fallbackToCPU();
    
    /**
     * @brief Attempt to recover GPU acceleration
     * @return true if GPU recovered
     */
    bool recoverGPU();
    
    /**
     * @brief Add a listener
     */
    void addListener(GPUAccelerationListener* listener);
    
    /**
     * @brief Remove a listener
     */
    void removeListener(GPUAccelerationListener* listener);
    
    /**
     * @brief Get backend name as string
     */
    static juce::String getBackendName(GPUBackend backend);
    
    /**
     * @brief Get session options for ONNX Runtime with GPU
     * Note: In actual implementation, this would return Ort::SessionOptions
     * @return Configuration string for logging/debugging
     */
    juce::String getOnnxSessionOptionsDescription() const;
    
    /**
     * @brief Estimate GPU memory required for a model
     * @param modelSizeBytes Model file size in bytes
     * @return Estimated GPU memory requirement
     */
    size_t estimateMemoryRequirement(size_t modelSizeBytes) const;
    
    /**
     * @brief Check if enough GPU memory is available
     * @param requiredBytes Required memory in bytes
     * @return true if enough memory available
     */
    bool hasEnoughMemory(size_t requiredBytes) const;
    
    /**
     * @brief Run GPU benchmark
     * @return Benchmark score (higher is better)
     */
    float runBenchmark();
    
private:
    GPUConfig currentConfig;
    GPUDeviceInfo activeDevice;
    GPUBackend activeBackend;
    GPUPerformanceMetrics metrics;
    
    bool gpuAvailable;
    bool gpuActive;
    
    std::vector<GPUDeviceInfo> availableDevices;
    std::vector<GPUAccelerationListener*> listeners;
    
    juce::CriticalSection gpuLock;
    
    /**
     * @brief Detect available GPU devices
     */
    void detectDevices();
    
    /**
     * @brief Initialize CUDA backend
     */
    bool initializeCUDA(int deviceIndex);
    
    /**
     * @brief Initialize DirectML backend
     */
    bool initializeDirectML(int deviceIndex);
    
    /**
     * @brief Initialize CoreML backend
     */
    bool initializeCoreML();
    
    /**
     * @brief Notify listeners of initialization result
     */
    void notifyInitialized(bool success, const juce::String& message);
    
    /**
     * @brief Notify listeners of GPU unavailability
     */
    void notifyUnavailable(const juce::String& reason);
    
    /**
     * @brief Notify listeners of CPU fallback
     */
    void notifyFallback(const juce::String& reason);
    
    /**
     * @brief Update performance metrics
     */
    void updateMetrics();
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GPUAccelerationManager)
};

//==============================================================================
/**
 * @brief GPU acceleration settings UI component
 */
class GPUSettingsComponent : public juce::Component
{
public:
    GPUSettingsComponent(GPUAccelerationManager* manager);
    ~GPUSettingsComponent() override;
    
    void paint(juce::Graphics& g) override;
    void resized() override;
    
    /**
     * @brief Refresh display
     */
    void refresh();
    
private:
    GPUAccelerationManager* gpuManager;
    
    juce::Label titleLabel;
    juce::Label statusLabel;
    juce::Label deviceLabel;
    juce::Label metricsLabel;
    
    juce::ComboBox backendSelector;
    juce::ComboBox deviceSelector;
    
    juce::ToggleButton enableGPUButton;
    juce::ToggleButton fallbackButton;
    juce::ToggleButton graphOptButton;
    
    juce::TextButton applyButton;
    juce::TextButton benchmarkButton;
    
    void setupUI();
    void updateStatus();
    void onApplyClicked();
    void onBenchmarkClicked();
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GPUSettingsComponent)
};

} // namespace MAEVN
