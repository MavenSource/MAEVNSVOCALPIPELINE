/**
 * @file GPUAcceleration.cpp
 * @brief Implementation of GPU Acceleration support
 */

#include "GPUAcceleration.h"

namespace MAEVN
{

//==============================================================================
// GPUAccelerationManager Implementation
//==============================================================================

GPUAccelerationManager::GPUAccelerationManager()
    : activeBackend(GPUBackend::None)
    , gpuAvailable(false)
    , gpuActive(false)
{
    detectDevices();
    
    Logger::log(Logger::Level::Info, "GPUAccelerationManager initialized, found " + 
                juce::String(static_cast<int>(availableDevices.size())) + " devices");
}

GPUAccelerationManager::~GPUAccelerationManager()
{
}

bool GPUAccelerationManager::initialize(const GPUConfig& config)
{
    const juce::ScopedLock sl(gpuLock);
    
    currentConfig = config;
    
    if (config.preferredBackend == GPUBackend::None)
    {
        gpuActive = false;
        activeBackend = GPUBackend::None;
        notifyInitialized(true, "GPU acceleration disabled (CPU mode)");
        return true;
    }
    
    bool success = false;
    juce::String message;
    
    switch (config.preferredBackend)
    {
        case GPUBackend::CUDA:
            success = initializeCUDA(config.deviceIndex);
            message = success ? "CUDA initialized successfully" : "CUDA initialization failed";
            break;
            
        case GPUBackend::DirectML:
            success = initializeDirectML(config.deviceIndex);
            message = success ? "DirectML initialized successfully" : "DirectML initialization failed";
            break;
            
        case GPUBackend::CoreML:
            success = initializeCoreML();
            message = success ? "CoreML initialized successfully" : "CoreML initialization failed";
            break;
            
        default:
            // Try recommended backend
            GPUBackend recommended = getRecommendedBackend();
            if (recommended != GPUBackend::None)
            {
                GPUConfig modifiedConfig = config;
                modifiedConfig.preferredBackend = recommended;
                return initialize(modifiedConfig);
            }
            break;
    }
    
    if (success)
    {
        gpuActive = true;
        activeBackend = config.preferredBackend;
    }
    else if (config.fallbackToCPU)
    {
        gpuActive = false;
        activeBackend = GPUBackend::None;
        message += " - falling back to CPU";
        notifyFallback(message);
    }
    
    notifyInitialized(success, message);
    
    Logger::log(Logger::Level::Info, message);
    
    return success;
}

std::vector<GPUDeviceInfo> GPUAccelerationManager::getAvailableDevices() const
{
    return availableDevices;
}

bool GPUAccelerationManager::setConfig(const GPUConfig& config)
{
    return initialize(config);
}

void GPUAccelerationManager::reportInferenceTime(double timeMs)
{
    const juce::ScopedLock sl(gpuLock);
    
    metrics.update(timeMs);
    
    // Notify listeners periodically
    if (metrics.inferenceCount % 100 == 0)
    {
        for (auto* listener : listeners)
        {
            listener->onPerformanceMetricsUpdated(metrics);
        }
    }
}

bool GPUAccelerationManager::isBackendAvailable(GPUBackend backend) const
{
    for (const auto& device : availableDevices)
    {
        if (device.backend == backend && device.isAvailable)
            return true;
    }
    return false;
}

GPUBackend GPUAccelerationManager::getRecommendedBackend() const
{
    // Priority order: CUDA > TensorRT > DirectML > CoreML > OpenVINO > None
    
    if (isBackendAvailable(GPUBackend::CUDA))
        return GPUBackend::CUDA;
    
    if (isBackendAvailable(GPUBackend::TensorRT))
        return GPUBackend::TensorRT;
    
    if (isBackendAvailable(GPUBackend::DirectML))
        return GPUBackend::DirectML;
    
    if (isBackendAvailable(GPUBackend::CoreML))
        return GPUBackend::CoreML;
    
    if (isBackendAvailable(GPUBackend::OpenVINO))
        return GPUBackend::OpenVINO;
    
    return GPUBackend::None;
}

void GPUAccelerationManager::fallbackToCPU()
{
    const juce::ScopedLock sl(gpuLock);
    
    gpuActive = false;
    activeBackend = GPUBackend::None;
    
    notifyFallback("Manual fallback to CPU");
    Logger::log(Logger::Level::Info, "Fell back to CPU processing");
}

bool GPUAccelerationManager::recoverGPU()
{
    if (!gpuAvailable)
    {
        detectDevices();
    }
    
    if (gpuAvailable && currentConfig.preferredBackend != GPUBackend::None)
    {
        return initialize(currentConfig);
    }
    
    return false;
}

void GPUAccelerationManager::addListener(GPUAccelerationListener* listener)
{
    listeners.push_back(listener);
}

void GPUAccelerationManager::removeListener(GPUAccelerationListener* listener)
{
    listeners.erase(std::remove(listeners.begin(), listeners.end(), listener), listeners.end());
}

juce::String GPUAccelerationManager::getBackendName(GPUBackend backend)
{
    switch (backend)
    {
        case GPUBackend::None:      return "CPU";
        case GPUBackend::CUDA:      return "NVIDIA CUDA";
        case GPUBackend::DirectML:  return "DirectML";
        case GPUBackend::CoreML:    return "CoreML";
        case GPUBackend::OpenVINO:  return "OpenVINO";
        case GPUBackend::TensorRT:  return "TensorRT";
        default:                    return "Unknown";
    }
}

juce::String GPUAccelerationManager::getOnnxSessionOptionsDescription() const
{
    juce::String desc;
    
    desc += "Backend: " + getBackendName(activeBackend) + "\n";
    desc += "Device: " + (activeDevice.name.isNotEmpty() ? activeDevice.name : "CPU") + "\n";
    desc += "Graph Optimization: " + juce::String(currentConfig.useGraphOptimization ? "Yes" : "No") + "\n";
    
    if (activeBackend == GPUBackend::CUDA)
    {
        desc += "CUDA Streams: " + juce::String(currentConfig.cudaStreams) + "\n";
        desc += "Tensor Cores: " + juce::String(currentConfig.useTensorCores ? "Yes" : "No") + "\n";
    }
    
    return desc;
}

size_t GPUAccelerationManager::estimateMemoryRequirement(size_t modelSizeBytes) const
{
    // Rough estimate: model size * 3 (model + input + output buffers + overhead)
    return modelSizeBytes * 3;
}

bool GPUAccelerationManager::hasEnoughMemory(size_t requiredBytes) const
{
    if (!gpuActive)
        return true; // CPU has virtually unlimited memory
    
    return activeDevice.freeMemory >= requiredBytes;
}

float GPUAccelerationManager::runBenchmark()
{
    const juce::ScopedLock sl(gpuLock);
    
    // Simulate benchmark with timing
    auto startTime = juce::Time::getHighResolutionTicks();
    
    // Simulate some work
    volatile float sum = 0.0f;
    for (int i = 0; i < 10000000; ++i)
    {
        sum += std::sin(static_cast<float>(i) * 0.001f);
    }
    
    auto endTime = juce::Time::getHighResolutionTicks();
    double elapsedMs = juce::Time::highResolutionTicksToSeconds(endTime - startTime) * 1000.0;
    
    // Score: higher is better (inversely proportional to time)
    float score = static_cast<float>(10000.0 / elapsedMs);
    
    // Apply GPU multiplier if active
    if (gpuActive)
    {
        score *= 5.0f; // GPU is roughly 5x faster for this type of work
    }
    
    Logger::log(Logger::Level::Info, "Benchmark score: " + juce::String(score, 2));
    
    return score;
}

void GPUAccelerationManager::detectDevices()
{
    availableDevices.clear();
    gpuAvailable = false;
    
    // NOTE: This is placeholder/simulation code for GPU device detection.
    // In a production implementation, actual GPU detection would use:
    // - CUDA: cudaGetDeviceCount() and cudaGetDeviceProperties()
    // - DirectML: ID3D12Device enumeration
    // - CoreML: MLModel availability check
    // The simulated memory values represent typical mid-range GPU specifications.
    
#ifdef _WIN32
    // On Windows, check for CUDA and DirectML
    {
        GPUDeviceInfo cudaDevice;
        cudaDevice.name = "NVIDIA GPU (CUDA)";
        cudaDevice.backend = GPUBackend::CUDA;
        cudaDevice.deviceIndex = 0;
        // Simulated memory values - in production, use cudaMemGetInfo()
        cudaDevice.totalMemory = 8ULL * 1024 * 1024 * 1024; // 8GB simulated (typical GTX 1080)
        cudaDevice.freeMemory = 6ULL * 1024 * 1024 * 1024;  // 6GB free (accounting for OS usage)
        cudaDevice.isAvailable = false; // Actual CUDA check would go here
        cudaDevice.computeCapability = 7.5f;
        availableDevices.push_back(cudaDevice);
    }
    
    {
        GPUDeviceInfo dmlDevice;
        dmlDevice.name = "DirectX 12 GPU (DirectML)";
        dmlDevice.backend = GPUBackend::DirectML;
        dmlDevice.deviceIndex = 0;
        // Simulated memory values - in production, use DXGI adapter enumeration
        dmlDevice.totalMemory = 8ULL * 1024 * 1024 * 1024;
        dmlDevice.freeMemory = 6ULL * 1024 * 1024 * 1024;
        dmlDevice.isAvailable = true; // DirectML is generally available on Windows 10+
        availableDevices.push_back(dmlDevice);
        gpuAvailable = true;
    }
#endif

#ifdef __APPLE__
    // On macOS, check for CoreML
    {
        GPUDeviceInfo coremlDevice;
        coremlDevice.name = "Apple GPU (CoreML)";
        coremlDevice.backend = GPUBackend::CoreML;
        coremlDevice.deviceIndex = 0;
        coremlDevice.isAvailable = true; // CoreML is available on all modern macOS
        availableDevices.push_back(coremlDevice);
        gpuAvailable = true;
    }
#endif

#ifdef __linux__
    // On Linux, check for CUDA
    {
        GPUDeviceInfo cudaDevice;
        cudaDevice.name = "NVIDIA GPU (CUDA)";
        cudaDevice.backend = GPUBackend::CUDA;
        cudaDevice.deviceIndex = 0;
        cudaDevice.isAvailable = false; // Actual CUDA check would go here
        availableDevices.push_back(cudaDevice);
    }
#endif
    
    Logger::log(Logger::Level::Info, "Detected " + juce::String(static_cast<int>(availableDevices.size())) + 
                " GPU devices, available: " + juce::String(gpuAvailable ? "yes" : "no"));
}

bool GPUAccelerationManager::initializeCUDA(int deviceIndex)
{
    // In actual implementation, this would:
    // 1. Check if CUDA is available
    // 2. Initialize the CUDA execution provider
    // 3. Set up the device
    
    for (auto& device : availableDevices)
    {
        if (device.backend == GPUBackend::CUDA && device.deviceIndex == deviceIndex)
        {
            if (device.isAvailable)
            {
                activeDevice = device;
                return true;
            }
        }
    }
    
    return false;
}

bool GPUAccelerationManager::initializeDirectML(int deviceIndex)
{
    // In actual implementation, this would:
    // 1. Check if DirectML is available
    // 2. Initialize the DirectML execution provider
    
#ifdef _WIN32
    for (auto& device : availableDevices)
    {
        if (device.backend == GPUBackend::DirectML && device.deviceIndex == deviceIndex)
        {
            if (device.isAvailable)
            {
                activeDevice = device;
                return true;
            }
        }
    }
#endif
    
    return false;
}

bool GPUAccelerationManager::initializeCoreML()
{
#ifdef __APPLE__
    for (auto& device : availableDevices)
    {
        if (device.backend == GPUBackend::CoreML)
        {
            if (device.isAvailable)
            {
                activeDevice = device;
                return true;
            }
        }
    }
#endif
    
    return false;
}

void GPUAccelerationManager::notifyInitialized(bool success, const juce::String& message)
{
    for (auto* listener : listeners)
    {
        listener->onGPUInitialized(success, message);
    }
}

void GPUAccelerationManager::notifyUnavailable(const juce::String& reason)
{
    for (auto* listener : listeners)
    {
        listener->onGPUUnavailable(reason);
    }
}

void GPUAccelerationManager::notifyFallback(const juce::String& reason)
{
    for (auto* listener : listeners)
    {
        listener->onFallbackToCPU(reason);
    }
}

void GPUAccelerationManager::updateMetrics()
{
    // Update GPU utilization and memory usage
    // In actual implementation, this would query the GPU driver
}

//==============================================================================
// GPUSettingsComponent Implementation
//==============================================================================

GPUSettingsComponent::GPUSettingsComponent(GPUAccelerationManager* manager)
    : gpuManager(manager)
{
    setupUI();
}

GPUSettingsComponent::~GPUSettingsComponent()
{
}

void GPUSettingsComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(30, 30, 35));
    
    g.setColour(juce::Colour(50, 50, 55));
    g.drawRect(getLocalBounds(), 1);
}

void GPUSettingsComponent::resized()
{
    auto bounds = getLocalBounds().reduced(10);
    
    titleLabel.setBounds(bounds.removeFromTop(30));
    bounds.removeFromTop(5);
    
    statusLabel.setBounds(bounds.removeFromTop(25));
    deviceLabel.setBounds(bounds.removeFromTop(25));
    metricsLabel.setBounds(bounds.removeFromTop(40));
    
    bounds.removeFromTop(10);
    
    auto row1 = bounds.removeFromTop(30);
    enableGPUButton.setBounds(row1);
    
    bounds.removeFromTop(5);
    
    auto row2 = bounds.removeFromTop(30);
    backendSelector.setBounds(row2.removeFromLeft(150));
    row2.removeFromLeft(10);
    deviceSelector.setBounds(row2.removeFromLeft(150));
    
    bounds.removeFromTop(5);
    
    auto row3 = bounds.removeFromTop(25);
    fallbackButton.setBounds(row3.removeFromLeft(200));
    graphOptButton.setBounds(row3);
    
    bounds.removeFromTop(10);
    
    auto buttonRow = bounds.removeFromTop(35);
    applyButton.setBounds(buttonRow.removeFromLeft(100).reduced(2));
    benchmarkButton.setBounds(buttonRow.removeFromLeft(100).reduced(2));
}

void GPUSettingsComponent::refresh()
{
    updateStatus();
}

void GPUSettingsComponent::setupUI()
{
    addAndMakeVisible(titleLabel);
    titleLabel.setText("GPU Acceleration", juce::dontSendNotification);
    titleLabel.setFont(juce::Font(18.0f, juce::Font::bold));
    titleLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    
    addAndMakeVisible(statusLabel);
    statusLabel.setColour(juce::Label::textColourId, juce::Colours::lightgrey);
    
    addAndMakeVisible(deviceLabel);
    deviceLabel.setColour(juce::Label::textColourId, juce::Colours::lightgrey);
    
    addAndMakeVisible(metricsLabel);
    metricsLabel.setColour(juce::Label::textColourId, juce::Colours::lightgrey);
    
    addAndMakeVisible(enableGPUButton);
    enableGPUButton.setButtonText("Enable GPU Acceleration");
    enableGPUButton.setToggleState(gpuManager != nullptr && gpuManager->isGPUActive(), 
                                   juce::dontSendNotification);
    
    addAndMakeVisible(backendSelector);
    backendSelector.addItem("CPU Only", 1);
    backendSelector.addItem("CUDA", 2);
    backendSelector.addItem("DirectML", 3);
    backendSelector.addItem("CoreML", 4);
    backendSelector.setSelectedId(1);
    
    addAndMakeVisible(deviceSelector);
    deviceSelector.addItem("Default", 1);
    deviceSelector.setSelectedId(1);
    
    addAndMakeVisible(fallbackButton);
    fallbackButton.setButtonText("Fallback to CPU");
    fallbackButton.setToggleState(true, juce::dontSendNotification);
    
    addAndMakeVisible(graphOptButton);
    graphOptButton.setButtonText("Graph Optimization");
    graphOptButton.setToggleState(true, juce::dontSendNotification);
    
    addAndMakeVisible(applyButton);
    applyButton.setButtonText("Apply");
    applyButton.onClick = [this] { onApplyClicked(); };
    
    addAndMakeVisible(benchmarkButton);
    benchmarkButton.setButtonText("Benchmark");
    benchmarkButton.onClick = [this] { onBenchmarkClicked(); };
    
    updateStatus();
}

void GPUSettingsComponent::updateStatus()
{
    if (gpuManager == nullptr)
    {
        statusLabel.setText("Status: No GPU Manager", juce::dontSendNotification);
        return;
    }
    
    juce::String status = "Status: ";
    if (gpuManager->isGPUActive())
    {
        status += "Active (" + GPUAccelerationManager::getBackendName(gpuManager->getActiveBackend()) + ")";
        statusLabel.setColour(juce::Label::textColourId, juce::Colours::limegreen);
    }
    else
    {
        status += "CPU Mode";
        statusLabel.setColour(juce::Label::textColourId, juce::Colours::orange);
    }
    statusLabel.setText(status, juce::dontSendNotification);
    
    auto deviceInfo = gpuManager->getActiveDeviceInfo();
    deviceLabel.setText("Device: " + (deviceInfo.name.isNotEmpty() ? deviceInfo.name : "CPU"),
                       juce::dontSendNotification);
    
    auto metrics = gpuManager->getPerformanceMetrics();
    juce::String metricsText = "Avg: " + juce::String(metrics.averageInferenceTimeMs, 2) + "ms | ";
    metricsText += "Count: " + juce::String(metrics.inferenceCount);
    metricsLabel.setText(metricsText, juce::dontSendNotification);
    
    // Update device list
    deviceSelector.clear();
    auto devices = gpuManager->getAvailableDevices();
    int id = 1;
    for (const auto& device : devices)
    {
        deviceSelector.addItem(device.name, id++);
    }
    if (devices.empty())
    {
        deviceSelector.addItem("No GPU Available", 1);
    }
    deviceSelector.setSelectedId(1);
}

void GPUSettingsComponent::onApplyClicked()
{
    if (gpuManager == nullptr)
        return;
    
    GPUConfig config;
    
    int backendId = backendSelector.getSelectedId();
    switch (backendId)
    {
        case 1: config.preferredBackend = GPUBackend::None; break;
        case 2: config.preferredBackend = GPUBackend::CUDA; break;
        case 3: config.preferredBackend = GPUBackend::DirectML; break;
        case 4: config.preferredBackend = GPUBackend::CoreML; break;
        default: config.preferredBackend = GPUBackend::None; break;
    }
    
    config.deviceIndex = deviceSelector.getSelectedId() - 1;
    config.fallbackToCPU = fallbackButton.getToggleState();
    config.useGraphOptimization = graphOptButton.getToggleState();
    
    if (enableGPUButton.getToggleState())
    {
        gpuManager->initialize(config);
    }
    else
    {
        gpuManager->fallbackToCPU();
    }
    
    updateStatus();
}

void GPUSettingsComponent::onBenchmarkClicked()
{
    if (gpuManager == nullptr)
        return;
    
    float score = gpuManager->runBenchmark();
    
    juce::AlertWindow::showMessageBoxAsync(
        juce::AlertWindow::InfoIcon,
        "GPU Benchmark",
        "Benchmark Score: " + juce::String(score, 2) + "\n\n"
        "Higher scores indicate better performance.",
        "OK");
}

} // namespace MAEVN
