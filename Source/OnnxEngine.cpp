/**
 * @file OnnxEngine.cpp
 * @brief Implementation of ONNX Runtime wrapper
 */

#include "OnnxEngine.h"
#include <fstream>

namespace MAEVN
{

//==============================================================================
// OnnxModel Implementation
//==============================================================================

OnnxModel::OnnxModel()
    : modelLoaded(false)
{
}

OnnxModel::~OnnxModel()
{
    unload();
}

bool OnnxModel::loadModel(const juce::String& modelPath)
{
    const juce::ScopedLock sl(modelLock);
    
    try
    {
        // Create ONNX Runtime environment
        environment = std::make_unique<Ort::Env>(ORT_LOGGING_LEVEL_WARNING, "MAEVN");
        
        // Configure session options for optimal performance
        sessionOptions = std::make_unique<Ort::SessionOptions>();
        sessionOptions->SetIntraOpNumThreads(1); // Real-time audio: single thread
        sessionOptions->SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);
        
        // Load the model
        #ifdef _WIN32
            session = std::make_unique<Ort::Session>(*environment, modelPath.toWideCharPointer(), *sessionOptions);
        #else
            session = std::make_unique<Ort::Session>(*environment, modelPath.toRawUTF8(), *sessionOptions);
        #endif
        
        // Get input/output metadata
        Ort::AllocatorWithDefaultOptions allocator;
        
        // Input info
        size_t numInputNodes = session->GetInputCount();
        if (numInputNodes > 0)
        {
            auto inputName = session->GetInputNameAllocated(0, allocator);
            inputNames.push_back(inputName.get());
            
            auto inputTypeInfo = session->GetInputTypeInfo(0);
            auto tensorInfo = inputTypeInfo.GetTensorTypeAndShapeInfo();
            inputShape = tensorInfo.GetShape();
        }
        
        // Output info
        size_t numOutputNodes = session->GetOutputCount();
        if (numOutputNodes > 0)
        {
            auto outputName = session->GetOutputNameAllocated(0, allocator);
            outputNames.push_back(outputName.get());
            
            auto outputTypeInfo = session->GetOutputTypeInfo(0);
            auto tensorInfo = outputTypeInfo.GetTensorTypeAndShapeInfo();
            outputShape = tensorInfo.GetShape();
        }
        
        modelLoaded = true;
        Logger::log(Logger::Level::Info, "ONNX model loaded: " + modelPath);
        return true;
    }
    catch (const Ort::Exception& e)
    {
        Logger::log(Logger::Level::Error, "Failed to load ONNX model: " + juce::String(e.what()));
        modelLoaded = false;
        return false;
    }
}

bool OnnxModel::runInference(const std::vector<float>& inputData,
                             const std::vector<int64_t>& inputShape,
                             std::vector<float>& outputData)
{
    const juce::ScopedLock sl(modelLock);
    
    if (!modelLoaded || !session)
        return false;
    
    try
    {
        // Create input tensor
        auto memoryInfo = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
        
        std::vector<int64_t> actualShape = inputShape.empty() ? this->inputShape : inputShape;
        
        Ort::Value inputTensor = Ort::Value::CreateTensor<float>(
            memoryInfo,
            const_cast<float*>(inputData.data()),
            inputData.size(),
            actualShape.data(),
            actualShape.size()
        );
        
        // Run inference
        auto outputTensors = session->Run(
            Ort::RunOptions{nullptr},
            inputNames.data(),
            &inputTensor,
            1,
            outputNames.data(),
            1
        );
        
        // Extract output data
        float* floatArray = outputTensors[0].GetTensorMutableData<float>();
        auto outputTensorInfo = outputTensors[0].GetTensorTypeAndShapeInfo();
        size_t outputSize = outputTensorInfo.GetElementCount();
        
        outputData.assign(floatArray, floatArray + outputSize);
        
        return true;
    }
    catch (const Ort::Exception& e)
    {
        Logger::log(Logger::Level::Error, "Inference failed: " + juce::String(e.what()));
        return false;
    }
}

void OnnxModel::unload()
{
    const juce::ScopedLock sl(modelLock);
    
    session.reset();
    sessionOptions.reset();
    environment.reset();
    
    inputNames.clear();
    outputNames.clear();
    inputShape.clear();
    outputShape.clear();
    
    modelLoaded = false;
}

//==============================================================================
// OnnxEngine Implementation
//==============================================================================

OnnxEngine::OnnxEngine()
    : initialized(false)
    , useGPU(false)
{
}

OnnxEngine::~OnnxEngine()
{
    unloadAllModels();
}

bool OnnxEngine::initialize()
{
    const juce::ScopedLock sl(engineLock);
    
    if (initialized)
        return true;
    
    try
    {
        // ONNX Runtime is initialized per-model
        initialized = true;
        Logger::log(Logger::Level::Info, "ONNX Engine initialized");
        return true;
    }
    catch (...)
    {
        Logger::log(Logger::Level::Error, "Failed to initialize ONNX Engine");
        return false;
    }
}

bool OnnxEngine::loadModel(const juce::String& role, const juce::String& modelPath)
{
    const juce::ScopedLock sl(engineLock);
    
    if (!initialized && !initialize())
        return false;
    
    // Check if file exists
    juce::File modelFile(modelPath);
    if (!modelFile.existsAsFile())
    {
        Logger::log(Logger::Level::Error, "Model file not found: " + modelPath);
        return false;
    }
    
    // Create and load model
    auto model = std::make_unique<OnnxModel>();
    if (model->loadModel(modelPath))
    {
        models[role] = std::move(model);
        modelPaths[role] = modelPath;
        Logger::log(Logger::Level::Info, "Loaded model [" + role + "]: " + modelPath);
        return true;
    }
    
    return false;
}

bool OnnxEngine::runInference(const juce::String& role,
                              const std::vector<float>& inputData,
                              const std::vector<int64_t>& inputShape,
                              std::vector<float>& outputData)
{
    const juce::ScopedLock sl(engineLock);
    
    auto it = models.find(role);
    if (it != models.end() && it->second->isReady())
    {
        return it->second->runInference(inputData, inputShape, outputData);
    }
    
    Logger::log(Logger::Level::Warning, "Model not found or not ready: " + role);
    return false;
}

int OnnxEngine::loadModelsFromConfig(const juce::String& configPath)
{
    const juce::ScopedLock sl(engineLock);
    
    juce::File configFile(configPath);
    if (!configFile.existsAsFile())
    {
        Logger::log(Logger::Level::Warning, "Config file not found: " + configPath);
        return 0;
    }
    
    auto config = parseConfigFile(configPath);
    if (!config.isObject())
        return 0;
    
    int loadedCount = 0;
    auto* obj = config.getDynamicObject();
    
    for (auto& prop : obj->getProperties())
    {
        juce::String role = prop.name.toString();
        juce::String path = prop.value.toString();
        
        // Make path absolute if relative (paths in config.json are typically relative)
        juce::File baseDir = configFile.getParentDirectory();
        juce::File modelFile = baseDir.getChildFile(path);
        
        if (loadModel(role, modelFile.getFullPathName()))
            loadedCount++;
    }
    
    Logger::log(Logger::Level::Info, "Loaded " + juce::String(loadedCount) + " models from config");
    return loadedCount;
}

bool OnnxEngine::reloadModel(const juce::String& role)
{
    const juce::ScopedLock sl(engineLock);
    
    auto pathIt = modelPaths.find(role);
    if (pathIt == modelPaths.end())
        return false;
    
    unloadModel(role);
    return loadModel(role, pathIt->second);
}

bool OnnxEngine::isModelReady(const juce::String& role) const
{
    const juce::ScopedLock sl(engineLock);
    
    auto it = models.find(role);
    return (it != models.end() && it->second->isReady());
}

void OnnxEngine::unloadModel(const juce::String& role)
{
    const juce::ScopedLock sl(engineLock);
    
    auto it = models.find(role);
    if (it != models.end())
    {
        it->second->unload();
        models.erase(it);
        Logger::log(Logger::Level::Info, "Unloaded model: " + role);
    }
}

void OnnxEngine::unloadAllModels()
{
    const juce::ScopedLock sl(engineLock);
    
    for (auto& pair : models)
    {
        pair.second->unload();
    }
    models.clear();
    modelPaths.clear();
    
    Logger::log(Logger::Level::Info, "All models unloaded");
}

juce::StringArray OnnxEngine::getLoadedModels() const
{
    const juce::ScopedLock sl(engineLock);
    
    juce::StringArray result;
    for (const auto& pair : models)
    {
        if (pair.second->isReady())
            result.add(pair.first);
    }
    return result;
}

void OnnxEngine::setUseGPU(bool shouldUseGPU)
{
    useGPU = shouldUseGPU;
    Logger::log(Logger::Level::Info, "GPU acceleration: " + juce::String(useGPU ? "enabled" : "disabled"));
}

juce::var OnnxEngine::parseConfigFile(const juce::String& configPath)
{
    juce::File file(configPath);
    juce::String content = file.loadFileAsString();
    
    juce::var result = juce::JSON::parse(content);
    return result;
}

} // namespace MAEVN
